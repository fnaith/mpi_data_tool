#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"
#include "../mdt_core/SparseFormatter.hpp"

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

void min_max_feature(const string& file,
                     boost::array<vector<Number>, 2> *counter) {
  try {
    ifstream input;
    file_open(file.c_str(), ifstream::in | ifstream::binary, &input);
    string line;
    while (getline(input, line)) {
      if (line.empty()) {
        counter->at(0).push_back(0.0);
        counter->at(1).push_back(0.0);
      } else {
        Number min_value(numeric_limits<Number>::max());
        Number max_value(numeric_limits<Number>::min());
        SparseFormatter formatter(line);
        Integer col;
        Number value;
        while (formatter.next(&col, &value)) {
          min_value = min(min_value, value);
          max_value = max(max_value, value);
        }
        counter->at(0).push_back(min_value);
        counter->at(1).push_back(max_value);
      }
    }
    input.close();
  } catch (const exception& e) {
    throw runtime_error(string("min_max_feature :\n") + e.what());
  }
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 5) {
      throw runtime_error("<file root path> <feature name> "
                          "<output folder name> <split number>\n");
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
        Integer task_id(0);
        vector<Integer> worker2task(worker_number, -1);
        vector<mpi::request> subcounter_reqs(worker_number);
        boost::array<vector<Number>, 2> counter;
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
              string min_output_file((sub_split_path / feature_name).string() + ".min");
              string max_output_file((sub_split_path / feature_name).string() + ".max");

              ofstream min_output;
              file_open(min_output_file.c_str(), ofstream::out | ofstream::binary, &min_output);
              boost::archive::text_oarchive(min_output) << reinterpret_cast<const vector<Number>&>(counter[0]);
              min_output.close();

              ofstream max_output;
              file_open(max_output_file.c_str(), ofstream::out | ofstream::binary, &max_output);
              boost::archive::text_oarchive(max_output) << reinterpret_cast<const vector<Number>&>(counter[1]);
              max_output.close();

              worker2task[i] = -1;
            }
            if (worker2task[i] == -1) {
              worker2task[i] = task_id;
              mpi::request task_id_req(world.isend(i, 0, worker2task[i]));
              task_id_req.wait();
              if (task_id < split_number) {
                subcounter_reqs[i] = world.irecv(i, 1, counter);
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
        boost::array<vector<Number>, 2> counter;
        while (true) {
          Integer task_id(-1);
          mpi::request task_id_req(world.irecv(worker_number, 0, task_id));
          task_id_req.wait();
          if (task_id == split_number) {
            break;
          }
          ostringstream split_id;
          split_id << task_id;
          filesystem::path split_folder(split_path / split_id.str());
          counter[0].clear();
          counter[1].clear();
          min_max_feature((split_folder / feature_name).string(), &counter);
          mpi::request subcounter_req(world.isend(worker_number, 1, counter));
          subcounter_req.wait();
        }
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "min_max_feature : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
