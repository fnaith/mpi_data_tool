#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"
#include "../mdt_core/SparseFormatter.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/mpi.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace mdt_core;
using namespace std;

namespace archive = boost::archive;
namespace filesystem = boost::filesystem;
namespace mpi = boost::mpi;

void cor_feature(const string& feature_file,
                 const vector<Number>& mean,
                 const vector<Number>& std,
                 const vector<Number>& target,
                 Number target_sum,
                 vector<Number> *counter) {
  try {
    ifstream input;
    file_open(feature_file.c_str(), ifstream::in | ifstream::binary, &input);
    counter->resize(mean.size());
    string line;
    Integer i(0);
    for (size_t i(0); i < mean.size(); ++i) {
      getline(input, line);
      Number cor(target_sum * (0.0 - mean[i]));
      SparseFormatter formatter(line);
      Integer col;
      Number value;
      while (formatter.next(&col, &value)) {
        cor += (target[col - 1] * value);
      }
      counter->at(i) = cor;
    }
    input.close();
  } catch (const exception& e) {
    throw runtime_error(string("cor_feature :\n") + e.what());
  }
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 6) {
      throw runtime_error("<file root path> <feature name> "
                          "<output folder name> <split number> "
                          "<target name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string feature_name(argv[2]);
    const string feature_file((root_path / feature_name).string());
    const filesystem::path output_folder(root_path / argv[3]);
    const string split_number_str(argv[4]);
    const Integer split_number(in_range(integer_from_str(split_number_str.c_str()),
                                        1, numeric_limits<Integer>::max()));
    const filesystem::path split_path(output_folder / split_number_str);
    const string target_file((root_path / argv[5]).string());
    const string target_mean_file(target_file + ".target_mean");
    const string target_std_file(target_file + ".target_std");

    mpi::environment env(argc, argv);
    mpi::communicator world;
    if (!world) {
      throw runtime_error("communicator fail\n");
    }
    const Integer worker_number(world.size() - 1);
    if (worker_number < 1) {
      throw runtime_error("worker_number must > 1\n");
    }

    if (world.rank() == worker_number) {
      clock_t start(clock());

      try {
        ifstream input;
        file_open(target_std_file.c_str(), ifstream::in | ifstream::binary, &input);
        Number target_std;
        boost::archive::text_iarchive(input) >> target_std;
        input.close();

        for (Integer i(0); i < worker_number; ++i) {
          mpi::request target_std_req(world.isend(i, 0, target_std));
          target_std_req.wait();
        }

        file_open(target_mean_file.c_str(), ifstream::in | ifstream::binary, &input);
        Number target_mean;
        boost::archive::text_iarchive(input) >> target_mean;
        input.close();

        file_open(target_file.c_str(), ifstream::in | ifstream::binary, &input);

        vector<Number> target;
        string line;
        while (getline(input, line)) {
          target.push_back(strtod(line.data(), NULL) - target_mean);
        }
        input.close();

        for (Integer i(0); i < worker_number; ++i) {
          mpi::request target_req(world.isend(i, 1, target));
          target_req.wait();
        }

        Integer task_id(0);
        vector<Integer> worker2task(worker_number, -1);
        vector<mpi::request> subcounter_reqs(worker_number);
        vector<Number> counter;
        while (true) {
          Integer task_done(0);
          for (Integer i(0); i < worker_number; ++i) {
            if (worker2task[i] == split_number) {
              ++task_done;
              continue;
            }
            if (worker2task[i] != -1 && subcounter_reqs[i].test()) {
              ostringstream id;
              id << worker2task[i];
              filesystem::path sub_split_path(split_path / id.str());
              string output_file((sub_split_path / feature_name).string() + ".cor");

              ofstream output;
              file_open(output_file.c_str(), ofstream::out | ofstream::binary, &output);
              boost::archive::text_oarchive(output) << reinterpret_cast<const vector<Number>&>(counter);
              output.close();

              worker2task[i] = -1;
            }
            if (worker2task[i] == -1) {
              worker2task[i] = task_id;
              mpi::request task_id_req(world.isend(i, 2, worker2task[i]));
              task_id_req.wait();
              if (task_id < split_number) {
                ostringstream id;
                id << worker2task[i];
                filesystem::path sub_split_path(split_path / id.str());
                string mean_file((sub_split_path / feature_name).string() + ".mean");
                string std_file((sub_split_path / feature_name).string() + ".std");
                vector<Number> tmp;

                ifstream input;

                file_open(mean_file.c_str(), ifstream::in | ifstream::binary, &input);
                boost::archive::text_iarchive(input) >> tmp;
                input.close();
                mpi::request mean_req(world.isend(i, 3, tmp));
                mean_req.wait();

                file_open(std_file.c_str(), ifstream::in | ifstream::binary, &input);
                boost::archive::text_iarchive(input) >> tmp;
                input.close();
                mpi::request squared_sum_req(world.isend(i, 4, tmp));
                squared_sum_req.wait();

                subcounter_reqs[i] = world.irecv(i, 5, counter);
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
        Number target_std;
        mpi::request target_std_req(world.irecv(worker_number, 0, target_std));
        target_std_req.wait();

        vector<Number> target;
        mpi::request target_req(world.irecv(worker_number, 1, target));
        target_req.wait();
        Number target_sum(0.0);
        for (size_t i(0); i < target.size(); ++i) {
          target_sum += target[i];
        }

        vector<Number> mean, std;
        vector<Number> counter;
        while (true) {
          Integer task_id(-1);
          mpi::request task_id_req(world.irecv(worker_number, 2, task_id));
          task_id_req.wait();
          if (task_id == split_number) {
            break;
          }
          mpi::request mean_req(world.irecv(worker_number, 3, mean));
          mean_req.wait();
          mpi::request squared_sum_req(world.irecv(worker_number, 4, std));
          squared_sum_req.wait();

          ostringstream split_id;
          split_id << task_id;
          filesystem::path split_folder(split_path / split_id.str());

          cor_feature((split_folder / feature_name).string(),
                      mean,
                      std,
                      target, target_sum,
                      &counter);
          mpi::request subcounter_req(world.isend(worker_number, 5, counter));
          subcounter_req.wait();
        }
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "cor_feature : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
