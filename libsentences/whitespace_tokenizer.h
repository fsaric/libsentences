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

#ifndef LIBTMT__TOKENIZERS__WHITESPACE_TOKENIZER_H
#define LIBTMT__TOKENIZERS__WHITESPACE_TOKENIZER_H

#include <boost/tokenizer.hpp>
#include <libsentences/utf8_iterator.h>
#include <libsentences/utf8_slice.h>

namespace libsentences {
class whitespace_token_separator {
public:
    bool operator()(utf8_iterator &next, const utf8_iterator &end,
            utf8_slice &token);
    void reset();
};

class whitespace_tokenizer {
    typedef boost::tokenizer<whitespace_token_separator, utf8_iterator,
            utf8_slice> tokenizer;
public:
    typedef tokenizer::iterator iterator;
    typedef tokenizer::const_iterator const_iterator;

    whitespace_tokenizer(const utf8_slice &slice);
    iterator begin() const;
    iterator end() const;
private:
    tokenizer tok;
};

inline whitespace_tokenizer::whitespace_tokenizer(const utf8_slice &slice) :
    tok(slice) {
}

inline whitespace_tokenizer::iterator whitespace_tokenizer::begin() const {
    return tok.begin();
}

inline whitespace_tokenizer::iterator whitespace_tokenizer::end() const {
    return tok.end();
}
}

#endif
