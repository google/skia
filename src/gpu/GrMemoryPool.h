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
 * Allocates memory in blocks and parcels out space in the blocks for allocation requests. It is
 * optimized for allocate / release speed over memory efficiency. The interface is designed to be
 * used to implement operator new and delete overrides. All allocations are expected to be released
 * before the pool's destructor is called. Allocations will be aligned to sizeof(std::max_align_t).
 *
 * All allocated objects must be released back to the memory pool before it can be destroyed.
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

    // Smallest block size allocated on the heap (not the smallest reservation via allocate()).
    static constexpr size_t kMinAllocationSize = 1 << 10;

    /**
     * Prealloc size is the amount of space to allocate at pool creation
     * time and keep around until pool destruction. The min alloc size is
     * the smallest allowed size of additional allocations. Both sizes are
     * adjusted to ensure that they are at least as large as kMinAllocationSize
     * and less than GrBlockAllocator::kMaxAllocationSize.
     *
     * Both sizes are what the pool will end up allocating from the system, and
     * portions of the allocated memory is used for internal bookkeeping.
     */
    static std::unique_ptr<GrMemoryPool> Make(size_t preallocSize, size_t minAllocSize);

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
        return 0 == this->size() && 0 == fAllocator.currentBlock()->metadata();
    }

    /**
     * Returns the total allocated size of the GrMemoryPool minus any preallocated amount
     */
    size_t size() const { return fAllocator.totalSize() - fAllocator.preallocSize(); }

    /**
     * Returns the preallocated size of the GrMemoryPool
     */
    size_t preallocSize() const {
        // Account for the debug-only fields in this count, the offset is 0 for release builds
        return offsetof(GrMemoryPool, fAllocator) + fAllocator.preallocSize();
    }

#ifdef SK_DEBUG
    void validate() const;
#endif

private:
    // Per-allocation overhead so that GrMemoryPool can always identify the block owning each and
    // release all occupied bytes, including any resulting from alignment padding.
    struct Header {
#ifdef SK_DEBUG
        int fSentinel; // known value to check for memory stomping (e.g., (CD)*)
        int fID;       // ID that can be used to track down leaks by clients.
#endif
        int fStart;
        int fEnd;
    };

    GrMemoryPool(size_t preallocSize, size_t minAllocSize);

#ifdef SK_DEBUG
    SkTHashSet<int>  fAllocatedIDs;
    int              fAllocationCount;
#endif

    GrBlockAllocator fAllocator; // Must be the last field, in order to use extra allocated space

    friend class GrOpMemoryPool;
};

class GrOp;

class GrOpMemoryPool {
public:
    static std::unique_ptr<GrOpMemoryPool> Make(size_t preallocSize, size_t minAllocSize);
    void operator delete(void* p) { ::operator delete(p); }

    template <typename Op, typename... OpArgs>
    std::unique_ptr<Op> allocate(OpArgs&&... opArgs) {
        auto mem = this->allocate(sizeof(Op));
        return std::unique_ptr<Op>(new (mem) Op(std::forward<OpArgs>(opArgs)...));
    }

    void* allocate(size_t size) { return fPool.allocate(size); }

    void release(std::unique_ptr<GrOp> op);

    bool isEmpty() const { return fPool.isEmpty(); }

private:
    GrOpMemoryPool(size_t preallocSize, size_t minAllocSize)
            : fPool(preallocSize - offsetof(GrOpMemoryPool, fPool), minAllocSize) {}

    GrMemoryPool fPool; // Must be the last field
};

#endif
