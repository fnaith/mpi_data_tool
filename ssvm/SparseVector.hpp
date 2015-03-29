#ifndef SSVM_SPARSEVECTOR_HPP_
#define SSVM_SPARSEVECTOR_HPP_

#include "NumberType.hpp"

#include <Eigen/Sparse>

namespace ssvm {

typedef Eigen::SparseVector<Number, Eigen::ColMajor, Integer> SparseVector;

}

#endif /* SSVM_SPARSEVECTOR_HPP_ */
