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
#include <stdexcept>

#include <libsentences/text_sentences.h>
#include <libsentences/sbd_model.h>
#include <libsentences/quotes_detector.h>
#include <libsentences/standard_tokenizer.h>

#include <boost/tokenizer.hpp>
#include <boost/iterator/iterator_facade.hpp>

using namespace std;

namespace libsentences {

class sentence_iterator_shared_data {
public:
    sentence_iterator_shared_data(const utf8_slice &text_,
                                  const sbd_model &model_,
                                  sbd_eol_handling eh_,
                                  const quotes_detector *qd_) :
        text(text_), model(model_), eh(eh_),
        tokenizer(text), tokens_end(tokenizer.end()), qd(qd_) {
    }

    utf8_slice text;
    const sbd_model &model;
    sbd_eol_handling eh;
    standard_tokenizer tokenizer;
    standard_tokenizer::const_iterator tokens_end;
    const quotes_detector *qd;
};

static bool has_newlines_between(const utf8_slice &a, const utf8_slice &b) {
    const char *p = a.ptr() + a.size(), *e = b.ptr();
    while (p < e) {
        if (*p++ == '\n') {
            return true;
        }
    }
    return false;
}

static bool has_multiple_newlines_between(const utf8_slice &a,
                                          const utf8_slice &b) {
    const char *p = a.ptr() + a.size(), *e = b.ptr();
    while (p < e) {
        if (*p++ == '\n') {
            while (p < e) {
                if (*p++ == '\n') {
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

sentence_iterator::sentence_iterator() : sd(0), sentence_start(0) {
}

sentence_iterator::sentence_iterator(
        const sentence_iterator_shared_data *sd_)
    : sd(sd_), sentence_start(0) {
}

static bool should_break_on_eos(sbd_eol_handling eh,
        const sbd_context &context) {
    if (eh == sbd_eol_handling::ignore_eol) {
        return false;
    }
    const utf8_slice &cur = context.get_token(0);
    const utf8_slice &next = context.get_token(1);
    if (eh == sbd_eol_handling::split_on_multiple_eols) {
        return has_multiple_newlines_between(cur, next);
    }
    // eh == sbd_eol_handling::split_on_eol
    return has_newlines_between(cur, next) &&
        (next.empty() || !next.begin()->is_lower());
}

void sentence_iterator::increment() {
    sbd_context quote_start_context;
    standard_tokenizer::const_iterator quote_start_it;
    int quote_dist;
    const quotes_detector::quote_pair_spec *active_quote = 0;

    sentence_start = 0;
    for (;;) {
        if (cur_token_it != sd->tokens_end) {
            context.add_token(*cur_token_it);
            ++cur_token_it;
        } else {
            if (context.get_token(1).empty()) {
                if (active_quote) {
                    active_quote = 0;
                    cur_token_it = quote_start_it;
                    context = quote_start_context;
                    goto not_a_quote;
                }
                break;
            }
            context.add_token(utf8_slice());
        }

        if (!sentence_start.ptr()) {
            sentence_start = context.get_token(0).begin();
        }

        if (sd->qd) {
            if (!active_quote) {
                active_quote = sd->qd->find_spec(context.get_token(0));
                if (active_quote) {
                    quote_dist = 0;
                    quote_start_it = cur_token_it;
                    quote_start_context = context;
                    continue;
                }
            } else {
                ++quote_dist;
                utf8_iterator i = context.get_token(0).begin();
                char32_t cur_char = i.dereference_and_increment();
                if (cur_char == active_quote->close &&
                           i == context.get_token(0).end()) {
                    active_quote = 0;
                } else if (quote_dist > active_quote->max_token_dist ||
                    (cur_char == active_quote->open &&
                     i == context.get_token(0).end())) {
                    active_quote = 0;
                    cur_token_it = quote_start_it;
                    context = quote_start_context;
                } else {
                    continue;
                }
            }
        }
not_a_quote:
        sentence_end = context.get_token(0).end();

        if (should_break_on_eos(sd->eh, context)) {
            context.clear_left_and_current();
            break;
        }
        if (sd->model.is_eos(context)) {
            break;
        }
    }
}

sentence_iterator::sentence_iterator(
        const sentence_iterator_shared_data *sd_,
        const standard_tokenizer::const_iterator &start)
    : sd(sd_), cur_token_it(start), sentence_start(0) {
    if (cur_token_it == sd->tokens_end) {
        return;
    }

    for (int i = 0; i < sbd_context::context_right; ++i) {
        if (cur_token_it != sd->tokens_end) {
            context.add_token(*cur_token_it);
            ++cur_token_it;
        } else {
            context.add_token(utf8_slice());
        }
    }
    increment();
}

utf8_slice sentence_iterator::dereference() const {
    return utf8_slice(sentence_start, sentence_end);
}

bool sentence_iterator::equal(const sentence_iterator &b) const {
    if (!b.sentence_start.ptr()) {
       return !sentence_start.ptr();
    }
    return sentence_start.ptr() &&
           sentence_end.ptr() == b.sentence_end.ptr();
}

text_sentences::text_sentences(
    const utf8_slice &text, const sbd_model &model,
    sbd_eol_handling eh, const quotes_detector *qd)
    : sd(new sentence_iterator_shared_data(text, model, eh, qd)) {
}

text_sentences::iterator text_sentences::begin() const {
    return sentence_iterator(sd.get(), sd->tokenizer.begin());
}

text_sentences::iterator text_sentences::end() const {
    return sentence_iterator(sd.get());
}

}
