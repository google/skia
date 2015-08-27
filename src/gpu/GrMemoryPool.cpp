/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMemoryPool.h"

#ifdef SK_DEBUG
    #define VALIDATE this->validate()
#else
    #define VALIDATE
#endif

GrMemoryPool::GrMemoryPool(size_t preallocSize, size_t minAllocSize) {
    SkDEBUGCODE(fAllocationCnt = 0);
    SkDEBUGCODE(fAllocBlockCnt = 0);

    minAllocSize = SkTMax<size_t>(minAllocSize, 1 << 10);
    fMinAllocSize = GrSizeAlignUp(minAllocSize + kPerAllocPad, kAlignment),
    fPreallocSize = GrSizeAlignUp(preallocSize + kPerAllocPad, kAlignment);
    fPreallocSize = SkTMax(fPreallocSize, fMinAllocSize);
    fSize = fPreallocSize;

    fHead = CreateBlock(fPreallocSize);
    fTail = fHead;
    fHead->fNext = nullptr;
    fHead->fPrev = nullptr;
    VALIDATE;
};

GrMemoryPool::~GrMemoryPool() {
    VALIDATE;
    SkASSERT(0 == fAllocationCnt);
    SkASSERT(fHead == fTail);
    SkASSERT(0 == fHead->fLiveCount);
    DeleteBlock(fHead);
};

void* GrMemoryPool::allocate(size_t size) {
    VALIDATE;
    size = GrSizeAlignUp(size, kAlignment);
    size += kPerAllocPad;
    if (fTail->fFreeSize < size) {
        size_t blockSize = size;
        blockSize = SkTMax<size_t>(blockSize, fMinAllocSize);
        BlockHeader* block = CreateBlock(blockSize);

        block->fPrev = fTail;
        block->fNext = nullptr;
        SkASSERT(nullptr == fTail->fNext);
        fTail->fNext = block;
        fTail = block;
        fSize += block->fSize;
        SkDEBUGCODE(++fAllocBlockCnt);
    }
    SkASSERT(fTail->fFreeSize >= size);
    intptr_t ptr = fTail->fCurrPtr;
    // We stash a pointer to the block header, just before the allocated space,
    // so that we can decrement the live count on delete in constant time.
    *reinterpret_cast<BlockHeader**>(ptr) = fTail;
    ptr += kPerAllocPad;
    fTail->fPrevPtr = fTail->fCurrPtr;
    fTail->fCurrPtr += size;
    fTail->fFreeSize -= size;
    fTail->fLiveCount += 1;

    SkDEBUGCODE(++fAllocationCnt);
    VALIDATE;
    return reinterpret_cast<void*>(ptr);
}

void GrMemoryPool::release(void* p) {
    VALIDATE;
    intptr_t ptr = reinterpret_cast<intptr_t>(p) - kPerAllocPad;
    BlockHeader* block = *reinterpret_cast<BlockHeader**>(ptr);
    if (1 == block->fLiveCount) {
        // the head block is special, it is reset rather than deleted
        if (fHead == block) {
            fHead->fCurrPtr = reinterpret_cast<intptr_t>(fHead) + kHeaderSize;
            fHead->fLiveCount = 0;
            fHead->fFreeSize = fPreallocSize;
        } else {
            BlockHeader* prev = block->fPrev;
            BlockHeader* next = block->fNext;
            SkASSERT(prev);
            prev->fNext = next;
            if (next) {
                next->fPrev = prev;
            } else {
                SkASSERT(fTail == block);
                fTail = prev;
            }
            fSize -= block->fSize;
            DeleteBlock(block);
            SkDEBUGCODE(fAllocBlockCnt--);
        }
    } else {
        --block->fLiveCount;
        // Trivial reclaim: if we're releasing the most recent allocation, reuse it
        if (block->fPrevPtr == ptr) {
            block->fFreeSize += (block->fCurrPtr - block->fPrevPtr);
            block->fCurrPtr = block->fPrevPtr;
        }
    }
    SkDEBUGCODE(--fAllocationCnt);
    VALIDATE;
}

GrMemoryPool::BlockHeader* GrMemoryPool::CreateBlock(size_t size) {
    size_t paddedSize = size + kHeaderSize;
    BlockHeader* block =
        reinterpret_cast<BlockHeader*>(sk_malloc_throw(paddedSize));
    // we assume malloc gives us aligned memory
    SkASSERT(!(reinterpret_cast<intptr_t>(block) % kAlignment));
    block->fLiveCount = 0;
    block->fFreeSize = size;
    block->fCurrPtr = reinterpret_cast<intptr_t>(block) + kHeaderSize;
    block->fPrevPtr = 0; // gcc warns on assigning nullptr to an intptr_t.
    block->fSize = paddedSize;
    return block;
}

void GrMemoryPool::DeleteBlock(BlockHeader* block) {
    sk_free(block);
}

void GrMemoryPool::validate() {
#ifdef SK_DEBUG
    BlockHeader* block = fHead;
    BlockHeader* prev = nullptr;
    SkASSERT(block);
    int allocCount = 0;
    do {
        allocCount += block->fLiveCount;
        SkASSERT(prev == block->fPrev);
        if (prev) {
            SkASSERT(prev->fNext == block);
        }

        intptr_t b = reinterpret_cast<intptr_t>(block);
        size_t ptrOffset = block->fCurrPtr - b;
        size_t totalSize = ptrOffset + block->fFreeSize;
        size_t userSize = totalSize - kHeaderSize;
        intptr_t userStart = b + kHeaderSize;

        SkASSERT(!(b % kAlignment));
        SkASSERT(!(totalSize % kAlignment));
        SkASSERT(!(userSize % kAlignment));
        SkASSERT(!(block->fCurrPtr % kAlignment));
        if (fHead != block) {
            SkASSERT(block->fLiveCount);
            SkASSERT(userSize >= fMinAllocSize);
        } else {
            SkASSERT(userSize == fPreallocSize);
        }
        if (!block->fLiveCount) {
            SkASSERT(ptrOffset ==  kHeaderSize);
            SkASSERT(userStart == block->fCurrPtr);
        } else {
            SkASSERT(block == *reinterpret_cast<BlockHeader**>(userStart));
        }
        prev = block;
    } while ((block = block->fNext));
    SkASSERT(allocCount == fAllocationCnt);
    SkASSERT(prev == fTail);
    SkASSERT(fAllocBlockCnt != 0 || fSize == fPreallocSize);
#endif
}
