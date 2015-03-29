#ifndef SSVM_TRIPLET_HPP_
#define SSVM_TRIPLET_HPP_

#include "NumberType.hpp"

#include <Eigen/Sparse>

namespace ssvm {

typedef Eigen::Triplet<Number, Integer> Triplet;

}

#endif /* SSVM_TRIPLET_HPP_ */
