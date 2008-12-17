/* libs/corecg/SkChunkAlloc.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkChunkAlloc.h"

struct SkChunkAlloc::Block {
    Block*  fNext;
    size_t  fFreeSize;
    char*   fFreePtr;
    // data[] follows
    
    void freeChain() {    // this can be null
        Block* block = this;
        while (block) {
            Block* next = block->fNext;
            sk_free(block);
            block = next;
        }
    };
    
    Block* tail() {
        Block* block = this;
        if (block) {
            for (;;) {
                Block* next = block->fNext;
                if (NULL == next) {
                    break;
                }
                block = next;
            }
        }
        return block;
    }
};

SkChunkAlloc::SkChunkAlloc(size_t minSize)
    : fBlock(NULL), fMinSize(SkAlign4(minSize)), fPool(NULL), fTotalCapacity(0)
{
}

SkChunkAlloc::~SkChunkAlloc() {
    this->reset();
}

void SkChunkAlloc::reset() {
    fBlock->freeChain();
    fBlock = NULL;
    fPool->freeChain();
    fPool = NULL;
    fTotalCapacity = 0;
}

void SkChunkAlloc::reuse() {
    if (fPool && fBlock) {
        fPool->tail()->fNext = fBlock;
    }
    fPool = fBlock;
    fBlock = NULL;
    fTotalCapacity = 0;
}

SkChunkAlloc::Block* SkChunkAlloc::newBlock(size_t bytes, AllocFailType ftype) {
    Block* block = fPool;

    if (block && bytes <= block->fFreeSize) {
        fPool = block->fNext;
        return block;
    }

    size_t  size = SkMax32((int32_t)bytes, (int32_t)fMinSize);

    block = (Block*)sk_malloc_flags(sizeof(Block) + size,
                        ftype == kThrow_AllocFailType ? SK_MALLOC_THROW : 0);

    if (block) {
        //    block->fNext = fBlock;
        block->fFreeSize = size;
        block->fFreePtr = (char*)block + sizeof(Block);
        
        fTotalCapacity += size;
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
    void* ptr = block->fFreePtr;

    block->fFreeSize -= bytes;
    block->fFreePtr += bytes;
    return ptr;
}

