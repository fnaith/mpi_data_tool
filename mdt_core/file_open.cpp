#include "file_open.hpp"

#include "file_read_error.hpp"
#include "file_write_error.hpp"

using namespace std;

namespace mdt_core {

void file_open(const string& path, const ios_base::openmode& mode, ifstream *ifs) {
  file_open(path.c_str(), mode, ifs);
}

void file_open(const char *path, const ios_base::openmode& mode, ifstream *ifs) {
  ifs->open(path, mode);
  if (!(*ifs)) {
    throw file_read_error(path);
  }
}

void file_open(const string& path, const ios_base::openmode& mode, ofstream *ofs) {
  file_open(path.c_str(), mode, ofs);
}

void file_open(const char *path, const ios_base::openmode& mode, ofstream *ofs) {
  ofs->open(path, mode);
  if (!(*ofs)) {
    throw file_write_error(path);
  }
}

}
