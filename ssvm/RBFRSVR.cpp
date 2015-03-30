#include "RBFRSVR.hpp"

#include "../mdt_core/file_open.hpp"
#include "SSVR.hpp"

#include <ctime>
#include <fstream>
#include <stdexcept>

using namespace mdt_core;
using namespace std;

namespace ssvm {

RBFRSVR::RBFRSVR(const Number R,
                 const Number G,
                 const Number C,
                 const Number ins)
  : reduce_A_(),
    W_(),
    R_(R),
    G_(G),
    C_(C),
    ins_(ins),
    iter_(-1),
    time_(-1.0) {
}

RBFRSVR::RBFRSVR(const SparseMatrix& A,
                 const Vector& y,
                 const Number R,
                 const Number G,
                 const Number C,
                 const Number ins)
  : reduce_A_(),
    W_(),
    R_(R),
    G_(G),
    C_(C),
    ins_(ins),
    iter_(-1),
    time_(-1.0) {
  try {
    clock_t start(clock());
    if (R_ <= 0.0 || R > 1.0) {
      throw runtime_error("reduce must in [0, 1)");
    }
    if (G_ <= 0.0) {
      throw runtime_error("gamma must > 0");
    }
    if (C_ <= 0.0) {
      throw runtime_error("weight must > 0");
    }
    if (ins_ <= 0.0) {
      throw runtime_error("epsilon must > 0");
    }
    const Integer instance_number(A.rows());
    if (instance_number < 1) {
      throw runtime_error("instance number must >= 1");
    }
    const Integer sample_number(static_cast<Integer>(max(instance_number * R_, 1.0)));
    reduce_A_ = A.topRows(sample_number);
    Matrix kernel_data;
    gaussian_kernel_(A, &kernel_data);
    kernel_data.rightCols(1).setOnes();
    W_.resize(kernel_data.cols());
    W_.setConstant(0.0);
    SSVR::train(kernel_data, y, C_, ins_, &W_, &iter_);
    time_ = (Number)(clock() - start) / CLOCKS_PER_SEC;
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVR"
                               "::training : \n") + e.what());
  }
}

RBFRSVR::RBFRSVR(const string& model_file)
  : reduce_A_(),
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
    throw runtime_error(string("RBFRSVR"
                               "::loading : \n") + e.what());
  }
}

void RBFRSVR::predict(const SparseMatrix& testing_data,
                      Matrix *kernel_data,
                      Vector *y) {
  try {
    if (testing_data.rows() < 1) {
      throw runtime_error("instance number must > 0");
    }
    gaussian_kernel_(testing_data, kernel_data);
    kernel_data->rightCols(1).setOnes();
    y->resize(kernel_data->rows());
    SSVR::predict(W_, *kernel_data, y);
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVR::predict : \n") + e.what());
  }
}

void RBFRSVR::dump(const string& model_file) const {
  try {
    ofstream ofs;
    file_open(model_file, ofstream::out | ofstream::binary, &ofs);
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVR::dump : \n") + e.what());
  }
}

void RBFRSVR::gaussian_kernel_(const SparseMatrix& A,
                               Matrix *kernel_data) {
  try {
    const Integer instance_number(A.rows());
    const Integer reduce_number(reduce_A_.rows());
   try{
    kernel_data->resize(instance_number, reduce_number + 1);
   } catch (const exception& e) {
    throw runtime_error(string("0 : \n") + e.what());
   }
    SparseMatrix stmp;
    Vector vtmp;
   try{
    stmp = reduce_A_.cwiseProduct(reduce_A_);
    vtmp = stmp * Vector::Ones(stmp.cols());
    stmp = SparseMatrix();
    kernel_data->leftCols(reduce_number).rowwise() = vtmp.transpose();
   } catch (const exception& e) {
    throw runtime_error(string("1 : \n") + e.what());
   }
   try{
    stmp = A.cwiseProduct(A);
    vtmp = stmp * Vector::Ones(stmp.cols());
    stmp = SparseMatrix();
    kernel_data->leftCols(reduce_number).colwise() += vtmp;
   } catch (const exception& e) {
    throw runtime_error(string("2 : \n") + e.what());
   }
   try{
    stmp = 2 * A * reduce_A_.transpose();
   } catch (const exception& e) {
    throw runtime_error(string("3 : \n") + e.what());
   }
   try{
    kernel_data->leftCols(reduce_number) -= stmp;
   } catch (const exception& e) {
    throw runtime_error(string("4 : \n") + e.what());
   }
    kernel_data->array() = (kernel_data->array() * -G_).exp();
  } catch (const exception& e) {
    throw runtime_error(string("RBFRSVR::gaussian_kernel_ : \n") + e.what());
  }
}

const SparseMatrix& RBFRSVR::reduce_A() const {
  return reduce_A_;
}

const Vector& RBFRSVR::W() const {
  return W_;
}

Number RBFRSVR::R() const {
  return R_;
}

Number RBFRSVR::G() const {
  return G_;
}

Number RBFRSVR::C() const {
  return C_;
}

Number RBFRSVR::ins() const {
  return ins_;
}

Integer RBFRSVR::iter() const {
  return iter_;
}

Number RBFRSVR::time() const {
  return time_;
}

}
