#include "../mdt_core/count_line.hpp"
#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"

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

    if (argc != 4) {
      throw runtime_error("<file root path> "
                          "<input orange file name> "
                          "<output libsvm file name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string orange_file((root_path / argv[2]).string());
    const string libsvm_file((root_path / argv[3]).string());

    ifstream input;
    file_open(orange_file.c_str(), ifstream::in | ifstream::binary, &input);

    ofstream output;
    file_open(libsvm_file.c_str(), ofstream::out | ofstream::binary, &output);

    string line;
    getline(input, line);
    if (line.find("class") != 0) {
      throw runtime_error("orange header format invalid");
    }
    const Integer dimension(in_range(count(line.begin(), line.end(), ','),
                                     1, numeric_limits<Integer>::max()));
    while (getline(input, line)) {
      size_t length(line.size());
      if (length > 0) {
        --length;
        if (line[length] == '\r') {
          line[length] = ',';
        } else {
          line.push_back(',');
        }
      }
      const char *head(line.data());
      char *tail;
      strtod(head, &tail);
      if (head == tail || *tail != ',') {
        throw runtime_error("target format invalid\n");
      }
      output.write(head, tail - head);
      output.put(' ');
      head = tail + 1;
      string tmp;
      for (Integer i(0); i < dimension; ++i) {
        double value(strtod(head, &tail));
        if (head == tail || *tail != ',') {
          throw runtime_error("feature format invalid\n");
        }
        tmp.assign(head, tail - head);
        if (tmp.find_first_of("123456789") != string::npos) {
          output << (i + 1) << ':' << tmp << ' ';
        }
        head = tail + 1;
      }
      if (*head != '\0') {
        throw runtime_error("instance format invalid\n");
      }
      output.put('\n');
    }

    input.close();
    output.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "orange_to_libsvm : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
