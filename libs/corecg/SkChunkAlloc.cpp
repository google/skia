/* libs/corecg/SkChunkAlloc.cpp
**
** Copyright 2006, Google Inc.
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

SkChunkAlloc::~SkChunkAlloc()
{
    this->reset();
}

void SkChunkAlloc::reset()
{
    Block* block = fBlock;

    while (block)
    {
        Block* next = block->fNext;
        sk_free(block);
        block = next;
    }
    fBlock = nil;
}

void* SkChunkAlloc::alloc(size_t bytes, AllocFailType ftype)
{
    bytes = SkAlign4(bytes);

    if (fBlock == nil || bytes > fBlock->fFreeSize)
    {
        size_t  size = SkMax32((S32)bytes, (S32)fMinSize);
        Block*  block = (Block*)sk_malloc_flags(sizeof(Block) + size, ftype == kThrow_AllocFailType ? SK_MALLOC_THROW : 0);
        if (block == nil)
            return nil;

        block->fNext = fBlock;
        block->fFreeSize = size;
        block->fFreePtr = (char*)block + sizeof(Block);
        fBlock = block;
    }

    SkASSERT(fBlock && bytes <= fBlock->fFreeSize);
    void* ptr = fBlock->fFreePtr;

    fBlock->fFreeSize -= bytes;
    fBlock->fFreePtr += bytes;
    return ptr;
}

