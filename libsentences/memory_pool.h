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

#ifndef LIBSENTENCES__MEMORY_POOL_H
#define LIBSENTENCES__MEMORY_POOL_H

#include <list>
#include <cstddef>
#include <cstdint>

namespace libsentences {

class memory_pool {
public:
    explicit memory_pool(size_t block_size = 32 * 1024);
    memory_pool(const memory_pool &) = delete;
    memory_pool(memory_pool &&);

    memory_pool &operator=(const memory_pool &) = delete;
    memory_pool &operator=(memory_pool &&);

    ~memory_pool();

    void *allocate(size_t buffer_size);
    void *allocate(size_t buffer_size, unsigned alignment);
private:
    void *slow_alloc(size_t buffer_size, unsigned alignment);

    void *block_start;
    void *block_end;
    std::list<void *> blocks;
    size_t block_size;
};

inline memory_pool::memory_pool(size_t block_size_) :
    block_start(0), block_end(0), block_size(block_size_) {
}

inline memory_pool::memory_pool(memory_pool &&b) :
    block_start(b.block_start), block_end(b.block_end),
    blocks(move(b.blocks)), block_size(std::move(b.block_size)) {
    b.block_start = 0;
    b.block_end = 0;
    b.block_size = 0;
}

inline void *memory_pool::allocate(size_t buffer_size) {
    void *local_block_start = block_start;
    void *new_block_start = static_cast<char *>(local_block_start) +
        buffer_size;
    if (new_block_start <= block_end) {
        block_start = new_block_start;
        return local_block_start;
    }
    return slow_alloc(buffer_size, 1);
}

inline void *memory_pool::allocate(size_t buffer_size, unsigned alignment) {
    void *local_block_start = block_start;
    void *aligned_start = static_cast<char *>(local_block_start) +
        ((-reinterpret_cast<uintptr_t>(local_block_start)) &
          (alignment - 1));
    void *new_block_start = static_cast<char *>(aligned_start) +
        buffer_size;
    if (new_block_start <= block_end) {
        block_start = new_block_start;
        return aligned_start;
    }
    return slow_alloc(buffer_size, alignment);
}

}

#endif
