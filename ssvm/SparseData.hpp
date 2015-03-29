#ifndef SSVM_SPARSEDATA_HPP_
#define SSVM_SPARSEDATA_HPP_

#include "SparseMatrix.hpp"

#include <string>

namespace ssvm {

class SparseData {
  typedef std::string string;
 public:
  static void read(const string& file, const Integer dimension,
                   SparseMatrix *matrix);
  static void read(const string& file, const Integer dimension,
                   const Integer begin, const Integer end,
                   SparseMatrix *matrix);
 private:
  explicit SparseData();
  ~SparseData();
  SparseData& operator=(const SparseData&);
  SparseData(const SparseData&);
};

}

#endif /* SSVM_SPARSEDATA_HPP_ */
