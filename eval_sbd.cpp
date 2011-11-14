/*
The code is licensed under the 2-clause, simplified BSD license.
Copyright 2011 by Frane Saric (frane.saric@gmail.com) . All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY FRANE SARIC ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL FRANE SARIC BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Frane Saric.
*/

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iomanip>

#include <libsentences/standard_tokenizer.h>

using namespace std;
using namespace libsentences;

static int cnt_lines(const utf8_iterator i,
                     const utf8_iterator j) {
    const char *p1 = i.ptr(), *p2 = j.ptr();
    return count(p1, p2, '\n');
}

static void read_file(const char *path, string &result) {
    ifstream ifs(path);
    if (!ifs.good()) {
        cerr << "Can't open " << path << '\n';
        exit(1);
    }
    streambuf *buf = ifs.rdbuf();
    size_t len = buf->pubseekoff(0, ios_base::end);
    buf->pubseekoff(0, ios_base::beg);

    result.resize(len);
    buf->sgetn(&result[0], len);
}

static bool files_token_equal(const string &path1, const string &path2) {
    string f1, f2;
    read_file(path1.c_str(), f1);
    read_file(path2.c_str(), f2);
    utf8_slice fc1 = f1, fc2 = f2;

    standard_tokenizer t1(fc1), t2(fc2);

    auto i1 = t1.begin(), i2 = t2.begin();
    auto e1 = t1.end(), e2 = t2.end();
    while (i1 != e1 && i2 != e2) {
        if (!utf8_slice::equal()(*i1, *i2)) {
            cerr << "Mismatch: " << *i1 << " " << *i2
                 << " (line "
                 << cnt_lines(fc1.begin(), (*i1).begin()) + 1 << ')'
                 << '\n';
            return false;
        }
        ++i1;
        ++i2;
    }
    if (i1 != e1 || i2 != e2) {
        cerr << "Different number of tokens in files.\n";
        return false;
    }
    return true;
}

static vector<int> get_newline_positions(const string &path) {
    vector<int> positions;
    int cur_pos = 0;
    string line;
    ifstream ifs(path);
    if (!ifs.good()) {
        throw runtime_error("Can't open file " + path);
    }
    for (string line; getline(ifs, line);) {
        int cnt_line_tokens = 0;
        auto &&tokens = standard_tokenizer(line);
        cnt_line_tokens += distance(tokens.begin(), tokens.end());
        if (cnt_line_tokens) {
            cur_pos += cnt_line_tokens;
            positions.push_back(cur_pos);
        }
    }
    return move(positions);
}

static void evaluate(const vector<int> &gold_pos,
                     const vector<int> &test_pos,
                     vector<int> *fp_list,
                     vector<int> *fn_list) {
    int tp = 0, fp = 0, fn = 0;
    if (fp_list) {
        fp_list->clear();
    }
    if (fn_list) {
        fn_list->clear();
    }

    auto gold_it = gold_pos.begin(), gold_end = gold_pos.end();
    auto test_it = test_pos.begin(), test_end = test_pos.end();

    while (gold_it != gold_end && test_it != test_end) {
        if (*gold_it < *test_it) { // missed a sentence break
            if (fn_list) {
                fn_list->push_back(*gold_it);
            }
            ++fn;
            ++gold_it;
        } else if (*test_it < *gold_it) { // false break
            if (fp_list) {
                fp_list->push_back(*test_it);
            }
            ++fp;
            ++test_it;
        } else { // match
            ++tp;
            ++gold_it;
            ++test_it;
        }
    }
    for (; gold_it < gold_end; ++gold_it) {
        if (fn_list) {
            fn_list->push_back(*gold_it);
        }
        ++fn;
    }

    for (; test_it < test_end; ++test_it) {
        if (fp_list) {
            fp_list->push_back(*test_it);
        }
    }

    double recall = static_cast<double>(tp) / (tp + fn);
    double precision = static_cast<double>(tp) / (tp + fp);
    double f1 = static_cast<double>(2 * tp) / (2 * tp + fp + fn);
    double accuracy = static_cast<double>(tp) / (tp + fp + fn);

    cout << fixed << setprecision(2);
    cout << "F1:        " << f1 * 100 << '\n';
    cout << "Recall:    " << recall * 100 << '\n';
    cout << "Precision: " << precision * 100 << '\n';
    cout << "Accuracy:  " << accuracy * 100 << '\n';
    cout << "TP:  " << tp << '\n';;
    cout << "FP:  " << fp << '\n';;
    cout << "FN:  " << fn << '\n';;
}

static void print_list(const vector<int> &list, const string &path) {
    string fc;
    read_file(path.c_str(), fc);
    utf8_slice prev, cur, next;
    int cur_list_pos = 0, list_size = list.size();
    int cur_token = 0;
    int cnt = 0;
    standard_tokenizer st(fc);
    for (standard_tokenizer::iterator i = st.begin(), e = st.end(); i != e;
         ++i) {
        prev = cur;
        cur = next;
        next = *i;

        if (cur_list_pos < list_size && list[cur_list_pos] == cur_token) {
            ++cur_list_pos;
            ++cnt;
            cout << cnt << ". " << prev << ' '
                 << cur << ' ' << next << '\n';
        }

        ++cur_token;
    }
    cout << '\n';
}

int main(int argc, char **argv) {
    try {
        string gold_file, test_file;
        bool print_fp = false, print_fn = false;
        if (argc <= 1 || !strcmp(argv[1], "-h") ||
                         !strcmp(argv[1], "--help")) {
            cerr << "Usage: " << argv[0]
                 << " [--print-fp] [--print-fn] gold.txt test.txt\n";
            return 1;
        }
        for (int i = 1; i < argc; ++i) {
            if (!strcmp(argv[i], "--print-fp")) {
                print_fp = true;
            } else if (!strcmp(argv[i], "--print-fn")) {
                print_fn = true;
            } else if (argv[i][0] == '-') {
                cerr << "Unknown option: " << argv[i] << '\n';
                return 1;
            } else if (gold_file.empty()) {
                gold_file = argv[i];
            } else {
                test_file = argv[i];
            }
        }
        if (gold_file.empty() || test_file.empty()) {
            throw runtime_error(
                    "You must specify both gold_file and test_file.");
        }
        if (!files_token_equal(gold_file, test_file)) {
            throw runtime_error("Files contain different tokens.");
        }
        vector<int> gold_nl_pos = get_newline_positions(gold_file);
        vector<int> test_nl_pos = get_newline_positions(test_file);

        vector<int> fp_list, fn_list;
        evaluate(gold_nl_pos, test_nl_pos, &fp_list, &fn_list);

        if (print_fp) {
            cout << "\nFP list:\n";
            print_list(fp_list, gold_file);
        }
        if (print_fn) {
            cout << "\nFN list:\n";
            print_list(fn_list, gold_file);
        }
    } catch (const exception &e) {
        cerr << e.what() << '\n';
    }
}
