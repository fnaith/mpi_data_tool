#ifndef SSVM_SSVC_HPP_
#define SSVM_SSVC_HPP_

#include "Matrix.hpp"
#include "SparseMatrix.hpp"
#include "Vector.hpp"

namespace ssvm {

class SSVC {
 public:
  static void make_constraint(const Integer negative_number,
                              SparseMatrix *A);
  static void train(const SparseMatrix& A,
                    const Integer negative_number,
                    const Number C,
                    Vector *w0,
                    Integer *counter);
  static void predict(const Vector& w,
                      const SparseMatrix& A,
                      Vector *y);

  static void make_constraint(const Integer negative_number,
                              Matrix *A);
  static void train(const Matrix& A,
                    const Integer negative_number,
                    const Number C,
                    Vector *w0,
                    Integer *counter);
  static void train_with_memory(const Matrix& A,
                                const Integer negative_number,
                                const Number C,
                                Vector *w0,
                                Integer *counter,
                                Matrix *support_vector);
  static void predict(const Vector& w,
                      const Matrix& A,
                      Vector *y);
 private:
  static Number SSVC::objf_(const Matrix& A,
                            const Vector& w,
                            const Number C);
  static Number armijo_(const Matrix& A,
                        const Vector& w,
                        const Number C,
                        const Vector& z,
                        const Number gap,
                        const Number obj1,
                        Vector *w2);

  static const Integer kTrainMaxIterator = 150;
  static const Integer kArmijoMaxIterator = 20;

  explicit SSVC();
  ~SSVC();
  SSVC& operator=(const SSVC&);
  SSVC(const SSVC&);
};

}

#endif /* SSVM_SSVC_HPP_ */
