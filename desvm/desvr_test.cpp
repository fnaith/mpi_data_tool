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
      throw runtime_error("<file root> <model folder> <data folder> <model name> "
                          "<split number> <feature dimension> "
                          "<A name> <y name> "
                          "<batch number> <loading number> "
                          "<output>");
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
    const string root(argv[1]);
    const string train(argv[2]);
    const string test(argv[3]);
    const string model(argv[4]);
    const Integer split(in_range(integer_from_str(argv[5]),
                                 1, numeric_limits<Integer>::max()));
    const Integer dimension(in_range(integer_from_str(argv[6]),
                                     1, numeric_limits<Integer>::max()));
    const string A_name(argv[7]);
    const string y_name(argv[8]);
    const Integer batch(in_range(integer_from_str(argv[9]),
                                 1, numeric_limits<Integer>::max()));
    const Integer loading_number(in_range(integer_from_str(argv[10]),
                                          1, numeric_limits<Integer>::max()));
    const string output(argv[11]);
    if (world.rank() == worker_number) {
      try {
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

        stringstream final_model_path;
        final_model_path << root << "/" + train + "/" << split << '/' << model;
        Integer total_instance_number(0);
        Number total_squared_error(0.0);
        LinearSSVR final_model(final_model_path.str());
        Matrix final_table;
        Vector y;
        Vector result;
        for (Integer b(0); b < batch; ++b) {
          stringstream y_path;
          y_path << root << "/" + test + "/" << batch << '/' << b << '/' << y_name;
          ifstream input;
          file_open(y_path.str(), ifstream::in | ifstream::binary, &input);
          y.resize(count_line(y_path.str().c_str()));
          string line;
          for (Integer i(0); i < y.size(); ++i) {
            getline(input, line);
            y(i) = strtod(line.data(), NULL);
          }
          input.close();

          const Integer instance_number(y.rows());
          final_table.resize(instance_number, split);
          result.resize(instance_number);
          
          Integer task_id(0);
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
          final_model.predict(&final_table, &result);
          
          total_instance_number += y.rows();
          total_squared_error += (y - result).squaredNorm();
        }
        ofstream ofs;
        file_open(output, ofstream::out | ofstream::binary | ofstream::app, &ofs);
        ofs << "model : " << model << endl;
        ofs << "total_instance_number : " << total_instance_number << endl;
        ofs << "root mean squared error : " << sqrt(total_squared_error / total_instance_number) << endl;
        ofs << endl;
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }
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
          submodel_path << root << "/" + train + "/" << split << '/' << task_id << '/' << model;
          RBFRSVR submodel(submodel_path.str().c_str());
          mpi::request submodel_req(world.isend(worker_number, 1, submodel));
          submodel_req.wait();
        }
        SparseMatrix A;
        Vector y;
        RBFRSVR submodel;
        Matrix kernel_data;
        Vector result;
        vector<Number> subpredict;
        for (Integer b(0); b < batch; ++b) {
          stringstream A_path, negative_path;
          A_path << root << "/" + test + "/" << batch << '/' << b << '/' << A_name;
          SparseData::read(A_path.str().c_str(), dimension, &A);
          Integer task_id(-1);
          while (true) {
            mpi::request task_id_req(world.irecv(worker_number, 0, task_id));
            task_id_req.wait();
            if (task_id == split) {
              break;
            }
            subpredict.clear();
            mpi::request submodel_req(world.irecv(worker_number, 1, submodel));
            submodel_req.wait();
            submodel.predict(A, &kernel_data, &result);
            for (Integer k(0); k < result.size(); ++k) {
              subpredict.push_back(result(k));
            }
            mpi::request subpredict_req(world.isend(worker_number, 2, subpredict));
            subpredict_req.wait();
          }
        }
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "desvr_test : " << e.what() << endl;
    return EXIT_FAILURE;
  }
}
