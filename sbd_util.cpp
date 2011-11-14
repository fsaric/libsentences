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

#include <iostream>
#include <stdexcept>

#include <libsentences/sbd_model.h>

using namespace std;

int main(int argc, char **argv) {
    try {
        if (argc <= 1 || !strcmp(argv[1], "-h") ||
            !strcmp(argv[1], "--help") || (argc != 3 && argc != 4)) {
            cerr << "Usage: " << argv[0]
                 << " train.txt model_output.txt [word_classes.txt]\n";
            return 1;
        }

        libsentences::sbd_model model = libsentences::sbd_model::train_model(
                    argv[1], argc == 4 ? argv[3] : "");
        model.save(argv[2]);
    } catch (const exception &e) {
        cerr << e.what() << '\n';
    }
}
