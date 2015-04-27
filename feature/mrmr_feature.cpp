#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"
#include "../mdt_core/SparseFormatter.hpp"
#include "../mdt_core/unordered_map.hpp"

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

typedef unordered_map<Integer, Number> ValueProb;
typedef unordered_map<Integer, ValueProb> PairProb;

void compute_joint_prob(const vector<Integer>& vec1,
                        const Integer *vec2,
                        Integer instance_number,
                        PairProb *pair_prob,
                        ValueProb *vec1_prob,
                        ValueProb *vec2_prob) {
  const Integer *vit2(vec2);
  for (vector<Integer>::const_iterator vit1(vec1.begin());
       vit1 != vec1.end(); ++vit1, ++vit2) {
    Integer v1(*vit1), v2(*vit2);
    {
      PairProb::iterator mit1(pair_prob->find(v1));
      if (mit1 == pair_prob->end()) {
        (*pair_prob)[v1] = ValueProb();
        mit1 = pair_prob->find(v1);
      }
      ValueProb::iterator mit2(mit1->second.find(v2));
      if (mit2 == mit1->second.end()) {
        mit1->second[v2] = 1.0;
      } else {
        ++(mit2->second);
      }
    }
    {
      ValueProb::iterator mit1(vec1_prob->find(v1));
      if (mit1 == vec1_prob->end()) {
        (*vec1_prob)[v1] = 1.0;
      } else {
        ++(mit1->second);
      }
    }
    {
      ValueProb::iterator mit2(vec2_prob->find(v2));
      if (mit2 == vec2_prob->end()) {
        (*vec2_prob)[v2] = 1.0;
      } else {
        ++(mit2->second);
      }
    }
  }
  
  for (PairProb::iterator mit1(pair_prob->begin()); mit1 != pair_prob->end(); ++mit1) {
    for (ValueProb::iterator mit2(mit1->second.begin()); mit2 != mit1->second.end(); ++mit2) {
      mit2->second /= instance_number;
    }
  }
  for (ValueProb::iterator mit(vec1_prob->begin()); mit != vec1_prob->end(); ++mit) {
    mit->second /= instance_number;
  }
  for (ValueProb::iterator mit(vec2_prob->begin()); mit != vec2_prob->end(); ++mit) {
    mit->second /= instance_number;
  }
}

Number compute_mutual_info(const PairProb& pair_prob,
                           const ValueProb& vec1_prob,
                           const ValueProb& vec2_prob) {
  Number mutual_info(0.0);
  for (PairProb::const_iterator mit1(pair_prob.begin()); mit1 != pair_prob.end(); ++mit1) {
    for (ValueProb::const_iterator mit2(mit1->second.begin()); mit2 != mit1->second.end(); ++mit2) {
      mutual_info += mit2->second * log(mit2->second / (vec1_prob.at(mit1->first) * vec2_prob.at(mit2->first)));
    }
  }
  mutual_info /= log(2.0);
  
  return mutual_info;
}

