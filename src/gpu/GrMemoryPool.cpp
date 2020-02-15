/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrMemoryPool.h"

#include "src/gpu/ops/GrOp.h"

#ifdef SK_DEBUG
    #include <atomic>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// GrMemoryPool implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

GrMemoryPool::GrMemoryPool(size_t allocationSize)
        : fAllocator(GrBlockAllocator::GrowthPolicy::kFixed, allocationSize,
                     allocationSize - offsetof(GrMemoryPool, fAllocator)) {
    SkDEBUGCODE(fAllocationCount = 0;)
}

GrMemoryPool::~GrMemoryPool() {
#ifdef SK_DEBUG
    int i = 0;
    int n = fAllocatedIDs.count();
    fAllocatedIDs.foreach([&i, n] (int id) {
        if (++i == 1) {
            SkDebugf("Leaked IDs (in no particular order): %d", id);
        } else if (i < 11) {
            SkDebugf(", %d%s", id, (n == i ? "\n" : ""));
        } else if (i == 11) {
            SkDebugf(", ...\n");
        }
    });
#endif
    SkASSERT(0 == fAllocationCount);
    SkASSERT(this->isEmpty());
}

void* GrMemoryPool::allocate(size_t size) {
    static_assert(alignof(Header) <= kAlignment);
    SkDEBUGCODE(this->validate();)

    GrBlockAllocator::ByteRange alloc = fAllocator.allocate<kAlignment, sizeof(Header)>(size);

    // Initialize header at the start of the reservation
    // GrBlockAllocator::Block* block = fAllocator.currentBlock();
    Header* header = static_cast<Header*>(alloc.fBlock->ptr(alloc.fAlignedOffset - sizeof(Header)));
    header->fBlock = alloc.fBlock;
    header->fStart = alloc.fStart;
    header->fEnd = alloc.fEnd;

    // Update live count within the block
    alloc.fBlock->setMetadata(alloc.fBlock->metadata() + 1);

#ifdef SK_DEBUG
    header->fSentinel = GrBlockAllocator::kAssignedMarker;
    header->fID = []{
        static std::atomic<int> nextID{1};
        return nextID++;
    }();

    // You can set a breakpoint here when a leaked ID is allocated to see the stack frame.
    fAllocatedIDs.add(header->fID);
    fAllocationCount++;
#endif

    // User-facing pointer is after the header padding
    return alloc.fBlock->ptr(alloc.fAlignedOffset);
}

void GrMemoryPool::release(void* p) {
    // NOTE: if we needed it, header - header->fBlock would equal the original alignedOffset value
    // returned by GrBlockAllocator::allocate()
    Header* header = reinterpret_cast<Header*>(reinterpret_cast<uintptr_t>(p) - sizeof(Header));
    SkASSERT(GrBlockAllocator::kAssignedMarker == header->fSentinel);

#ifdef SK_DEBUG
    header->fSentinel = GrBlockAllocator::kFreedMarker;
    fAllocatedIDs.remove(header->fID);
    fAllocationCount--;
#endif

    int alive = header->fBlock->metadata();
    if (alive == 1) {
        // This was last allocation in the block, so remove it
        fAllocator.releaseBlock(header->fBlock);
    } else {
        // Update count and release storage of the allocation itself
        header->fBlock->setMetadata(alive - 1);
        header->fBlock->release(header->fStart, header->fEnd);
    }
}

#ifdef SK_DEBUG
void GrMemoryPool::validate() const {
    fAllocator.validate();

    int allocCount = 0;
    for (const auto* b : GrBlockAllocator::Blocks(&fAllocator)) {
        allocCount += b->metadata();
    }
    SkASSERT(allocCount == fAllocationCount);
    SkASSERT(fAllocationCount == fAllocatedIDs.count());
    SkASSERT(allocCount > 0 || this->isEmpty());
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// GrOpMemoryPool implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

void GrOpMemoryPool::release(std::unique_ptr<GrOp> op) {
    GrOp* tmp = op.release();
    SkASSERT(tmp);
    tmp->~GrOp();
    fPool.release(tmp);
}
