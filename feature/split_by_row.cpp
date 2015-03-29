#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/filesystem.hpp>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace mdt_core;
using namespace std;

namespace filesystem = boost::filesystem;

int main(int argc, char *argv[]) {
  try {
    clock_t start(clock());

    if (argc != 6) {
      throw runtime_error("<file root path> <input file name> "
                          "<output folder name> <split number> "
                          "<target name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string input_name(argv[2]);
    const string input_file((root_path / input_name).string());
    const filesystem::path output_folder(root_path / argv[3]);
    filesystem::create_directories(output_folder);
    const string split_number_str(argv[4]);
    const Integer split_number(in_range(integer_from_str(split_number_str.c_str()),
                                        1, numeric_limits<Integer>::max()));
    const filesystem::path split_path(output_folder / split_number_str);
    const string instance_number_file((root_path / argv[5]).string() + ".instance_number");

    ifstream input;

    file_open(instance_number_file.c_str(), ifstream::in | ifstream::binary, &input);
    Integer instance_number;
    boost::archive::text_iarchive(input) >> instance_number;
    input.close();

    file_open(input_file.c_str(), ifstream::in | ifstream::binary, &input);

    string line;
    for (int64_t i(0); i < split_number; ++i) {
      ostringstream id;
      id << i;
      filesystem::path sub_split_path(split_path / id.str());
      filesystem::create_directories(sub_split_path);
      string output_file((sub_split_path / input_name).string());

      ofstream output;
      file_open(output_file.c_str(), ofstream::out | ofstream::binary, &output);

      const int64_t begin(i * instance_number / split_number);
      const int64_t end((i + 1) * instance_number / split_number);
      for (int64_t j(begin); j < end; ++j) {
        getline(input, line);
        output.write(line.data(), line.size());
        output.put('\n');
      }
    }

    input.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "split_by_row : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
