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

#ifndef TMT__STRINGS__UNICODE_CHAR_H
#define TMT__STRINGS__UNICODE_CHAR_H

#include <unictype.h>
#include <string>
#include <ostream>
#include <functional>

namespace libsentences {

class unicode_char {
public:
    unicode_char();
    explicit unicode_char(char c);
    unicode_char(char32_t ucs);
    operator char32_t() const;
    operator std::string() const;
    bool operator==(const unicode_char &) const;
    bool operator!=(const unicode_char &) const;

    uc_general_category_t category() const;
    int category_idx() const;

    unicode_char lower() const;
    unicode_char upper() const;
    unicode_char title() const;

    bool is_lower() const;
    bool is_upper() const;
    bool is_letter() const;
    bool is_digit() const;
    bool is_whitespace() const;
private:
    char32_t ucs;
};

inline unicode_char::unicode_char() {
}

inline unicode_char::unicode_char(char c) : ucs(c) {
}

inline unicode_char::unicode_char(char32_t ucs_) : ucs(ucs_) {
}

inline unicode_char::operator char32_t() const {
    return ucs;
}

inline uc_general_category_t unicode_char::category() const {
    return uc_general_category(ucs);
}

inline int unicode_char::category_idx() const {
    unsigned mask = uc_general_category(ucs).bitmask;
    static const int shuffle[32] = {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };
    return shuffle[(mask * 0x077CB531U) >> 27];
}

inline bool unicode_char::is_letter() const {
    return (category().bitmask & UC_CATEGORY_MASK_L) != 0;
}

inline bool unicode_char::is_digit() const {
    return (category().bitmask & UC_CATEGORY_MASK_N) != 0;
}

inline bool unicode_char::is_whitespace() const {
    return (category().bitmask & UC_CATEGORY_MASK_Z) != 0;
}

inline bool unicode_char::is_lower() const {
    return (category().bitmask & UC_CATEGORY_MASK_Ll) != 0;
}

inline bool unicode_char::is_upper() const {
    return (category().bitmask & UC_CATEGORY_MASK_Lu) != 0;
}

inline bool unicode_char::operator==(const unicode_char &b) const {
    return ucs == b.ucs;
}

inline bool unicode_char::operator!=(const unicode_char &b) const {
    return ucs != b.ucs;
}

}

inline std::ostream &operator<<(std::ostream &os,
        const uc_general_category_t &category) {
    return os << uc_general_category_name(category);
}

inline std::string &operator+=(std::string &s, const libsentences::unicode_char &c) {
    unsigned ucs = static_cast<char32_t>(c);

    if (ucs <= 0x7F) {
        s += static_cast<char>(ucs);
    } else {
        if (ucs <= 0x7FF) {
            s += static_cast<char>(0xC0 | (ucs >> 6));
        } else {
            if (ucs <= 0xFFFF) {
                s += static_cast<char>(0xE0 | (ucs >> 12));
            } else {
                s += static_cast<char>(0xF0 | (ucs >> 18));
                s += static_cast<char>(0x80 | ((ucs >> 12) & 0x3F));
            }
            s += static_cast<char>(0x80 | ((ucs >> 6) & 0x3F));
        }
        s += static_cast<char>(0x80 | (ucs & 0x3F));
    }
    return s;
}

namespace libsentences {
inline unicode_char::operator std::string() const {
    std::string rez;
    rez += *this;
    return std::move(rez);
}
}

#endif
