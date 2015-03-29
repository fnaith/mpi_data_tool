#include "../mdt_core/count_line.hpp"
#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_read_error.hpp"
#include "../mdt_core/file_write_error.hpp"

#include <boost/archive/text_oarchive.hpp>
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

    if (argc != 8) {
      throw runtime_error("<file root path> "
                          "<output folder name> <split number> "
                          "<feature name> "
                          "<sample name> <sample rate> <sample split>\n");
    }
    const filesystem::path root_path(argv[1]);
    const filesystem::path output_folder(root_path / argv[2]);
    const string instance_number_file((output_folder / "instance_number").string());
    const string split_number_str(argv[3]);
    const Integer split_number(in_range(integer_from_str(split_number_str.c_str()),
                                        1, numeric_limits<Integer>::max()));
    const filesystem::path split_path(output_folder / split_number_str);
    const string feature_name(argv[4]);
    const string sample_name(argv[5]);
    const filesystem::path split(output_folder / split_number_str);
    const Number sample_rate(in_range(number_from_str(argv[6]),
                                      numeric_limits<Number>::epsilon(), 1.0));
    const Integer sample_split(in_range(integer_from_str(argv[7]),
                                        1, numeric_limits<Integer>::max()));
    const Integer least_sample_number(max(static_cast<Integer>(split_number * sample_rate / sample_split), 1));

    for (Integer i(0); i < sample_split; ++i) {
      const Integer begin(i * split_number / sample_split);
      const Integer end((i + 1) * split_number / sample_split);
      ostringstream split_id;
      split_id << i;
      string sample_output((split / split_id.str() / sample_name).string());
      ofstream output;
      output.open(sample_output.c_str(), ofstream::out | ofstream::binary);
      if (!output) {
        throw file_write_error(sample_output);
      }
      for (Integer j(begin); j < min(begin + least_sample_number, end); ++j) {
        ostringstream sapmle_id;
        sapmle_id << j;
        string input_file((split / sapmle_id.str() / feature_name).string());
        ifstream input(input_file.c_str(), ofstream::out | ofstream::binary);
        if (!input) {
          throw file_read_error(input_file);
        }
        string content((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
        output << content;
      }
      output.close();
    }

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "sample_split : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
