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

#include <libsentences/utf8_slice.h>
#include <unicase.h>

std::ostream &libsentences::operator<<(std::ostream &os, const libsentences::utf8_slice &slice) {
    os.write(slice.ptr(), slice.size());
    return os;
}

bool libsentences::utf8_slice::has_digits() const {
    for (libsentences::utf8_iterator i = p, e = p + len; i < e;) {
        libsentences::unicode_char c = i.dereference_and_increment();

        if (c.is_digit()) {
            return true;
        }
    }
    return false;
}

bool libsentences::utf8_slice::all_characters_match(unsigned mask) const {
    for (libsentences::utf8_iterator i = p, e = p + len; i < e;) {
        libsentences::unicode_char c = i.dereference_and_increment();

        if (!(c.category().bitmask & mask)) {
            return false;
        }
    }
    return true;
}

std::string operator+(const std::string &a, const libsentences::utf8_slice &b) {
    std::string rez(a);
    rez.append(b.ptr(), b.size());
    return std::move(rez);
}

std::string operator+(const libsentences::utf8_slice &a, const std::string &b) {
    std::string rez(a.ptr(), a.size());
    rez.append(b);
    return std::move(rez);
}

std::string operator+(std::string &&a, const libsentences::utf8_slice &b) {
    return std::move(a.append(b.ptr(), b.size()));
}

std::string operator+(const libsentences::utf8_slice &a, std::string &&b) {
    return std::move(b.insert(0, a.ptr(), a.size()));
}

std::string &operator+=(std::string &a, const libsentences::utf8_slice &b) {
    a.append(b.ptr(), b.size());
    return a;
}
