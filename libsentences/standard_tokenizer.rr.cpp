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

#include <libsentences/standard_tokenizer.h>

namespace libsentences {

class utf8_remapper
{
public:
	utf8_remapper(utf8_iterator i_, utf8_iterator end_) : i(i_), end(end_) {
	}
	int operator*() const {
		if (i.ptr() >= end.ptr()) {
			return 0;
        }
		unicode_char c = *i;
		int val = static_cast<char32_t>(c);
		if (val <= 127) return val;
		return c.category_idx() + 128;
	}
	operator utf8_iterator() const {
		return i;
    }
	utf8_remapper &operator++() {
		++i;
		return *this;
	}
	utf8_remapper &operator--() {
		--i;
		return *this;
	}
	utf8_remapper operator+(int x) const {
		if (x != 1) {
			throw;
        }
		utf8_remapper rez(i, end);
		++rez;
		return rez;
	}
private:
	utf8_iterator i, end;
};

bool standard_token_separator::operator()(utf8_iterator &next,
        const utf8_iterator &end, utf8_slice &token) {
    utf8_remapper p(next, end);
    utf8_remapper m = p, l = p, ctx = p;
	static const bool emit_space = false;
again:
// old uMidNum = [.,-/];
// wordExt = ((upper '-' (lower | uNumeric) (w | uNumeric)*) |
//		   (uNumeric '-' (w | uNumeric)+) |
//		   (lower '-' (upper | uNumeric) (w | uNumeric)*) |
//		   (upper '-' (upper | uNumeric) (w | uNumeric)*)
//		   );
// num_ext = number '-' (w | uNumeric)+;
#define YYFILL(n)
/*!re2c

re2c:define:YYCTYPE = int;
re2c:define:YYCURSOR = p;
re2c:define:YYMARKER = m;
re2c:yyfill:enable = 0;
re2c:define:YYCTXMARKER = ctx;
re2c:yych:conversion = 1;
re2c:indent:top = 0;

Lu = [\u0080];
Ll = [\u0081];
Lt = [\u0082];
Lm = [\u0083];
Lo = [\u0084];
Mn = [\u0085];
Mc = [\u0086];
Me = [\u0087];
Nd = [\u0088];
Nl = [\u0089];
No = [\u008A];
Pc = [\u008B];
Pd = [\u008C];
Ps = [\u008D];
Pe = [\u008E];
Pi = [\u008F];
Pf = [\u0090];
Po = [\u0091];
Sm = [\u0092];
Sc = [\u0093];
Sk = [\u0094];
So = [\u0095];
Zs = [\u0096];
Zl = [\u0097];
Zp = [\u0098];
Cc = [\u0099];
Cf = [\u009A];
Cs = [\u009B];
Co = [\u009C];
Cn = [\u009D];

w = [a-z]|[A-Z]|Lu|Ll|Lt|Lm|Lo;

lower = [a-z]|Ll;
upper = [A-Z]|Lu;

uNumeric = [0-9];
uMidNum = [.,];
ascii_az = [a-z];
horizSpace = [\u0020\u0009];
horizSpaceMissing = [\u00A0];
vertSpace = [\u000A\u000D];
punct = [.?!]+;

wordChunk = w ('\'' w)*;
number = uNumeric (uNumeric | uMidNum uNumeric | wordChunk)*;
wordExt = ((upper '-' (lower | uNumeric) (wordChunk | uNumeric)*) |
		   (uNumeric '-' wordChunk (wordChunk | uNumeric)*) |
		   (lower '-' (upper | uNumeric) (wordChunk | uNumeric)*) |
		   (upper '-' (upper | uNumeric) (wordChunk | uNumeric)*)
		   );
word = (wordChunk | uNumeric)+ wordExt? | wordExt;
num_ext = number [eE] [+-]? (w | uNumeric)+;

abbrev = w ("." w)+;

tlds =
"al"|"ad"|"am"|"at"|"az"|"by"|"be"|"ba"|"bg"|"hr"|"cy"|"cz"|"dk"|"ee"|"fo"|
"fi"|"fr"|"ge"|"de"|"gi"|"gr"|"hu"|"is"|"ie"|"im"|"it"|"je"|"lv"|"li"|"lt"|
"lu"|"mt"|"md"|"nl"|"no"|"pl"|"pt"|"ro"|"ru"|"es"|"se"|"ch"|"ua"|"uk"|"yu"|
"jp"|"com"|"org"|"net"|"edu"|"gov"|"mil"|"biz"|"info"|"eu";
uriStart = ascii_az+ "://";
uriDomain = [a-z0-9A-Z_-];
uriPath = [a-z0-9A-Z?=%!@#&_] | '-';

uri = uriDomain+ ('.' uriDomain+)* ('.' tlds) (':'[0-9]+)?
	  ('/' (uriPath+ ('.' uriPath+)*)?)*;

uriFull = uriStart? uri;

(punct | number | word | num_ext) { goto emit; }
uriFull/([\u0000]|Nd|Nl|No|Pc|Pd|Ps|Pe|Pi|Pf|Po|Zl|Zp|Zs|horizSpace|vertSpace|[.,();"]|'['|']'|'\'') { goto emit; }
(abbrev ) ([\u0020-\u00FF]\(w | uNumeric)) { m = ctx = --p; goto emit; }

(horizSpace | vertSpace | Zs | ([\u0001-\u001F]) | (Cc\[\u0000]) | Cf)+ { if (emit_space) goto emit; else { l = p; goto again; } }
[\u0001-\u00FF] { goto emit; }
[\u0000] { return false; }
*/
/*
*/
emit:
    token = utf8_slice(utf8_iterator(l), utf8_iterator(p));
    next = utf8_iterator(p);
    return true;
}

void standard_token_separator::reset() {
}
}
