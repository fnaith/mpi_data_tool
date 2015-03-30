#include "../ssvm/SparseData.hpp"
#include "../ssvm/SSVR.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace ssvm;
using namespace std;

int main(int argc, char *[]) {
  try {
    if (argc != 1) {
      throw runtime_error("dont input");
    }
    const char *A_file("iris.A");
    const char *y_file("iris.y");
    const Integer dim(4);
    const Number C(100.0);
    const Number ins(0.1);

    SparseMatrix data;
    SparseData::read(A_file, dim, &data);
    Matrix A(data);
    SparseData::read(y_file, 1, &data);
    Vector y(data);
    data = SparseMatrix();

    SSVR::make_constraint(&A);
    Vector w0(A.cols());
    w0.setConstant(0.0);
    Integer counter;
    SSVR::train(A, y, C, ins, &w0, &counter);
    cout << w0.transpose() << '/' << counter << endl << endl;

    SSVR::predict(w0, A, &y);
    cout << y.transpose() << endl << endl;

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("test_ssvr : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
