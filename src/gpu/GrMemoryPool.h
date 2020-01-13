/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMemoryPool_DEFINED
#define GrMemoryPool_DEFINED

#include "src/gpu/GrBlockAllocator.h"

#ifdef SK_DEBUG
    #include "include/private/SkTHash.h"
#endif

/**
 * Allocates memory in blocks and parcels out space in the blocks for allocation requests. All
 * heap-allocated blocks will It is optimized for allocate / release speed over memory efficiency.
 * The interface is designed to be used to implement operator new and delete overrides. All
 * allocations are expected to be released before the pool's destructor is called. Allocations will
 * be aligned to sizeof(std::max_align_t).
 *
 * All allocated objects must be released back to the memory pool before it can be destroyed.
 */
class GrMemoryPool : protected GrBlockAllocator {
public:
    // Guaranteed alignment of pointer returned by allocate().
    static constexpr size_t kAlignment = alignof(std::max_align_t);

    // Smallest block size allocated on the heap (not the smallest reservation via allocate()).
    static constexpr size_t kMinAllocationSize = 1 << 10;

    /**
     * Prealloc size is the amount of space to allocate at pool creation time and keep around until
     * pool destruction.
     *
     * The size is what the pool will end up allocating from the system, and portions of the
     * allocated memory are used for internal bookkeeping. 'preallocSize' is clamped to the min and
     * max allocation sizes.
     */
    static std::unique_ptr<GrMemoryPool> Make(size_t preallocSize) {
        return GrBlockAllocator::Prealloc<GrMemoryPool>(std::max(preallocSize, kMinAllocationSize));
    }

    ~GrMemoryPool();
    void operator delete(void* p) { ::operator delete(p); }

    /**
     * Allocates memory. The memory must be freed with release() before the GrMemoryPool is deleted.
     */
    void* allocate(size_t size);
    /**
     * p must have been returned by allocate().
     */
    void release(void* p);

    /**
     * Returns true if there are no unreleased allocations.
     */
    bool isEmpty() const {
        // If size is the same as preallocSize, there aren't any heap blocks, so currentBlock()
        // is the inline head block.
        return 0 == this->size() && 0 == *this->liveCount();
    }

    /**
     * Returns the total allocated size of the GrMemoryPool minus any preallocated amount
     * (does not include the preallocSize for historic reasons).
     */
    size_t size() const { return this->GrBlockAllocator::size() - this->preallocSize(); }

    /**
     * Returns the preallocated size of the GrMemoryPool
     */
    using GrBlockAllocator::preallocSize;

private:
    friend class GrBlockAllocator; // For private constructor
    friend class GrOpMemoryPool;   // ""

    // GrMemoryPool per-allocation overhead so that it can always identify the block owning each
    struct AllocHeader {
        AllocHeader(Block block, int offset, int size);

#ifdef SK_DEBUG
        int   fSentinel; // known value to check for memory stomping (e.g., (CD)*)
        int   fID;       // ID that can be used to track down leaks by clients.
#endif

        Block fBlock;  // block owning the allocation
        int   fOffset; // offset from block to the start of the allocation
        int   fSize;   // total bytes for the allocation
    };

    GrMemoryPool(void* headBlock, int blockSize);

    // GrMemoryPool uses the block allocator's per-block metadata to track the number of live allocs
    const int* liveCount() const { return BlockData(this->currentBlock()); }
    int* liveCount() { return BlockData(this->currentBlock()); }
    const int* liveCount(Block b) const { return BlockData(b); }
    int* liveCount(Block b) { return BlockData(b); }

#ifdef SK_DEBUG
    void validate() const;

    SkTHashSet<int> fAllocatedIDs;
    int             fAllocationCount;
#endif
};

class GrOp;

/**
 * A specialized variant on GrMemoryPool that provides convenience functions for allocating and
 * releasing GrOps.
 */
class GrOpMemoryPool : private GrMemoryPool {
public:
    static std::unique_ptr<GrOpMemoryPool> Make(size_t preallocSize) {
        return GrBlockAllocator::Prealloc<GrOpMemoryPool>(std::max(preallocSize,
                                                                   kMinAllocationSize));
    }

    void operator delete(void* p) { ::operator delete(p); }

    template <typename Op, typename... OpArgs>
    std::unique_ptr<Op> allocate(OpArgs&&... opArgs) {
        auto mem = this->allocate(sizeof(Op));
        return std::unique_ptr<Op>(new (mem) Op(std::forward<OpArgs>(opArgs)...));
    }

    void release(std::unique_ptr<GrOp> op);

    using GrMemoryPool::allocate;
    using GrMemoryPool::isEmpty;

private:
    friend class GrBlockAllocator; // For private constructor

    GrOpMemoryPool(void* headBlock, int blockSize)
            : GrMemoryPool(headBlock, blockSize) {}
};

#endif
