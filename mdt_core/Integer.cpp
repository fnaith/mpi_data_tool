#include "Integer.hpp"

#include <stdexcept>
#include <cstdlib>

using namespace std;

namespace mdt_core {

Integer integer_from_str(const char *str) {
  char *tail;
  long i(strtol(str, &tail, 10));
  Integer integer(static_cast<Integer>(i));
  if (str == tail || integer != i) {
    throw runtime_error("integer_from_str :\n"
                        "convert fail\n");
  }
  return integer;
}

}
