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

#include <libsentences/text_sentences.h>
#include <libsentences/quotes_detector.h>
#include <libsentences/sbd_model.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

static void read_file(const char *path, string &result) {
    ifstream ifs(path);
    if (!ifs.good()) {
        cerr << "Can't open " << path << '\n';
        exit(1);
    }
    streambuf *buf = ifs.rdbuf();
    size_t len = buf->pubseekoff(0, ios_base::end);
    buf->pubseekoff(0, ios_base::beg);

    result.resize(len);
    buf->sgetn(&result[0], len);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " model input.txt\n";
        return 1;
    }
    // --quotes_spec=»«:200,«»:20,():20,"":20,'':20

    string contents;
    read_file(argv[2], contents);

    auto model = libsentences::sbd_model::load(argv[1]);
    libsentences::quotes_detector qd;
    //qd.add_specs("»«:200,«»:20,():20,\"\":20,'':20");
    int cnt = 0;
    libsentences::text_sentences ts(
                contents,
                model, 
                libsentences::sbd_eol_handling::split_on_multiple_eols,
                &qd);
    for (libsentences::text_sentences::iterator i = ts.begin(),
         e = ts.end(); i != e; ++i) {
        cout << *i << '\n';
        ++cnt;
    }
    cerr << "Sentence count: " << cnt << '\n';
}
