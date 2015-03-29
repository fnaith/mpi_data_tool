#include "file_write_error.hpp"

#include <sstream>

using namespace std;

namespace mdt_core {

runtime_error file_write_error(const string& path) {
  return file_write_error(path.c_str());
}

runtime_error file_write_error(const char *path) {
  try {
    ostringstream what_arg;
    what_arg << "cannot write file :\n";
    what_arg << path << '\n';
    return runtime_error(what_arg.str());
  } catch (const exception&) {
    return runtime_error("file_write_error\n");
  }
}

}
