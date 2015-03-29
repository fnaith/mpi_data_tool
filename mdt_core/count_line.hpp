#ifndef MDT_CORE_COUNT_LINE_HPP_
#define MDT_CORE_COUNT_LINE_HPP_

#include "Integer.hpp"

#include <string>

namespace mdt_core {

Integer count_line(const std::string& file);
Integer count_line(const char *file);

}

#endif /* MDT_CORE_COUNT_LINE_HPP_ */
