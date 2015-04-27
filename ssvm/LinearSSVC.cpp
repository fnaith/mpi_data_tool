#include "LinearSSVC.hpp"

#include "../mdt_core/file_open.hpp"
#include "SSVC.hpp"

#include <ctime>
#include <fstream>
#include <stdexcept>

using namespace mdt_core;
using namespace std;

namespace ssvm {

LinearSSVC::LinearSSVC(const Number C)
  : W_(),
    C_(C),
    iter_(-1),
    time_(-1.0) {
}

LinearSSVC::LinearSSVC(const Integer negative_number,
                       const Number C,
                       SparseMatrix *A)
  : W_(),
    C_(C),
    iter_(-1),
    time_(-1.0) {
  try {
    clock_t start(clock());
    if (negative_number < 1) {
      throw runtime_error("negative number must > 0");
    }
    if (C_ <= 0.0) {
      throw runtime_error("weight must > 0");
    }
    if (A->rows() < 2) {
      throw runtime_error("instance number must >= 2");
    }
    SSVC::make_constraint(negative_number, A);
    W_.resize(A->cols());
    W_.setConstant(0.0);
    SSVC::train(*A, negative_number, C_, &W_, &iter_);
    time_ = (Number)(clock() - start) / CLOCKS_PER_SEC;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVC"
                     "::training"
                     "::SparseMatrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVC"
                          "::training"
                          "::SparseMatrix\n");
    }
    throw runtime_error(message);
  }
}

LinearSSVC::LinearSSVC(const Integer negative_number,
                       const Number C,
                       Matrix *A)
  : W_(),
    C_(C),
    iter_(-1),
    time_(-1.0) {
  try {
    clock_t start(clock());
    if (negative_number < 1) {
      throw runtime_error("negative number must > 0");
    }
    if (C_ <= 0.0) {
      throw runtime_error("weight must > 0");
    }
    if (A->rows() < 2) {
      throw runtime_error("instance number must >= 2");
    }
    SSVC::make_constraint(negative_number, A);
    W_.resize(A->cols());
    W_.setConstant(0.0);
    SSVC::train(*A, negative_number, C_, &W_, &iter_);
    time_ = (Number)(clock() - start) / CLOCKS_PER_SEC;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVC"
                     "::training"
                     "::Matrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVC"
                          "::training"
                          "::Matrix\n");
    }
    throw runtime_error(message);
  }
}

LinearSSVC::LinearSSVC(const Integer negative_number,
                       const Number C,
                       Matrix *A,
                       Matrix *support_vector)
  : W_(),
    C_(C),
    iter_(-1),
    time_(-1.0) {
  try {
    clock_t start(clock());
    if (negative_number < 1) {
      throw runtime_error("negative number must > 0");
    }
    if (C_ <= 0.0) {
      throw runtime_error("weight must > 0");
    }
    if (A->rows() < 2) {
      throw runtime_error("instance number must >= 2");
    }
    A->rightCols(1).setConstant(-1.0);
    A->bottomRows(negative_number) *= -1.0;
    W_.resize(A->cols());
    W_.setConstant(0.0);
    SSVC::train_with_memory(*A, negative_number, C_, &W_, &iter_, support_vector);
    time_ = (Number)(clock() - start) / CLOCKS_PER_SEC;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVC"
                     "::training"
                     "::Matrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVC"
                          "::training"
                          "::Matrix\n");
    }
    throw runtime_error(message);
  }
}

LinearSSVC::LinearSSVC(const string& model_file)
  : W_(),
    C_(-1.0),
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
      message.assign("LinearSSVC"
                     "::loading :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVC::loading\n");
    }
    throw runtime_error(message);
  }
}

void LinearSSVC::predict(Matrix *A,
                         Vector *y) {
  try {
    if (A->rows() < 1) {
      throw runtime_error("instance number must > 0");
    }
    SSVC::make_constraint(0, A);
    y->resize(A->rows());
    SSVC::predict(W_, *A, y);
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVC::predict :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVC::predict\n");
    }
    throw runtime_error(message);
  }
}

void LinearSSVC::dump(const string& model_file) const {
  try {
    ofstream ofs;
    file_open(model_file, ofstream::out | ofstream::binary, &ofs);
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("LinearSSVC::dump :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("LinearSSVC::dump\n");
    }
    throw runtime_error(message);
  }
}

const Vector& LinearSSVC::W() const {
  return W_;
}

const Number LinearSSVC::C() const {
  return C_;
}

const Integer LinearSSVC::iter() const {
  return iter_;
}

const Number LinearSSVC::time() const {
  return time_;
}

}
