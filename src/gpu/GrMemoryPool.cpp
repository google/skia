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
// GrBlockAllocator implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

GrBlockAllocator::GrBlockAllocator(void* preallocStart, uint32_t preallocSize, GrowthPolicy policy,
                                   uint8_t blockMetadataBytes)
        : fHeadOffset(SkTo<uint8_t>(reinterpret_cast<uintptr_t>(preallocStart) -
                                    reinterpret_cast<uintptr_t>(this)))
        , fMetadataBytes(SkTo<uint8_t>(GrSizeAlignUp(blockMetadataBytes, alignof(BlockHeader))))
        , fGrowthPolicy(static_cast<uint64_t>(policy))
        , fN0((policy == GrowthPolicy::kLinear || policy == GrowthPolicy::kExponential) ? 1 : 0)
        , fN1(1) {
    SkASSERT(fHeadOffset % alignof(BlockHeader) == 0);
    // Initialize the head block (which becomes the current tail), but since it hasn't been init'ed
    // yet, we can't just call head()
    void* headPtr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + fHeadOffset);
    fTail = new (headPtr) BlockHeader(SkTo<uint32_t>(preallocSize - fHeadOffset), fMetadataBytes);
}

GrBlockAllocator::~GrBlockAllocator() {
    // GrBlockAllocator assumes that any blocks remaining to be deleted are POD so the per-block
    // clean-up is a no-op.
    this->releaseAllBlocks([](Block b, int) { return 0; });
}

GrBlockAllocator::BlockHeader::BlockHeader(uint32_t size, uint8_t metadataBytes)
         : fNext(nullptr)
         , fPrev(nullptr)
         , fSize(size)
         , fCursor(sizeof(BlockHeader) + metadataBytes) {
    SkASSERT(size > sizeof(BlockHeader) + metadataBytes);
    SkDEBUGCODE(fSentinel = kAssignedMarker;)
    if (metadataBytes > 0) {
        memset(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + sizeof(BlockHeader)),
               0, metadataBytes);
    }
}

GrBlockAllocator::BlockHeader::~BlockHeader() {
    SkASSERT(fSentinel == kAssignedMarker);
    SkDEBUGCODE(fSentinel = kFreedMarker;) // FWIW
}

uint32_t GrBlockAllocator::BlockHeader::cursor(uint8_t align, uint8_t allocMetadataBytes) const {
    return GrSizeAlignUp(fCursor + allocMetadataBytes, align);
}

void GrBlockAllocator::releaseBlock(Block block) {
    BlockHeader* blockPtr = BlockToHeader(block);
    BlockHeader* head = this->head();

    if (head != blockPtr) {
        // Unlink blockPtr from the double-linked list of blocks
        SkASSERT(blockPtr->fPrev);
        blockPtr->fPrev->fNext = blockPtr->fNext;
        if (blockPtr->fNext) {
            SkASSERT(fTail != blockPtr);
            blockPtr->fNext->fPrev = blockPtr->fPrev;
        } else {
            SkASSERT(fTail == blockPtr);
            fTail = blockPtr->fPrev;
        }

        delete blockPtr;
    } else {
        // Reset the cursor of the head block so that it can be reused
        blockPtr->fCursor = sizeof(BlockHeader) + fMetadataBytes;
        if (fMetadataBytes > 0) {
            memset(reinterpret_cast<void*>(block + sizeof(BlockHeader)), 0, fMetadataBytes);
        }
    }
}

