#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"

#include "../ssvm/RBFRSVR.hpp"
#include "../ssvm/SparseData.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <boost/mpi.hpp>

using namespace mdt_core;
using namespace ssvm;
using namespace std;

namespace mpi = boost::mpi;

int main(int argc, char *argv[]) {
  try {
    if (argc != 12) {
      throw runtime_error("<file root> <data folder> <model name> "
                          "<split number> <feature dimension> "
                          "<A name> <y name> "
                          "<reduce rate> <gamma> <weight> <epsilon>");
    }
    mpi::environment env(argc, argv);
    mpi::communicator world;
    if (!world) {
      throw runtime_error("communicator fail");
    }
    const Integer worker_number(world.size() - 1);
    if (worker_number < 1) {
      throw runtime_error("worker_number must > 1");
    }
    const string root(argv[1] + string("/") + argv[2]);
    const string model(argv[3]);
    const Integer split(in_range(integer_from_str(argv[4]),
                                 1, numeric_limits<Integer>::max()));
    const Integer dimension(in_range(integer_from_str(argv[5]),
                                     1, numeric_limits<Integer>::max()));
    const string A_name(argv[6]);
    const string y_name(argv[7]);
    const Number reduce(in_range(number_from_str(argv[8]),
                                 numeric_limits<Number>::epsilon(), numeric_limits<Number>::max()));
    const Number gamma(in_range(number_from_str(argv[9]),
                                numeric_limits<Number>::epsilon(), numeric_limits<Number>::max()));
    const Number weight(in_range(number_from_str(argv[10]),
                                 numeric_limits<Number>::epsilon(), numeric_limits<Number>::max()));
    const Number epsilon(in_range(number_from_str(argv[11]),
                                  numeric_limits<Number>::epsilon(), numeric_limits<Number>::max()));
    if (world.rank() == worker_number) {
      clock_t start(clock());

      try {
        Integer task_id(0);
        vector<RBFRSVR> submodels(split);
        vector<Integer> worker2task(worker_number, -1);
        vector<mpi::request> submodel_reqs(worker_number);
        while (true) {
          Integer task_done(0);
          for (Integer i(0); i < worker_number; ++i) {
            if (worker2task[i] == split) {
              ++task_done;
              continue;
            }
            if (worker2task[i] != -1 && submodel_reqs[i].test()) {
              stringstream submodel_path;
              submodel_path << root << '/' << split << '/' 
                            << worker2task[i] << '/' << model;
              submodels[worker2task[i]].dump(submodel_path.str());
              submodels[worker2task[i]] = RBFRSVR();
              worker2task[i] = -1;
            }
            if (worker2task[i] == -1) {
              worker2task[i] = task_id;
              mpi::request task_id_req(world.isend(i, 0, worker2task[i]));
              task_id_req.wait();
              if (task_id < split) {
                submodel_reqs[i] = world.irecv(i, 1, submodels[worker2task[i]]);
                ++task_id;
              }
            }
          }
          if (task_done == worker_number) {
            break;
          }
        }
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }

      cout << (double)(clock() - start) / CLOCKS_PER_SEC;
    } else {
      try {
        SparseMatrix A;
        Vector y;
        while (true) {
          Integer task_id(-1);
          mpi::request task_id_req(world.irecv(worker_number, 0, task_id));
          task_id_req.wait();
          if (task_id == split) {
            break;
          }
          stringstream split_path;
          split_path << root << '/' << split << '/' << task_id << '/';
          SparseData::read(split_path.str() + A_name, dimension, &A);

          ifstream input;
          file_open(split_path.str() + y_name, ifstream::in | ifstream::binary, &input);
          y.resize(A.rows());
          string line;
          for (Integer i(0); i < y.size(); ++i) {
            getline(input, line);
            y(i) = strtod(line.data(), NULL);
          }
          input.close();

          RBFRSVR submodel(A, y, reduce, gamma, weight, epsilon);
          mpi::request submodel_req(world.isend(worker_number, 1, submodel));
          submodel_req.wait();
        }
        A = SparseMatrix();
        y = Vector();
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "desvr_distributed : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
