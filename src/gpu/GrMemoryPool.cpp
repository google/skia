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

namespace {

int64_t pow2_align64(int64_t x, int64_t align) {
    SkASSERT(SkIsPow2(align));
    return (x + (align - 1)) & ~(align - 1);
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GrBlockAllocator implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

GrBlockAllocator::GrBlockAllocator(void* preallocStart, int preallocSize, GrowthPolicy policy,
                                   int blockMetadataBytes)
        : fHeadOffset(SkTo<uint8_t>(reinterpret_cast<uintptr_t>(preallocStart) -
                                    reinterpret_cast<uintptr_t>(this)))
        , fMetadataBytes(SkTo<uint8_t>(GrSizeAlignUp(blockMetadataBytes, alignof(BlockHeader))))
        , fGrowthPolicy(static_cast<uint64_t>(policy))
        , fN0((policy == GrowthPolicy::kLinear || policy == GrowthPolicy::kExponential) ? 1 : 0)
        , fN1(1) {
    SkASSERT(reinterpret_cast<uintptr_t>(preallocStart) > reinterpret_cast<uintptr_t>(this));
    SkASSERT(preallocSize >= kMinAllocationSize && preallocSize <= kMaxAllocationSize);
    SkASSERT(fHeadOffset % alignof(BlockHeader) == 0);
    // Initialize the head block (which becomes the current tail), but since it hasn't been init'ed
    // yet, we can't just call head()
    void* headPtr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + fHeadOffset);
    fTail = new (headPtr) BlockHeader(preallocSize - fHeadOffset, fMetadataBytes);
}

GrBlockAllocator::~GrBlockAllocator() {
    // GrBlockAllocator assumes that any blocks remaining to be deleted are POD so the per-block
    // clean-up is a no-op.
    this->releaseAllBlocks([](Block b, int) { return 0; });
}

GrBlockAllocator::BlockHeader::BlockHeader(int size, int metadataBytes)
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

int GrBlockAllocator::BlockHeader::cursor(int align, int allocMetadataBytes) const {
    SkASSERT(align > 0 && SkIsPow2(align) && allocMetadataBytes >= 0);
    // This will safely go below 0 w/o UB if there's not enough room for align+metadata
    int invertedCursor = (fSize - fCursor - allocMetadataBytes) & ~(align - 1);
    if (invertedCursor > 0) {
        // Sanity check with higher precision
        SkASSERT(pow2_align64((int64_t) fCursor + (int64_t) allocMetadataBytes, align)
                 == ((int64_t) fSize - (int64_t) invertedCursor));
        return fSize - invertedCursor;
    } else {
        SkASSERT(pow2_align64((int64_t) fCursor + (int64_t) allocMetadataBytes, align) > fSize);
        return fSize;
}

size_t GrBlockAllocator::size() const {
    // Use size_t since the sum across all blocks could exceed 'int', even though each block won't
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
    // Use size_t for same reason as in size()
    size_t used = fHeadOffset;
    const BlockHeader* b = this->head();
    while(b) {
        used += b->fCursor;
        b = b->fNext;
    }
    SkASSERT(used <= this->size());
    return used;
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

GrBlockAllocator::Block GrBlockAllocator::addBlock(int minimumSize, int align, int allocMetadataBytes) {
    // NOTE: unlike allocate(), avail(), and resize() where efforts are taken to remain safely in
    // 'int' land, this does all math with higher precision to keep logic simple. The actual
    // malloc cost of the block will significantly outweigh this extra numeric cost.
    static_assert(std::numeric_limits<int64_t>::max() >= 2 * kMaxAllocationSize);
    SkASSERT(minimumSize > 0 && allocMetadataBytes >= 0);

    int64_t allocSize = minimumSize + pow2_align64(sizeof(BlockHeader) + fMetadataBytes + allocMetadataBytes, align);
    if (allocSize > kMaxAllocationSize) {
        // There is no possible way for the allocate() request to fit in the block limits
        abort();
    }

    // Calculate the 'next' size per growth policy sequence
    {
        static constexpr int kMaxN = (1 << 22) - 1;
        GrowthPolicy gp = static_cast<GrowthPolicy>(fGrowthPolicy);
        int64_t nextN1 = fN0 + fN1;
        int64_t nextN0;
        if (gp == GrowthPolicy::kFixed || gp == GrowthPolicy::kLinear) {
            nextN0 = fN0;
        } else if (gp == GrowthPolicy::kFibonacci) {
            nextN0 = fN1;
        } else {
            SkASSERT(gp == GrowthPolicy::kExponential);
            nextN0 = nextN1;
        }
        fN0 = std::min(kMaxN, nextN0); // FIXME need casts here?
        fN1 = std::min(kMaxN, nextN1);

        allocSize = std::max(allocSize, this->preallocSize() * nextN1);
        // Then round to a nice boundary since the block isn't maxing out:
        //   if allocSize > 32K, aligns on 4K boundary otherwise aligns on max_align_t, to play
        //   nicely with jeMalloc (from SkArenaAlloc).
        int64_t mask = allocSize > (1 << 15) ? (1 << 12) - 1 : alignof(std::max_align_t) - 1;
        allocSize = std::min((allocSize + mask) & ~mask, kMaxAllocationSize);
    }

    // Create new block and append to the linked list of blocks in this allocator
    void* mem = operator new(allocSize);
    BlockHeader* next = new (mem) BlockHeader(SkTo<int>(allocSize), fMetadataBytes);

    fTail->fNext = next;
    next->fPrev = fTail;
    fTail = next;
    return reinterpret_cast<Block>(next);
}

void* GrBlockAllocator::allocate(int size, int align, int allocMetadataBytes) {
    SkASSERT(size > 0);

    int alignedCursor = fTail->cursor(align, allocMetadataBytes);
    if (fTail->fSize - alignedCursor < size) {
        // Not enough room in current block
        this->addBlock(size, align, allocMetadataBytes);
        alignedCursor = fTail->cursor(align, allocMetadataBytes);
    }
    SkASSERT(fTail->fSize - alignedCursor >= size);
    fTail->fCursor = alignedCursor + size;
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(fTail) + alignedCursor);
}

bool GrBlockAllocator::resize(Block block, void* p, int currentSize, int requestedSize) {
    SkASSERT(currentSize > 0 && requestedSize >= 0);
    BlockHeader* blockPtr = BlockToHeader(block);

    // Can only resize if 'p' is the last allocation from the block.
    if (reinterpret_cast<uintptr_t>(p) + currentSize == block + blockPtr->fCursor) {
        // Can resize if the requested size is less than the currentSize + avail, but since p
        // has already been aligned and reserved its allocation metadata, there's no need to
        // take that into account a second time, or worry about UB.
        int maxResize = (blockPtr->fSize - blockPtr->fCursor) + currentSize;
        if (requestedSize <= maxResize) {
            // Not UB, since each shift fits leaves fCursor <= fSize
            blockPtr->fCursor -= currentSize;
            blockPtr->fCursor += requestedSize;
            SkASSERT(blockPtr->fCursor >= 0 && blockPtr->fCursor <= blockPtr->fSize);
            return true;
        }
    }
    return false;
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

GrMemoryPool::GrMemoryPool(void* preallocStart, int preallocSize)
        : GrBlockAllocator(preallocStart, preallocSize, GrowthPolicy::kFixed, sizeof(LiveCount)) {
    SkDEBUGCODE(fAllocationCount = 0;)
    SkDEBUGCODE(this->validate();)
};

GrMemoryPool::~GrMemoryPool() {
#ifdef SK_DEBUG
    this->validate();

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
        this->GrBlockAllocator::release(block, p, allocHeader.fSize, sizeof(AllocHeaders));
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
