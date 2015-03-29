#include "../ssvm/SparseData.hpp"
#include "../ssvm/LinearSSVC.hpp"

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
    LinearSSVC ssvc(model);
    const Integer dim(ssvc.W().size() - 1);

    Vector y;
    SparseMatrix tes_sdata;
    Matrix tes_data;

    SparseData::read(tes_file, dim, &tes_sdata);
    tes_data = tes_sdata;
    tes_sdata = SparseMatrix();
    ssvc.predict(&tes_data, &y);
    cout << y.transpose() << endl << endl;

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("linear_ssvc_predict : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
