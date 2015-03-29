#include "../mdt_core/count_line.hpp"
#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/filesystem.hpp>

#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace mdt_core;
using namespace std;

namespace filesystem = boost::filesystem;

struct Pair {
  string v;
  Integer r;
};

int main(int argc, char *argv[]) {
  try {
    clock_t start(clock());

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
    const string dimension_file(feature_file + ".dimension");
    const string tmp_file_postfix(".triplet");

    ifstream input;
    file_open(dimension_file.c_str(), ifstream::in | ifstream::binary, &input);

    Integer dimension;
    boost::archive::text_iarchive(input) >> dimension;
    Integer fold(dimension / split_number);
    if (dimension % split_number != 0) {
      ++fold;
    }
    input.close();

    for (Integer i(0); i < split_number; ++i) {
      ostringstream id;
      id << i;
      filesystem::path sub_split_path(split_path / id.str());
      filesystem::create_directories(sub_split_path);
      filesystem::remove((sub_split_path / feature_name).string() + tmp_file_postfix);
    }

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "clean_element : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
