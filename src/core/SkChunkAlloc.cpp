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

    size_t blockSize() const {
        char* start = this->startOfData();
        size_t bytes = fFreePtr - start;
        return fFreeSize + bytes;
    }

    void reset() {
        fNext = nullptr;
        fFreeSize = this->blockSize();
        fFreePtr = this->startOfData();
    }

    char* startOfData() const {
        return reinterpret_cast<char*>(SkAlign8(reinterpret_cast<size_t>(this + 1)));
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
        return ptr >= this->startOfData() && ptr < fFreePtr;
    }
};

///////////////////////////////////////////////////////////////////////////////

SkChunkAlloc::SkChunkAlloc(size_t minSize) {
    if (minSize < MIN_CHUNKALLOC_BLOCK_SIZE) {
        minSize = MIN_CHUNKALLOC_BLOCK_SIZE;
    }

    fBlock = nullptr;
    fMinSize = minSize;
    fChunkSize = fMinSize;
    fTotalCapacity = 0;
    fTotalUsed = 0;
    SkDEBUGCODE(fTotalLost = 0;)
    SkDEBUGCODE(fBlockCount = 0;)
}

SkChunkAlloc::~SkChunkAlloc() {
    this->reset();
}

void SkChunkAlloc::reset() {
    Block::FreeChain(fBlock);
    fBlock = nullptr;
    fChunkSize = fMinSize;  // reset to our initial minSize
    fTotalCapacity = 0;
    fTotalUsed = 0;
    SkDEBUGCODE(fTotalLost = 0;)
    SkDEBUGCODE(fBlockCount = 0;)
}

void SkChunkAlloc::rewind() {
    SkDEBUGCODE(this->validate();)

    Block* largest = fBlock;

    if (largest) {
        Block* next;
        for (Block* cur = largest->fNext; cur; cur = next) {
            next = cur->fNext;
            if (cur->blockSize() > largest->blockSize()) {
                sk_free(largest);
                largest = cur;
            } else {
                sk_free(cur);
            }
        }

        largest->reset();
        fTotalCapacity = largest->blockSize();
        SkDEBUGCODE(fBlockCount = 1;)
    } else {
        fTotalCapacity = 0;
        SkDEBUGCODE(fBlockCount = 0;)
    }

    fBlock = largest;
    fChunkSize = fMinSize;  // reset to our initial minSize
    fTotalUsed = 0;
    SkDEBUGCODE(fTotalLost = 0;)
    SkDEBUGCODE(this->validate();)
}

SkChunkAlloc::Block* SkChunkAlloc::newBlock(size_t bytes, AllocFailType ftype) {
    size_t size = bytes;
    if (size < fChunkSize) {
        size = fChunkSize;
    }

    Block* block = (Block*)sk_malloc_flags(SkAlign8(sizeof(Block)) + size,
                        ftype == kThrow_AllocFailType ? SK_MALLOC_THROW : 0);

    if (block) {
        block->fFreeSize = size;
        block->fFreePtr = block->startOfData();

        fTotalCapacity += size;
        SkDEBUGCODE(fBlockCount += 1;)

        fChunkSize = increase_next_size(fChunkSize);
    }
    return block;
}

SkChunkAlloc::Block* SkChunkAlloc::addBlockIfNecessary(size_t bytes, AllocFailType ftype) {
    SkASSERT(SkIsAlign8(bytes));

    if (!fBlock || bytes > fBlock->fFreeSize) {
        Block* block = this->newBlock(bytes, ftype);
        if (!block) {
            return nullptr;
        }
#ifdef SK_DEBUG
        if (fBlock) {
            fTotalLost += fBlock->fFreeSize;
        }
#endif
        block->fNext = fBlock;
        fBlock = block;
    }

    SkASSERT(fBlock && bytes <= fBlock->fFreeSize);
    return fBlock;
}

void* SkChunkAlloc::alloc(size_t bytes, AllocFailType ftype) {
    SkDEBUGCODE(this->validate();)

    bytes = SkAlign8(bytes);

    Block* block = this->addBlockIfNecessary(bytes, ftype);
    if (!block) {
        return nullptr;
    }

    char* ptr = block->fFreePtr;

    fTotalUsed += bytes;
    block->fFreeSize -= bytes;
    block->fFreePtr = ptr + bytes;
    SkDEBUGCODE(this->validate();)
    SkASSERT(SkIsAlign8((size_t)ptr));
    return ptr;
}

size_t SkChunkAlloc::unalloc(void* ptr) {
    SkDEBUGCODE(this->validate();)

    size_t bytes = 0;
    Block* block = fBlock;
    if (block) {
        char* cPtr = reinterpret_cast<char*>(ptr);
        char* start = block->startOfData();
        if (start <= cPtr && cPtr < block->fFreePtr) {
            bytes = block->fFreePtr - cPtr;
            fTotalUsed -= bytes;
            block->fFreeSize += bytes;
            block->fFreePtr = cPtr;
        }
    }
    SkDEBUGCODE(this->validate();)
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

#ifdef SK_DEBUG
void SkChunkAlloc::validate() {
    int numBlocks = 0;
    size_t totCapacity = 0;
    size_t totUsed = 0;
    size_t totLost = 0;
    size_t totAvailable = 0;

    for (Block* temp = fBlock; temp; temp = temp->fNext) {
        ++numBlocks;
        totCapacity += temp->blockSize();
        totUsed += temp->fFreePtr - temp->startOfData();
        if (temp == fBlock) {
            totAvailable += temp->fFreeSize;
        } else {
            totLost += temp->fFreeSize;
        }
    }

    SkASSERT(fBlockCount == numBlocks);
    SkASSERT(fTotalCapacity == totCapacity);
    SkASSERT(fTotalUsed == totUsed);
    SkASSERT(fTotalLost == totLost);
    SkASSERT(totCapacity == totUsed + totLost + totAvailable);
}
#endif
