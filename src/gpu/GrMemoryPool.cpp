/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMemoryPool.h"

#if GR_DEBUG
    #define VALIDATE this->validate()
#else
    #define VALIDATE
#endif

GrMemoryPool::GrMemoryPool(size_t preallocSize, size_t minAllocSize) {
    GR_DEBUGCODE(fAllocationCnt = 0);

    minAllocSize = GrMax<size_t>(minAllocSize, 1 << 10);
    fMinAllocSize = GrSizeAlignUp(minAllocSize + kPerAllocPad, kAlignment),
    fPreallocSize = GrSizeAlignUp(preallocSize + kPerAllocPad, kAlignment);
    fPreallocSize = GrMax(fPreallocSize, fMinAllocSize);

    fHead = CreateBlock(fPreallocSize);
    fTail = fHead;
    fHead->fNext = NULL;
    fHead->fPrev = NULL;
    VALIDATE;
};

GrMemoryPool::~GrMemoryPool() {
    VALIDATE;
    GrAssert(0 == fAllocationCnt);
    GrAssert(fHead == fTail);
    GrAssert(0 == fHead->fLiveCount);
    DeleteBlock(fHead);
};

void* GrMemoryPool::allocate(size_t size) {
    VALIDATE;
    size = GrSizeAlignUp(size, kAlignment);
    size += kPerAllocPad;
    if (fTail->fFreeSize < size) {
        int blockSize = size;
        blockSize = GrMax<size_t>(blockSize, fMinAllocSize);
        BlockHeader* block = CreateBlock(blockSize);

        block->fPrev = fTail;
        block->fNext = NULL;
        GrAssert(NULL == fTail->fNext);
        fTail->fNext = block;
        fTail = block;
    }
    GrAssert(fTail->fFreeSize >= size);
    intptr_t ptr = fTail->fCurrPtr;
    // We stash a pointer to the block header, just before the allocated space,
    // so that we can decrement the live count on delete in constant time.
    *reinterpret_cast<BlockHeader**>(ptr) = fTail;
    ptr += kPerAllocPad;
    fTail->fPrevPtr = fTail->fCurrPtr;
    fTail->fCurrPtr += size;
    fTail->fFreeSize -= size;
    fTail->fLiveCount += 1;
    GR_DEBUGCODE(++fAllocationCnt);
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
            fHead->fCurrPtr = reinterpret_cast<intptr_t>(fHead) +
                                kHeaderSize;
            fHead->fLiveCount = 0;
            fHead->fFreeSize = fPreallocSize;
        } else {
            BlockHeader* prev = block->fPrev;
            BlockHeader* next = block->fNext;
            GrAssert(prev);
            prev->fNext = next;
            if (next) {
                next->fPrev = prev;
            } else {
                GrAssert(fTail == block);
                fTail = prev;
            }
            DeleteBlock(block);
        }
    } else {
        --block->fLiveCount;
        // Trivial reclaim: if we're releasing the most recent allocation, reuse it
        if (block->fPrevPtr == ptr) {
            block->fFreeSize += (block->fCurrPtr - block->fPrevPtr);
            block->fCurrPtr = block->fPrevPtr;
        }
    }
    GR_DEBUGCODE(--fAllocationCnt);
    VALIDATE;
}

GrMemoryPool::BlockHeader* GrMemoryPool::CreateBlock(size_t size) {
    BlockHeader* block =
        reinterpret_cast<BlockHeader*>(GrMalloc(size + kHeaderSize));
    // we assume malloc gives us aligned memory
    GrAssert(!(reinterpret_cast<intptr_t>(block) % kAlignment));
    block->fLiveCount = 0;
    block->fFreeSize = size;
    block->fCurrPtr = reinterpret_cast<intptr_t>(block) + kHeaderSize;
    block->fPrevPtr = 0; // gcc warns on assigning NULL to an intptr_t.
    return block;
}

void GrMemoryPool::DeleteBlock(BlockHeader* block) {
    GrFree(block);
}

void GrMemoryPool::validate() {
#ifdef SK_DEBUG
    BlockHeader* block = fHead;
    BlockHeader* prev = NULL;
    GrAssert(block);
    int allocCount = 0;
    do {
        allocCount += block->fLiveCount;
        GrAssert(prev == block->fPrev);
        if (NULL != prev) {
            GrAssert(prev->fNext == block);
        }

        intptr_t b = reinterpret_cast<intptr_t>(block);
        size_t ptrOffset = block->fCurrPtr - b;
        size_t totalSize = ptrOffset + block->fFreeSize;
        size_t userSize = totalSize - kHeaderSize;
        intptr_t userStart = b + kHeaderSize;

        GrAssert(!(b % kAlignment));
        GrAssert(!(totalSize % kAlignment));
        GrAssert(!(userSize % kAlignment));
        GrAssert(!(block->fCurrPtr % kAlignment));
        if (fHead != block) {
            GrAssert(block->fLiveCount);
            GrAssert(userSize >= fMinAllocSize);
        } else {
            GrAssert(userSize == fPreallocSize);
        }
        if (!block->fLiveCount) {
            GrAssert(ptrOffset ==  kHeaderSize);
            GrAssert(userStart == block->fCurrPtr);
        } else {
            GrAssert(block == *reinterpret_cast<BlockHeader**>(userStart));
        }
        prev = block;
    } while ((block = block->fNext));
    GrAssert(allocCount == fAllocationCnt);
    GrAssert(prev == fTail);
#endif
}
