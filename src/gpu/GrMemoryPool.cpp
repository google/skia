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

GrMemoryPool::GrMemoryPool(void* headBlock, int blockSize, GrowthPolicy policy)
        : GrBlockAllocator(headBlock, blockSize, policy) {
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

GrMemoryPool::AllocHeader::AllocHeader(Block block, int offset, int size)
        : fBlock(block)
        , fOffset(offset)
        , fSize(size) {
    SkDEBUGCODE(fSentinel = kAssignedMarker;)
    SkDEBUGCODE(fID = []{
        static std::atomic<int> nextID{1};
        return nextID++;
    }();)
}

void* GrMemoryPool::allocate(size_t size) {
    SkDEBUGCODE(this->validate();)

    AllocHeader* header;
    void* alloc = this->GrBlockAllocator::allocate<kAlignment>(size, &header);
    *this->liveCount() += 1;

#ifdef SK_DEBUG
    // You can set a breakpoint here when a leaked ID is allocated to see the stack frame.
    fAllocatedIDs.add(header->fID);
    fAllocationCount++;
#endif
    return alloc;
}

void GrMemoryPool::release(void* p) {
    AllocHeader* header = Metadata<kAlignment, AllocHeader>(p);
    SkASSERT(kAssignedMarker == header->fSentinel);

#ifdef SK_DEBUG
    header->fSentinel = kFreedMarker;
    fAllocatedIDs.remove(header->fID);
    fAllocationCount--;
#endif

    int* alive = this->liveCount(header->fBlock);
    if (*alive == 1) {
        // This was last allocation in the block, so remove it
        this->releaseBlock(header->fBlock);
    } else {
        // Update count and release storage of the allocation itself
        *alive -= 1;
        this->GrBlockAllocator::release(header->fBlock, header->fOffset, header->fSize);
    }
}

#ifdef SK_DEBUG
void GrMemoryPool::validate() const {
    this->GrBlockAllocator::validate();

    int allocCount = 0;
    for (Block b : Blocks(this)) {
        allocCount += *this->liveCount(b);
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
    this->GrMemoryPool::release(tmp);
}
