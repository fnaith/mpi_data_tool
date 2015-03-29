#ifndef SSVM_RBFRSVC_HPP_
#define SSVM_RBFRSVC_HPP_

#include "Matrix.hpp"
#include "SparseMatrix.hpp"
#include "Vector.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include <string>

namespace ssvm {

class RBFRSVC {
  friend class boost::serialization::access;

  typedef std::string string;
 public:
  explicit RBFRSVC(const Number R=-1.0,
                   const Number G=-1.0,
                   const Number C=-1.0);
  explicit RBFRSVC(const SparseMatrix& positive_data,
                   const SparseMatrix& negative_data,
                   const Number R,
                   const Number G,
                   const Number C);
  explicit RBFRSVC(const string& model_file);

  void predict(const SparseMatrix& testing_data,
               Matrix *kernel_data,
               Vector *y);

  void dump(const string& model_file) const;

  const SparseMatrix& reduce_positive() const;
  const SparseMatrix& reduce_negative() const;
  const Vector& W() const;
  const Number R() const;
  const Number G() const;
  const Number C() const;
  const Integer iter() const;
  const Number time() const;
 protected:
  template<class Archive>
  void save(Archive& ar, const unsigned Integer) const {
    save_sparse_matrix(ar, reduce_positive_);
    save_sparse_matrix(ar, reduce_negative_);
    save_vector(ar, W_);
    ar & R_;
    ar & G_;
    ar & C_;
    ar & iter_;
    ar & time_;
  }

  template<class Archive>
  void load(Archive& ar, const unsigned Integer) {
    load_sparse_matrix(ar, reduce_positive_);
    load_sparse_matrix(ar, reduce_negative_);
    load_vector(ar, W_);
    ar & R_;
    ar & G_;
    ar & C_;
    ar & iter_;
    ar & time_;
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

  void gaussian_kernel_(const SparseMatrix& positive_data,
                        const SparseMatrix& negative_data,
                        Matrix *kernel_data);
  void gaussian_kernel_save_(const SparseMatrix& positive_data,
                             const SparseMatrix& negative_data,
                             Matrix *kernel_data);

  SparseMatrix reduce_positive_;
  SparseMatrix reduce_negative_;
  Vector W_;
  Number R_;
  Number G_;
  Number C_;
  Integer iter_;
  Number time_;
};

}

#endif /* SSVM_RBFRSVC_HPP_ */
