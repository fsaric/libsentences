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

#ifndef LIBTMT__STRINGS__UTF8_ITERATOR_H
#define LIBTMT__STRINGS__UTF8_ITERATOR_H

#include <libsentences/unicode_char.h>

#include <boost/iterator/iterator_facade.hpp>

namespace libsentences {
class utf8_iterator :
    public boost::iterator_facade<
        utf8_iterator,
        const unicode_char,
        boost::bidirectional_traversal_tag,
        const unicode_char> {
public:
    utf8_iterator();
    utf8_iterator(const char *p);
    utf8_iterator(const std::string &s);

    unicode_char dereference_and_increment();
    const char *ptr() const;

    bool operator<(const utf8_iterator &) const;
    bool operator<=(const utf8_iterator &) const;
    bool operator>(const utf8_iterator &) const;
    bool operator>=(const utf8_iterator &) const;
private:
    unicode_char dereference() const;
    bool equal(const utf8_iterator &b) const;
    void increment();
    void decrement();
    friend class boost::iterator_core_access;

    int bytes() const;

    static const int utf8_bytes[256];

    const char *p;
};

inline utf8_iterator::utf8_iterator() : p(0) {
}

inline utf8_iterator::utf8_iterator(const char *pp) : p(pp) {
}

inline utf8_iterator::utf8_iterator(const std::string &s) : p(s.c_str()) {
}

inline const char *utf8_iterator::ptr() const {
    return p;
}

inline int utf8_iterator::bytes() const {
    return utf8_bytes[static_cast<uint8_t>(*p)];
}

inline unicode_char utf8_iterator::dereference_and_increment() {
    uint8_t uc1 = *p;
    if (!(uc1 & 0x80)) {
        ++p;
        return uc1;
    }
    uint8_t uc2 = p[1] & 0x3F;
    if ((uc1 & 0xE0) == 0xC0) {
        p += 2;
        return ((uc1 & 0x1F) << 6) | uc2;
    }
    uint8_t uc3 = p[2] & 0x3F;
    if ((uc1 & 0xF0) == 0xE0) {
        p += 3;
        return ((uc1 & 0xF) << 12) | (uc2 << 6) | uc3;
    }
    uint8_t uc4 = p[3] & 0x3F;
    if ((uc1 & 0xF8) == 0xF0) {
        p += 4;
        return ((uc1 & 0x7) << 18) | (uc2 << 12) | (uc3 << 6) | uc4;
    }
    return 0;
}

inline unicode_char utf8_iterator::dereference() const {
    uint8_t uc1 = *p;
    if (!(uc1 & 0x80)) {
        return uc1;
    }
    uint8_t uc2 = p[1] & 0x3F;
    if ((uc1 & 0xE0) == 0xC0) {
        return ((uc1 & 0x1F) << 6) | uc2;
    }
    uint8_t uc3 = p[2] & 0x3F;
    if ((uc1 & 0xF0) == 0xE0) {
        return ((uc1 & 0xF) << 12) | (uc2 << 6) | uc3;
    }
    uint8_t uc4 = p[3] & 0x3F;
    if ((uc1 & 0xF8) == 0xF0) {
        return ((uc1 & 0x7) << 18) | (uc2 << 12) | (uc3 << 6) | uc4;
    }
    return 0;
}

inline bool utf8_iterator::equal(const utf8_iterator &b) const {
    return p == b.p;
}

inline void utf8_iterator::decrement() {
	do --p; while (static_cast<uint8_t>(*p) >> 6 == 2);
}

inline void utf8_iterator::increment() {
    p += bytes();
}

inline bool utf8_iterator::operator<(const utf8_iterator &b) const {
    return p < b.p;
}

inline bool utf8_iterator::operator<=(const utf8_iterator &b) const {
    return p <= b.p;
}

inline bool utf8_iterator::operator>(const utf8_iterator &b) const {
    return p > b.p;
}

inline bool utf8_iterator::operator>=(const utf8_iterator &b) const {
    return p >= b.p;
}
}

std::ostream &operator<<(std::ostream &os, libsentences::unicode_char uc);

#endif
