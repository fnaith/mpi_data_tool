#include "SSVC.hpp"

#include <stdexcept>
#include <string>

using namespace std;

namespace ssvm {

void SSVC::make_constraint(const Integer negative_number,
                           SparseMatrix *A) {
  try {
    const Integer rows(A->rows());
    const Integer cols(A->cols());
    A->conservativeResize(rows, cols + 1);
    for (Integer i(0); i < rows; ++i) {
      A->coeffRef(i, cols) = -1.0;
    }
    A->bottomRows(negative_number) *= -1.0;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::make_constraint"
                     "::SparseMatrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::make_constraint"
                          "::SparseMatrix\n");
    }
    throw runtime_error(message);
  }
}

void SSVC::train(const SparseMatrix& A,
                 const Integer negative_number,
                 const Number C,
                 Vector *w0,
                 Integer *counter) {
  try {
    const Integer instance_number(A.rows()), feature_number(A.cols());
    Number flag(1.0);
    *counter = 0;
    SparseMatrix sparse_hessian(feature_number, feature_number);
    Matrix hessian(feature_number, feature_number);
    Vector d(instance_number);
    Vector gradz(feature_number);
    Vector z(feature_number);
    SparseMatrix support_vector;
    while (flag > 1e-4) {
      ++(*counter);
      d.resize(instance_number);
      d = Vector::Ones(instance_number) - A * *w0;
      Integer offset(0);
      for (Integer i(0); i < instance_number; ++i) {
        if (d(i) > 0) {
          ++offset;
        }
      }
      if (offset == 0) {
        break;
      }
      support_vector.resize(offset, feature_number);
      offset = 0;
      for (Integer i(0); i < instance_number; ++i) {
        if (d(i) > 0.0) {
          support_vector.row(offset) = A.row(i);
          d(offset) = d(i);
          ++offset;
        }
      }
      d.conservativeResize(offset);
      gradz = *w0 / C - support_vector.transpose() * d;
      if (gradz.squaredNorm() / feature_number < 1e-5) {
        break;
      }
      sparse_hessian = support_vector.transpose() * support_vector;
      hessian = sparse_hessian;
      hessian.diagonal() += Vector::Ones(feature_number) / C;
      z = -hessian.ldlt().solve(gradz);
      const Number obj1(objf_(A, *w0, C));
      const Number gap(z.dot(gradz));
      gradz = *w0 + z;
      if ((obj1 - objf_(A, gradz, C)) > 1e-8) {
        *w0 = gradz;
      } else {
        *w0 += armijo_(A, *w0, C, z, gap, obj1, &gradz) * z;
      }
      flag = z.norm();
      if (*counter == kTrainMaxIterator) {
        break;
      }
    }
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::train"
                     "::SparseMatrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::train"
                          "::SparseMatrix\n");
    }
    throw runtime_error(message);
  }
}

void SSVC::predict(const Vector& w,
                   const SparseMatrix& A,
                   Vector *y) {
  try{
    *y = A * w;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::predict"
                     "::SparseMatrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::predict"
                          "::SparseMatrix\n");
    }
    throw runtime_error(message);
  }
}


void SSVC::make_constraint(const Integer negative_number,
                           Matrix *A) {
  try {
    A->conservativeResize(A->rows(), A->cols() + 1);
    A->rightCols(1).setConstant(-1.0);
    A->bottomRows(negative_number) *= -1.0;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::make_constraint"
                     "::Matrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::make_constraint"
                          "::Matrix\n");
    }
    throw runtime_error(message);
  }
}

void SSVC::train(const Matrix& A,
                 const Integer negative_number,
                 const Number C,
                 Vector *w0,
                 Integer *counter) {
  try {
    Matrix support_vector;
    train_with_memory(A, negative_number, C, w0, counter, &support_vector);
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::train"
                     "::Matrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::train"
                          "::Matrix\n");
    }
    throw runtime_error(message);
  }
}

void SSVC::train_with_memory(const Matrix& A,
                             const Integer negative_number,
                             const Number C,
                             Vector *w0,
                             Integer *counter,
                             Matrix *support_vector) {
  try {
    const Integer instance_number(A.rows()), feature_number(A.cols());
    Number flag(1.0);
    *counter = 0;
    Matrix hessian(feature_number, feature_number);
    Vector d(instance_number);
    Vector gradz(feature_number);
    Vector z(feature_number);
    while (flag > 1e-4) {
      ++(*counter);
      d.resize(instance_number);
      d = Vector::Ones(instance_number) - A * *w0;
      Integer offset(0);
      for (Integer i(0); i < instance_number; ++i) {
        if (d(i) > 0) {
          ++offset;
        }
      }
      if (offset == 0) {
        break;
      }
      support_vector->resize(offset, feature_number);
      offset = 0;
      for (Integer i(0); i < instance_number; ++i) {
        if (d(i) > 0.0) {
          support_vector->row(offset) = A.row(i);
          d(offset) = d(i);
          ++offset;
        }
      }
      d.conservativeResize(offset);
      gradz = *w0 / C - support_vector->transpose() * d;
      if (gradz.squaredNorm() / feature_number < 1e-5) {
        break;
      }
      hessian = support_vector->transpose() * (*support_vector);
      hessian.diagonal() += Vector::Ones(feature_number) / C;
      z = -hessian.ldlt().solve(gradz);
      const Number obj1(objf_(A, *w0, C));
      const Number gap(z.dot(gradz));
      gradz = *w0 + z;
      if ((obj1 - objf_(A, gradz, C)) > 1e-8) {
        *w0 = gradz;
      } else {
        *w0 += armijo_(A, *w0, C, z, gap, obj1, &gradz) * z;
      }
      flag = z.norm();
      if (*counter == kTrainMaxIterator) {
        break;
      }
    }
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::train_with_memory"
                     "::Matrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::train_with_memory"
                          "::Matrix\n");
    }
    throw runtime_error(message);
  }
}

void SSVC::predict(const Vector& w,
                   const Matrix& A,
                   Vector *y) {
  try{
    *y = A * w;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::predict"
                     "::Matrix :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::predict"
                          "::Matrix\n");
    }
    throw runtime_error(message);
  }
}

Number SSVC::objf_(const Matrix& A,
                   const Vector& w,
                   const Number C) {
  return 0.5 * ((Vector::Ones(A.rows()) - A * w).cwiseMax(0).squaredNorm() +
                w.squaredNorm() / C);
}

Number SSVC::armijo_(const Matrix& A,
                     const Vector& w,
                     const Number C,
                     const Vector& z,
                     const Number gap,
                     const Number obj1,
                     Vector *w2) {
  Number diff(0.0);
  Number stepsize(0.5);
  Integer count(0);
  while (diff < -0.05 * stepsize * gap) {
    ++count;
    stepsize *= 0.5;
    *w2 = w + stepsize * z;
    diff = obj1 - objf_(A, *w2, C);
    if (count == kArmijoMaxIterator) {
      break;
    }
  }
  return stepsize;
}

}
