#include "count_line.hpp"

#include "file_open.hpp"

#include <algorithm>
#include <stdexcept>

using namespace std;

typedef istreambuf_iterator<char> BufferIterator;
typedef BufferIterator::difference_type Difference;

namespace mdt_core {

Integer count_line(const string& file) {
  return count_line(file.c_str());
}

Integer count_line(const char *file) {
  try {
    ifstream input;
    file_open(file, ifstream::in | ifstream::binary, &input);
    Difference diff(count(BufferIterator(input), BufferIterator(), '\n'));
    Integer count(static_cast<Integer>(diff));
    if (static_cast<Difference>(count) != diff) {
      throw runtime_error("convert fail\n");
    }
    return count;
  } catch (const exception& e) {
    string message;
    try {
      message.assign("count_line :\n");
      message += e.what();
    } catch (const exception&) {
      throw runtime_error("count_line\n");
    }
    throw runtime_error(message);
  }
}

}
