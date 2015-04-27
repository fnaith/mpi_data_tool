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
#define FEATURE_TYPE_NUMBER 6
typedef boost::array<bool, FEATURE_TYPE_NUMBER> Config;
typedef boost::array<vector<Number>, FEATURE_TYPE_NUMBER> Result;

const char *FEATURE_TYPE_NAME[FEATURE_TYPE_NUMBER] = {
  "min", "max", "mean", "std", "cor", "info"
};

void compute_joint_prob(const vector<Integer>& vec1,
                        const vector<Number>& vec2,
                        Integer instance_number,
                        PairProb *pair_prob,
                        ValueProb *vec1_prob,
                        ValueProb *vec2_prob) {
  vector<Integer>::const_iterator vit1(vec1.begin());
  vector<Number>::const_iterator vit2(vec2.begin());
  for (; vit1 != vec1.end(); ++vit1, ++vit2) {
    Integer v1(*vit1), v2(static_cast<Integer>(*vit2));
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
  for (PairProb::iterator mit1(pair_prob->begin());
       mit1 != pair_prob->end(); ++mit1) {
    for (ValueProb::iterator mit2(mit1->second.begin());
         mit2 != mit1->second.end(); ++mit2) {
      mit2->second /= instance_number;
    }
  }
  for (ValueProb::iterator mit(vec1_prob->begin());
       mit != vec1_prob->end(); ++mit) {
    mit->second /= instance_number;
  }
  for (ValueProb::iterator mit(vec2_prob->begin());
       mit != vec2_prob->end(); ++mit) {
    mit->second /= instance_number;
  }
}

Number compute_mutual_info(const PairProb& pair_prob,
                           const ValueProb& vec1_prob,
                           const ValueProb& vec2_prob) {
  Number mutual_info(0.0);
  for (PairProb::const_iterator mit1(pair_prob.begin());
       mit1 != pair_prob.end(); ++mit1) {
    for (ValueProb::const_iterator mit2(mit1->second.begin());
         mit2 != mit1->second.end(); ++mit2) {
      mutual_info += (mit2->second *
                      log(mit2->second / (vec1_prob.at(mit1->first) *
                                          vec2_prob.at(mit2->first))));
    }
  }
  mutual_info /= log(2.0);
  return mutual_info;
}

void feature_understanding(const string& file,
                           const Integer instance_number,
                           const vector<Number>& target, const Number target_mean, const Number target_std,
                           const bool need_min, const bool need_max, const bool need_mean,
                           const bool need_std, const bool need_cor, const bool need_info,
                           vector<Number> *mins, vector<Number> *maxs, vector<Number> *means,
                           vector<Number> *stds, vector<Number> *cors, vector<Number> *infos,
                           vector<Integer> *buffer) {
  try {
    ifstream input;
    file_open(file.c_str(), ifstream::in | ifstream::binary, &input);
    string line;
    if (need_info) {
      buffer->resize(instance_number);
    }
    while (getline(input, line)) {
      Integer element_counter(0);
      Number minimum(numeric_limits<Number>::max());
      Number maximum(numeric_limits<Number>::min());
      Number sum(0.0);
      Number squared_sum(0.0);
      Number dot_product_of_target(0.0);
      if (need_info) {
        fill(buffer->begin(), buffer->end(), 0);
      }
      SparseFormatter formatter(line);
      Integer col;
      Number value;
      while (formatter.next(&col, &value)) {
        ++element_counter;
        minimum = min(minimum, value);
        maximum = max(maximum, value);
        sum += value;
        squared_sum += (value * value);
        if (need_cor) {
          dot_product_of_target += (value * target[col - 1]);
        }
        if (need_info) {
          buffer->at(col - 1) = static_cast<Integer>(value);
        }
      }
      if (element_counter < instance_number) {
        minimum = min(minimum, 0.0);
      }
      if (element_counter < instance_number) {
        maximum = max(maximum, 0.0);
      }
      Number mean(sum / instance_number);
      Number std(sqrt(squared_sum / instance_number - mean * mean));
      if (need_min) {
        mins->push_back(minimum);
      }
      if (need_max) {
        maxs->push_back(maximum);
      }
      if (need_mean) {
        means->push_back(mean);
      }
      if (need_std) {
        stds->push_back(std);
      }
      if (need_cor) {
        cors->push_back((dot_product_of_target / instance_number - mean * target_mean) / (std * target_std));
      }
      if (need_info) {
        PairProb pair_prob;
        ValueProb vec1_prob, vec2_prob;
        compute_joint_prob(*buffer, target, instance_number,
                           &pair_prob, &vec1_prob, &vec2_prob);
        infos->push_back(compute_mutual_info(pair_prob, vec1_prob, vec2_prob));
      }
    }
    input.close();
  } catch (const exception& e) {
    string message;
    try {
      ostringstream what_arg;
      what_arg << "feature_understanding :\n";
      what_arg << e.what() << '\n';
      message = what_arg.str();
    } catch (const exception&) {
      cout << e.what() << endl;
      throw runtime_error("feature_understanding\n");
    }
    throw runtime_error(message);
  }
}

int main(int argc, char *argv[]) {
  try {
    const int DEFAULT_ARGC(6);
    if (argc < DEFAULT_ARGC) {
      throw runtime_error("<file root path> <feature name> "
                          "<output folder name> <split number> "
                          "<target name> "
                          "[min] [max] [mean] "
                          "[std(standard deviation)] "
                          "[cor](pearson correlation) "
                          "[info](mutual Information) \n");
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
    const string instance_number_file(target_file + ".instance_number");

    mpi::environment env(argc, argv);
    mpi::communicator world;
    if (!world) {
      throw runtime_error("communicator fail\n");
    }
    const Integer worker_number(world.size() - 1);
    if (worker_number < 1) {
      throw runtime_error("worker_number must > 1\n");
    }

    Config need;
    for (int i(0); i < FEATURE_TYPE_NUMBER; ++i) {
      need[i] = false;
    }
    Integer instance_number;
    vector<Number> target;
    Number target_mean(0.0);
    Number target_std(0.0);

    if (world.rank() == worker_number) {
      clock_t start(clock());

      bool need_target(false);
      {
        for (int i(DEFAULT_ARGC); i < argc; ++i) {
          string option(argv[i]);
          for (int j(0); j < FEATURE_TYPE_NUMBER; ++j) {
            if (option == FEATURE_TYPE_NAME[j]) {
              need[j] = true;
              if (option == "cor" || option == "info") {
                need_target = true;
              }
              break;
            }
          }
        }
        for (Integer i(0); i < worker_number; ++i) {
          mpi::request config_req(world.isend(i, 0, need));
          config_req.wait();
        }
      }
      {
        ifstream input;
        file_open(instance_number_file.c_str(), ifstream::in | ifstream::binary, &input);
        boost::archive::text_iarchive(input) >> instance_number;
        input.close();

        for (Integer i(0); i < worker_number; ++i) {
          mpi::request instance_number_req(world.isend(i, 1, instance_number));
          instance_number_req.wait();
        }
      }
      if (need_target) {
        target.resize(instance_number);
        ifstream input;
        string line;
        file_open(target_file.c_str(), ifstream::in | ifstream::binary, &input);
        for (vector<Number>::iterator it(target.begin());
             it != target.end(); ++it) {
          getline(input, line);
          (*it) = strtod(line.data(), NULL);
          target_mean += (*it);
          target_std += ((*it) * (*it));
        }
        target_mean /= instance_number;
        target_std = sqrt(target_std / instance_number - target_mean * target_mean);
        line.swap(string());
        input.close();
      }
      for (Integer i(0); i < worker_number; ++i) {
        mpi::request target_req(world.isend(i, 2, target));
        target_req.wait();
        mpi::request target_mean_req(world.isend(i, 3, target_mean));
        target_mean_req.wait();
        mpi::request target_std_req(world.isend(i, 4, target_std));
        target_std_req.wait();
      }

      Integer task_id(0);
      vector<Integer> worker2task(worker_number, -1);
      vector<mpi::request> result_reqs(worker_number);
      Result result;
      while (true) {
        Integer task_done(0);
        for (Integer i(0); i < worker_number; ++i) {
          if (worker2task[i] == split_number) {
            ++task_done;
            continue;
          }
          if (worker2task[i] != -1 && result_reqs[i].test()) {
            ostringstream id;
            id << worker2task[i];
            filesystem::path sub_split_path(split_path / id.str());
            for (int j(0); j < FEATURE_TYPE_NUMBER; ++j) {
              string output_file((sub_split_path / feature_name).string() + "." + FEATURE_TYPE_NAME[j]);
              ofstream output;
              file_open(output_file.c_str(), ofstream::out | ofstream::binary, &output);
              boost::archive::text_oarchive(output) << reinterpret_cast<const vector<Number>&>(result[2]);
              output.close();
            }
            worker2task[i] = -1;
          }
          if (worker2task[i] == -1) {
            worker2task[i] = task_id;
            mpi::request task_id_req(world.isend(i, 5, worker2task[i]));
            task_id_req.wait();
            if (task_id < split_number) {
              result_reqs[i] = world.irecv(i, 6, result);
              ++task_id;
            }
          }
        }
        if (task_done == worker_number) {
          break;
        }
      }

      cout << (double)(clock() - start) / CLOCKS_PER_SEC;
    } else {
      {
        mpi::request config_req(world.irecv(worker_number, 0, need));
        config_req.wait();
      }
      {
        mpi::request instance_number_req(world.irecv(worker_number, 1, instance_number));
        instance_number_req.wait();
      }
      {
        mpi::request target_req(world.irecv(worker_number, 2, target));
        target_req.wait();
        mpi::request target_mean_req(world.irecv(worker_number, 3, target_mean));
        target_mean_req.wait();
        mpi::request target_std_req(world.irecv(worker_number, 4, target_std));
        target_std_req.wait();
      }
      Result result;
      vector<Integer> buffer;
      while (true) {
        Integer task_id(-1);
        mpi::request task_id_req(world.irecv(worker_number, 5, task_id));
        task_id_req.wait();
        if (task_id == split_number) {
          break;
        }
        ostringstream split_id;
        split_id << task_id;
        filesystem::path split_folder(split_path / split_id.str());
        for (Result::iterator it(result.begin());
             it != result.end(); ++it) {
           it->clear();
        }
        feature_understanding((split_folder / feature_name).string(),
                              instance_number,
                              target, target_mean, target_std,
                              need[0], need[1], need[2],
                              need[3], need[4], need[5],
                              &result[0], &result[1], &result[2],
                              &result[3], &result[4], &result[5],
                              &buffer);
        mpi::request result_req(world.isend(worker_number, 6, result));
        result_req.wait();
      }
    }
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "feature_understanding : \n"
         << e.what() << endl;
    return EXIT_FAILURE;
  }
}
