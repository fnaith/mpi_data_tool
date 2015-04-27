#include "../mdt_core/allocator.hpp"
#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace mdt_core;
using namespace std;

namespace filesystem = boost::filesystem;

struct Element {
  string v;
  Integer r;
  Integer c;
};

typedef deque<Element, mdt_core::allocator<Element>> Elements;

int main(int argc, char *argv[]) {
  try {
    clock_t start(clock());
    if (argc != 5) {
      throw runtime_error("<file root path> <feature name> "
                          "<output folder name> <split number>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string feature_name(argv[2]);
    const filesystem::path output_folder(root_path / argv[3]);
    filesystem::create_directories(output_folder);
    const string split_number_str(argv[4]);
    const Integer split_number(in_range(integer_from_str(split_number_str.c_str()),
                                        1, numeric_limits<Integer>::max()));
    const filesystem::path split_path(output_folder / split_number_str);
    const string feature_file((root_path / feature_name).string());
    const string feature_count_file(feature_file + ".feature_count");
    const string tmp_file_postfix(".triplet");

    for (Integer i(0); i < split_number; ++i) {
      ostringstream id;
      id << i;
      filesystem::path sub_split_path(split_path / id.str());
      filesystem::create_directories(sub_split_path);
      string output_file((sub_split_path / feature_name).string() + tmp_file_postfix);

      ofstream output;
      file_open(output_file.c_str(), ofstream::out | ofstream::binary, &output);
      output.close();
    }

    ifstream input;
    file_open(feature_count_file.c_str(), ifstream::in | ifstream::binary, &input);

    vector<Integer> feature_count;
    boost::archive::text_iarchive(input) >> feature_count;
    input.close();

    Integer dimension(static_cast<Integer>(feature_count.size()));
    int64_t sum(0);
    for (Integer i(0); i < dimension; ++i) {
      sum += feature_count[i];
    }
    int64_t threshold(sum / split_number);
    if (sum % split_number != 0) {
      ++threshold;
    }
    sum = 0;
    Integer fold(0);
    for (Integer i(0); i < dimension; ++i) {
      sum += feature_count[i];
      feature_count[i] = fold;
      if (sum / threshold - 1 >= fold) {
        if (fold < split_number - 1) {
          ++fold;
        }
      }
    }

    file_open(feature_file.c_str(), ifstream::in | ifstream::binary, &input);

    vector<Elements> elementses(split_number);
    string line;
    Element tmp_element;
    tmp_element.r = 0;
    while (getline(input, line)) {
      ++(tmp_element.r);
      const char *head(line.data());
      char *tail;
      while (*head != '\0') {
        Integer index(static_cast<Integer>(strtol(head, &tail, 10)));
        head = tail + 1;
        strtod(head, &tail);
        tmp_element.c = index;
        try {
          tmp_element.v.assign(head, tail - head);
          elementses[feature_count[tmp_element.c - 1]].push_back(tmp_element);
        } catch (const exception&) {
          for (Integer i(0); i < split_number; ++i) {
            ostringstream id;
            id << i;
            filesystem::path sub_split_path(split_path / id.str());
            string output_file((sub_split_path / feature_name).string() + tmp_file_postfix);

            ofstream output;
            file_open(output_file.c_str(), ofstream::out | ofstream::binary | ofstream::app, &output);

            Elements& element(elementses[i]);
            for (Elements::const_iterator it(element.begin()); it != element.end(); ++it) {
              output << it->r << '\t' << it->c << '\t' << it->v << "\t\n";
            }
            elementses[i] = Elements();
            output.close();
          }
          tmp_element.v.assign(head, tail - head);
          elementses[feature_count[tmp_element.c - 1]].push_back(tmp_element);
        }
        head = tail + 1;
      }
    }

    input.close();

    Integer begin(0), end;
    for (Integer i(0); i < split_number; ++i) {
      ostringstream id;
      id << i;
      filesystem::path sub_split_path(split_path / id.str());
      string feature_range_file((sub_split_path / feature_name).string() + ".feature_range");
      string output_file((sub_split_path / feature_name).string() + tmp_file_postfix);

      ofstream output;
      file_open(feature_range_file.c_str(), ofstream::out | ofstream::binary, &output);

      for (Integer j(begin); j < dimension; ++j) {
        if (feature_count[j] != i) {
          end = j;
          break;
        }
      }
      if (i == split_number - 1) {
        end = dimension;
      }
      ++begin;++end;
      boost::archive::text_oarchive(output) << begin << end;
      --begin;--end;
      begin = end;
      output.close();

      file_open(output_file.c_str(), ofstream::out | ofstream::binary | ofstream::app, &output);
      Elements& element(elementses[i]);
      for (Elements::const_iterator it(element.begin()); it != element.end(); ++it) {
        output << it->r << '\t' << it->c << '\t' << it->v << "\t\n";
      }
      elementses[i] = Elements();
      output.close();
    }

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "split_row_sparse_to_element : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
