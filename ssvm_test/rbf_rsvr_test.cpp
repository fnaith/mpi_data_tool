#include "../ssvm/SparseData.hpp"
#include "../ssvm/RBFRSVR.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 4) {
      throw runtime_error("<model name>"
                          "<A file> <y file>");
    }
    const char *model(argv[1]);
    const char *A_file(argv[2]);
    const char *y_file(argv[3]);
    RBFRSVR ssvr(model);
    const Integer dim(ssvr.reduce_A().cols());

    SparseMatrix A;
    SparseData::read(y_file, 1, &A);
    Vector y(A);
    SparseData::read(A_file, dim, &A);
    Matrix kernel_data;

    Vector result(y.size());
    ssvr.predict(A, &kernel_data, &result);

    cout << "2norm-rel-err : " << ((y - result).norm() / y.norm()) << endl << endl;

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("rbf_rsvr_test : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
