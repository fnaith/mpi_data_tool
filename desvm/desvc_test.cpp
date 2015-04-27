#include "../mdt_core/count_line.hpp"
#include "../mdt_core/in_range.hpp"

#include "../ssvm/LinearSSVC.hpp"
#include "../ssvm/RBFRSVC.hpp"
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
                          "<pos name> <neg name> "
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
    const string positive_name(argv[7]);
    const string negative_name(argv[8]);
    const Integer batch(in_range(integer_from_str(argv[9]),
                                 1, numeric_limits<Integer>::max()));
    const Integer loading_number(in_range(integer_from_str(argv[10]),
                                          1, numeric_limits<Integer>::max()));
    const string output(argv[11]);
    if (world.rank() == worker_number) {
      clock_t start(clock());

      try {
        Integer task_id(0);
        vector<RBFRSVC> submodels(split);
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
        Integer total_positive_number(0), total_positive_error(0);
        Integer total_negative_number(0), total_negative_error(0);
        LinearSSVC final_model(final_model_path.str());
        Matrix final_table;
        Vector result;
        for (Integer b(0); b < batch; ++b) {
          stringstream positive_path;
          positive_path << root << "/" + test + "/" << batch << '/' << b << '/' << positive_name;
          Integer positive_number(count_line(positive_path.str().c_str()));
          stringstream negative_path;
          negative_path << root << "/" + test + "/" << batch << '/' << b << '/' << negative_name;
          Integer negative_number(count_line(negative_path.str().c_str()));

          const Integer instance_number(positive_number + negative_number);
          final_table.resize(instance_number, split + 1);
          final_table.resize(instance_number, split);
          result.resize(instance_number);

          Integer task_id(0);
          vector<vector<Number>> subpredicts(split);
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
                Number *src(subpredicts[worker2task[i]].data());
                for (Integer j(0); j < instance_number; ++j, ++dst, ++src) {
                  *dst = *src;
                }
                subpredicts[worker2task[i]] = vector<Number>();
                worker2task[i] = -1;
              }
              if (worker2task[i] == -1) {
                worker2task[i] = task_id;
                mpi::request task_id_req(world.isend(i, 0, worker2task[i]));
                task_id_req.wait();
                if (task_id < split) {
                  mpi::request submodel_req(world.isend(i, 1, submodels[worker2task[i]]));
                  submodel_req.wait();
                  subpredict_reqs[i] = world.irecv(i, 2, subpredicts[worker2task[i]]);
                  ++task_id;
                }
              }
            }
            if (task_done == worker_number) {
              break;
            }
          }
          final_model.predict(&final_table, &result);
          Integer positive_error(0);
          for (Integer i(0); i < positive_number; ++i) {
            if (result(i) < 0) {
              ++positive_error;
            }
          }
          Integer negative_error(0);
          for (Integer i(positive_number); i < instance_number; ++i) {
            if (result(i) > 0) {
              ++negative_error;
            }
          }
          total_positive_number += positive_number;
          total_positive_error += positive_error;
          total_negative_number += negative_number;
          total_negative_error += negative_error;
        }
        ofstream ofs(output, ofstream::out | ofstream::binary | ofstream::app);
        ofs << "model : " << model << endl;
        ofs << "positive_error : " << total_positive_error << endl;
        ofs << "positive_number : " << total_positive_number << endl;
        ofs << "negative_error : " << total_negative_error << endl;
        ofs << "negative_number : " << total_negative_number << endl;
        ofs << "acc : " << (1 - (Number)(total_positive_error + total_negative_error) / (total_positive_number + total_negative_number)) << endl;
        ofs << endl;
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
          submodel_path << root << "/" + train + "/" << split << '/' << task_id << '/' << model;
          RBFRSVC submodel(submodel_path.str().c_str());
          mpi::request submodel_req(world.isend(worker_number, 1, submodel));
          submodel_req.wait();
        }
        SparseMatrix positive_data, negative_data;
        RBFRSVC submodel;
        Matrix kernel_data;
        Vector result;
        vector<Number> subpredict;
        for (Integer b(0); b < batch; ++b) {
          stringstream positive_path, negative_path;
          positive_path << root << "/" + test + "/" << batch << '/' << b << '/' << positive_name;
          SparseData::read(positive_path.str().c_str(), dimension, &positive_data);
          negative_path << root << "/" + test + "/" << batch << '/' << b << '/' << negative_name;
          SparseData::read(negative_path.str().c_str(), dimension, &negative_data);
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
            submodel.predict(positive_data, &kernel_data, &result);
            for (Integer k(0); k < result.size(); ++k) {
              subpredict.push_back(result(k));
            }
            submodel.predict(negative_data, &kernel_data, &result);
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
    cout << "desvm_test : " << e.what() << endl;
    return EXIT_FAILURE;
  }
}
