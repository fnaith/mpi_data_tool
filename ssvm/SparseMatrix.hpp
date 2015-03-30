#ifndef SSVM_SPARSEMATRIX_HPP_
#define SSVM_SPARSEMATRIX_HPP_

#include "Triplets.hpp"

#include <stdexcept>
#include <string>

namespace ssvm {

typedef Eigen::SparseMatrix<Number, Eigen::RowMajor, Integer> SparseMatrix;

template <class Archive>
void save_sparse_matrix(Archive& ar,
                        const SparseMatrix& matrix) {
  try {
    const Integer rows(matrix.rows()), cols(matrix.cols());
    ar & rows;
    ar & cols;
    Integer length(matrix.nonZeros());
    ar & length;
    for (Integer k(0); k < matrix.outerSize(); ++k) {
      for (SparseMatrix::InnerIterator it(matrix, k); it; ++it) {
        length = it.col();
        ar & k;
        ar & length;
        ar & it.value();
      }
    }
  } catch (const std::exception& e) {
    std::string message;
    try {
      message.assign("save_sparse_matrix :\n");
      message += e.what();
    } catch (const std::exception&) {
      throw std::runtime_error("save_sparse_matrix\n");
    }
    throw std::runtime_error(message);
  }
}

template <class Archive>
void load_sparse_matrix(Archive& ar,
                        SparseMatrix& matrix) {
  try {
    Integer rows, cols, length;
    ar & rows;
    ar & cols;
    matrix.resize(rows, cols);
    ar & length;
    Number value;
    Triplets triplets;
    while (length--) {
      ar & rows;
      ar & cols;
      ar & value;
      triplets.push_back(Triplet(rows, cols, value));
    }
    matrix.setFromTriplets(triplets.begin(), triplets.end());
  } catch (const std::exception& e) {
    std::string message;
    try {
      message.assign("load_sparse_matrix :\n");
      message += e.what();
    } catch (const std::exception&) {
      throw std::runtime_error("load_sparse_matrix\n");
    }
    throw std::runtime_error(message);
  }
}

}

#endif /* SSVM_SPARSEMATRIX_HPP_ */
