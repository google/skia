/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrBlockAllocator.h"

#ifdef SK_DEBUG
#include <vector>
#endif

GrBlockAllocator::GrBlockAllocator(GrowthPolicy policy, size_t blockIncrementBytes,
                                   size_t additionalPreallocBytes)
        : fTail(&fHead)
        // Round up to the nearest max-aligned value, and then divide so that fBlockSizeIncrement
        // can effectively fit higher byte counts in its 16 bits of storage
        , fBlockIncrement(SkTo<uint16_t>(GrAlignTo(blockIncrementBytes, kBlockIncrementUnits)
                                                / kBlockIncrementUnits))
        , fGrowthPolicy(static_cast<uint64_t>(policy))
        , fN0((policy == GrowthPolicy::kLinear || policy == GrowthPolicy::kExponential) ? 1 : 0)
        , fN1(1)
        // The head block always fills remaiing space from GrBlockAllocator's size, because it's
        // inline, but can take over the specified number of bytes immediately after it.
        , fHead(nullptr, additionalPreallocBytes + BaseHeadBlockSize()) {
    SkASSERT(fBlockIncrement >= 1);
    SkASSERT(additionalPreallocBytes <= kMaxAllocationSize);
}

GrBlockAllocator::Block::Block(Block* prev, int allocationSize)
         : fNext(nullptr)
         , fPrev(prev)
         , fSize(allocationSize)
         , fCursor(kDataStart)
         , fMetadata(0) {
    SkASSERT(allocationSize >= (int) sizeof(Block));
    SkDEBUGCODE(fSentinel = kAssignedMarker;)
}

GrBlockAllocator::Block::~Block() {
    SkASSERT(fSentinel == kAssignedMarker);
    SkDEBUGCODE(fSentinel = kFreedMarker;) // FWIW
}

size_t GrBlockAllocator::totalSize() const {
    // Use size_t since the sum across all blocks could exceed 'int', even though each block won't
    size_t size = offsetof(GrBlockAllocator, fHead);
    for (const Block* b : this->blocks()) {
        size += b->fSize;
    }
    SkASSERT(size >= this->preallocSize());
    return size;
}

size_t GrBlockAllocator::totalUsableSpace() const {
    size_t size = 0;
    for (const Block* b : this->blocks()) {
        size += (b->fSize - kDataStart);
    }
    SkASSERT(size >= this->preallocUsableSpace());
    return size;
}

size_t GrBlockAllocator::totalSpaceInUse() const {
    size_t size = 0;
    for (const Block* b : this->blocks()) {
        size += (b->fCursor - kDataStart);
    }
    SkASSERT(size <= this->totalUsableSpace());
    return size;
}

GrBlockAllocator::Block* GrBlockAllocator::findOwningBlock(const void* p) {
    // When in doubt, search in reverse to find an overlapping block.
    uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
    for (Block* b : this->rblocks()) {
        uintptr_t lowerBound = reinterpret_cast<uintptr_t>(b) + kDataStart;
        uintptr_t upperBound = reinterpret_cast<uintptr_t>(b) + b->fSize;
        if (lowerBound <= ptr && ptr < upperBound) {
            SkASSERT(b->fSentinel == kAssignedMarker);
            return b;
        }
    }
    return nullptr;
}

void GrBlockAllocator::releaseBlock(Block* block) {
    if (block->fPrev) {
        // Unlink block from the double-linked list of blocks
        SkASSERT(block != &fHead);
        block->fPrev->fNext = block->fNext;
        if (block->fNext) {
            SkASSERT(fTail != block);
            block->fNext->fPrev = block->fPrev;
        } else {
            SkASSERT(fTail == block);
            fTail = block->fPrev;
        }

        delete block;
    } else {
        // Reset the cursor of the head block so that it can be reused
        SkASSERT(block == &fHead);
        block->fCursor = kDataStart;
        block->fMetadata = 0;
        // Unlike in reset(), we don't set the head's next block to null because there are
        // potentially heap-allocated blocks that are still connected to it.
    }

    // Decrement growth policy (opposite of addBlock()'s increment operations)
    GrowthPolicy gp = static_cast<GrowthPolicy>(fGrowthPolicy);
    if (fN0 > 0 && (fN1 > 1 || gp == GrowthPolicy::kFibonacci)) {
        SkASSERT(gp != GrowthPolicy::kFixed); // fixed never needs undoing, fN0 always is 0
        if (gp == GrowthPolicy::kLinear) {
            fN1 = fN1 - fN0;
        } else if (gp == GrowthPolicy::kFibonacci) {
            // Subtract n0 from n1 to get the prior 2 terms in the fibonacci sequence
            int temp = fN1 - fN0; // yields prior fN0
            fN1 = fN1 - temp;     // yields prior fN1
            fN0 = temp;
        } else {
            SkASSERT(gp == GrowthPolicy::kExponential);
            // Divide by 2 to undo the 2N update from addBlock
            fN1 = fN1 >> 1;
            fN0 = fN1;
        }
    }

    SkASSERT(fN1 >= 1 && fN0 >= 0);
}

