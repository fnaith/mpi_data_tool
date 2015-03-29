#include "../mdt_core/file_open.hpp"

#include <boost/filesystem.hpp>

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

    if (argc != 5) {
      throw runtime_error("<file root path> <input file name> "
                          "<feature name> <target name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string input_file((root_path / argv[2]).string());
    const string feature_file((root_path / argv[3]).string());
    const string target_file((root_path / argv[4]).string());

    ifstream input;
    file_open(input_file.c_str(), ifstream::in | ifstream::binary, &input);

    ofstream feature_output;
    file_open(feature_file.c_str(), ofstream::out | ofstream::binary, &feature_output);

    ofstream target_output;
    file_open(target_file.c_str(), ofstream::out | ofstream::binary, &target_output);

    string line;
    while (getline(input, line)) {
      const char *head(line.data());
      char *tail;
      strtod(head, &tail);
      if (head == tail) {
        throw runtime_error("target format invalid\n");
      }
      target_output.write(head, tail - head);
      target_output.put('\n');
      size_t last_not_space(line.find_last_not_of(" \r\n"));
      size_t length(last_not_space - (tail - head));
      if (length != 0) {
        feature_output.write(tail + 1, length);
        feature_output.put(' ');
      }
      feature_output.put('\n');
    }

    input.close();
    feature_output.close();
    target_output.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "libsvm_separate_target_sparse : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
