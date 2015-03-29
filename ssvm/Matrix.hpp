#ifndef SSVM_MATRIX_HPP_
#define SSVM_MATRIX_HPP_

#include "NumberType.hpp"

#include <Eigen/Dense>

namespace ssvm {

typedef Eigen::Matrix<Number, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> Matrix;

}

#endif /* SSVM_MATRIX_HPP_ */
