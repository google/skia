/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMemoryPool_DEFINED
#define GrMemoryPool_DEFINED

#include "include/private/GrTypesPriv.h"

#include "include/core/SkRefCnt.h"

#ifdef SK_DEBUG
#include "include/private/SkTHash.h"
#endif

/**
 * Allocates memory in blocks and parcels out space in the blocks for allocation
 * requests. It is optimized for allocate / release speed over memory
 * efficiency. The interface is designed to be used to implement operator new
 * and delete overrides. All allocations are expected to be released before the
 * pool's destructor is called. Allocations will be aligned to
 * sizeof(std::max_align_t).
 */
class GrMemoryPool {
public:
#ifdef SK_FORCE_8_BYTE_ALIGNMENT
    // This is an issue for WASM builds using emscripten, which had
    // std::max_align_t = 16, but was returning pointers only aligned to 8
    // bytes. https://github.com/emscripten-core/emscripten/issues/10072
    // Since Skia does not use "long double" (16 bytes), we should be ok to
    // force it back to 8 bytes until emscripten is fixed.
    static constexpr size_t kAlignment = 8;
#else
    // Guaranteed alignment of pointer returned by allocate().
    static constexpr size_t kAlignment = alignof(std::max_align_t);
#endif
    // Minimum size this class will allocate at once.
    static constexpr size_t kMinAllocationSize = 1 << 10;

    /**
     * Prealloc size is the amount of space to allocate at pool creation
     * time and keep around until pool destruction. The min alloc size is
     * the smallest allowed size of additional allocations. Both sizes are
     * adjusted to ensure that they are at least as large as kMinAllocationSize.
     *
     * Both sizes are what the pool will end up allocating from the system, and
     * portions of the allocated memory is used for internal bookkeeping.
     */
    static std::unique_ptr<GrMemoryPool> Make(size_t preallocSize, size_t minAllocSize);
    void operator delete(void* p) { ::operator delete(p); }

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

    /**
     * Returns the preallocated size of the GrMemoryPool
     */
    size_t preallocSize() const { return fHead->fSize; }


private:
    GrMemoryPool(void* preallocStart, size_t preallocSize, size_t minAllocSize);

    struct BlockHeader;

    static BlockHeader* CreateBlock(size_t size);
    static BlockHeader* InitBlock(void* mem, size_t blockSize);

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
        int32_t fID;             ///< ID that can be used to track down leaks by clients.
#endif
        BlockHeader* fHeader;    ///< pointer back to the block header in which an alloc resides
    };

    size_t                            fSize;
    size_t                            fMinAllocSize;
    BlockHeader*                      fHead;
    BlockHeader*                      fTail;
#ifdef SK_DEBUG
    int                               fAllocationCnt;
    int                               fAllocBlockCnt;
    SkTHashSet<int32_t>               fAllocatedIDs;
#endif

    friend class GrOpMemoryPool;

    static constexpr size_t kHeaderSize  = GrAlignTo(sizeof(BlockHeader), kAlignment);
    static constexpr size_t kPerAllocPad = GrAlignTo(sizeof(AllocHeader), kAlignment);
};

class GrOp;

class GrOpMemoryPool {
public:
    static std::unique_ptr<GrOpMemoryPool> Make(size_t preallocSize, size_t minAllocSize);
    void operator delete(void* p) { ::operator delete(p); }

    ~GrOpMemoryPool();

    template <typename Op, typename... OpArgs>
    std::unique_ptr<Op> allocate(OpArgs&&... opArgs) {
        auto mem = this->pool()->allocate(sizeof(Op));
        return std::unique_ptr<Op>(new (mem) Op(std::forward<OpArgs>(opArgs)...));
    }

    void* allocate(size_t size) { return this->pool()->allocate(size); }

    void release(std::unique_ptr<GrOp> op);

    bool isEmpty() const { return this->pool()->isEmpty(); }

private:
    GrMemoryPool* pool() const;

    GrOpMemoryPool() = default;
};

#endif
