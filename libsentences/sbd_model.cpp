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

#include <libsentences/sbd_model.h>
#include <libsentences/sbd_context.h>

#include <libsentences/standard_tokenizer.h>
#include <libsentences/whitespace_tokenizer.h>
#include <libsentences/memory_pool.h>

#include <algorithm>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <cstdlib>
#include <array>

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

namespace libsentences {

struct token_data {
    array<double, sbd_context::context_size> features;
    unsigned classes_mask;
    utf8_slice token;

    token_data() : classes_mask(0) {
        features.fill(0.);
    }
};

class standard_sbd_context_generator {
public:
    enum class global_feature_values : unsigned {
        prev_prev_cap = 0,
        prev_prev_digits,
        prev_prev_abbrev,
        prev_prev_year,
        prev_cap,
        prev_digits,
        prev_abbrev,
        prev_year,
        next_cap,
        next_digits,
        next_abbrev,
        next_year,
        initials,
        first_initial,
        prev_capital_dot_capital,
        n_features
    };
    static const unsigned n_global_features =
        static_cast<unsigned>(global_feature_values::n_features);

    template<class T, class G, class W>
    static void get_context(const sbd_context &context,
                            T token_feature_func,
                            G global_feature_func,
                            W word_class_feature_func) {
        token_feature_func(0);

        bool prev_cap = false, next_cap = false;
        const utf8_slice &context_prev = context.get_token(-1);
        if (!context_prev.empty()) {
            token_feature_func(-1);
            word_class_feature_func(-1);
            utf8_iterator i = context_prev.begin();
            if (i.dereference_and_increment().is_upper()) {
                global_feature_func(static_cast<int>(
                            global_feature_values::prev_cap));
                prev_cap = true;
                if (i != context_prev.end() &&
                    i.dereference_and_increment() == unicode_char('.') &&
                    i != context_prev.end() &&
                    i.dereference_and_increment().is_upper() &&
                    i == context_prev.end()) {
                    global_feature_func(static_cast<int>(
                        global_feature_values::prev_capital_dot_capital));
                }
            } else if (context_prev.all_characters_match(
                        UC_CATEGORY_MASK_N)) {
                global_feature_func(static_cast<int>(
                            global_feature_values::prev_digits));
                int len = context_prev.n_codepoints();
                if (len == 3 || len == 4) {
                    global_feature_func(static_cast<int>(
                                global_feature_values::prev_year));
                    {
                        const utf8_slice &context_prev_prev = context.get_token(-2);
                        if (!context_prev_prev.empty()) {
                            token_feature_func(-2);
                            word_class_feature_func(-2);
                            utf8_iterator i = context_prev_prev.begin();
                            if (i.dereference_and_increment().is_upper()) {
                                global_feature_func(static_cast<int>(
                                            global_feature_values::prev_prev_cap));
                            } else if (context_prev_prev.all_characters_match(
                                        UC_CATEGORY_MASK_N)) {
                                global_feature_func(static_cast<int>(
                                        global_feature_values::prev_prev_digits));
                                int len = context_prev_prev.n_codepoints();
                                if (len == 3 || len == 4) {
                                    global_feature_func(static_cast<int>(
                                            global_feature_values::prev_prev_year));
                                }
                            }
                        }
                    }
                }
            }
        }
        const utf8_slice &context_next = context.get_token(1);
        if (!context_next.empty()) {
            token_feature_func(1);
            word_class_feature_func(1);
            if (context_next.begin()->is_upper()) {
                global_feature_func(static_cast<int>(
                            global_feature_values::next_cap));
                next_cap = true;
            } else if (context_next.all_characters_match(
                        UC_CATEGORY_MASK_N)) {
                global_feature_func(static_cast<int>(
                            global_feature_values::next_digits));
                int len = context_next.n_codepoints();
                if (len == 3 || len == 4) {
                    global_feature_func(static_cast<int>(
                                global_feature_values::next_year));
                }
            }
        }
        const utf8_slice &context_cur = context.get_token(0);
        if (prev_cap && next_cap && context_cur.size() == 1 &&
            context_cur.ptr()[0] == '.' &&
            context_prev.n_codepoints() == 1) {
            if (context_next.n_codepoints() == 1) {
                global_feature_func(static_cast<int>(
                            global_feature_values::initials));
            } else {
                global_feature_func(static_cast<int>(
                            global_feature_values::first_initial));
            }
        }
    }
};

template<class ContextGenerator>
class sbd_model_params {
public:
    typedef ContextGenerator context_generator;
    static const int n_global_features =
        ContextGenerator::n_global_features;

