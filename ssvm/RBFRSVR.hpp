#ifndef SSVM_RBFRSVR_HPP_
#define SSVM_RBFRSVR_HPP_

#include "Matrix.hpp"
#include "SparseMatrix.hpp"
#include "Vector.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include <string>

namespace ssvm {

class RBFRSVR {
  friend class boost::serialization::access;

  typedef std::string string;
 public:
  explicit RBFRSVR(const Number R=-1.0,
                   const Number G=-1.0,
                   const Number C=-1.0,
                   const Number ins=-1.0);
  explicit RBFRSVR(const SparseMatrix& A,
                   const Vector& y,
                   const Number R,
                   const Number G,
                   const Number C,
                   const Number ins);
  explicit RBFRSVR(const string& model_file);

  void predict(const SparseMatrix& testing_data,
               Matrix *kernel_data,
               Vector *y);

  void dump(const string& model_file) const;

  const SparseMatrix& reduce_A() const;
  const Vector& W() const;
  const Number R() const;
  const Number G() const;
  const Number C() const;
  const Number ins() const;
  const Integer iter() const;
  const Number time() const;
 private:
  template<class Archive>
  void save(Archive& ar, const unsigned Integer) const {
    save_sparse_matrix(ar, reduce_A_);
    save_vector(ar, W_);
    ar & R_;
    ar & G_;
    ar & C_;
    ar & ins_;
    ar & iter_;
    ar & time_;
  }

  template<class Archive>
  void load(Archive& ar, const unsigned Integer) {
    load_sparse_matrix(ar, reduce_A_);
    load_vector(ar, W_);
    ar & R_;
    ar & G_;
    ar & C_;
    ar & ins_;
    ar & iter_;
    ar & time_;
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

  void gaussian_kernel_(const SparseMatrix& A,
                        Matrix *kernel_data);

  SparseMatrix reduce_A_;
  Vector W_;
  Number R_;
  Number G_;
  Number C_;
  Number ins_;
  Integer iter_;
  Number time_;
};

}

#endif /* SSVM_RBFRSVR_HPP_ */
