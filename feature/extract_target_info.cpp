#include "../mdt_core/count_line.hpp"
#include "../mdt_core/file_open.hpp"
#include "../mdt_core/Integer.hpp"
#include "../mdt_core/Number.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace mdt_core;
using namespace std;

namespace archive = boost::archive;
namespace filesystem = boost::filesystem;

void extract_targets(const string& file,
                     boost::array<Number, 2> *sum) {
  try {
    ifstream input;
    file_open(file.c_str(), ifstream::in | ifstream::binary, &input);

    string line;
    while (getline(input, line)) {
      Number value(strtod(line.data(), NULL));
      sum->at(0) += value;
      sum->at(1) += (value * value);
    }
  } catch (const exception& e) {
    string message;
    try {
      message.assign("extract_targets :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("extract_targets\n");
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
    const string instance_number_file(target_file + ".instance_number");
    const string target_mean_file(target_file + ".target_mean");
    const string target_std_file(target_file + ".target_std");

    ifstream input;
    file_open(instance_number_file.c_str(), ifstream::in | ifstream::binary, &input);

    Integer instance_number;
    archive::text_iarchive(input) >> instance_number;
    input.close();

    boost::array<Number, 2> sum;
    sum[0] = 0.0;
    sum[1] = 0.0;
    extract_targets(target_file, &sum);
    sum[0] /= instance_number;
    sum[1] = sqrt(sum[1] / instance_number - sum[0] * sum[0]);

    ofstream output;

    file_open(target_mean_file.c_str(), ofstream::out | ofstream::binary, &output);
    archive::text_oarchive(output) << sum[0];
    output.close();

    file_open(target_std_file.c_str(), ofstream::out | ofstream::binary, &output);
    archive::text_oarchive(output) << sum[1];
    output.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "extract_target_info : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
