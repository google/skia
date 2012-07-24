/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMemoryPool_DEFINED
#define GrMemoryPool_DEFINED

#include "GrTypes.h"

/**
 * Allocates memory in blocks and parcels out space in the blocks for allocation
 * requests. It is optimized for allocate / release speed over memory
 * effeciency. The interface is designed to be used to implement operator new
 * and delete overrides. All allocations are expected to be released before the
 * pool's destructor is called. Allocations will be 8-byte aligned.
 */
class GrMemoryPool {
public:
    /**
     * Prealloc size is the amount of space to make available at pool creation
     * time and keep around until pool destruction. The min alloc size is the
     * smallest allowed size of additional allocations.
     */
    GrMemoryPool(size_t preallocSize, size_t minAllocSize);

    ~GrMemoryPool();

    /**
     * Allocates memory. The memory must be freed with release().
     */
    void* allocate(size_t size);

    /**
     * p must have been returned by allocate()
     */
    void release(void* p);

    /**
     * Returns true if there are no unreleased allocations.
     */
    bool isEmpty() const { return fTail == fHead && !fHead->fLiveCount; }

private:
    struct BlockHeader;

    BlockHeader* CreateBlock(size_t size);

    void DeleteBlock(BlockHeader* block);

    void validate();

    struct BlockHeader {
        BlockHeader* fNext;      ///< doubly-linked list of blocks.
        BlockHeader* fPrev;
        int          fLiveCount; ///< number of outstanding allocations in the
                                 ///< block.
        intptr_t     fCurrPtr;   ///< ptr to the start of blocks free space.
        intptr_t     fPrevPtr;   ///< ptr to the last allocation made
        size_t       fFreeSize;  ///< amount of free space left in the block.
    };

    enum {
        // We assume this alignment is good enough for everybody.
        kAlignment    = 8,
        kHeaderSize   = GR_CT_ALIGN_UP(sizeof(BlockHeader), kAlignment),
        kPerAllocPad  = GR_CT_ALIGN_UP(sizeof(BlockHeader*), kAlignment),
    };
    size_t                            fPreallocSize;
    size_t                            fMinAllocSize;
    BlockHeader*                      fHead;
    BlockHeader*                      fTail;
#if GR_DEBUG
    int                               fAllocationCnt;
#endif
};

#endif
