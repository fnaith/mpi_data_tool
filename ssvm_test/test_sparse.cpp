#include "../ssvm/SparseData.hpp"
#include "../ssvm/SparseMatrix.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace ssvm;
using namespace std;

int main(int argc, char *argv[]) {
  try {
    if (argc != 1) {
      throw runtime_error("dont input");
    }

    SparseMatrix data;
    SparseData::read("iris.pos", 4, &data);
    cout << data << endl << endl;
    SparseData::read("iris.neg", 4, &data);
    cout << data << endl << endl;

    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << string("test_sparse : \n") + e.what() << endl;
    return EXIT_FAILURE;
  }
}
