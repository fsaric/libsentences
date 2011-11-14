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

#include <libsentences/quotes_detector.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

namespace libsentences {

void quotes_detector::add_spec(const quote_pair_spec &qps) {
    if (is_open_candidate.empty()) {
        is_open_candidate.resize(256);
    }
    is_open_candidate[qps.open & 0xFF] = 1;
    specs.emplace_back(qps);
}

const quotes_detector::quote_pair_spec *quotes_detector::find_spec(
        const utf8_slice &token) const {
    if (is_open_candidate.empty()) {
        return 0;
    }
    char32_t open;
    {
        utf8_iterator i = token.begin();
        open = i.dereference_and_increment();
        if (i != token.end() || !is_open_candidate[open & 0xFF]) {
            return 0;
        }
    }
    for (unsigned i = 0; i < specs.size(); ++i) {
        if (specs[i].open == open) {
            return &specs[i];
        }
    }
    return 0;
}

void quotes_detector::add_specs(const std::string &specs_string) {
    vector<string> specs;
    boost::split(specs, specs_string, boost::is_any_of(","));

    for (unsigned spec_idx = 0; spec_idx < specs.size(); ++spec_idx) {
        const string &spec = specs[spec_idx];
        vector<string> spec_parts;
        boost::split(spec_parts, spec, boost::is_any_of(":"));
        if (spec_parts.size() != 2 || spec_parts[0].empty() ||
            spec_parts[1].empty()) {
            throw runtime_error("Invalid quotes specification.");
        }
        libsentences::utf8_slice brackets(spec_parts[0]);
        if (distance(brackets.begin(), brackets.end()) != 2) {
            throw runtime_error("Expected exactly two characters.");
        }
        libsentences::utf8_iterator bracket_it = brackets.begin();

        int max_dist = boost::lexical_cast<int>(spec_parts[1]);
        if (max_dist > 1000 || max_dist < 0) {
            throw runtime_error("Invalid maximum distance.");
        }
        char32_t left = bracket_it.dereference_and_increment();
        char32_t right = bracket_it.dereference_and_increment();
        quote_pair_spec qps;
        qps.open = left;
        qps.close = right;
        qps.max_token_dist = max_dist;
        add_spec(qps);
    }
}

}
