#include "../ssvm/SparseData.hpp"
#include "../ssvm/RBFRSVR.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 3) {
      throw runtime_error("<model name> <test file>");
    }
    const char *model(argv[1]);
    const char *tes_file(argv[2]);
    RBFRSVR ssvr(model);
    const Integer dim(ssvr.reduce_A().cols());

    Vector y;
    SparseMatrix tes_data;
    Matrix kernel_data;

    SparseData::read(tes_file, dim, &tes_data);
    ssvr.predict(tes_data, &kernel_data, &y);
    cout << y.transpose() << endl << endl;

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("rbf_rsvr_predict : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
