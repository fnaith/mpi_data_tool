#include "../mdt_core/count_line.hpp"
#include "../mdt_core/in_range.hpp"
#include "../mdt_core/file_open.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/filesystem.hpp>

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

struct Pair {
  string v;
  Integer r;
};

int main(int argc, char *argv[]) {
  try {
    clock_t start(clock());

    if (argc != 5) {
      throw runtime_error("<file root path> <feature name> "
                          "<output folder name> <split number>\n");
    }
    const filesystem::path root_path(argv[1]);
    const string feature_name(argv[2]);
    const string feature_file((root_path / feature_name).string());
    const filesystem::path output_folder(root_path / argv[3]);
    filesystem::create_directories(output_folder);
    const string split_number_str(argv[4]);
    const Integer split_number(in_range(integer_from_str(split_number_str.c_str()),
                                        1, numeric_limits<Integer>::max()));
    const filesystem::path split_path(output_folder / split_number_str);
    const string tmp_file_postfix(".triplet");

    string line;
    Pair tmp_pair;
    for (Integer i(0); i < split_number; ++i) {
      ostringstream id;
      id << i;
      filesystem::path sub_split_path(split_path / id.str());
      filesystem::create_directories(sub_split_path);
      string output_file((sub_split_path / feature_name).string());
      string feature_range_file((sub_split_path / feature_name).string() + ".feature_range");
      string input_file(output_file + tmp_file_postfix);

      ifstream input;

      file_open(feature_range_file.c_str(), ifstream::in | ifstream::binary, &input);
      Integer begin, end;
      boost::archive::text_iarchive(input) >> begin >> end;
      input.close();

      Integer output_error(0);
      vector<deque<Pair>> pairs(end - begin);

      file_open(input_file.c_str(), ifstream::in | ifstream::binary, &input);

      while (getline(input, line)) {
        const char *head(line.data());
        char *tail;
        while (*head != '\0') {
          Integer r(strtol(head, &tail, 10));
          head = tail + 1;
          Integer c(strtol(head, &tail, 10));
          head = tail + 1;
          strtod(head, &tail);
          tmp_pair.r = r;
          tmp_pair.v = string(head, tail - head);
          try {
            pairs[c - begin].push_back(tmp_pair);
          } catch (const exception&) {
            ostringstream error_id;
            error_id << output_error;

            ofstream output;
            file_open((output_file + error_id.str()).c_str(), ofstream::out | ofstream::binary, &output);

            for (size_t j(0); j < pairs.size(); ++j) {
              deque<Pair>& pair(pairs[j]);
              for (deque<Pair>::const_iterator it(pair.begin()); it != pair.end(); ++it) {
                output << it->r << ':' << it->v << ' ';
              }
              pairs[j] = deque<Pair>();
              output.put('\n');
            }
            output.close();
            ++output_error;
            pairs[c - begin].push_back(tmp_pair);
          }
          head = tail + 1;
        }
      }
      if (output_error != 0) {
        ostringstream error_id;
        error_id << output_error;

        ofstream output;
        file_open((output_file + error_id.str()).c_str(), ofstream::out | ofstream::binary, &output);

        for (size_t j(0); j < pairs.size(); ++j) {
          deque<Pair>& pair(pairs[j]);
          for (deque<Pair>::const_iterator it(pair.begin()); it != pair.end(); ++it) {
            output << it->r << ':' << it->v << ' ';
          }
          pairs[j] = deque<Pair>();
          output.put('\n');
        }
        output.close();

        ++output_error;
        vector<ifstream *> inputs(output_error);

        file_open(output_file.c_str(), ofstream::out | ofstream::binary, &output);

        for (Integer j(0); j < output_error; ++j) {
          ostringstream error_id;
          error_id << j;
          inputs[j] = new ifstream;
          file_open((output_file + error_id.str()).c_str(), ifstream::in | ifstream::binary, inputs[j]);
        }
        for (size_t j(0); j < pairs.size(); ++j) {
          for (Integer k(0); k < output_error; ++k) {
            getline(*(inputs[k]), line);
            output << line;
          }
          output.put('\n');
        }
        output.close();

        for (Integer j(0); j < output_error; ++j) {
          inputs[j]->close();
          delete inputs[j];
          ostringstream error_id;
          error_id << j;
          filesystem::remove(output_file + error_id.str());
        }
      } else {
        ofstream output;
        file_open(output_file.c_str(), ofstream::out | ofstream::binary, &output);

        for (size_t j(0); j < pairs.size(); ++j) {
          deque<Pair>& pair(pairs[j]);
          for (deque<Pair>::const_iterator it(pair.begin()); it != pair.end(); ++it) {
            output << it->r << ':' << it->v << ' ';
          }
          pairs[j] = deque<Pair>();
          output.put('\n');
        }
        output.close();
      }
    }

    cout << (double)(clock() - start) / CLOCKS_PER_SEC << endl;
    return EXIT_SUCCESS;
  } catch (const exception& e) {
    cout << "merge_element_to_colume_sparse : \n" << e.what() << endl;
    return EXIT_FAILURE;
  }
}