GrBlockAllocator::Block GrBlockAllocator::addBlock(uint32_t minimumSize, uint8_t align, uint8_t allocMetadataBytes) {
    static constexpr uint32_t kMaxN = (1 << 22) - 1;
    static constexpr uint32_t kMaxAllocation = UINT32_MAX; //(1 << 31) - 1;

    // Calculate the 'next' size per growth sequence
    GrowthPolicy gp = static_cast<GrowthPolicy>(fGrowthPolicy);
    uint32_t nextN1 = fN0 + fN1;
    uint32_t nextN0;
    if (gp == GrowthPolicy::kFixed || gp == GrowthPolicy::kLinear) {
        nextN0 = fN0;
    } else if (gp == GrowthPolicy::kFibonacci) {
        nextN0 = fN1;
    } else {
        SkASSERT(gp == GrowthPolicy::kExponential);
        nextN0 = nextN1;
    }
    fN0 = std::min(kMaxN, nextN0);
    fN1 = std::min(kMaxN, nextN1);

    // FIXME may be much cleaner to allow it to go to size_t in this case, but that really only
    // makes the code valid if size_t > uint32_t. Otherwise we'd have the same overflow issues to deal with.
    // So maybe I just accept the slightly higher branching.
    uint32_t allocSize;
    uint32_t preallocSize = SkTo<uint32_t>(this->preallocSize());
    if (kMaxAllocation / preallocSize < nextN1) {
        allocSize = kMaxAllocation;
    } else {
        allocSize = preallocSize * nextN1;
    }
    // size_t allocSize = this->preallocSize() * nextN1;

    // Ensure the block will satisfy the minimum size for the requested allocation
    // FIXME assert this one...
    // uint32_t requestedSize = GrUIAlignUp(minimumSize + allocMetadataBytes + sizeof(BlockHeader) + fMetadataBytes, align);
    uint32_t requestedSize = GrPow2AlignUp(minimumSize + allocMetadataBytes + sizeof(BlockHeader) + fMetadataBytes, align - 1);
    allocSize = std::max(allocSize, requestedSize);

    // Round up to a nice allocation number, and play nicely with jeMalloc;
    // if allocSize > 32K, aligns on 4K boundary otherwise aligns on max_align_t (from SkArenaAlloc)
    uint32_t mask = allocSize > (1 << 15) ? (1 << 12) - 1 : alignof(std::max_align_t) - 1;
    if (allocSize > kMaxAllocation - mask) {
        allocSize = kMaxAllocation;
    } else {
        allocSize = (allocSize + mask) & ~mask;
    }

    // Clamp to 32-bit allocation limit
    // allocSize = std::min(kMaxAllocation, allocSize);
    SkASSERT(allocSize >= requestedSize);

    // Create new block in allocSize and append to the linked list of blocks in this allocator
    void* mem = operator new(allocSize);
    BlockHeader* next = new (mem) BlockHeader(allocSize, fMetadataBytes);

    fTail->fNext = next;
    next->fPrev = fTail;
    fTail = next;
    return reinterpret_cast<Block>(next);
}

void GrBlockAllocator::releaseAllBlocks(VisitBlockProc dtor) {
    int ct = 0;
    BlockHeader* toFree = fTail;
    while(toFree) {
        BlockHeader* prev = toFree->fPrev;
        ct = dtor(reinterpret_cast<Block>(toFree), ct);
        if (prev) {
            SkASSERT(toFree != this->head());
            delete toFree;
        } else {
            // Invoke dtor of the head block, but don't 'delete' it since it's embedded with this
            SkASSERT(toFree == this->head());
            toFree->~BlockHeader();
        }

        toFree = prev;
    }
    // This ensures that if a subclass calls releaseAllBlocks(), then GrBlockAllocator's dtor will
    // not try to re-delete everything.
    fTail = nullptr;
}

size_t GrBlockAllocator::size() const {
    size_t size = fHeadOffset;
    const BlockHeader* b = this->head();
    while(b) {
        size += b->fSize;
        b = b->fNext;
    }
    SkASSERT(size >= this->preallocSize());
    return size;
}

size_t GrBlockAllocator::used() const {
    size_t used = fHeadOffset;
    const BlockHeader* b = this->head();
    while(b) {
        used += b->fCursor;
        b = b->fNext;
    }
    SkASSERT(used <= this->size());
    return used;
}

