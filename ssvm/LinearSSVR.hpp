#ifndef SSVM_LINEARSSVR_HPP_
#define SSVM_LINEARSSVR_HPP_

#include "Matrix.hpp"
#include "Vector.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/split_member.hpp>

#include <string>

namespace ssvm {

class LinearSSVR {
  friend class boost::serialization::access;

  typedef std::string string;
 public:
  explicit LinearSSVR(const Number C=-1.0,
                      const Number ins=-1.0);
  explicit LinearSSVR(const Vector& y,
                      const Number C,
                      const Number ins,
                      Matrix *A);
  explicit LinearSSVR(const Vector& y,
                      const Number C,
                      const Number ins,
                      Matrix *A,
                      Matrix *support_vector);
  explicit LinearSSVR(const string& model_file);

  void predict(Matrix *data, Vector *result);

  void dump(const string& model_file) const;

  const Vector& W() const;
  const Number C() const;
  const Number ins() const;
  const Integer iter() const;
  const Number time() const;
 private:
  template<class Archive>
  void save(Archive& ar, const unsigned Integer) const {
    save_vector(ar, W_);
    ar & C_;
    ar & ins_;
    ar & iter_;
    ar & time_;
  }

  template<class Archive>
  void load(Archive& ar, const unsigned Integer) {
    load_vector(ar, W_);
    ar & C_;
    ar & ins_;
    ar & iter_;
    ar & time_;
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

  Vector W_;
  Number C_;
  Number ins_;
  Integer iter_;
  Number time_;
};

}

#endif /* SSVM_LINEARSSVR_HPP_ */
