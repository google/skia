/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAllocPool.h"

#include "GrTypes.h"

#define GrAllocPool_MIN_BLOCK_SIZE      ((size_t)128)

struct GrAllocPool::Block {
    Block*  fNext;
    char*   fPtr;
    size_t  fBytesFree;
    size_t  fBytesTotal;

    static Block* Create(size_t size, Block* next) {
        SkASSERT(size >= GrAllocPool_MIN_BLOCK_SIZE);

        Block* block = (Block*)sk_malloc_throw(sizeof(Block) + size);
        block->fNext = next;
        block->fPtr = (char*)block + sizeof(Block);
        block->fBytesFree = size;
        block->fBytesTotal = size;
        return block;
    }

    bool canAlloc(size_t bytes) const {
        return bytes <= fBytesFree;
    }

    void* alloc(size_t bytes) {
        SkASSERT(bytes <= fBytesFree);
        fBytesFree -= bytes;
        void* ptr = fPtr;
        fPtr += bytes;
        return ptr;
    }

    size_t release(size_t bytes) {
        SkASSERT(bytes > 0);
        size_t free = GrMin(bytes, fBytesTotal - fBytesFree);
        fBytesFree += free;
        fPtr -= free;
        return bytes - free;
    }

    bool empty() const { return fBytesTotal == fBytesFree; }
};

///////////////////////////////////////////////////////////////////////////////

GrAllocPool::GrAllocPool(size_t blockSize) {
    fBlock = NULL;
    fMinBlockSize = GrMax(blockSize, GrAllocPool_MIN_BLOCK_SIZE);
    SkDEBUGCODE(fBlocksAllocated = 0;)
}

GrAllocPool::~GrAllocPool() {
    this->reset();
}

void GrAllocPool::reset() {
    this->validate();

    Block* block = fBlock;
    while (block) {
        Block* next = block->fNext;
        sk_free(block);
        block = next;
    }
    fBlock = NULL;
    SkDEBUGCODE(fBlocksAllocated = 0;)
}

void* GrAllocPool::alloc(size_t size) {
    this->validate();

    if (!fBlock || !fBlock->canAlloc(size)) {
        size_t blockSize = GrMax(fMinBlockSize, size);
        fBlock = Block::Create(blockSize, fBlock);
        SkDEBUGCODE(fBlocksAllocated += 1;)
    }
    return fBlock->alloc(size);
}

void GrAllocPool::release(size_t bytes) {
    this->validate();

    while (bytes && NULL != fBlock) {
        bytes = fBlock->release(bytes);
        if (fBlock->empty()) {
            Block* next = fBlock->fNext;
            sk_free(fBlock);
            fBlock = next;
            SkDEBUGCODE(fBlocksAllocated -= 1;)
        }
    }
}

#ifdef SK_DEBUG

void GrAllocPool::validate() const {
    Block* block = fBlock;
    int count = 0;
    while (block) {
        count += 1;
        block = block->fNext;
    }
    SkASSERT(fBlocksAllocated == count);
}

#endif