    sbd_model_params() : bias(0.) {
        global_features.fill(0.);
    }

    bool is_eos(const sbd_context &context) const;
    bool is_eos_candidate(const utf8_slice &token) const;
    int get_or_add_feature(const utf8_slice &feature);
    void set_eos_chars(const vector<char32_t> &eos_chars);
    const vector<char32_t> &get_eos_chars() const;

    double bias;
    unordered_map<utf8_slice, int, utf8_slice::hash, utf8_slice::equal>
        token_to_idx;
    memory_pool token_pool;
    typedef array<double, ContextGenerator::n_global_features>
        global_features_type;
    global_features_type global_features;
    typedef vector<array<double, sbd_context::context_size>>
        word_class_features_type;
    word_class_features_type word_class_features;
    typedef vector<token_data> token_features_type;
    token_features_type token_features;
private:
    vector<char32_t> eos_chars;
    vector<char> eos_char_filter;
};

template<class ContextGenerator>
void sbd_model_params<ContextGenerator>::set_eos_chars(
        const vector<char32_t> &eos_chars_) {
    eos_chars = eos_chars_;
    eos_char_filter.resize(256);
    fill(eos_char_filter.begin(), eos_char_filter.end(), 0);
    for (unsigned i = 0; i < eos_chars.size(); ++i) {
        eos_char_filter[eos_chars[i] & 0xFF] = 1;
    }
}

template<class ContextGenerator>
const vector<char32_t> &sbd_model_params<ContextGenerator>::
    get_eos_chars() const {
    return eos_chars;
}

template<class ContextGenerator>
bool sbd_model_params<ContextGenerator>::is_eos_candidate(
        const utf8_slice &token) const {
    char32_t c = *--token.end();
    if (!eos_char_filter[c & 0xFF]) {
        return false;
    }
    for (unsigned i = 0; i < eos_chars.size(); ++i) {
        if (c == eos_chars[i]) {
            return true;
        }
    }
    return false;
}

template<class ContextGenerator>
int sbd_model_params<ContextGenerator>::get_or_add_feature(
        const utf8_slice &feature) {
    auto f_it = token_to_idx.find(feature);
    if (f_it != token_to_idx.end()) {
        return f_it->second;
    }
    char *str = static_cast<char *>(token_pool.allocate(feature.size()));
    memcpy(str, feature.ptr(), feature.size());
    int feature_idx = token_to_idx.size();
    utf8_slice new_token = utf8_slice(str, feature.size());
    token_to_idx.insert(make_pair(new_token, feature_idx));
    token_features.emplace_back();
    token_features.back().token = new_token;
    return feature_idx;
}

template<class TokenFeaturesType>
class lookup_context_token_features {
public:
    lookup_context_token_features(
            double &w_, const int *token_idx_,
            const TokenFeaturesType &token_features_) :
        w(w_), token_idx(token_idx_), token_features(token_features_) {
    }
    void operator()(int idx) const {
        idx += sbd_context::context_left;
        int i = token_idx[idx];
        if (i != -1) {
            w += token_features[i].features[idx];
        }
    }
private:
    double &w;
    const int *token_idx;
    const TokenFeaturesType &token_features;
};

template<class GlobalFeaturesType>
class lookup_context_global_features {
public:
    lookup_context_global_features(
            double &w_,
            const GlobalFeaturesType &global_features_) :
        w(w_), global_features(global_features_) {
    }
    void operator()(int idx) const {
        w += global_features[idx];
    }
private:
    double &w;
    const GlobalFeaturesType &global_features;
};

template<class TokenFeaturesType, class WordClassFeaturesType>
class lookup_context_word_class_features {
public:
    lookup_context_word_class_features(
            double &w_,
            const int *token_idx_,
            const TokenFeaturesType &token_features_,
            const WordClassFeaturesType &word_class_features_) :
        w(w_), token_idx(token_idx_), token_features(token_features_),
        word_class_features(word_class_features_) {
    }
    void operator()(int idx) const {
        idx += sbd_context::context_left;
        int i = token_idx[idx];
        if (i != -1) {
            unsigned mask = token_features[i].classes_mask;
            for (int bit = 0; mask; ++bit) {
                if (mask & (1 << bit)) {
                    mask &= ~(1 << bit);
                    w += word_class_features[bit][idx];
                }
            }
        }
    }
private:
    double &w;
    const int *token_idx;
    const TokenFeaturesType &token_features;
    const WordClassFeaturesType &word_class_features;
};

template<class ContextGenerator>
bool sbd_model_params<ContextGenerator>::is_eos(
        const sbd_context &context) const {
    double w = 0;
    int token_idx[sbd_context::context_size];
    for (int k = 0; k < sbd_context::context_size; ++k) {
        auto i = token_to_idx.find(
                context.get_token(k - sbd_context::context_left));
        token_idx[k] = i != token_to_idx.end() ? i->second : -1;
    }
    context_generator::get_context(context,
            lookup_context_token_features<token_features_type>
                (w, token_idx, token_features),
            lookup_context_global_features<global_features_type>
                (w, global_features),
            lookup_context_word_class_features<token_features_type,
                                               word_class_features_type>
                (w, token_idx, token_features, word_class_features)
            );
    w += bias;
    return w > 0;
}

static vector<char32_t> collect_eos_chars(const vector<string> &lines,
                                          int min_freq = 0) {
    typedef unordered_map<char32_t, int> eos_char_cnt_map;
    eos_char_cnt_map eos_char_cnt;
    vector<char32_t> rez;

    for (unsigned line_idx = 0; line_idx < lines.size(); ++line_idx) {
        const string &line = lines[line_idx];
        if (!line.empty()) {
            ++eos_char_cnt[*--utf8_slice(line).end()];
        }
    }
    vector<pair<int, char32_t>> freq_eos_list;
    for (eos_char_cnt_map::iterator it = eos_char_cnt.begin(),
         e = eos_char_cnt.end(); it != e; ++it) {
        if (it->second >= min_freq) {
            freq_eos_list.emplace_back(it->second, it->first);
        }
    }
    sort(freq_eos_list.begin(), freq_eos_list.end());
    for (int i = freq_eos_list.size() - 1; i >= 0; --i) {
        rez.push_back(freq_eos_list[i].second);
    }

    return move(rez);
}

typedef sbd_model_params<standard_sbd_context_generator> model_params;
class sbd_model::private_data {
public: 
    model_params params;
};

sbd_model::sbd_model() : data(new private_data) {
}

bool sbd_model::is_eos(const sbd_context &context) const {
    const model_params &params = data->params;
    return params.is_eos_candidate(context.get_token(0)) &&
           params.is_eos(context);
}

static void load_word_classes(model_params &params,
        const std::string &path) {
    if (path.empty()) {
        return;
    }
    unordered_set<string> class_names;
    ifstream ifs(path);
    if (!ifs.good()) {
        throw runtime_error ("Can't open word classes file.");
    }
    for (string line; getline(ifs, line);) {
        vector<string> columns;
        {
            istringstream iss(line);
            for (string column; iss >> column;) {
                columns.push_back(column);
            }
        }
        if (columns.size() < 2) {
            continue;
        }
        if (columns[0].size() < 2 ||
            columns[0][columns[0].size() - 1] != ':') {
            throw runtime_error(
                    "Expected \"word_class: word1 word2 ... wordN");
        }
        const string &class_name = columns[0];
        if (class_names.count(class_name)) {
            throw runtime_error("Duplicate class name.");
        }
        int class_idx = class_names.size();
        class_names.insert(class_name);

        for (unsigned i = 1; i < columns.size(); ++i) {
            int feature_idx = params.get_or_add_feature(columns[i]);
            params.token_features[feature_idx].classes_mask |=
                1 << class_idx;
        }
    }
    params.word_class_features.resize(class_names.size());
    for (unsigned i = 0; i < params.word_class_features.size(); ++i) {
        params.word_class_features[i].fill(0.);
    }
}

typedef vector<pair<unsigned, double>> sample_type;

class word_features_to_sample {
public:
    word_features_to_sample(
            const sbd_context &context_,
            sample_type &current_sample_,
            model_params &params_) :
        context(context_),
        current_sample(current_sample_),
        params(params_) {
    }
    void operator()(int idx) const {
        int offset = 1 + params.global_features.size() +
                     params.word_class_features.size() *
                     sbd_context::context_size;
        int w_idx = params.get_or_add_feature(context.get_token(idx));
        current_sample.emplace_back(
                offset + w_idx * sbd_context::context_size +
                (idx + sbd_context::context_left), 1.);
    }
private:
    const sbd_context &context;
    sample_type &current_sample;
    model_params &params;
};

class global_features_to_sample {
public:
    global_features_to_sample(
            sample_type &current_sample_) :
        current_sample(current_sample_) {
    }
    void operator()(int idx) const {
        int offset = 1;
        current_sample.emplace_back(offset + idx, 1.);
    }
private:
    sample_type &current_sample;
};

class word_class_features_to_sample {
public:
    word_class_features_to_sample(
            const sbd_context &context_,
            sample_type &current_sample_,
            const model_params &params_) :
        context(context_), current_sample(current_sample_),
        params(params_) {
    }
    void operator()(int idx) const {
        if (params.word_class_features.empty()) {
            return;
        }
        auto i = params.token_to_idx.find(context.get_token(idx));
        if (i != params.token_to_idx.end()) {
            int offset = 1 + params.global_features.size();
            unsigned mask =
                params.token_features[i->second].classes_mask;
            for (int bit = 0; mask; ++bit) {
                if (mask & (1 << bit)) {
                    mask &= ~(1 << bit);
                    int feature_idx = offset +
                        bit * sbd_context::context_size +
                        (idx + sbd_context::context_left);
                    current_sample.emplace_back(feature_idx, 1.);
                }
            }
        }
    }
private:
    const sbd_context &context;
    sample_type &current_sample;
    const model_params &params;
};

static void generate_features(
        model_params &params,
        const string &train_path,
        vector<sample_type> *samples,
        vector<double> *labels) {
    vector<string> lines;
    {
        ifstream ifs(train_path);
        if (!ifs.good()) {
            throw runtime_error ("Can't train file.");
        }
        for (string line; getline(ifs, line); ) {
            lines.push_back(line);
        }

        params.set_eos_chars(collect_eos_chars(lines, 2));
    }
    sbd_context context;
    sample_type current_sample;
    word_features_to_sample wfs(context, current_sample, params);
    global_features_to_sample gfs(current_sample);
    word_class_features_to_sample wcfs(context, current_sample, params);

    context = sbd_context();
    bool was_eos = false;
    for (unsigned line_idx = 0; line_idx < lines.size(); ++line_idx) {
        const string &line = lines[line_idx];
        auto tokens = standard_tokenizer(line);
        for (auto tcur = tokens.begin(), tend = tokens.end();
             tcur != tend; ++tcur) {
            context.add_token(*tcur);
            const utf8_slice &center_word = context.get_token(0);
            if (!center_word.empty() &&
                params.is_eos_candidate(center_word)) {
                double label = was_eos ? 1. : -1.;

                current_sample.clear();
                current_sample.emplace_back(0, 1.); // bias
                model_params::context_generator::get_context(
                        context, wfs, gfs, wcfs);
                sort(current_sample.begin(), current_sample.end());
                labels->push_back(label);
                samples->push_back(move(current_sample));
            }
            was_eos = false;
        }
        was_eos = true;
    }
    // ignore last sample
}

class sample_frequency_threshold {
public:
    sample_frequency_threshold(
            const vector<int> &count_, int min_frequency_) :
        count(count_), min_frequency(min_frequency_) {
    }
    bool operator()(const pair<unsigned, double> &e) const {
        return count[e.first] < min_frequency;
    }
private:
    const vector<int> &count;
    int min_frequency;
};

static void remove_low_frequency_features(vector<sample_type> &samples,
                                          int min_frequency) {
    vector<int> count;
    for (unsigned sample_idx = 0; sample_idx < samples.size();
         ++sample_idx) {
        const sample_type &sample = samples[sample_idx];
        for (unsigned i = 0; i < sample.size(); ++i) {
            if (sample[i].first >= count.size()) {
                count.resize(sample[i].first + 1);
            }
            ++count[sample[i].first];
        }
    }

    for (unsigned sample_idx = 0; sample_idx < samples.size();
         ++sample_idx) {
        sample_type &sample = samples[sample_idx];
        sample.erase(remove_if(sample.begin(), sample.end(),
                               sample_frequency_threshold(
                                   count, min_frequency)),
                     sample.end());
    }
}

static sample_type my_train(const vector<sample_type> &samples,
                            const vector<double> &y) {
    int m = samples.size();
    int n = 0;
    for (int i = 0; i < m; ++i) {
        int cur_dim = samples[i].back().first;
        if (cur_dim > n) {
            n = cur_dim;
        }
    }
    ++n;
    double lambda = 1e-4;

    vector<float> w(n);
    vector<int> perm(m);
    for (int i = 0; i < m; ++i) {
        perm[i] = i;
    }
    double eta0 = 1;
    double t = 0;
    double w_scale = 1;
    for (int iter = 0; iter < 100; ++iter) {
        random_shuffle(perm.begin(), perm.end());

        for (int i = 0; i < m; ++i, ++t) {
            double eta = eta0 / (1 + lambda * eta0 * t);

            int cur_idx = perm[i];
            const sample_type &sample = samples[cur_idx];
            double cur_y = y[cur_idx];
            w_scale *= 1 - eta * lambda * 0.01;
            double dot_p = 0;
            for (unsigned i = 0; i < sample.size(); ++i) {
                dot_p += w[sample[i].first];// * sample[i].second;
            }
            dot_p = dot_p * w_scale;

            /*// <logistic regression>
            double tmp = cur_y / (1 + exp(dot_p * cur_y));
            // </logistic regression> */

            // <hinge loss>
            // max(1 - w * x * y, 0)
            if (dot_p * cur_y >= 1) {
                continue;
            }
            double tmp = cur_y;
            // </hinge loss> */

            tmp *= eta / w_scale;
            for (unsigned i = 0; i < sample.size(); ++i) {
                w[sample[i].first] += tmp;// * sample[i].second;
            }
        }
        if (w_scale < 0.1) {
            for (int i = 0; i < n; ++i) {
                w[i] *= w_scale;
            }
            w_scale = 1;
        }
    }
    for (int i = 0; i < n; ++i) {
        w[i] *= w_scale;
    }
    sample_type w_sparse;
    for (int i = 0; i < n; ++i) {
        if (w[i]) {
            w_sparse.emplace_back(i, w[i]);
        }
    }
    return move(w_sparse);
}

static void train_params(model_params &params, const string &train_path) {
    vector<double> labels;
    vector<sample_type> samples;

    generate_features(params, train_path, &samples, &labels);
    remove_low_frequency_features(samples, 5);
    sample_type weights = my_train(samples, labels);

    unsigned bias_end = 1;
    unsigned global_features_end = bias_end + params.global_features.size();
    unsigned word_class_features_end = global_features_end +
        params.word_class_features.size() * sbd_context::context_size;

    params.token_to_idx.clear();
    vector<token_data> old_token_features = move(params.token_features);
    params.token_features.clear();
    for (unsigned i = 0; i < weights.size(); ++i) {
        unsigned idx = weights[i].first;
        double v = weights[i].second;
        if (!v) {
            continue;
        }
        if (idx < bias_end) {
            params.bias = v;
        } else if (idx < global_features_end) {
            params.global_features[idx - bias_end] = v;
        } else if (idx < word_class_features_end) {
            int tmp = idx - global_features_end;
            params.word_class_features[tmp / sbd_context::context_size]
                [tmp % sbd_context::context_size] = v;
        } else {
            int tmp = idx - word_class_features_end;
            int new_idx = params.get_or_add_feature(
                old_token_features[tmp / sbd_context::context_size].token);
            params.token_features[new_idx].features
                [tmp % sbd_context::context_size] = v;
        }
    }
    for (unsigned i = 0; i < old_token_features.size(); ++i) {
        const token_data &t = old_token_features[i];
        if (t.classes_mask) {
            int idx = params.get_or_add_feature(t.token);
            params.token_features[idx].classes_mask = t.classes_mask;
        }
    }
}

sbd_model sbd_model::train_model(const string &sbd_text_path,
                                 const string &word_classes_path) {
    sbd_model rez;
    model_params &params = rez.data->params;
    load_word_classes(params, word_classes_path);
    train_params(params, sbd_text_path);

    return move(rez);
}

void sbd_model::save(const std::string &path) const {
    ofstream ofs(path);
    const model_params &params = data->params;

    {
        auto eos_chars = params.get_eos_chars();
        for (unsigned i = 0; i < eos_chars.size(); ++i) {
            ofs << libsentences::unicode_char(eos_chars[i]).operator string()
                << (i + 1 == eos_chars.size() ? '\n' : ' ');
        }
    }
    ofs << params.bias << '\n';
    for (unsigned i = 0; i < params.global_features.size(); ++i) {
        ofs << params.global_features[i]
            << (i + 1 == params.global_features.size() ? '\n' : ' ');
    }
    ofs << params.word_class_features.size() << '\n';
    for (unsigned i = 0; i < params.word_class_features.size(); ++i) {
        for (int j = 0; j < sbd_context::context_size; ++j) {
            ofs << params.word_class_features[i][j]
                << (j == 2 ? '\n' : ' ');
        }
    }
    vector<unsigned> nonzero_tokens;
    for (unsigned i = 0; i < params.token_features.size(); ++i) {
        if (accumulate(params.token_features[i].features.begin(),
                       params.token_features[i].features.end(), 0.)) {
            nonzero_tokens.push_back(i);
        }
    }
    ofs << nonzero_tokens.size() << '\n';
    for (unsigned idx = 0; idx < nonzero_tokens.size(); ++idx) {
        unsigned i = nonzero_tokens[idx];
        ofs << params.token_features[i].token;
        for (int j = 0; j < sbd_context::context_size; ++j) {
            ofs << ' ' << params.token_features[i].features[j];
        }
        ofs << '\n';
    }
    for (int bit = 0; bit < 32; ++bit) {
        vector<unsigned> token_list;
        for (unsigned i = 0; i < params.token_features.size(); ++i) {
            if (params.token_features[i].classes_mask & (1 << bit)) {
                token_list.push_back(i);
            }
        }
        if (token_list.empty()) {
            break;
        }
        for (unsigned i = 0; i < token_list.size(); ++i) {
            ofs << params.token_features[token_list[i]].token
                << (i + 1 == token_list.size() ? '\n' : ' ');
        }
    }
}

sbd_model sbd_model::load(const std::string &path) {
    sbd_model model;
    model_params &params = model.data->params;

    ifstream ifs(path);
    if (!ifs) {
        throw runtime_error("Error while opening model file.");
    }

    {
        string line;
        if (!getline(ifs, line)) {
            throw runtime_error("Expected line with eos chars.");
        }
        istringstream iss(line);
        string tmp;
        vector<char32_t> eos_chars;
        while (iss >> tmp) {
            eos_chars.push_back(*--utf8_slice(tmp).end());
        }
        if (eos_chars.empty()) {
            throw runtime_error("No eos chars specified.");
        }
        params.set_eos_chars(eos_chars);
    }
    if (!(ifs >> params.bias)) {
        throw runtime_error("Expected bias value.");
    }
    for (unsigned i = 0; i < params.global_features.size(); ++i) {
        if (!(ifs >> params.global_features[i])) {
            throw runtime_error("Expected global feature.");
        }
    }

    {
        unsigned n;
        if (!(ifs >> n)) {
            throw runtime_error("Missing word class features.");
        }
        params.word_class_features.resize(n);
        for (unsigned i = 0; i < params.word_class_features.size(); ++i) {
            for (int j = 0; j < sbd_context::context_size; ++j) {
                if (!(ifs >> params.word_class_features[i][j])) {
                    throw runtime_error("Expected word class feature.");
                }
            }
        }
    }
    {
        unsigned n;
        if (!(ifs >> n)) {
            throw runtime_error("Expected token features.");
        }
        string token;
        for (unsigned i = 0; i < n; ++i) {
            if (!(ifs >> token)) {
                throw runtime_error("Expected token feature.");
            }
            int idx = params.get_or_add_feature(token);
            for (int j = 0; j < sbd_context::context_size; ++j) {
                if (!(ifs >> params.token_features[idx].features[j])) {
                    throw runtime_error("Expected token feature.");
                }
            }
        }
    }
    {
        string tmp;
        getline(ifs, tmp);
    }
    for (int bit = 0; bit < 32; ++bit) {
        string line;
        string token;
        if (!getline(ifs, line)) {
            break;
        }
        istringstream iss(line);
        while (iss >> token) {
            int idx = params.get_or_add_feature(token);
            params.token_features[idx].classes_mask |= 1 << bit;
        }
    }
    return move(model);
}

}
