#include "RBFRSVC.hpp"

#include "../mdt_core/file_open.hpp"
#include "SSVC.hpp"

#include <ctime>
#include <fstream>
#include <stdexcept>

using namespace mdt_core;
using namespace std;

namespace ssvm {

RBFRSVC::RBFRSVC(const Number R,
                 const Number G,
                 const Number C)
  : reduce_positive_(),
    reduce_negative_(),
    W_(),
    R_(R),
    G_(G),
    C_(C),
    iter_(-1),
    time_(-1.0) {
}

RBFRSVC::RBFRSVC(const SparseMatrix& positive_data,
                 const SparseMatrix& negative_data,
                 const Number R,
                 const Number G,
                 const Number C)
  : reduce_positive_(),
    reduce_negative_(),
    W_(),
    R_(R),
    G_(G),
    C_(C),
    iter_(-1),
    time_(-1.0) {
  try {
    clock_t start(clock());
    if (R_ <= 0.0 || R_ > 1.0) {
      throw runtime_error("reduce must in [0, 1)");
    }
    if (G_ <= 0.0) {
      throw runtime_error("gamma must > 0");
    }
    if (C_ <= 0.0) {
      throw runtime_error("weight must > 0");
    }
    const Integer positive_number(positive_data.rows());
    if (positive_number < 1) {
      throw runtime_error("positive_number must >= 1");
    }
    const Integer positive_sample(static_cast<Integer>(max(positive_number * R_, 1.0)));
    const Integer negative_number(negative_data.rows());
    if (negative_number < 1) {
      throw runtime_error("negative_number must >= 1");
    }
    const Integer negative_sample(static_cast<Integer>(max(negative_number * R_, 1.0)));
    reduce_positive_ = positive_data.topRows(positive_sample);
    reduce_negative_ = negative_data.topRows(negative_sample);
    Matrix kernel_data;
    gaussian_kernel_(positive_data, negative_data, &kernel_data);
    SSVC::make_constraint(negative_number, &kernel_data);
    W_.resize(kernel_data.cols());
    W_.setConstant(0.0);
    SSVC::train(kernel_data, negative_number, C_, &W_, &iter_);
    time_ = (Number)(clock() - start) / CLOCKS_PER_SEC;
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVC"
                               "::training : \n") + e.what());
  }
}

RBFRSVC::RBFRSVC(const string& model_file)
  : reduce_positive_(),
    reduce_negative_(),
    W_(),
    R_(-1.0),
    G_(-1.0),
    C_(-1.0),
    iter_(-1),
    time_(-1.0) {
  try {
    ifstream ifs;
    file_open(model_file, ifstream::in | ifstream::binary, &ifs);
    boost::archive::text_iarchive ia(ifs);
    ia >> *this;
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVC"
                               "::loading : \n") + e.what());
  }
}

void RBFRSVC::predict(const SparseMatrix& testing_data,
                      Matrix *kernel_data,
                      Vector *y) {
  try {
    if (testing_data.rows() < 1) {
      throw runtime_error("instance number must > 0");
    }
    gaussian_kernel_(testing_data, SparseMatrix(0, 0), kernel_data);
    SSVC::make_constraint(0, kernel_data);
    y->resize(kernel_data->rows());
    SSVC::predict(W_, *kernel_data, y);
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVC::predict : \n") + e.what());
  }
}

void RBFRSVC::dump(const string& model_file) const {
  try {
    ofstream ofs;
    file_open(model_file, ofstream::out | ofstream::binary, &ofs);
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVC::dump : \n") + e.what());
  }
}

void RBFRSVC::gaussian_kernel_(const SparseMatrix& positive_data,
                               const SparseMatrix& negative_data,
                               Matrix *kernel_data) {
  try {
    const Integer positive_number(positive_data.rows());
    const Integer positive_sample(reduce_positive_.rows());
    const Integer negative_number(negative_data.rows());
    const Integer negative_sample(reduce_negative_.rows());
    kernel_data->resize(positive_number + negative_number, positive_sample + negative_sample);
    SparseMatrix stmp;
    Vector vtmp;
    stmp = reduce_positive_.cwiseProduct(reduce_positive_);
    vtmp = stmp * Vector::Ones(stmp.cols());
    stmp = SparseMatrix();
    kernel_data->leftCols(positive_sample).rowwise() = vtmp.transpose();
    stmp = reduce_negative_.cwiseProduct(reduce_negative_);
    vtmp = stmp * Vector::Ones(stmp.cols());
    stmp = SparseMatrix();
    kernel_data->rightCols(negative_sample).rowwise() = vtmp.transpose();
    stmp = positive_data.cwiseProduct(positive_data);
    vtmp = stmp * Vector::Ones(stmp.cols());
    stmp = SparseMatrix();
    kernel_data->topRows(positive_number).colwise() += vtmp;
    if (negative_number != 0) {
      stmp = negative_data.cwiseProduct(negative_data);
      vtmp = stmp * Vector::Ones(stmp.cols());
      stmp = SparseMatrix();
      kernel_data->bottomRows(negative_number).colwise() += vtmp;
    }
    stmp = 2 * positive_data * reduce_positive_.transpose();
    kernel_data->topLeftCorner(positive_number, positive_sample) -= stmp;
    stmp = 2 * positive_data * reduce_negative_.transpose();
    kernel_data->topRightCorner(positive_number, negative_sample) -= stmp;
    if (negative_number != 0) {
      stmp = 2 * negative_data * reduce_positive_.transpose();
      kernel_data->bottomLeftCorner(negative_number, positive_sample) -= stmp;
      stmp = 2 * negative_data * reduce_negative_.transpose();
      kernel_data->bottomRightCorner(negative_number, negative_sample) -= stmp;
    }
    kernel_data->array() = (kernel_data->array() * -G_).exp();
  } catch (const exception& e) {
    (void)e;
    gaussian_kernel_save_(positive_data, negative_data, kernel_data);
  }
}

