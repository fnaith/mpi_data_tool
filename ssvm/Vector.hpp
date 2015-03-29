#ifndef SSVM_VECTOR_HPP_
#define SSVM_VECTOR_HPP_

#include "NumberType.hpp"

#include <Eigen/Dense>

#include <stdexcept>
#include <string>

namespace ssvm {

typedef Eigen::Matrix<Number, Eigen::Dynamic, 1, Eigen::ColMajor> Vector;

template <class Archive>
void save_vector(Archive& ar,
                 const Vector& vector) {
  try {
    Integer size(vector.size());
    ar & size;
    const Number *data(vector.data());
    while (size--) {
      ar & *data;
      ++data;
    }
  } catch (const std::exception& e) {
    string message;
    try {
      message.assign("save_vector :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("save_vector\n");
    }
    throw runtime_error(message);
  }
}

template <class Archive>
void load_vector(Archive& ar,
                 Vector& vector) {
  try {
    Integer size;
    ar & size;
    vector.resize(size);
    Number *data(vector.data());
    while (size--) {
      ar & *data;
      ++data;
    }
  } catch (const std::exception& e) {
    string message;
    try {
      message.assign("load_vector :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("load_vector\n");
    }
    throw runtime_error(message);
  }
}

}

#endif /* SSVM_VECTOR_HPP_ */
