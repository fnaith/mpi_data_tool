#include "../mdt_core/file_open.hpp"
#include "../mdt_core/Integer.hpp"
#include "../mdt_core/file_open.hpp"
#include "../mdt_core/SparseFormatter.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace mdt_core;
using namespace std;

namespace filesystem = boost::filesystem;

int main(int argc, char *argv[]) {
  try {
    clock_t start(clock());

    if (argc != 3) {
      throw runtime_error("<file root path> "
                          "<feature name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string feature_file((root_path / argv[2]).string());
    const string dimension_file(feature_file + ".dimension");

    ifstream input;
    file_open(feature_file.c_str(), ifstream::in | ifstream::binary, &input);

    string line;
    Integer max_dimension(0);
    while (getline(input, line)) {
      SparseFormatter formatter(line);
      Integer col;
      Number value;
      while (formatter.next(&col, &value)) {
        max_dimension = max(max_dimension, col);
      }
    }

    input.close();

    ofstream output;
    file_open(dimension_file.c_str(), ofstream::out | ofstream::binary, &output);
    boost::archive::text_oarchive(output) << max_dimension;
    output.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "extract_dimension : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
