/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMemoryPool_DEFINED
#define GrMemoryPool_DEFINED

#include "src/base/SkBlockAllocator.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

#ifdef SK_DEBUG
#include "src/core/SkTHash.h"
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
    // https://github.com/emscripten-core/emscripten/issues/10072
    // Since Skia does not use "long double" (16 bytes), we should be ok to force it back to 8 bytes
    // until emscripten is fixed.
    static constexpr size_t kAlignment = 8;
#else
    // Guaranteed alignment of pointer returned by allocate().
    static constexpr size_t kAlignment = alignof(std::max_align_t);
#endif

    // Smallest block size allocated on the heap (not the smallest reservation via allocate()).
    inline static constexpr size_t kMinAllocationSize = 1 << 10;

    /**
     * Prealloc size is the amount of space to allocate at pool creation
     * time and keep around until pool destruction. The min alloc size is
     * the smallest allowed size of additional allocations. Both sizes are
     * adjusted to ensure that they are at least as large as kMinAllocationSize
     * and less than SkBlockAllocator::kMaxAllocationSize.
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
        return fAllocator.currentBlock() == fAllocator.headBlock() &&
               fAllocator.currentBlock()->metadata() == 0;
    }

    /**
     * In debug mode, this reports the IDs of unfreed nodes via `SkDebugf`. This reporting is also
     * performed automatically whenever a GrMemoryPool is destroyed.
     * In release mode, this method is a no-op.
     */
    void reportLeaks() const;

    /**
     * Returns the total allocated size of the GrMemoryPool minus any preallocated amount
     */
    size_t size() const { return fAllocator.totalSize() - fAllocator.preallocSize(); }

    /**
     * Returns the preallocated size of the GrMemoryPool
     */
    size_t preallocSize() const {
        // Account for the debug-only fields in this count, the offset is 0 for release builds
        static_assert(std::is_standard_layout<GrMemoryPool>::value, "");
        return offsetof(GrMemoryPool, fAllocator) + fAllocator.preallocSize();
    }

    /**
     * Frees any scratch blocks that are no longer being used.
     */
    void resetScratchSpace() {
        fAllocator.resetScratchSpace();
    }

#ifdef SK_DEBUG
    void validate() const;
#endif

private:
    // Per-allocation overhead so that GrMemoryPool can always identify the block owning each and
    // release all occupied bytes, including any resulting from alignment padding.
    struct Header {
        int fStart;
        int fEnd;
#if defined(SK_DEBUG)
        int fID;       // ID that can be used to track down leaks by clients.
#endif
#if defined(SK_DEBUG) || defined(SK_SANITIZE_ADDRESS)
        uint32_t fSentinel; // set to a known value to check for memory stomping; poisoned in ASAN mode
#endif
    };

    GrMemoryPool(size_t preallocSize, size_t minAllocSize);

#ifdef SK_DEBUG
    // Because this exists preallocSize wants to use offsetof, so keep GrMemoryPool standard layout
    // without depending on THashSet being standard layout. Note that std::unique_ptr may not be
    // standard layout.
    struct Debug{
        skia_private::THashSet<int> fAllocatedIDs;
        int                         fAllocationCount;
    };
    Debug* fDebug{nullptr};
#endif

    SkBlockAllocator fAllocator; // Must be the last field, in order to use extra allocated space
};
#endif
