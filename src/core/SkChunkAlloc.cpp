/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChunkAlloc.h"

// Don't malloc any chunks smaller than this
#define MIN_CHUNKALLOC_BLOCK_SIZE   1024

// Return the new min blocksize given the current value
static size_t increase_next_size(size_t size) {
    return size + (size >> 1);
}

///////////////////////////////////////////////////////////////////////////////

struct SkChunkAlloc::Block {
    Block*  fNext;
    size_t  fFreeSize;
    char*   fFreePtr;
    // data[] follows

    char* startOfData() {
        return reinterpret_cast<char*>(this + 1);
    }

    static void FreeChain(Block* block) {
        while (block) {
            Block* next = block->fNext;
            sk_free(block);
            block = next;
        }
    };

    bool contains(const void* addr) const {
        const char* ptr = reinterpret_cast<const char*>(addr);
        return ptr >= (const char*)(this + 1) && ptr < fFreePtr;
    }
};

///////////////////////////////////////////////////////////////////////////////

SkChunkAlloc::SkChunkAlloc(size_t minSize) {
    if (minSize < MIN_CHUNKALLOC_BLOCK_SIZE) {
        minSize = MIN_CHUNKALLOC_BLOCK_SIZE;
    }

    fBlock = NULL;
    fMinSize = minSize;
    fChunkSize = fMinSize;
    fTotalCapacity = 0;
    fBlockCount = 0;
}

SkChunkAlloc::~SkChunkAlloc() {
    this->reset();
}

void SkChunkAlloc::reset() {
    Block::FreeChain(fBlock);
    fBlock = NULL;
    fChunkSize = fMinSize;  // reset to our initial minSize
    fTotalCapacity = 0;
    fBlockCount = 0;
}

SkChunkAlloc::Block* SkChunkAlloc::newBlock(size_t bytes, AllocFailType ftype) {
    size_t size = bytes;
    if (size < fChunkSize) {
        size = fChunkSize;
    }

    Block* block = (Block*)sk_malloc_flags(sizeof(Block) + size,
                        ftype == kThrow_AllocFailType ? SK_MALLOC_THROW : 0);

    if (block) {
        //    block->fNext = fBlock;
        block->fFreeSize = size;
        block->fFreePtr = block->startOfData();

        fTotalCapacity += size;
        fBlockCount += 1;

        fChunkSize = increase_next_size(fChunkSize);
    }
    return block;
}

void* SkChunkAlloc::alloc(size_t bytes, AllocFailType ftype) {
    bytes = SkAlign4(bytes);

    Block* block = fBlock;

    if (block == NULL || bytes > block->fFreeSize) {
        block = this->newBlock(bytes, ftype);
        if (NULL == block) {
            return NULL;
        }
        block->fNext = fBlock;
        fBlock = block;
    }

    SkASSERT(block && bytes <= block->fFreeSize);
    char* ptr = block->fFreePtr;

    block->fFreeSize -= bytes;
    block->fFreePtr = ptr + bytes;
    return ptr;
}

size_t SkChunkAlloc::unalloc(void* ptr) {
    size_t bytes = 0;
    Block* block = fBlock;
    if (block) {
        char* cPtr = reinterpret_cast<char*>(ptr);
        char* start = block->startOfData();
        if (start <= cPtr && cPtr < block->fFreePtr) {
            bytes = block->fFreePtr - cPtr;
            block->fFreeSize += bytes;
            block->fFreePtr = cPtr;
        }
    }
    return bytes;
}

bool SkChunkAlloc::contains(const void* addr) const {
    const Block* block = fBlock;
    while (block) {
        if (block->contains(addr)) {
            return true;
        }
        block = block->fNext;
    }
    return false;
}
