#include "SparseData.hpp"

#include "../mdt_core/count_line.hpp"
#include "../mdt_core/file_open.hpp"
#include "../mdt_core/SparseFormatter.hpp"
#include "SparseMatrix.hpp"

#include <fstream>
#include <limits>
#include <stdexcept>

using namespace mdt_core;
using namespace std;

namespace ssvm {

void SparseData::read(const string& file, const Integer dimension,
                      SparseMatrix *matrix) {
  read(file, dimension, 0, count_line(file), matrix);
}

void SparseData::read(const string& file, const Integer dimension,
                      const Integer begin, const Integer end,
                      SparseMatrix *matrix) {
  try {
    if (begin < 0) {
      throw runtime_error("begin must >= 0");
    }
    if (end < 0) {
      throw runtime_error("end must >= 0");
    }
    if (begin > end) {
      throw runtime_error("begin must <= end");
    }
    ifstream input;
    file_open(file, ifstream::in | ifstream::binary, &input);
    for (Integer i(0); i < begin; ++i) {
      input.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    const Integer range(end - begin);
    matrix->resize(range, dimension);
    string line;
    Triplets triplets;
    for (Integer row(0); row < range; ++row) {
      if (!getline(input, line)) {
        throw runtime_error("no more line");
      }
      SparseFormatter formatter(line);
      Integer col;
      Number value;
      while (formatter.next(&col, &value)) {
        triplets.push_back(Triplet(row, col - 1, value));
      }
    }
    matrix->setFromTriplets(triplets.begin(), triplets.end());
  } catch (const exception& e) {
    string message;
    try {
      message.assign("SparseData::read :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("SparseData::read\n");
    }
    throw runtime_error(message);
  }
}

}