uint32_t GrBlockAllocator::avail(Block block, uint8_t align, uint8_t allocMetadataBytes) const {
    const BlockHeader* blockPtr = BlockToHeader(block);
    // size_t alignedCursor = blockPtr->cursor(align, allocMetadataBytes);
    // uint32_t alignPadding = GrUIAlignUpPad(fTail->fCursor, align);
    uint32_t alignPadding = GrPow2AlignUpPad(fTail->fCursor, align - 1);
    if (allocMetadataBytes > alignPadding) {
        // alignPadding += GrUIAlignUp(allocMetadataBytes - alignPadding, align);
        alignPadding += GrPow2AlignUp(allocMetadataBytes - alignPadding, align - 1);
    }

    // if (alignedCursor < blockPtr->fSize) {
    if (fTail->fCursor < fTail->fSize - alignPadding) {
        return blockPtr->fSize - fTail->fCursor - alignPadding;
        // return blockPtr->fSize - SkTo<uint32_t>(alignedCursor);
    } else {
        return 0;
    }
}

bool GrBlockAllocator::resize(Block block, void* p, uint32_t currentSize, uint32_t requestedSize) {
    BlockHeader* blockPtr = BlockToHeader(block);

    // Can only resize if 'p' is the last allocation from the block.
    if (reinterpret_cast<uintptr_t>(p) + currentSize == block + blockPtr->fCursor) {
        // Can resize if the requested size is less than the currentSize + avail, but since p
        // has already been aligned and reserved its allocation metadata, there's no need to
        // take that into account a second time.
        uint32_t maxResize = (blockPtr->fSize - blockPtr->fCursor) + currentSize;
        if (requestedSize <= maxResize) {
            // Not UB, since each shift fits into 32 bits
            blockPtr->fCursor -= currentSize;
            blockPtr->fCursor += requestedSize;
            return true;
        }
    }
    return false;
}

void* GrBlockAllocator::allocate(uint32_t size, uint8_t align, uint8_t allocMetadataBytes) {
    // SkASSERT(SkTFitsIn<uint32_t>(size + GrSizeAlignUp(allocMetadataBytes, align)));

    // size_t alignedCursor = fTail->cursor(align, allocMetadataBytes);
    uint32_t alignedCursor = GrPow2AlignUp(fTail->fCursor + allocMetadataBytes, align - 1);
    if (alignedCursor + size > fTail->fSize) {
    //     // Must add a new block
        this->addBlock(size, align, allocMetadataBytes);
        // alignedCursor = fTail->cursor(align, allocMetadataBytes);
        alignedCursor = GrPow2AlignUp(fTail->fCursor + allocMetadataBytes, align - 1);
    }
    // SkASSERT(SkTFitsIn<uint32_t>(alignedCursor + size));
    // SkASSERT(fTail->fSize - SkTo<uint32_t>(alignedCursor) >= SkTo<uint32_t>(size));
    // // Add some asserts here

    fTail->fCursor = alignedCursor + size;
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(fTail) + alignedCursor);

    // uint32_t alignPadding = GrUIAlignUpPad(fTail->fCursor, align);
    // uint32_t alignPadding = GrPow2AlignUpPad(fTail->fCursor, align - 1);
    // if (allocMetadataBytes > alignPadding) {
        // alignPadding += GrUIAlignUp(allocMetadataBytes - alignPadding, align);
        // alignPadding += GrPow2AlignUp(allocMetadataBytes - alignPadding, align - 1);
    // }

    // uint32_t remaining = fTail->fSize - size;
    // if (size > fTail->fSize ||
        // fTail->fCursor > fTail->fSize - alignPadding ||
        // fTail->fCursor + alignPadding > fTail->fSize - size) {
        // Need a new block
        // this->addBlock(size, align, allocMetadataBytes);
        // alignPadding = GrUIAlignUpPad(fTail->fCursor, align);
        // alignPadding = GrPow2AlignUpPad(fTail->fCursor, align - 1);
        // if (allocMetadataBytes > alignPadding) {
            // alignPadding += GrUIAlignUp(allocMetadataBytes - alignPadding, align);
            // alignPadding += GrPow2AlignUp(allocMetadataBytes - alignPadding, align - 1);
        // }
    // }

    // uint32_t alignCursor = fTail->fCursor + alignPadding;
    // fTail->fCursor = alignCursor + size;
    // return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(fTail) + alignCursor);
}

