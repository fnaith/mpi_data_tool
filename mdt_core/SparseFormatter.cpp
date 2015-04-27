#include "SparseFormatter.hpp"

#include <cstdlib>
#include <fstream>

namespace mdt_core {

SparseFormatter::SparseFormatter(const string& line)
  : head_(line.data()),
    tail_(NULL) {
}

}
