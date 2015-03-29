#include "../ssvm/SparseData.hpp"
#include "../ssvm/LinearSSVC.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace mdt_core;
using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 6) {
      throw runtime_error("<positive file> <negative file> "
                          "<feature dimension> "
                          "<weight> "
                          "<model name>");
    }
    const char *pos_file(argv[1]);
    const char *neg_file(argv[2]);
    const Integer dim(integer_from_str(argv[3]));
    const Number C(number_from_str(argv[4]));
    const char *model(argv[5]);

    SparseMatrix pos_data;
    SparseData::read(pos_file, dim, &pos_data);
    SparseMatrix neg_data;
    SparseData::read(neg_file, dim, &neg_data);

    const Integer pos_size(pos_data.rows()), neg_size(neg_data.rows());
    Matrix tra_data(pos_size + neg_size, dim);
    tra_data.topRows(pos_size) = pos_data;
    pos_data = SparseMatrix();
    tra_data.bottomRows(neg_size) = neg_data;
    neg_data = SparseMatrix();

    LinearSSVC ssvc(neg_size, C, &tra_data);
    ssvc.dump(model);

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("linear_ssvc_train : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
