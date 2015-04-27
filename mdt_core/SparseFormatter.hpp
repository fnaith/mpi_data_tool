#ifndef SSVM_SPARSEFORMATTER_HPP_
#define SSVM_SPARSEFORMATTER_HPP_

#include "Integer.hpp"
#include "Number.hpp"

#include <string>

namespace mdt_core {

class SparseFormatter {
  typedef std::runtime_error runtime_error;
  typedef std::string string;
 public:
  explicit SparseFormatter(const string& line);

  bool next(Integer *index, Number *value);
 private:
  static const char kPairSeparator_ = ':';
  static const char kElementSeparator_ = ' ';

  SparseFormatter& operator=(const SparseFormatter&);
  SparseFormatter(const SparseFormatter&);

  const char *head_;
  char *tail_;
};

inline bool SparseFormatter::next(Integer *index, Number *value) {
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

#endif /* SSVM_SPARSEFORMATTER_HPP_ */
