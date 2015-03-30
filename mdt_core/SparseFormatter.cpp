#include "SparseFormatter.hpp"

#include <cstdlib>
#include <fstream>
#include <stdexcept>

using namespace std;

namespace mdt_core {

SparseFormatter::SparseFormatter(const string& line)
  : head_(line.data()),
    tail_(NULL) {
}

bool SparseFormatter::next(Integer *index, Number *value) {
  if (*head_ == '\0') {
    return false;
  }
  *index = strtol(head_, &tail_, 10);
  if (head_ == tail_ || *tail_ != kPairSeparator_) {
    throw runtime_error("SparseFormatter::next\n"
                        "index format invalid\n");
  }
  head_ = tail_ + 1;
  *value = strtod(head_, &tail_);
  if (head_ == tail_ || *tail_ != kElementSeparator_) {
    throw runtime_error("SparseFormatter::next\n"
                        "value format invalid\n");
  }
  head_ = tail_ + 1;
  return true;
}

}