int main(int argc, char *argv[]) {
  try {
    if (argc != 8) {
      throw runtime_error("<file root path> <feature name> "
                          "<output folder name> <split number> "
                          "<target name> <select number> "
                          "<in memory(1 as true)>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string feature_name(argv[2]);
    const string feature_file((root_path / feature_name).string());
    const string dimension_file(feature_file + ".dimension");
    Integer dimension;
    {
      ifstream input;
      file_open(dimension_file.c_str(), ifstream::in | ifstream::binary, &input);
      boost::archive::text_iarchive(input) >> dimension;
      input.close();
    }
    const filesystem::path output_folder(root_path / argv[3]);
    const string split_number_str(argv[4]);
    const Integer split_number(in_range(integer_from_str(split_number_str.c_str()),
                                        1, numeric_limits<Integer>::max()));
    const filesystem::path split_path(output_folder / split_number_str);
    const string target_name(argv[5]);
    const string target_file((root_path / target_name).string());
    const string instance_number_file(target_file + ".instance_number");
    Integer instance_number;
    {
      ifstream input;
      file_open(instance_number_file.c_str(), ifstream::in | ifstream::binary, &input);
      boost::archive::text_iarchive(input) >> instance_number;
      input.close();
    }
    const string select_number_str(argv[6]);
    const Integer select_number(in_range(integer_from_str(select_number_str.c_str()),
                                         1, dimension));
    const bool in_memory(string(argv[7]) == "1");

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
      vector<Integer> vec(instance_number);
      vector<Number> IXX(dimension, 0.0);
      vector<Number> IXC(dimension);
      vector<Integer> best_feature_idx(select_number);
      best_feature_idx.clear();
      {
        ifstream input;
        file_open(target_file.c_str(), ifstream::in | ifstream::binary, &input);
        string line;
        for (Integer i(0); i < instance_number; ++i) {
          getline(input, line);
          vec[i] = static_cast<Integer>(strtod(line.data(), NULL));
        }
        input.close();
      }
      bool mrmr_done(false);
      if (in_memory) {
        bool memory_fail(false);
        vector<Integer *> data(dimension);
        for (size_t i(0); i < data.size(); ++i) {
          data[i] = (Integer *)malloc(instance_number * sizeof(Integer));
          if (data[i] == NULL) {
            for (size_t j(0); j < i; ++j) {
              free(data[j]);
            }
            memory_fail = true;
            throw runtime_error("memory_fail");
          }
        }

        if (!memory_fail) {
          {
            Integer column_counter(0);
            for (Integer i(0); i < split_number; ++i) {
              ostringstream split_id;
              split_id << i;
              filesystem::path split_folder(split_path / split_id.str());
              string feature_file((split_folder / feature_name).string());
              ifstream input;
              string line;
              file_open(feature_file.c_str(), ifstream::in | ifstream::binary, &input);
              while (getline(input, line)) {
                Integer *column(data[column_counter]);
                fill(column, column + instance_number, 0);
                SparseFormatter formatter(line);
                Integer col;
                Number value;
                while (formatter.next(&col, &value)) {
                  column[col - 1] = static_cast<Integer>(value);
                }
                ++column_counter;
              }
              input.close();
            }
          }
          cout << "overhead : "
               << (double)(clock() - start) / CLOCKS_PER_SEC << endl;

          for (Integer s(0); s < select_number; ++s) {
            clock_t start(clock());

            for (Integer i(0); i < dimension; ++i) {
              PairProb pair_prob;
              ValueProb vec1_prob, vec2_prob;
              Integer *vec2(data[i]);
              compute_joint_prob(vec, vec2, instance_number, &pair_prob, &vec1_prob, &vec2_prob);
              Number tmp_IXX(compute_mutual_info(pair_prob, vec1_prob, vec2_prob));
              if (s == 0) {
                IXC[i] = tmp_IXX;
              } else {
                IXX[i] += tmp_IXX;
              }
            }

            size_t size(best_feature_idx.size() + 1);
            Number max_value(IXC[0] - IXX[0] / size);
            Integer max_idx(0);
            for (Integer i(1); i < dimension; ++i) {
              Number value(IXC[i] - IXX[i] / size);
              if (value > max_value) {
                max_value = value;
                max_idx = i;
              }
            }
            best_feature_idx.push_back(max_idx);
            IXX[max_idx] = numeric_limits<Number>::max();

            cout << (double)(clock() - start) / CLOCKS_PER_SEC << ' '
                 << max_idx << ' '
                 << max_value << endl;
          }
          for (size_t i(0); i < data.size(); ++i) {
            free(data[i]);
          }
          mrmr_done = true;
        }
      } else {
        cout << "overhead : "
             << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
      }
      for (Integer i(0); i < worker_number; ++i) {
        mpi::request mrmr_done_req(world.isend(i, 0, mrmr_done));
        mrmr_done_req.wait();
      }
      if (!mrmr_done) {
        vector<Integer> begins(split_number);
        for (Integer i(0); i < split_number; ++i) {
          ostringstream split_id;
          split_id << i;
          filesystem::path split_folder(split_path / split_id.str());
          string feature_range_file((split_folder / feature_name).string() + ".feature_range");

          ifstream input;
          file_open(feature_range_file.c_str(), ifstream::in | ifstream::binary, &input);
          boost::archive::text_iarchive(input) >> begins[i];
          --begins[i];
          input.close();
        }

        for (Integer i(0); i < worker_number; ++i) {
          mpi::request instance_number_req(world.isend(i, 1, instance_number));
          instance_number_req.wait();
        }
        string line;
        for (Integer s(0); s < select_number; ++s) {
          clock_t start(clock());

          if (s != 0) {
            for (Integer i(split_number - 1); i >= 0; --i) {
              if (best_feature_idx.back() >= begins[i]) {
                ostringstream split_id;
                split_id << i;
                filesystem::path split_folder(split_path / split_id.str());
                string feature_file((split_folder / feature_name).string());

                ifstream input;
                file_open(feature_file.c_str(), ifstream::in | ifstream::binary, &input);
                for (Integer j(begins[i]); j < best_feature_idx.back(); ++j) {
                  input.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                getline(input, line);
                fill(vec.begin(), vec.end(), 0);
                SparseFormatter formatter(line);
                Integer col;
                Number value;
                while (formatter.next(&col, &value)) {
                  vec[col - 1] = static_cast<Integer>(value);
                }
                input.close();
                break;
              }
            }
          }

          for (Integer i(0); i < worker_number; ++i) {
            mpi::request vec_req(world.isend(i, 2, vec));
            vec_req.wait();
          }

          Integer task_id(0);
          vector<Integer> worker2task(worker_number, -1);
          vector<mpi::request> subcounter_reqs(worker_number);
          vector<Number> tmp_IXX;
          while (true) {
            Integer task_done(0);
            for (Integer i(0); i < worker_number; ++i) {
              if (worker2task[i] == split_number) {
                ++task_done;
                continue;
              }
              if (worker2task[i] != -1 && subcounter_reqs[i].test()) {
                Integer begin(begins[worker2task[i]]);
                for (size_t j(0); j < tmp_IXX.size(); ++j) {
                  if (s == 0) {
                    IXC[begin + j] = tmp_IXX[j];
                  } else {
                    IXX[begin + j] += tmp_IXX[j];
                  }
                }
                worker2task[i] = -1;
              }
              if (worker2task[i] == -1) {
                worker2task[i] = task_id;
                mpi::request task_id_req(world.isend(i, 3, worker2task[i]));
                task_id_req.wait();
                if (task_id < split_number) {
                  subcounter_reqs[i] = world.irecv(i, 4, tmp_IXX);
                  ++task_id;
                }
              }
            }
            if (task_done == worker_number) {
              break;
            }
          }
          size_t size(best_feature_idx.size() + 1);
          Number max_value(IXC[0] - IXX[0] / size);
          Integer max_idx(0);
          for (Integer i(1); i < dimension; ++i) {
            Number value(IXC[i] - IXX[i] / size);
            if (value > max_value) {
              max_value = value;
              max_idx = i;
            }
          }
          best_feature_idx.push_back(max_idx);
          IXX[max_idx] = numeric_limits<Number>::max();
        
          cout << (double)(clock() - start) / CLOCKS_PER_SEC << ' '
               << max_idx << ' '
               << max_value << endl;
        }
      }
    } else {
      bool mrmr_done;
      mpi::request mrmr_done_req(world.irecv(worker_number, 0, mrmr_done));
      mrmr_done_req.wait();
      if (!mrmr_done) {
        Integer instance_number;
        mpi::request instance_number_req(world.irecv(worker_number, 1, instance_number));
        instance_number_req.wait();

        vector<Integer> vec1, vec2;
        string line;
        for (Integer s(0); s < select_number; ++s) {
          mpi::request vec_req(world.irecv(worker_number, 2, vec1));
          vec_req.wait();
          vec2.resize(vec1.size());
          while (true) {
            Integer task_id(-1);
            mpi::request task_id_req(world.irecv(worker_number, 3, task_id));
            task_id_req.wait();
            if (task_id == split_number) {
              break;
            }
            ostringstream split_id;
            split_id << task_id;
            filesystem::path split_folder(split_path / split_id.str());
            string feature_file((split_folder / feature_name).string());
            string feature_range_file(feature_file + ".feature_range");

            ifstream input;
            file_open(feature_range_file.c_str(), ifstream::in | ifstream::binary, &input);
            Integer begin, end;
            boost::archive::text_iarchive(input) >> begin >> end;
            input.close();

            file_open(feature_file.c_str(), ifstream::in | ifstream::binary, &input);
            vector<Number> tmp_IXX(end - begin);
            for (size_t i(0); i < tmp_IXX.size(); ++i) {
              fill(vec2.begin(), vec2.end(), 0);
              getline(input, line);
              SparseFormatter formatter(line);
              Integer col;
              Number value;
              while (formatter.next(&col, &value)) {
                vec2[col - 1] = static_cast<Integer>(value);
              }
              PairProb pair_prob;
              ValueProb vec1_prob, vec2_prob;
              compute_joint_prob(vec1, vec2.data(), instance_number,
                                 &pair_prob, &vec1_prob, &vec2_prob);
              tmp_IXX[i] = compute_mutual_info(pair_prob, vec1_prob, vec2_prob);
            }
            mpi::request subcounter_req(world.isend(worker_number, 4, tmp_IXX));
            subcounter_req.wait();
            input.close();
          }
        }
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "mrmr_feature : \n"
         << e.what() << endl;
    return EXIT_FAILURE;
  }
}
