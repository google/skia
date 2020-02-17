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

std::unique_ptr<GrMemoryPool> GrMemoryPool::Make(size_t preallocSize, size_t minAllocSize) {
    static_assert(sizeof(GrMemoryPool) < GrMemoryPool::kMinAllocationSize);

    preallocSize = std::clamp(preallocSize, kMinAllocationSize,
                              (size_t) GrBlockAllocator::kMaxAllocationSize);
    minAllocSize = std::clamp(minAllocSize, kMinAllocationSize,
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

    // size = GrAlignTo(size, kAlignment);
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

static GrBlockAllocator::Block* find_block(GrBlockAllocator* allocator, void* p, int start, int padding) {
    // SO this actually kind of works well for the push/pop but it absolutely suuuuucks for queue/random
    // access patterns... but if we can't get down to 8 bytes on these platforms, the extra size really seems to suck....
    //
    // Now they only used to track 'last' allocation, so maaaaybe I could switch it to that as well?
    // but that would need to be stored on metadata of the block, and .......... auagugugugu
    // for (const GrBlockAllocator::Block* b : GrBlockAllocator::RBlocks(allocator)) {
    //     if (b->isOwner(p)) {
    //         return const_cast<GrBlockAllocator::Block*>(b);
    //     }
    // }
    // return nullptr;
    if (allocator->currentBlock()->isOwner(p)) {
        return allocator->currentBlock();
    }
    // block + start + offset = p, and start + offset = align((start + 8), 8)
    // int offset = (header->fStart + sizeof(Header) + kAlignment - 1) & ~(kAlignment - 1);
    int offset = (start + padding + GrMemoryPool::kAlignment - 1) & ~(GrMemoryPool::kAlignment - 1);
    // SkDebugf("padding: %d, start: %d, align: %d, mod (start + padding): %d\n",
            // padding, start, GrMemoryPool::kAlignment, (start + padding) % GrMemoryPool::kAlignment);
    // SkDebugf("%d vs. %d\n", offset, padding + start);
    return reinterpret_cast<GrBlockAllocator::Block*>(reinterpret_cast<uintptr_t>(p) - offset);
}

void GrMemoryPool::release(void* p) {
    // NOTE: if we needed it, (p - header->fBlock) would equal the original alignedOffset value
    // returned by GrBlockAllocator::allocate()
    Header* header = reinterpret_cast<Header*>(reinterpret_cast<uintptr_t>(p) - sizeof(Header));
    SkASSERT(GrBlockAllocator::kAssignedMarker == header->fSentinel);

#ifdef SK_DEBUG
    header->fSentinel = GrBlockAllocator::kFreedMarker;
    fAllocatedIDs.remove(header->fID);
    fAllocationCount--;
#endif

    // GrBlockAllocator::Block* block = find_block(&fAllocator, p, header->fStart, sizeof(Header));
    GrBlockAllocator::Block* block = header->fBlock;



    int alive = block->metadata();
    // int alive = header->fBlock->metadata();
    // block->setMetadata(alive - 1);

    if (alive == 1) {
        // This was last allocation in the block, so remove it
        // fAllocator.releaseBlock(header->fBlock);
        fAllocator.releaseBlock(block);
        // SkDebugf("releasing block\n");
    } else {
        // Update count and release storage of the allocation itself
        // header->fBlock->setMetadata(alive - 1);
        // header->fBlock->release(header->fStart, header->fEnd);
        block->setMetadata(alive - 1);
        block->release(header->fStart, header->fEnd);
        // SkDebugf("releasing alloc: %d\n", res);
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

std::unique_ptr<GrOpMemoryPool> GrOpMemoryPool::Make(size_t preallocSize, size_t minAllocSize) {
    static_assert(sizeof(GrOpMemoryPool) < GrMemoryPool::kMinAllocationSize);

    preallocSize = std::clamp(preallocSize, GrMemoryPool::kMinAllocationSize,
                              (size_t) GrBlockAllocator::kMaxAllocationSize);
    minAllocSize = std::clamp(minAllocSize, GrMemoryPool::kMinAllocationSize,
                              (size_t) GrBlockAllocator::kMaxAllocationSize);
    void* mem = operator new(preallocSize);
    return std::unique_ptr<GrOpMemoryPool>(new (mem) GrOpMemoryPool(preallocSize, minAllocSize));
}

void GrOpMemoryPool::release(std::unique_ptr<GrOp> op) {
    GrOp* tmp = op.release();
    SkASSERT(tmp);
    tmp->~GrOp();
    fPool.release(tmp);
}
