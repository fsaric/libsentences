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

#ifndef LIBSENTENCES__SENTENCES__QUOTES_DETECTOR_H
#define LIBSENTENCES__SENTENCES__QUOTES_DETECTOR_H

#include <vector>
#include <string>
#include <libsentences/utf8_slice.h>

namespace libsentences {

class sbd_context;

class quotes_detector {
public:
    struct quote_pair_spec {
        char32_t open;
        char32_t close;
        int max_token_dist;
    };

    void add_spec(const quote_pair_spec &qps);
    void add_specs(const std::string &specs);
    const quote_pair_spec *find_spec(const utf8_slice &token) const;
private:
    std::vector<quote_pair_spec> specs;
    std::vector<char> is_open_candidate;
};

}

#endif
