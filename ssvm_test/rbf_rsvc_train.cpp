#include "../ssvm/SparseData.hpp"
#include "../ssvm/RBFRSVC.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace mdt_core;
using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 8) {
      throw runtime_error("<positive file> <negative file> "
                          "<feature dimension> "
                          "<reduce> <gamma> <weight> "
                          "<model name>");
    }
    const char *pos_file(argv[1]);
    const char *neg_file(argv[2]);
    const Integer dim(integer_from_str(argv[3]));
    const Number R(number_from_str(argv[4]));
    const Number G(number_from_str(argv[5]));
    const Number C(number_from_str(argv[6]));
    const char *model(argv[7]);

    SparseMatrix pos_data;
    SparseData::read(pos_file, dim, &pos_data);
    SparseMatrix neg_data;
    SparseData::read(neg_file, dim, &neg_data);

    RBFRSVC ssvc(pos_data, neg_data, R, G, C);
    ssvc.dump(model);

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("rbf_rsvc_train : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
