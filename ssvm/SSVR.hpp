#ifndef SSVM_SSVR_HPP_
#define SSVM_SSVR_HPP_

#include "Matrix.hpp"
#include "Vector.hpp"

namespace ssvm {

class SSVR {
 public:
  static void make_constraint(Matrix *A);
  static void train(const Matrix& A,
                    const Vector& y,
                    const Number C,
                    const Number ins,
                    Vector *w0,
                    Integer *counter);
  static void train_with_memory(const Matrix& A,
                                const Vector& y,
                                const Number C,
                                const Number ins,
                                Vector *w0,
                                Integer *counter,
                                Matrix *support_vector);
  static void predict(const Vector& w,
                      const Matrix& A,
                      Vector *y);
 private:
  static void insen_minus_(const Number d,
                           Vector *v);
  static void insen_(const Number d,
                     Vector *v);
  static Number armijo_(const Number r_squared_norm,
                        const Matrix& A,
                        const Vector& y,
                        const Vector& w0,
                        const Number C,
                        const Number ins,
                        const Vector& descent,
                        Vector *h,
                        Vector *w1);

  static const Integer kTrainMaxIterator = 150;

  explicit SSVR();
  ~SSVR();
  SSVR& operator=(const SSVR&);
  SSVR(const SSVR&);
};

}

#endif /* SSVM_SSVR_HPP_ */