#include "../ssvm/SparseData.hpp"
#include "../ssvm/LinearSSVC.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 4) {
      throw runtime_error("<model name> "
                          "<positive file> <negative file>");
    }
    const char *model(argv[1]);
    const char *pos_file(argv[2]);
    const char *neg_file(argv[3]);
    LinearSSVC ssvc(model);
    const Integer dim(ssvc.W().size() - 1);

    Vector y;
    SparseMatrix tes_sdata;
    Matrix tes_data;

    SparseData::read(pos_file, dim, &tes_sdata);
    tes_data = tes_sdata;
    tes_sdata = SparseMatrix();
    ssvc.predict(&tes_data, &y);
    const Integer all_pos(tes_data.rows());
    Integer pos_err(0);
    for (Integer i(0); i < all_pos; ++i) {
      if (y(i) < 0.0) {
        ++pos_err;
      }
    }

    SparseData::read(neg_file, dim, &tes_sdata);
    tes_data = tes_sdata;
    tes_sdata = SparseMatrix();
    ssvc.predict(&tes_data, &y);
    const Integer all_neg(tes_data.rows());
    Integer neg_err(0);
    for (Integer i(0); i < all_neg; ++i) {
      if (y(i) > 0.0) {
        ++neg_err;
      }
    }

    cout << "all_pos : " << all_pos << endl;
    cout << "pos_err : " << pos_err << endl;
    cout << "all_neg : " << all_neg << endl;
    cout << "neg_err : " << neg_err << endl;
    cout << "acc : " << 1 - (pos_err + neg_err) / (Number)(all_pos + all_neg) << endl << endl;

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("linear_ssvc_test : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
