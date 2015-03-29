#include "../ssvm/SparseData.hpp"
#include "../ssvm/LinearSSVR.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 4) {
      throw runtime_error("<model name> "
                          "<A file> <y file>");
    }
    const char *model(argv[1]);
    const char *A_file(argv[2]);
    const char *y_file(argv[3]);
    LinearSSVR ssvr(model);
    const Integer dim(ssvr.W().size() - 1);

    SparseMatrix data;
    SparseData::read(A_file, dim, &data);
    Matrix A(data);
    SparseData::read(y_file, 1, &data);
    Vector y(data);
    data = SparseMatrix();

    Vector result(y.size());
    ssvr.predict(&A, &result);

    cout << "2norm-rel-err : " << ((y - result).norm() / y.norm()) << endl << endl;

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("linear_ssvr_test : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
