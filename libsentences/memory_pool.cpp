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

#include "libsentences/memory_pool.h"

#include <cstdlib>
#include <list>
#include <stdexcept>

namespace libsentences {

memory_pool::~memory_pool() {
    for (std::list<void *>::iterator i = blocks.begin(), j = blocks.end();
         i != j; ++i) {
        free(*i);
    }
}

memory_pool &memory_pool::operator=(memory_pool &&b) {
    for (std::list<void *>::iterator i = blocks.begin(), j = blocks.end();
         i != j; ++i) {
        free(*i);
    }

    block_start = b.block_start;
    block_end = b.block_end;
    block_size = b.block_size;
    b.block_start = 0;
    b.block_end = 0;
    b.block_size = 0;
    blocks = move(b.blocks);

    return *this;
}

void *memory_pool::slow_alloc(size_t buffer_size, unsigned alignment) {
    if (!block_size) {
        throw std::runtime_error("Using uninitiallized memory pool.");
    }
    if (buffer_size > block_size) {
        void *block = malloc(buffer_size + alignment - 1);
        blocks.push_back(block);
        void *aligned_start = static_cast<char *>(block) +
            ((-reinterpret_cast<uintptr_t>(block)) & (alignment - 1));
        return aligned_start;
    }
    size_t effective_block_size = block_size + alignment - 1;
    void *block = malloc(effective_block_size);
    blocks.push_back(block);
    void *aligned_start = static_cast<char *>(block) +
        ((-reinterpret_cast<uintptr_t>(block)) & (alignment - 1));
    block_start = static_cast<char *>(aligned_start) + buffer_size;
    block_end = static_cast<char *>(block) + effective_block_size;
    return aligned_start;
}

}
