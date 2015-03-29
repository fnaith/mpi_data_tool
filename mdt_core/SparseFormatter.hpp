#ifndef SSVM_SPARSEFORMATTER_HPP_
#define SSVM_SPARSEFORMATTER_HPP_

#include "Integer.hpp"
#include "Number.hpp"

#include <string>

namespace mdt_core {

class SparseFormatter {
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

}

#endif /* SSVM_SPARSEFORMATTER_HPP_ */
