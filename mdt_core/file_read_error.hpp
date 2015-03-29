#ifndef MDT_CORE_FILE_READ_ERROR_HPP_
#define MDT_CORE_FILE_READ_ERROR_HPP_

#include <stdexcept>
#include <string>

namespace mdt_core {

std::runtime_error file_read_error(const std::string& path);
std::runtime_error file_read_error(const char *path);

}

#endif /* MDT_CORE_FILE_READ_ERROR_HPP_ */
