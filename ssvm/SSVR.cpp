#include "SSVR.hpp"

#include <stdexcept>
#include <string>

using namespace std;

namespace ssvm {

void SSVR::make_constraint(Matrix *A) {
  try {
    A->conservativeResize(A->rows(), A->cols() + 1);
    A->rightCols(1).setOnes();
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::make_constraint :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::make_constraint\n");
    }
    throw runtime_error(message);
  }
}

void SSVR::train(const Matrix& A,
                 const Vector& y,
                 const Number C,
                 const Number ins,
                 Vector *w0,
                 Integer *counter) {
  try {
    Matrix support_vector;
    train_with_memory(A, y, C, ins, w0, counter, &support_vector);
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::train :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::train\n");
    }
    throw runtime_error(message);
  }
}

void SSVR::train_with_memory(const Matrix& A,
                             const Vector& y,
                             const Number C,
                             const Number ins,
                             Vector *w0,
                             Integer *counter,
                             Matrix *support_vector) {
  try {
    const Integer instance_number(A.rows()), feature_number(A.cols());
    Number flag(1.0);
    *counter = 0;
    Matrix hessian(feature_number, feature_number);
    Vector h(instance_number);
    Vector gradient(feature_number);
    Vector z(feature_number);
    while (flag > 1e-4) {
      ++(*counter);
      h = y - A * *w0;
      insen_minus_(ins, &h);
      gradient = *w0 / C - A.transpose() * h;
      if (gradient.squaredNorm() / feature_number < 1e-5) {
        break;
      }
      const Number r_squared_norm(h.squaredNorm());
      Integer offset(0);
      for (Integer i(0); i < instance_number; ++i) {
        if (h(i) != 0.0) {
          ++offset;
        }
      }
      if (offset == 0) {
        break;
      }
      support_vector->resize(offset, feature_number);
      offset = 0;
      for (Integer i(0); i < instance_number; ++i) {
        if (h(i) != 0.0) {
          support_vector->row(offset) = A.row(i);
          ++offset;
        }
      }
      hessian = support_vector->transpose() * (*support_vector);
      hessian.diagonal() += Vector::Ones(feature_number) / C;
      z = hessian.ldlt().solve(-gradient);
      const Number stepsize(armijo_(r_squared_norm, A, y, *w0, C, ins, z,
                                    &h, &gradient));
      z = stepsize * z;
      *w0 += z;
      flag = z.norm();
      if (*counter == kTrainMaxIterator) {
        break;
      }
    }
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::train_with_memory :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::train_with_memory\n");
    }
    throw runtime_error(message);
  }
}

void SSVR::predict(const Vector& w,
                   const Matrix& A,
                   Vector *y) {
  try{
    *y = A * w;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SSVR::predict :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SSVR::predict\n");
    }
    throw runtime_error(message);
  }
}

void SSVR::insen_minus_(const Number d,
                        Vector *v) {
  Integer size(v->size());
  Number *data(v->data());
  Number value;
  while (size--) {
    value = max(abs(*data) - d, 0.0);
    *data = (*data < -d) ? -value : value;
    ++data;
  }
}

void SSVR::insen_(const Number d,
                  Vector *v) {
  Integer size(v->size());
  Number *data(v->data());
  while (size--) {
    *data = max(abs(*data) - d, 0.0);
    ++data;
  }
}

Number SSVR::armijo_(const Number r_squared_norm,
                     const Matrix& A,
                     const Vector& y,
                     const Vector& w0,
                     const Number C,
                     const Number ins,
                     const Vector& descent,
                     Vector *h,
                     Vector *w1) {
  const Number obj1(w0.squaredNorm() / C + r_squared_norm);
  Number stepsize(1.0);
  while (true) {
    *w1 = w0 + stepsize * descent;
    *h = y - A * *w1;
    insen_(ins, h);
    if (w1->squaredNorm() / C + h->squaredNorm() < obj1) {
      break;
    }
    stepsize *= 0.5;
    if (stepsize < 1e-5) {
      break;
    }
  }
  return stepsize;
}

}
