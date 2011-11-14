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

#ifndef LIBTMT__STRINGS__UTF8_SLICE_H
#define LIBTMT__STRINGS__UTF8_SLICE_H

#include <algorithm>
#include <cstring>
#include <libsentences/utf8_iterator.h>
#include <vector>
#include <cstdint>

namespace libsentences {
class utf8_slice {
public:
    typedef utf8_iterator iterator;
    typedef utf8_iterator const_iterator;
    class hash;
    class equal;

    utf8_slice();
    utf8_slice(const std::string &str);
    utf8_slice(const char *c_str);
    utf8_slice(const char *first, const char *last);
    utf8_slice(const char *p, int64_t len);
    utf8_slice(const utf8_iterator &first, const utf8_iterator &last);
    utf8_slice(const utf8_slice &slice);

    void assign(const char *first, const char *last);

    operator std::string() const;
    std::string string() const;
    const char *ptr() const;
    int64_t size() const;
    bool empty() const;
    int64_t n_codepoints() const;

    bool has_digits() const;
    bool all_characters_match(unsigned mask) const;

    utf8_iterator begin() const;
    utf8_iterator end() const;
private:
    const char *p;
    int64_t len;
};

inline utf8_slice::utf8_slice() : p(0), len(0) {
}

inline utf8_slice::utf8_slice(const std::string &str) : p(str.c_str()),
    len(str.size()) {
}

inline utf8_slice::utf8_slice(const char *c_str) : p(c_str),
    len(strlen(c_str)) {
}

inline utf8_slice::utf8_slice(const utf8_iterator &first, const
        utf8_iterator &last) :
    p(first.ptr()), len(last.ptr() - p) {
}

inline utf8_slice::utf8_slice(const char *first, const char *last) :
    p(first), len(last - first) {
}

inline utf8_slice::utf8_slice(const char *p_, int64_t len_) :
    p(p_), len(len_) {
}

inline utf8_slice::utf8_slice(const utf8_slice &slice) : p(slice.p),
    len(slice.len) {
}

inline void utf8_slice::assign(const char *first, const char *last) {
    p = first;
    len = last - first;
}

inline utf8_slice::operator std::string() const {
    return std::string(p, len);
}

inline std::string utf8_slice::string() const {
    return std::string(p, p + len);
}

inline utf8_iterator utf8_slice::begin() const {
    return utf8_iterator(p);
}

inline utf8_iterator utf8_slice::end() const {
    return utf8_iterator(p + len);
}

inline const char *utf8_slice::ptr() const {
    return p;
}

inline int64_t utf8_slice::size() const {
    return len;
}

inline int64_t utf8_slice::n_codepoints() const {
    return std::distance(begin(), end());
}

inline bool utf8_slice::empty() const {
    return !len;
}

class utf8_slice::hash {
public:
    size_t operator()(const utf8_slice &s) const {
        int64_t l = s.size();
        uint32_t hash = l;
        const char *p = s.ptr();
        for (int64_t i = 0; i < l; ++i) {
            hash ^= static_cast<unsigned char>(p[i]);
            hash = (hash << 24) + (hash << 8) + hash * 0x93;
        }
        return hash;
    }
};

class utf8_slice::equal {
public:
    bool operator()(const utf8_slice &a, const utf8_slice &b) const {
        int64_t al = a.size(), bl = b.size();
        if (al != bl) {
            return false;
        }
        return memcmp(a.ptr(), b.ptr(), al) == 0;
    }
};

std::ostream &operator<<(std::ostream &os, const libsentences::utf8_slice &slice);

}

std::string operator+(const std::string &a, const libsentences::utf8_slice &b);
std::string operator+(std::string &&a, const libsentences::utf8_slice &b);
std::string operator+(const libsentences::utf8_slice &a, const std::string &b);
std::string operator+(const libsentences::utf8_slice &a, std::string &&b);
std::string &operator+=(std::string &a, const libsentences::utf8_slice &b);

#endif
