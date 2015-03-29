#ifndef MDT_CORE_FILE_OPEN_HPP_
#define MDT_CORE_FILE_OPEN_HPP_

#include <fstream>
#include <string>

namespace mdt_core {

void file_open(const std::string& path, const std::ios_base::openmode& mode, std::ifstream *ifs);
void file_open(const char *path, const std::ios_base::openmode& mode, std::ifstream *ifs);

void file_open(const std::string& path, const std::ios_base::openmode& mode, std::ofstream *ofs);
void file_open(const char *path, const std::ios_base::openmode& mode, std::ofstream *ofs);

}

#endif /* MDT_CORE_FILE_OPEN_HPP_ */
