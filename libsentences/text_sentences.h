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

#ifndef LIBTMT__SENTENCES__TEXT_SENTENCES_H
#define LIBTMT__SENTENCES__TEXT_SENTENCES_H

#include <stdexcept>

#include <libsentences/standard_tokenizer.h>
#include <libsentences/sbd_context.h>

#include <boost/iterator/iterator_facade.hpp>
#include <memory>

namespace libsentences {

class sbd_model;
class quotes_detector;
class sentence_iterator_shared_data;

enum class sbd_eol_handling {
    ignore_eol,
    split_on_eol,
    split_on_multiple_eols
};

class sentence_iterator :
    public boost::iterator_facade<
        sentence_iterator,
        utf8_slice,
        boost::forward_traversal_tag,
        utf8_slice> {
public:
    sentence_iterator();
    explicit sentence_iterator(const sentence_iterator_shared_data *sd);
    sentence_iterator(const sentence_iterator_shared_data *sd,
            const standard_tokenizer::const_iterator &start);
private:
    utf8_slice dereference() const;
    bool equal(const sentence_iterator &b) const;
    void increment();
    friend class boost::iterator_core_access;
private:
    const sentence_iterator_shared_data *sd;
    standard_tokenizer::const_iterator cur_token_it;
    sbd_context context;
    utf8_iterator sentence_start, sentence_end;
};

class text_sentences {
public:
    typedef sentence_iterator iterator;

    text_sentences(
        const utf8_slice &text,
        const sbd_model &model,
        sbd_eol_handling eh = sbd_eol_handling::split_on_multiple_eols,
        const quotes_detector *qd = 0);

    iterator begin() const;
    iterator end() const;
private:
    std::shared_ptr<sentence_iterator_shared_data> sd;
};

}

#endif
