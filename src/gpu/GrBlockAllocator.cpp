/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrBlockAllocator.h"

GrBlockAllocator::GrBlockAllocator(void* headBlock, int blockSize, GrowthPolicy policy)
        : fHeadOffset(SkTo<uint16_t>(reinterpret_cast<uintptr_t>(headBlock) -
                                     reinterpret_cast<uintptr_t>(this)))
        , fGrowthPolicy(static_cast<uint64_t>(policy))
        , fN0((policy == GrowthPolicy::kLinear || policy == GrowthPolicy::kExponential) ? 1 : 0)
        , fN1(1) {
    SkASSERT(reinterpret_cast<uintptr_t>(headBlock) > reinterpret_cast<uintptr_t>(this));
    SkASSERT(blockSize >= sizeof(BlockHeader0) && blockSize <= MaxBlockSize(alignof(BlockHeader)));
    SkASSERT(fHeadOffset % alignof(BlockHeader) == 0);
    // Initialize the head block (which becomes the current tail), but since it hasn't been init'ed
    // yet, we can't just call head()
    void* headPtr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + fHeadOffset);
    fTail = new (headPtr) BlockHeader(blockSize);
}

GrBlockAllocator::BlockHeader::BlockHeader(int size)
         : fNext(nullptr)
         , fPrev(nullptr)
         , fSize(size)
         , fCursor(sizeof(BlockHeader))
         , fMetadata(0) {
    SkASSERT(size > sizeof(BlockHeader));
    SkDEBUGCODE(fSentinel = kAssignedMarker;)
}

GrBlockAllocator::BlockHeader::~BlockHeader() {
    SkASSERT(fSentinel == kAssignedMarker);
    SkDEBUGCODE(fSentinel = kFreedMarker;) // FWIW
}

size_t GrBlockAllocator::size() const {
    // Use size_t since the sum across all blocks could exceed 'int', even though each block won't
    size_t size = fHeadOffset;
    for (Block b : Blocks(this)) {
        size += BlockToHeader(b)->fSize;
    }
    SkASSERT(size >= this->preallocSize());
    return size;
}

size_t GrBlockAllocator::used() const {
    // Use size_t for same reason as in size()
    size_t used = fHeadOffset;
    for (Block b : Blocks(this)) {
        used += BlockToHeader(b)->fCursor;
    }
    SkASSERT(used <= this->size());
    return used;
}

void GrBlockAllocator::releaseBlock(Block block) {
    BlockHeader* blockPtr = BlockToHeader(block);

    if (blockPtr->fPrev) {
        // Unlink blockPtr from the double-linked list of blocks
        SkASSERT(blockPtr != this->head());
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
        SkASSERT(blockPtr == this->head());
        blockPtr->fCursor = sizeof(BlockHeader);
        blockPtr->fMetadata = 0;
        // Unlike in reset(), we don't set the head's next block to null because there are
        // potentially heap-allocated blocks that are still connected to it.
    }
}

void GrBlockAllocator::reset() {
    // We can't use the RBlocks for-range since we're destroying the linked list as we go
    BlockHeader* toFree = fTail;
    while(toFree) {
        BlockHeader* prev = toFree->fPrev;
        if (prev) {
            SkASSERT(toFree != this->head());
            delete toFree;
        } else {
            // Invoke dtor of the head block, but don't 'delete' it since it's embedded with this
            SkASSERT(toFree == this->head());
            // This ensures that if a subclass calls reset(), then GrBlockAllocator's dtor will not
            // try to re-delete everything
            fTail = toFree;
            // the head block's prev is already null, it's next block was deleted last iteration
            fTail->fNext = nullptr;
            fTail->fCursor = sizeof(BlockHeader);
            fTail->fMetadata = 0;
        }

        toFree = prev;
    }
}