#ifdef SK_DEBUG
int GrBlockAllocator::validate(VisitBlockProc visitor) const {
    const BlockHeader* block = this->head();
    const BlockHeader* prev = nullptr;

    int ct = 0;
    do {
        SkASSERT(kAssignedMarker == block->fSentinel);
        SkASSERT(prev == block->fPrev);
        if (prev) {
            SkASSERT(prev->fNext == block);
        }

        SkASSERT(block->fCursor >= sizeof(BlockHeader) + fMetadataBytes);
        SkASSERT(block->fCursor <= block->fSize);

        ct = visitor(reinterpret_cast<Block>(block), ct);

        prev = block;
    } while ((block = block->fNext));
    SkASSERT(prev == fTail);
    return ct;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// GrArenaAlloc implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// GrMemoryPool implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

GrMemoryPool::GrMemoryPool(void* preallocStart, uint32_t preallocSize)
        : GrBlockAllocator(preallocStart, preallocSize, GrowthPolicy::kFixed, sizeof(LiveCount)) {
    SkDEBUGCODE(fAllocationCount = 0;)
    SkDEBUGCODE(this->validate();)
};

GrMemoryPool::~GrMemoryPool() {
#ifdef SK_DEBUG
    this->validate();

    int i = 0;
    int n = fAllocatedIDs.count();
    fAllocatedIDs.foreach([&i, n] (int32_t id) {
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
};

void* GrMemoryPool::allocate(size_t size) {
    SkDEBUGCODE(this->validate();)

    void* alloc = this->GrBlockAllocator::allocate(size, kAlignment, sizeof(AllocHeader));
    // Update metadata for the block and the allocation
    Block block = this->currentBlock();
    (*BlockMetadata<LiveCount>(block)) += 1;

    AllocHeader allocHeader;
#ifdef SK_DEBUG
    allocHeader.fSentinel = kAssignedMarker;
    allocHeader.fID = []{
        static std::atomic<int32_t> nextID{1};
        return nextID++;
    }();
    // You can set a breakpoint here when a leaked ID is allocated to see the stack frame.
    fAllocatedIDs.add(allocHeader.fID);
    fAllocationCount++;
#endif

    // All blocks fit into 32-bits so this should never fail. We stash the offset to the block ptr
    // so we can decrement the live count on delete in constant time.
    allocHeader.fOffset = SkTo<uint32_t>(reinterpret_cast<uintptr_t>(alloc) - block);
    allocHeader.fSize = size;
    SetMetadata(alloc, allocHeader);

    SkDEBUGCODE(this->validate();)
    return alloc;
}

void GrMemoryPool::release(void* p) {
    SkDEBUGCODE(this->validate();)

    AllocHeader allocHeader = GetMetadata<AllocHeader>(p);
    SkASSERT(kAssignedMarker == allocHeader.fSentinel);

#ifdef SK_DEBUG
    allocHeader.fSentinel = kFreedMarker;
    SetMetadata(p, allocHeader);
    fAllocatedIDs.remove(allocHeader.fID);
    fAllocationCount--;
#endif

    // Reconstruct the block that this allocation was part of
    Block block = reinterpret_cast<uintptr_t>(p) - allocHeader.fOffset;

    int* liveCount = BlockMetadata<LiveCount>(block);
    (*liveCount) -= 1;
    if (!(*liveCount)) {
        // Block can be freed too
        this->releaseBlock(block);
    } else {
        // Try to trivially reclaim space at the end of the block
        this->GrBlockAllocator::release(block, p, allocHeader.fSize);
    }

    SkDEBUGCODE(this->validate();)
}

#ifdef SK_DEBUG
void GrMemoryPool::validate() const {
    int allocCount = this->GrBlockAllocator::validate([](Block b, int ct) {
        const int* liveCount = BlockMetadata<LiveCount>(b);
        return ct + *liveCount;
    });
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
