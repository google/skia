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

    /**
     * Returns the total allocated size of the GrMemoryPool minus any preallocated amount
     */
    size_t size() const { return fSize; }

private:
    struct BlockHeader;

    static BlockHeader* CreateBlock(size_t size);

    static void DeleteBlock(BlockHeader* block);

    void validate();

    struct BlockHeader {
#ifdef SK_DEBUG
        uint32_t     fBlockSentinal;  ///< known value to check for bad back pointers to blocks
#endif
        BlockHeader* fNext;      ///< doubly-linked list of blocks.
        BlockHeader* fPrev;
        int          fLiveCount; ///< number of outstanding allocations in the
                                 ///< block.
        intptr_t     fCurrPtr;   ///< ptr to the start of blocks free space.
        intptr_t     fPrevPtr;   ///< ptr to the last allocation made
        size_t       fFreeSize;  ///< amount of free space left in the block.
        size_t       fSize;      ///< total allocated size of the block
    };

    static const uint32_t kAssignedMarker = 0xCDCDCDCD;
    static const uint32_t kFreedMarker    = 0xEFEFEFEF;

    struct AllocHeader {
#ifdef SK_DEBUG
        uint32_t fSentinal;      ///< known value to check for memory stomping (e.g., (CD)*)
#endif
        BlockHeader* fHeader;    ///< pointer back to the block header in which an alloc resides
    };

    enum {
        // We assume this alignment is good enough for everybody.
        kAlignment    = 8,
        kHeaderSize   = GR_CT_ALIGN_UP(sizeof(BlockHeader), kAlignment),
        kPerAllocPad  = GR_CT_ALIGN_UP(sizeof(AllocHeader), kAlignment),
    };
    size_t                            fSize;
    size_t                            fPreallocSize;
    size_t                            fMinAllocSize;
    BlockHeader*                      fHead;
    BlockHeader*                      fTail;
#ifdef SK_DEBUG
    int                               fAllocationCnt;
    int                               fAllocBlockCnt;
#endif
};

#endif
