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

void mean_feature(const string& file,
                  const Integer instance_number,
                  vector<Number> *counter) {
  try {
    ifstream input;
    file_open(file.c_str(), ifstream::in | ifstream::binary, &input);

    string line;
    while (getline(input, line)) {
      Number sum(0.0);
      SparseFormatter formatter(line);
      Integer col;
      Number value;
      while (formatter.next(&col, &value)) {
        sum += value;
      }
      counter->push_back(sum / instance_number);
    }
  } catch (const exception& e) {
    throw runtime_error(string("mean_feature :\n") + e.what());
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
    filesystem::create_directories(output_folder);
    const string split_number_str(argv[4]);
    const Integer split_number(in_range(integer_from_str(split_number_str.c_str()),
                                        1, numeric_limits<Integer>::max()));
    const filesystem::path split_path(output_folder / split_number_str);
    const string instance_number_file((root_path / argv[5]).string() + ".instance_number");

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
        file_open(instance_number_file.c_str(), ifstream::in | ifstream::binary, &input);
        Integer instance_number;
        boost::archive::text_iarchive(input) >> instance_number;
        input.close();

        for (Integer i(0); i < worker_number; ++i) {
          mpi::request instance_number_req(world.isend(i, 0, instance_number));
          instance_number_req.wait();
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
              string output_file((sub_split_path / feature_name).string() + ".mean");

              ofstream output;
              file_open(output_file.c_str(), ofstream::out | ofstream::binary, &output);
              boost::archive::text_oarchive(output) << reinterpret_cast<const vector<Number>&>(counter);
              output.close();

              worker2task[i] = -1;
            }
            if (worker2task[i] == -1) {
              worker2task[i] = task_id;
              mpi::request task_id_req(world.isend(i, 1, worker2task[i]));
              task_id_req.wait();
              if (task_id < split_number) {
                subcounter_reqs[i] = world.irecv(i, 2, counter);
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
        Integer instance_number;
        mpi::request instance_number_req(world.irecv(worker_number, 0, instance_number));
        instance_number_req.wait();

        vector<Number> counter;
        while (true) {
          Integer task_id(-1);
          mpi::request task_id_req(world.irecv(worker_number, 1, task_id));
          task_id_req.wait();
          if (task_id == split_number) {
            break;
          }
          ostringstream split_id;
          split_id << task_id;
          filesystem::path split_folder(split_path / split_id.str());
          counter.clear();
          mean_feature((split_folder / feature_name).string(), instance_number, &counter);
          mpi::request subcounter_req(world.isend(worker_number, 2, counter));
          subcounter_req.wait();
        }
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "mean_feature : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
