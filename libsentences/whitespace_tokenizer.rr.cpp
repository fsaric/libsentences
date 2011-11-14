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

#include <libsentences/whitespace_tokenizer.h>

namespace libsentences {
class remapper
{
public:
	remapper(const char *i_, const char *end_) : i(i_), end(end_) {
	}
	int operator*() const {
		if (i >= end) {
			return 0;
        }
        return static_cast<unsigned char>(*i);
	}
	operator const char *() const {
		return i;
    }
	remapper &operator++() {
		++i;
		return *this;
	}
	remapper &operator--() {
		--i;
		return *this;
	}
	remapper operator+(int x) const {
		remapper rez(i, end);
		++rez;
		return rez;
	}
private:
	const char *i, *end;
};

bool whitespace_token_separator::operator()(utf8_iterator &next,
        const utf8_iterator &end, utf8_slice &token) {
    remapper p(next.ptr(), end.ptr());
    remapper l = p;
	static const bool emitSpace = false;
again:
#define YYFILL(n)
/*!re2c

re2c:define:YYCTYPE = int;
re2c:define:YYCURSOR = p;
re2c:yyfill:enable = 0;
re2c:yych:conversion = 1;
re2c:indent:top = 0;

wchar = [^ \t\n\r\v\u0000];

wchar+ { goto emit; }
[\u0001-\u00FF] { if (emitSpace) goto emit; else { l = p; goto again; } }
[\u0000] { return false; }
*/
/*
*/
emit:
    token = utf8_slice(static_cast<const char *>(l), static_cast<const char *>(p));
    next = utf8_iterator(static_cast<const char *>(p));
    return true;
}

void whitespace_token_separator::reset() {
}
}
