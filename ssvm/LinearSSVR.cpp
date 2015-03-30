#include "LinearSSVR.hpp"

#include "../mdt_core/file_open.hpp"
#include "SSVR.hpp"

#include <ctime>
#include <fstream>
#include <stdexcept>

using namespace mdt_core;
using namespace std;

namespace ssvm {

LinearSSVR::LinearSSVR(const Number C,
                       const Number ins)
  : W_(),
    C_(C),
    ins_(ins),
    iter_(-1),
    time_(-1.0) {
}

LinearSSVR::LinearSSVR(const Vector& y,
                       const Number C,
                       const Number ins,
                       Matrix *A)
  : W_(),
    C_(C),
    ins_(ins),
    iter_(-1),
    time_(-1.0) {
  try {
    clock_t start(clock());
    if (C_ <= 0.0) {
      throw runtime_error("weight must > 0");
    }
    if (ins_ <= 0.0) {
      throw runtime_error("epsilon must > 0");
    }
    if (A->rows() < 1) {
      throw runtime_error("instance number must > 0");
    }
    SSVR::make_constraint(A);
    W_.resize(A->cols());
    W_.setConstant(0.0);
    SSVR::train(*A, y, C_, ins_, &W_, &iter_);
    time_ = (Number)(clock() - start) / CLOCKS_PER_SEC;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVR"
                     "::training :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVR"
                          "::training\n");
    }
    throw runtime_error(message);
  }
}

LinearSSVR::LinearSSVR(const Vector& y,
                       const Number C,
                       const Number ins,
                       Matrix *A,
                       Matrix *support_vector)
  : W_(),
    C_(C),
    ins_(ins),
    iter_(-1),
    time_(-1.0) {
  try {
    clock_t start(clock());
    if (C_ <= 0.0) {
      throw runtime_error("weight must > 0");
    }
    if (ins_ <= 0.0) {
      throw runtime_error("epsilon must > 0");
    }
    if (A->rows() < 1) {
      throw runtime_error("instance number must > 0");
    }
    A->rightCols(1).setOnes();
    W_.resize(A->cols());
    W_.setConstant(0.0);
    SSVR::train_with_memory(*A, y, C_, ins_, &W_, &iter_, support_vector);
    time_ = (Number)(clock() - start) / CLOCKS_PER_SEC;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVR"
                     "::training_save :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVR"
                          "::training_save\n");
    }
    throw runtime_error(message);
  }
}

LinearSSVR::LinearSSVR(const string& model_file)
  : W_(),
    C_(-1.0),
    ins_(-1.0),
    iter_(-1),
    time_(-1.0) {
  try {
    ifstream ifs;
    file_open(model_file, ifstream::in | ifstream::binary, &ifs);
    boost::archive::text_iarchive ia(ifs);
    ia >> *this;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVR"
                     "::loading :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVR"
                          "::loading\n");
    }
    throw runtime_error(message);
  }
}

void LinearSSVR::predict(Matrix *A,
                         Vector *y) {
  try {
    if (A->rows() < 1) {
      throw runtime_error("instance number must > 0");
    }
    SSVR::make_constraint(A);
    y->resize(A->rows());
    SSVR::predict(W_, *A, y);
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVR::predict :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVR::predict\n");
    }
    throw runtime_error(message);
  }
}

void LinearSSVR::dump(const string& model_file) const {
  try {
    ofstream ofs;
    file_open(model_file, ofstream::out | ofstream::binary, &ofs);
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVR::dump :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVR::dump\n");
    }
    throw runtime_error(message);
  }
}

const Vector& LinearSSVR::W() const {
  return W_;
}

Number LinearSSVR::C() const {
  return C_;
}

Number LinearSSVR::ins() const {
  return ins_;
}

Integer LinearSSVR::iter() const {
  return iter_;
}

Number LinearSSVR::time() const {
  return time_;
}

}
