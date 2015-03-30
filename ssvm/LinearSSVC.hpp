#ifndef SSVM_LINEARSSVC_HPP_
#define SSVM_LINEARSSVC_HPP_

#include "Matrix.hpp"
#include "SparseMatrix.hpp"
#include "Vector.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include <string>

namespace ssvm {

class LinearSSVC {
  friend class boost::serialization::access;

  typedef std::string string;
 public:
  explicit LinearSSVC(const Number C=-1.0);
  explicit LinearSSVC(const Integer negative_number,
                      const Number C,
                      SparseMatrix *A);
  explicit LinearSSVC(const Integer negative_number,
                      const Number C,
                      Matrix *A);
  explicit LinearSSVC(const Integer negative_number,
                      const Number C,
                      Matrix *A,
                      Matrix *support_vector);
  explicit LinearSSVC(const string& model_file);

  void predict(Matrix *data, Vector *result);

  void dump(const string& model_file) const;

  const Vector& W() const;
  Number C() const;
  Integer iter() const;
  Number time() const;
 private:
  template<class Archive>
  void save(Archive& ar, const unsigned) const {
    save_vector(ar, W_);
    ar & C_;
    ar & iter_;
    ar & time_;
  }

  template<class Archive>
  void load(Archive& ar, const unsigned) {
    load_vector(ar, W_);
    ar & C_;
    ar & iter_;
    ar & time_;
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

  Vector W_;
  Number C_;
  Integer iter_;
  Number time_;
};

}

#endif /* SSVM_LINEARSSVC_HPP_ */
