#include "../mdt_core/Integer.hpp"
#include "../mdt_core/file_open.hpp"
#include "../mdt_core/SparseFormatter.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/vector.hpp>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace mdt_core;
using namespace std;

namespace filesystem = boost::filesystem;

int main(int argc, char *argv[]) {
  try {
    clock_t start(clock());
    if (argc != 3) {
      throw runtime_error("<file root path> <feature name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string feature_name(argv[2]);
    const string feature_file((root_path / feature_name).string());
    const string dimension_file(feature_file + ".dimension");
    const string feature_count_file(feature_file + ".feature_count");

    ifstream input;
    file_open(dimension_file.c_str(), ifstream::in | ifstream::binary, &input);

    Integer dimension;
    boost::archive::text_iarchive(input) >> dimension;
    input.close();

    vector<Integer> feature_count(dimension, 0);

    file_open(feature_file.c_str(), ifstream::in | ifstream::binary, &input);

    string line;
    while (getline(input, line)) {
      SparseFormatter formatter(line);
      Integer col;
      Number value;
      while (formatter.next(&col, &value)) {
        ++(feature_count[col - 1]);
      }
    }

    input.close();

    ofstream output;
    file_open(feature_count_file.c_str(), ofstream::out | ofstream::binary, &output);
    boost::archive::text_oarchive(output) << reinterpret_cast<const vector<Integer>&>(feature_count);
    output.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "count_feature : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
