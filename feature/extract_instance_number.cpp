#include "../mdt_core/count_line.hpp"
#include "../mdt_core/file_open.hpp"

#include <boost/archive/text_oarchive.hpp>
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

    if (argc != 3) {
      throw runtime_error("<file root path> "
                          "<instance_number file name>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string target_file((root_path / argv[2]).string());
    const string instance_number_file(target_file + ".instance_number");
    const Integer instance_number(count_line(target_file.c_str()));

    ofstream output;
    file_open(instance_number_file.c_str(), ofstream::out | ofstream::binary, &output);
    boost::archive::text_oarchive(output) << instance_number;
    output.close();

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "extract_instance_number : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
