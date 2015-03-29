#include "in_range.hpp"

#include <stdexcept>
#include <sstream>

using namespace std;

namespace mdt_core {

Integer in_range(Integer v, Integer l, Integer u) {
  if (v < l) {
    try {
      ostringstream what_arg;
      what_arg << "in_range Integer :\n"
                  "value must >= lower bound :\n"
                  "value = " << v << ", "
                  "lower bound = " << l << '\n';
      throw runtime_error(what_arg.str());
    } catch (const exception&) {
      throw runtime_error("in_range Integer:\n"
                          "lower bound error\n");
    }
  }
  if (v > u) {
    try {
      ostringstream what_arg;
      what_arg << "in_range Integer :\n"
                  "value must <= upper bound :\n"
                  "value = " << v << ", "
                  "upper bound = " << u << '\n';
      throw runtime_error(what_arg.str());
    } catch (const exception&) {
      throw runtime_error("in_range Integer:\n"
                          "upper bound error\n");
    }
  }
  return v;
}

Number in_range(Number v, Number l, Number u) {
  if (v < l) {
    try {
      ostringstream what_arg;
      what_arg << "in_range Number :\n"
                  "value must >= lower bound :\n"
                  "value = " << v << ", "
                  "lower bound = " << l << '\n';
      throw runtime_error(what_arg.str());
    } catch (const exception&) {
      throw runtime_error("in_range Number :\n"
                          "lower bound error\n");
    }
  }
  if (v > u) {
    try {
      ostringstream what_arg;
      what_arg << "in_range Number :\n"
                  "value must <= upper bound :\n"
                  "value = " << v << ", "
                  "upper bound = " << u << '\n';
      throw runtime_error(what_arg.str());
    } catch (const exception&) {
      throw runtime_error("in_range Number :\n"
                          "upper bound error\n");
    }
  }
  return v;
}

}