void RBFRSVC::gaussian_kernel_save_(const SparseMatrix& positive_data,
                                    const SparseMatrix& negative_data,
                                    Matrix *kernel_data) {
  try {
    const Integer positive_number(positive_data.rows());
    const Integer positive_sample(reduce_positive_.rows());
    const Integer negative_number(negative_data.rows());
    const Integer negative_sample(reduce_negative_.rows());
    kernel_data->resize(positive_number + negative_number, positive_sample + negative_sample);
    const Integer feature_number(positive_data.cols());
    Vector tmp(max(max(max(positive_number, positive_sample),
                       max(negative_number, negative_sample)),
                   feature_number));
    tmp.resize(feature_number);
    Number *data;
    for (Integer i(0); i < positive_sample; ++i) {
      data = tmp.data();
      for (Integer j(0); j < feature_number; ++j) {
        *data = reduce_positive_.coeff(i, j);
        ++data;
      }
      kernel_data->block(0, i, positive_number, 1) = -2.0 * positive_data * tmp;
    }
    tmp.resize(feature_number);
    for (Integer i(0); i < negative_sample; ++i) {
      data = tmp.data();
      for (Integer j(0); j < feature_number; ++j) {
        *data = reduce_negative_.coeff(i, j);
        ++data;
      }
      kernel_data->block(0, positive_sample + i, positive_number, 1) = -2.0 * positive_data * tmp;
    }
    if (negative_number != 0) {
      for (Integer i(0); i < positive_sample; ++i) {
        data = tmp.data();
        for (Integer j(0); j < feature_number; ++j) {
          *data = reduce_positive_.coeff(i, j);
          ++data;
        }
        kernel_data->block(positive_number, i, negative_number, 1) = -2.0 * negative_data * tmp;
      }
      for (Integer i(0); i < negative_sample; ++i) {
        data = tmp.data();
        for (Integer j(0); j < feature_number; ++j) {
          *data = reduce_negative_.coeff(i, j);
          ++data;
        }
        kernel_data->block(positive_number, positive_sample + i, negative_number, 1) = -2.0 * negative_data * tmp;
      }
    }
    tmp.resize(positive_sample);
    tmp = reduce_positive_.cwiseProduct(reduce_positive_) * Vector::Ones(feature_number);
    kernel_data->leftCols(positive_sample).rowwise() += tmp.transpose();
    tmp.resize(negative_sample);
    tmp = reduce_negative_.cwiseProduct(reduce_negative_) * Vector::Ones(feature_number);
    kernel_data->rightCols(negative_sample).rowwise() += tmp.transpose();
    tmp.resize(positive_number);
    tmp = positive_data.cwiseProduct(positive_data) * Vector::Ones(feature_number);
    kernel_data->topRows(positive_number).colwise() += tmp;
    if (negative_number != 0) {
      tmp.resize(negative_number);
      tmp = negative_data.cwiseProduct(negative_data) * Vector::Ones(feature_number);
      kernel_data->bottomRows(negative_number).colwise() += tmp;
    }
    *kernel_data = (kernel_data->array() * -G_).exp();
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVC::gaussian_kernel_save_ : \n") + e.what());
  }
}

const SparseMatrix& RBFRSVC::reduce_positive() const {
  return reduce_positive_;
}

const SparseMatrix& RBFRSVC::reduce_negative() const {
  return reduce_negative_;
}

const Vector& RBFRSVC::W() const {
  return W_;
}

const Number RBFRSVC::R() const {
  return R_;
}

const Number RBFRSVC::G() const {
  return G_;
}

const Number RBFRSVC::C() const {
  return C_;
}

const Integer RBFRSVC::iter() const {
  return iter_;
}

const Number RBFRSVC::time() const {
  return time_;
}

}
