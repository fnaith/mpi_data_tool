#include "../ssvm/SparseData.hpp"
#include "../ssvm/RBFRSVR.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace mdt_core;
using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 9) {
      throw runtime_error("<A file> <y file> "
                          "<feature dimension> "
                          "<reduce> <gamma> <weight> <epsilon> "
                          "<model name>");
    }
    const char *A_file(argv[1]);
    const char *y_file(argv[2]);
    const Integer dim(integer_from_str(argv[3]));
    const Number R(number_from_str(argv[4]));
    const Number G(number_from_str(argv[5]));
    const Number C(number_from_str(argv[6]));
    const Number ins(number_from_str(argv[7]));
    const char *model(argv[8]);

    SparseMatrix A;
    SparseData::read(y_file, 1, &A);
    Vector y(A);
    SparseData::read(A_file, dim, &A);

    RBFRSVR ssvr(A, y, R, G, C, ins);
    ssvr.dump(model);

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("rbf_rsvr_train : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
