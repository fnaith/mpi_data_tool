#include "Number.hpp"

#include <stdexcept>
#include <cstdlib>

using namespace std;

namespace mdt_core {

Number number_from_str(const char *str) {
  char *tail;
  double d(strtod(str, &tail));
  Number number(static_cast<Number>(d));
  if (str == tail || number != d) {
    throw runtime_error("number_from_str :\n"
                        "convert fail\n");
  }
  return number;
}

}
