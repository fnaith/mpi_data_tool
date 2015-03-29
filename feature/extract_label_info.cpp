#include "../mdt_core/file_open.hpp"
#include "../mdt_core/Integer.hpp"
#include "../mdt_core/Number.hpp"
#include "../mdt_core/unordered_map.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace mdt_core;
using namespace std;

namespace archive = boost::archive;
namespace filesystem = boost::filesystem;

typedef unordered_map<string, Integer> Map;

void extract_label_info(const string& file,
                        Map *targets) {
  try {
    ifstream input;
    file_open(file.c_str(), ifstream::in | ifstream::binary, &input);

    string line;
    while (getline(input, line)) {
      Map::iterator it(targets->find(line));
      if (it == targets->cend()) {
        (*targets)[line] = 1;
      } else {
        ++(it->second);
      }
    }
  } catch (const exception& e) {
    string message;
    try {
      message.assign("extract_label_info :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("extract_label_info\n");
    }
    throw runtime_error(message);
  }
}

int main(int argc, char *argv[]) {
  try {
    clock_t start(clock());

    if (argc != 3) {
      throw runtime_error("<file root path> "
                          "<target name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string target_file((root_path / argv[2]).string());
    const string label_file(target_file + ".label_number");
    const string labels_file(target_file + ".labels");
    const string label_count_file(target_file + ".label_count");
    const string label_entropy_file(target_file + ".label_entropy");

    Map targets;
    extract_label_info(target_file, &targets);

    const Integer label_number(static_cast<Integer>(targets.size()));
    vector<string> labels;
    vector<Integer> label_count;
    for(Map::const_iterator it(targets.cbegin()); it != targets.cend(); ++it) {
      labels.push_back(it->first);
    }
    sort(labels.begin(), labels.end());

    Integer instance_number(0);
    for (Integer i(0); i < label_number; ++i) {
      Integer count(targets[labels[i]]);
      label_count.push_back(count);
      instance_number += count;
    }

    Number label_entropy(0.0);
    for (Integer i(0); i < label_number; ++i) {
      Number p(label_count[1] / static_cast<Number>(instance_number));
      label_entropy -= (p * log(p) / log(2));
    }

    ofstream output;

    file_open(label_file.c_str(), ofstream::out | ofstream::binary, &output);
    archive::text_oarchive(output) << label_number;
    output.close();

    file_open(labels_file.c_str(), ofstream::out | ofstream::binary, &output);
    archive::text_oarchive(output) << reinterpret_cast<const vector<string>&>(labels);
    output.close();

    file_open(label_count_file.c_str(), ofstream::out | ofstream::binary, &output);
    archive::text_oarchive(output) << reinterpret_cast<const vector<Integer>&>(label_count);
    output.close();

    file_open(label_entropy_file.c_str(), ofstream::out | ofstream::binary, &output);
    archive::text_oarchive(output) << label_entropy;
    output.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "extract_label_info : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