void GrBlockAllocator::reset() {
    // We can't use the RBlocks for-range since we're destroying the linked list as we go
    Block* toFree = fTail;
    while(toFree) {
        Block* prev = toFree->fPrev;
        if (prev) {
            SkASSERT(toFree != &fHead);
            delete toFree;
        } else {
            SkASSERT(toFree == &fHead);
            fTail = toFree;
            // the head block's prev is already null, it's next block was deleted last iteration
            fTail->fNext = nullptr;
            fTail->fCursor = kDataStart;
            fTail->fMetadata = 0;
        }

        toFree = prev;
    }

    GrowthPolicy gp = static_cast<GrowthPolicy>(fGrowthPolicy);
    fN0 = (gp == GrowthPolicy::kLinear || gp == GrowthPolicy::kExponential) ? 1 : 0;
    fN1 = 1;
}

void GrBlockAllocator::addBlock(int minimumSize, int maxSize) {
    SkASSERT(minimumSize > (int) sizeof(Block) && minimumSize <= maxSize);

    // Max positive value for uint:23 storage (decltype(fN0) picks up uint64_t, not uint:23).
    static constexpr int kMaxN = (1 << 23) - 1;
    static_assert(2 * kMaxN <= std::numeric_limits<int32_t>::max()); // Growth policy won't overflow

    // Calculate the 'next' size per growth policy sequence
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

    // However, must guard against overflow here, since all the size-based asserts prevented
    // alignment/addition overflows, while multiplication requires 2x bits instead of x+1.
    int sizeIncrement = fBlockIncrement * kBlockIncrementUnits;
    int allocSize;
    if (maxSize / sizeIncrement < nextN1) {
        // The growth policy would overflow, so use the max. We've already confirmed that maxSize
        // will be sufficient for the requested minimumSize
        allocSize = maxSize;
    } else {
        allocSize = std::max(minimumSize, sizeIncrement * nextN1);
        // Then round to a nice boundary since the block isn't maxing out:
        //   if allocSize > 32K, aligns on 4K boundary otherwise aligns on max_align_t, to play
        //   nicely with jeMalloc (from SkArenaAlloc).
        int mask = allocSize > (1 << 15) ? ((1 << 12) - 1) : (kBlockIncrementUnits - 1);
        allocSize = std::min((allocSize + mask) & ~mask, maxSize);
    }

    // Create new block and append to the linked list of blocks in this allocator
    void* mem = operator new(allocSize);
    fTail->fNext = new (mem) Block(fTail, allocSize);
    fTail = fTail->fNext;
}

#ifdef SK_DEBUG
void GrBlockAllocator::validate() const {
    std::vector<const Block*> blocks;
    const Block* prev = nullptr;
    for (const Block* block : this->blocks()) {
        blocks.push_back(block);

        SkASSERT(kAssignedMarker == block->fSentinel);
        SkASSERT(prev == block->fPrev);
        if (prev) {
            SkASSERT(prev->fNext == block);
        }

        SkASSERT(block->fSize >= (int) sizeof(Block));
        SkASSERT(block->fCursor >= kDataStart);
        SkASSERT(block->fCursor <= block->fSize);

        prev = block;
    }
    SkASSERT(prev == fTail);
    SkASSERT(blocks.size() > 0);
    SkASSERT(blocks[0] == &fHead);

    // Confirm reverse iteration matches forward iteration
    size_t j = blocks.size();
    for (const Block* b : this->rblocks()) {
        SkASSERT(b == blocks[j - 1]);
        j--;
    }
    SkASSERT(j == 0);
}
#endif
