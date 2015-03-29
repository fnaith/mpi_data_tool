#include "../mdt_core/count_line.hpp"
#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"

#include "../ssvm/LinearSSVR.hpp"
#include "../ssvm/RBFRSVR.hpp"
#include "../ssvm/SparseData.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
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
    if (argc != 14) {
      throw runtime_error("<file root> <data folder> <model name> "
                          "<split number> <feature dimension> "
                          "<A name> <y name> "
                          "<final weight> <final epsilon> "
                          "<loading number> <sample A name> <sample y name> <sample number>");
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
    const Number final_weight(in_range(number_from_str(argv[8]),
                                       numeric_limits<Number>::epsilon(), numeric_limits<Number>::max()));
    const Number final_epsilon(in_range(number_from_str(argv[9]),
                                        numeric_limits<Number>::epsilon(), numeric_limits<Number>::max()));
    const Integer loading_number(in_range(integer_from_str(argv[10]),
                                          1, numeric_limits<Integer>::max()));
    const string sample_A_name(argv[11]);
    const string sample_y_name(argv[12]);
    const Integer sample_number(in_range(integer_from_str(argv[13]),
                                          1, numeric_limits<Integer>::max()));
    if (worker_number - loading_number < 0) {
      throw runtime_error("too much loading_number");
    }
    if (world.rank() == worker_number) {
      clock_t start(clock());

      try {
        Integer instance_number(0);
        for (Integer i(0); i < sample_number; ++i) {
          stringstream y_path;
          y_path << root << '/' << split << '/' << i << '/' << sample_y_name;
          instance_number += count_line(y_path.str().c_str());
        }
        Matrix final_table(instance_number, split + 1);
        Matrix support_vector(instance_number, split + 1);

        Integer task_id(0);
        vector<RBFRSVR> submodels(split);
        vector<Integer> loading2task(loading_number, -1);
        vector<mpi::request> submodel_reqs(loading_number);
        while (true) {
          Integer task_done(0);
          for (Integer i(0); i < loading_number; ++i) {
            if (loading2task[i] == split) {
              ++task_done;
              continue;
            }
            if (loading2task[i] != -1 && submodel_reqs[i].test()) {
              loading2task[i] = -1;
            }
            if (loading2task[i] == -1) {
              loading2task[i] = task_id;
              mpi::request task_id_req(world.isend(i + (worker_number - loading_number), 0, loading2task[i]));
              task_id_req.wait();
              if (task_id < split) {
                submodel_reqs[i] = world.irecv(i + (worker_number - loading_number), 1, submodels[loading2task[i]]);
                ++task_id;
              }
            }
          }
          if (task_done == loading_number) {
            break;
          }
        }
        loading2task = vector<Integer>();
        submodel_reqs = vector<mpi::request>();

        task_id = 0;
        vector<Number> subpredict;
        vector<Integer> worker2task(worker_number, -1);
        vector<mpi::request> subpredict_reqs(worker_number);
        while (true) {
          Integer task_done(0);
          for (Integer i(0); i < worker_number; ++i) {
            if (worker2task[i] == split) {
              ++task_done;
              continue;
            }
            if (worker2task[i] != -1 && subpredict_reqs[i].test()) {
              Number *dst(final_table.data() + worker2task[i] * instance_number);
              Number *src(subpredict.data());
              for (Integer j(0); j < instance_number; ++j, ++dst, ++src) {
                *dst = *src;
              }
              worker2task[i] = -1;
            }
            if (worker2task[i] == -1) {
              worker2task[i] = task_id;
              mpi::request task_id_req(world.isend(i, 0, worker2task[i]));
              task_id_req.wait();
              if (task_id < split) {
                mpi::request submodel_req(world.isend(i, 1, submodels[worker2task[i]]));
                submodel_req.wait();
                subpredict_reqs[i] = world.irecv(i, 2, subpredict);
                ++task_id;
              }
            }
          }
          if (task_done == worker_number) {
            break;
          }
        }
        subpredict = vector<Number>();

        Vector y(instance_number);
        Integer offset(0);
        for (Integer i(0); i < sample_number; ++i) {
          vector<Number> tmp_y;
          stringstream y_path;
          y_path << root << '/' << split << '/' << i << '/' << sample_y_name;

          ifstream input;
          file_open(y_path.str(), ifstream::in | ifstream::binary, &input);
          string line;
          while (getline(input, line)) {
            tmp_y.push_back(strtod(line.data(), NULL));
          }
          input.close();

          for (size_t j(0); j < tmp_y.size(); ++j) {
            y(offset) = tmp_y[j];
            ++offset;
          }
        }

        LinearSSVR final_model(y, final_weight, final_epsilon, &final_table, &support_vector);
        stringstream final_model_path;
        final_model_path << root << '/' << split << '/' << model;
        final_model.dump(final_model_path.str());
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }

      cout << (double)(clock() - start) / CLOCKS_PER_SEC;
    } else {
      try {
        Integer task_id(-1);
        while (world.rank() >= (worker_number - loading_number)) {
          mpi::request task_id_req(world.irecv(worker_number, 0, task_id));
          task_id_req.wait();
          if (task_id == split) {
            break;
          }
          stringstream submodel_path;
          submodel_path << root << '/' << split << '/' << task_id << '/' << model;
          RBFRSVR submodel(submodel_path.str().c_str());
          mpi::request submodel_req(world.isend(worker_number, 1, submodel));
          submodel_req.wait();
        }

        task_id = -1;
        RBFRSVR submodel;
        vector<SparseMatrix> As(sample_number);
        for (Integer i(0); i < sample_number; ++i) {
          stringstream A_path;
          A_path << root << '/' << split << '/' << i << '/' << sample_A_name;
          SparseData::read(A_path.str().c_str(), dimension, &(As[i]));
        }
        Matrix kernel_data;
        Vector result;
        vector<Number> subpredict;
        while (true) {
          mpi::request task_id_req(world.irecv(worker_number, 0, task_id));
          task_id_req.wait();
          if (task_id == split) {
            break;
          }
          mpi::request submodel_req(world.irecv(worker_number, 1, submodel));
          submodel_req.wait();
          subpredict.clear();
          for (Integer i(0); i < sample_number; ++i) {
            submodel.predict(As[i], &kernel_data, &result);
            for (Integer k(0); k < result.size(); ++k) {
              subpredict.push_back(result(k));
            }
          }
          mpi::request subpredict_req(world.isend(worker_number, 2, subpredict));
          subpredict_req.wait();
        }
        submodel = RBFRSVR();
        As = vector<SparseMatrix>();
        kernel_data = Matrix();
        result = Vector();
        subpredict = vector<Number>();
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "desvr_ensemble : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
