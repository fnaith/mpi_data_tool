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

static const Number cut_points[][4] = {{0.0},
                                       {-0.43, 0.43},
                                       {-0.67, 0.0, 0.67},
                                       {-0.84, -0.25, 0.25, 0.84}};

void map_to_sax(vector<Number> *vec, Integer alphabet_size) {
  for (vector<Number>::iterator it(vec->begin()); it != vec->end(); ++it) {
    Number tmp(*it);
    *it = 0;
    for (Integer j(alphabet_size - 2); j >= 0; --j) {
      if (tmp >= cut_points[alphabet_size - 2][j]) {
        *it = j + 1;
        break;
      }
    }
  }
}

void sax_feature(const string& feature_file,
                 const vector<Number>& mean,
                 const vector<Number>& std,
                 Integer alphabet_size,
                 vector<Number> *tmp,
                 vector<string> *sax) {
  try {
    ifstream input;
    file_open(feature_file.c_str(), ifstream::in | ifstream::binary, &input);

    fill(tmp->begin(), tmp->end(), 0.0);
    string line;
    for (size_t i(0); i < mean.size(); ++i) {
      getline(input, line);
      SparseFormatter formatter(line);
      Integer col;
      Number value;
      while (formatter.next(&col, &value)) {
        tmp->at(col - 1) = value;
      }

      for (vector<Number>::iterator it(tmp->begin()); it != tmp->end(); ++it) {
        *it = (*it - mean[i]) / std[i];
      }

      map_to_sax(tmp, alphabet_size);

      vector<Integer> counter(alphabet_size, 0);
      for (vector<Number>::const_iterator it(tmp->begin()); it != tmp->end(); ++it) {
        counter[static_cast<Integer>(*it)] += 1;
      }

      ostringstream output;
      Integer max_idx(static_cast<Integer>(distance(counter.begin(), max_element(counter.begin(), counter.end()))));
      Integer j(0);
      for (vector<Number>::const_iterator it(tmp->begin()); it != tmp->end(); ++it, ++j) {
        if (static_cast<Integer>(*it) != max_idx) {
          if (static_cast<Integer>(*it) == 0) {
            output << (j + 1) << ':' << max_idx << ' ';
          } else {
            output << (j + 1) << ':' << *it << ' ';
          }
        }
      }
      sax->push_back(output.str());
    }
  } catch (const exception& e) {
    throw runtime_error(string("sax_feature :\n") + e.what());
  }
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 7) {
      throw runtime_error("<file root path> <feature name> "
                          "<output folder name> <split number> "
                          "<target name> <alphabat size>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string feature_name(argv[2]);
    const string feature_file((root_path / feature_name).string());
    const filesystem::path output_folder(root_path / argv[3]);
    const string split_number_str(argv[4]);
    const Integer split_number(in_range(integer_from_str(split_number_str.c_str()),
                                        1, numeric_limits<Integer>::max()));
    const filesystem::path split_path(output_folder / split_number_str);
    const string target_name(argv[5]);
    const string target_file((root_path / target_name).string());
    const string instance_number_file((root_path / target_name).string() + ".instance_number");
    const Integer alphabat_size(in_range(integer_from_str(argv[6]),
                                         2, 5));

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
        vector<string> sax;
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
              string output_file((sub_split_path / feature_name).string() + ".sax");

              ofstream output;
              file_open(output_file.c_str(), ofstream::out | ofstream::binary, &output);
              for (vector<string>::iterator it(sax.begin()); it != sax.end(); ++it) {
                output << (*it) << '\n';
              }
              output.close();

              worker2task[i] = -1;
            }
            if (worker2task[i] == -1) {
              worker2task[i] = task_id;
              mpi::request task_id_req(world.isend(i, 1, worker2task[i]));
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
                mpi::request mean_req(world.isend(i, 2, tmp));
                mean_req.wait();

                file_open(std_file.c_str(), ifstream::in | ifstream::binary, &input);
                boost::archive::text_iarchive(input) >> tmp;
                input.close();
                mpi::request squared_sum_req(world.isend(i, 3, tmp));
                squared_sum_req.wait();

                subcounter_reqs[i] = world.irecv(i, 4, sax);
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

        vector<Number> tmp(instance_number);
        vector<Number> mean, std;
        vector<string> sax;

        while (true) {
          Integer task_id(-1);
          mpi::request task_id_req(world.irecv(worker_number, 1, task_id));
          task_id_req.wait();
          if (task_id == split_number) {
            break;
          }
          mpi::request mean_req(world.irecv(worker_number, 2, mean));
          mean_req.wait();
          mpi::request squared_sum_req(world.irecv(worker_number, 3, std));
          squared_sum_req.wait();

          ostringstream split_id;
          split_id << task_id;
          filesystem::path split_folder(split_path / split_id.str());
          sax.clear();
          sax_feature((split_folder / feature_name).string(),
                      mean,
                      std,
                      alphabat_size, &tmp, &sax);
          mpi::request subcounter_req(world.isend(worker_number, 4, sax));
          subcounter_req.wait();
        }
      } catch (const exception& e) {
        cout << '<' << world.rank() << '>' << e.what() << endl;
        throw;
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "sax_feature : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
