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

    if (argc != 7) {
      throw runtime_error("<file root path> "
                          "<input A name> <input y name> "
                          "<pos label> "
                          "<output pos name> <output neg name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string input_A_file((root_path / argv[2]).string());
    const string input_y_file((root_path / argv[3]).string());
    const string pos_label(argv[4]);
    const string output_pos_file((root_path / argv[5]).string());
    const string output_neg_file((root_path / argv[6]).string());

    ifstream input_A;
    file_open(input_A_file.c_str(), ifstream::in | ifstream::binary, &input_A);

    ifstream input_y;
    file_open(input_y_file.c_str(), ifstream::in | ifstream::binary, &input_y);

    ofstream output_pos;
    file_open(output_pos_file.c_str(), ofstream::out | ofstream::binary, &output_pos);

    ofstream output_neg;
    file_open(output_neg_file.c_str(), ofstream::out | ofstream::binary, &output_neg);

    string line;
    while (getline(input_y, line)) {
      ofstream& output(line == pos_label ? output_pos : output_neg);
      getline(input_A, line);
      output << line << '\n';
    }

    input_A.close();
    input_y.close();
    output_pos.close();
    output_neg.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "sparse_separate_pos_neg : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
