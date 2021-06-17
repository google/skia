/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrMemoryPool.h"

#include "include/private/SkTPin.h"
#include "src/core/SkASAN.h"

#ifdef SK_DEBUG
    #include <atomic>
#endif

#include <tuple>

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrMemoryPool> GrMemoryPool::Make(size_t preallocSize, size_t minAllocSize) {
    static_assert(sizeof(GrMemoryPool) < GrMemoryPool::kMinAllocationSize);

    preallocSize = SkTPin(preallocSize, kMinAllocationSize,
                          (size_t) GrBlockAllocator::kMaxAllocationSize);
    minAllocSize = SkTPin(minAllocSize, kMinAllocationSize,
                          (size_t) GrBlockAllocator::kMaxAllocationSize);
    void* mem = operator new(preallocSize);
    return std::unique_ptr<GrMemoryPool>(new (mem) GrMemoryPool(preallocSize, minAllocSize));
}

GrMemoryPool::GrMemoryPool(size_t preallocSize, size_t minAllocSize)
        : fAllocator(GrBlockAllocator::GrowthPolicy::kFixed, minAllocSize,
                     preallocSize - offsetof(GrMemoryPool, fAllocator) - sizeof(GrBlockAllocator)) {
    SkDEBUGCODE(fAllocationCount = 0;)
}

GrMemoryPool::~GrMemoryPool() {
    this->reportLeaks();
    SkASSERT(0 == fAllocationCount);
    SkASSERT(this->isEmpty());
}

void GrMemoryPool::reportLeaks() const {
#ifdef SK_DEBUG
    int i = 0;
    int n = fAllocatedIDs.count();
    for (int id : fAllocatedIDs) {
        if (++i == 1) {
            SkDebugf("Leaked %d IDs (in no particular order): %d%s", n, id, (n == i) ? "\n" : "");
        } else if (i < 11) {
            SkDebugf(", %d%s", id, (n == i ? "\n" : ""));
        } else if (i == 11) {
            SkDebugf(", ...\n");
            break;
        }
    }
#endif
}

void* GrMemoryPool::allocate(size_t size) {
    static_assert(alignof(Header) <= kAlignment);
    SkDEBUGCODE(this->validate();)

    GrBlockAllocator::ByteRange alloc = fAllocator.allocate<kAlignment, sizeof(Header)>(size);

    // Initialize GrMemoryPool's custom header at the start of the allocation
    Header* header = static_cast<Header*>(alloc.fBlock->ptr(alloc.fAlignedOffset - sizeof(Header)));
    header->fStart = alloc.fStart;
    header->fEnd = alloc.fEnd;

    // Update live count within the block
    alloc.fBlock->setMetadata(alloc.fBlock->metadata() + 1);

#if defined(SK_SANITIZE_ADDRESS)
    sk_asan_poison_memory_region(&header->fSentinel, sizeof(header->fSentinel));
#elif defined(SK_DEBUG)
    header->fSentinel = GrBlockAllocator::kAssignedMarker;
#endif

#if defined(SK_DEBUG)
    header->fID = []{
        static std::atomic<int> nextID{1};
        return nextID.fetch_add(1, std::memory_order_relaxed);
    }();

    // You can set a breakpoint here when a leaked ID is allocated to see the stack frame.
    fAllocatedIDs.add(header->fID);
    fAllocationCount++;
#endif

    // User-facing pointer is after the header padding
    return alloc.fBlock->ptr(alloc.fAlignedOffset);
}

void GrMemoryPool::release(void* p) {
    Header* header = reinterpret_cast<Header*>(reinterpret_cast<intptr_t>(p) - sizeof(Header));

#if defined(SK_SANITIZE_ADDRESS)
    sk_asan_unpoison_memory_region(&header->fSentinel, sizeof(header->fSentinel));
#elif defined(SK_DEBUG)
    SkASSERT(GrBlockAllocator::kAssignedMarker == header->fSentinel);
    header->fSentinel = GrBlockAllocator::kFreedMarker;
#endif

#if defined(SK_DEBUG)
    fAllocatedIDs.remove(header->fID);
    fAllocationCount--;
#endif

    GrBlockAllocator::Block* block = fAllocator.owningBlock<kAlignment>(header, header->fStart);

#if defined(SK_DEBUG)
    // (p - block) matches the original alignedOffset value from GrBlockAllocator::allocate().
    intptr_t alignedOffset = (intptr_t)p - (intptr_t)block;
    SkASSERT(p == block->ptr(alignedOffset));

    // Scrub the block contents to prevent use-after-free errors.
    memset(p, 0xDD, header->fEnd - alignedOffset);
#endif

    int alive = block->metadata();
    if (alive == 1) {
        // This was last allocation in the block, so remove it
        fAllocator.releaseBlock(block);
    } else {
        // Update count and release storage of the allocation itself
        block->setMetadata(alive - 1);
        block->release(header->fStart, header->fEnd);
    }
}

#ifdef SK_DEBUG
void GrMemoryPool::validate() const {
    fAllocator.validate();

    int allocCount = 0;
    for (const auto* b : fAllocator.blocks()) {
        allocCount += b->metadata();
    }
    SkASSERT(allocCount == fAllocationCount);
    SkASSERT(fAllocationCount == fAllocatedIDs.count());
    SkASSERT(allocCount > 0 || this->isEmpty());
}
#endif
