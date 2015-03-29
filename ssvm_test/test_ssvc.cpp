#include "../ssvm/SparseData.hpp"
#include "../ssvm/SSVC.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 1) {
      throw runtime_error("dont input");
    }
    const char *pos_file("iris.pos");
    const char *neg_file("iris.neg");
    const Integer dim(4);
    const Number C(100.0);

    SparseMatrix pos_data;
    SparseData::read(pos_file, dim, &pos_data);
    SparseMatrix neg_data;
    SparseData::read(neg_file, dim, &neg_data);

    const Integer pos_size(pos_data.rows()), neg_size(neg_data.rows());
    Matrix tra_data(pos_size + neg_size, dim);
    tra_data.topRows(pos_size) = pos_data;
    pos_data = SparseMatrix();
    tra_data.bottomRows(neg_size) = neg_data;

    SSVC::make_constraint(neg_size, &tra_data);
    Vector w0(tra_data.cols());
    w0.setConstant(0.0);
    Integer counter;
    SSVC::train(tra_data, neg_size, C, &w0, &counter);
    cout << w0.transpose() << '/' << counter << endl << endl;

    Vector y(tra_data.rows());
    SSVC::predict(w0, tra_data, &y);
    cout << y.transpose() << endl << endl;

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("test_ssvc : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
