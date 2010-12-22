/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrAllocPool.h"

#define GrAllocPool_MIN_BLOCK_SIZE      ((size_t)128)

struct GrAllocPool::Block {
    Block*  fNext;
    char*   fPtr;
    size_t  fBytesFree;
    size_t  fBytesTotal;

    static Block* Create(size_t size, Block* next) {
        GrAssert(size >= GrAllocPool_MIN_BLOCK_SIZE);

        Block* block = (Block*)GrMalloc(sizeof(Block) + size);
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
        GrAssert(bytes <= fBytesFree);
        fBytesFree -= bytes;
        void* ptr = fPtr;
        fPtr += bytes;
        return ptr;
    }
    
    size_t release(size_t bytes) {
        GrAssert(bytes > 0);
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
    GR_DEBUGCODE(fBlocksAllocated = 0;)
}

GrAllocPool::~GrAllocPool() {
    this->reset();
}

void GrAllocPool::reset() {
    this->validate();

    Block* block = fBlock;
    while (block) {
        Block* next = block->fNext;
        GrFree(block);
        block = next;
    }
    fBlock = NULL;
    GR_DEBUGCODE(fBlocksAllocated = 0;)
}

void* GrAllocPool::alloc(size_t size) {
    this->validate();
    
    if (!fBlock || !fBlock->canAlloc(size)) {
        size_t blockSize = GrMax(fMinBlockSize, size);
        fBlock = Block::Create(blockSize, fBlock);
        GR_DEBUGCODE(fBlocksAllocated += 1;)
    }
    return fBlock->alloc(size);
}

void GrAllocPool::release(size_t bytes) {
    this->validate();
    
    while (bytes && NULL != fBlock) {
        bytes = fBlock->release(bytes);
        if (fBlock->empty()) {
            Block* next = fBlock->fNext;
            GrFree(fBlock);
            fBlock = next;
            GR_DEBUGCODE(fBlocksAllocated -= 1;)
        }
    }
}


#if GR_DEBUG

void GrAllocPool::validate() const {
    Block* block = fBlock;
    int count = 0;
    while (block) {
        count += 1;
        block = block->fNext;
    }
    GrAssert(fBlocksAllocated == count);
}

#endif


