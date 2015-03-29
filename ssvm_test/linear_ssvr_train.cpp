#include "../ssvm/SparseData.hpp"
#include "../ssvm/LinearSSVR.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace mdt_core;
using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 7) {
      throw runtime_error("<A file> <y file> "
                          "<feature dimension> "
                          "<weight> <epsilon> "
                          "<model name>");
    }
    const char *A_file(argv[1]);
    const char *y_file(argv[2]);
    const Integer dim(integer_from_str(argv[3]));
    const Number C(number_from_str(argv[4]));
    const Number ins(number_from_str(argv[5]));
    const char *model(argv[6]);

    SparseMatrix data;
    SparseData::read(A_file, dim, &data);
    Matrix A(data);
    SparseData::read(y_file, 1, &data);
    Vector y(data);
    data = SparseMatrix();

    LinearSSVR ssvr(y, C, ins, &A);
    ssvr.dump(model);

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("linear_ssvr_train : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