void GrBlockAllocator::addBlock(int minimumSize, int maxSize) {
    SkASSERT(minimumSize > sizeof(BlockHeader) && minimumSize <= maxSize);
    // Calculate the 'next' size per growth policy sequence
    static constexpr int kMaxN = (1 << 22) - 1; // Max positive value for uint:23 storage
    static_assert(2 * kMaxN <= std::numeric_limits<int32_t>::max()); // Growth policy won't overflow

    GrowthPolicy gp = static_cast<GrowthPolicy>(fGrowthPolicy);
    int nextN1 = fN0 + fN1;
    int nextN0;
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

    // However, must guard against overflow here, since the all the size-based asserts prevented
    // alignment/addition overflows, while multiplication requires x^2 precision instead of 2x.
    int initialSize = this->head()->fSize + fHeadOffset;
    int allocSize;
    if (maxSize / initialSize < nextN1) {
        // The growth policy would overflow, so use the max. We've already confirmed that maxSize
        // will be sufficient for the requested minimumSize
        allocSize = maxSize;
    } else {
        allocSize = std::max(minimumSize, initialSize * nextN1);
        // Then round to a nice boundary since the block isn't maxing out:
        //   if allocSize > 32K, aligns on 4K boundary otherwise aligns on max_align_t, to play
        //   nicely with jeMalloc (from SkArenaAlloc).
        int mask = allocSize > (1 << 15) ? (1 << 12) - 1 : alignof(std::max_align_t) - 1;
        allocSize = std::min((allocSize + mask) & ~mask, maxSize);
    }

    // Create new block and append to the linked list of blocks in this allocator
    void* mem = operator new(allocSize);
    BlockHeader* next = new (mem) BlockHeader(allocSize);

    fTail->fNext = next;
    next->fPrev = fTail;
    fTail = next;
}

bool GrBlockAllocator::resize(Block block, int offset, int size, size_t newSize) {
    BlockHeader* bp = BlockToHeader(block);
    // 'offset' and 'size' should have come from a prior call to allocate() w/in this block
    SkASSERT(offset >= sizeof(BlockHeader) && offset + size <= bp->fSize);

    if (newSize > kMaxAllocationSize + kMaxAllocMetadataSize) {
        // Cannot possibly satisfy the resize and could overflow subsequent math
        return false;
    }
    if (bp->fCursor == offset + size) {
        // The last allocation can be shifted assuming newSize fits in the size of the block
        int nextCursor = offset + (int) newSize;
        if (nextCursor < bp->fSize) {
            bp->fCursor = nextCursor;
            return true;
        }
    }
    // Not enough room, or not the last allocation so couldn't grow the reserved space
    return false;
}

// NOTE: release is equivalent to resize(block, offset, size, 0), and the compiler can optimize
// most of the additions away, but it wasn't able to remove the unnecessary branch comparing the
// new cursor to the block size, so release() gets a specialization.
bool GrBlockAllocator::release(Block block, int offset, int size) {
    BlockHeader* bp = BlockToHeader(block);
    // 'offset' and 'size' should have come from a prior call to allocate() w/in this block
    SkASSERT(offset >= sizeof(BlockHeader) && offset + size <= bp->fSize);
    if (bp->fCursor == offset + size) {
        bp->fCursor = offset;
        return true;
    } else {
        return false;
    }
}

#ifdef SK_DEBUG
int GrBlockAllocator::validate() const {
    std::vector<Block> blocks;
    const BlockHeader* prev = nullptr;
    for (Block b : Blocks(this)) {
        blocks.push_back(b);

        const BlockHeader* block = BlockToHeader(b);
        SkASSERT(kAssignedMarker == block->fSentinel);
        SkASSERT(prev == block->fPrev);
        if (prev) {
            SkASSERT(prev->fNext == block);
        }

        SkASSERT(block->fSize >= sizeof(BlockHeader));
        SkASSERT(block->fCursor >= sizeof(BlockHeader));
        SkASSERT(block->fCursor <= block->fSize);

        prev = block;
    }
    SkASSERT(prev == fTail);
    SkASSERT(blocks.size() > 0);
    SkASSERT(BlockToHeader(blocks[0]) == this->head());

    // Confirm reverse iteration matches forward iteration
    size_t j = blocks.size() - 1;
    for (Block b : RBlocks(this)) {
        SkASSERT(b == blocks[j]);
        j--;
    }
    SkASSERT(j == 0);
}
#endif
