#ifndef MDT_CORE_FILE_WRITE_ERROR_HPP_
#define MDT_CORE_FILE_WRITE_ERROR_HPP_

#include <stdexcept>
#include <string>

namespace mdt_core {

std::runtime_error file_write_error(const std::string& path);
std::runtime_error file_write_error(const char *path);

}

#endif /* MDT_CORE_FILE_WRITE_ERROR_HPP_ */
