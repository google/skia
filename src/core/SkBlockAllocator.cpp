/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlockAllocator.h"

#ifdef SK_DEBUG
#include <vector>
#endif

SkBlockAllocator::SkBlockAllocator(GrowthPolicy policy, size_t blockIncrementBytes,
                                   size_t additionalPreallocBytes)
        : fTail(&fHead)
        // Round up to the nearest max-aligned value, and then divide so that fBlockSizeIncrement
        // can effectively fit higher byte counts in its 16 bits of storage
        , fBlockIncrement(SkTo<uint16_t>(GrAlignTo(blockIncrementBytes, kAddressAlign)
                                                / kAddressAlign))
        , fGrowthPolicy(static_cast<uint64_t>(policy))
        , fN0((policy == GrowthPolicy::kLinear || policy == GrowthPolicy::kExponential) ? 1 : 0)
        , fN1(1)
        // The head block always fills remaining space from SkBlockAllocator's size, because it's
        // inline, but can take over the specified number of bytes immediately after it.
        , fHead(/*prev=*/nullptr, additionalPreallocBytes + BaseHeadBlockSize()) {
    SkASSERT(fBlockIncrement >= 1);
    SkASSERT(additionalPreallocBytes <= kMaxAllocationSize);
}

SkBlockAllocator::Block::Block(Block* prev, int allocationSize)
         : fNext(nullptr)
         , fPrev(prev)
         , fSize(allocationSize)
         , fCursor(kDataStart)
         , fMetadata(0)
         , fAllocatorMetadata(0) {
    SkASSERT(allocationSize >= (int) sizeof(Block));
    SkDEBUGCODE(fSentinel = kAssignedMarker;)

    this->poisonRange(kDataStart, fSize);
}

SkBlockAllocator::Block::~Block() {
    this->unpoisonRange(kDataStart, fSize);

    SkASSERT(fSentinel == kAssignedMarker);
    SkDEBUGCODE(fSentinel = kFreedMarker;) // FWIW
}

size_t SkBlockAllocator::totalSize() const {
    // Use size_t since the sum across all blocks could exceed 'int', even though each block won't
    size_t size = offsetof(SkBlockAllocator, fHead) + this->scratchBlockSize();
    for (const Block* b : this->blocks()) {
        size += b->fSize;
    }
    SkASSERT(size >= this->preallocSize());
    return size;
}

size_t SkBlockAllocator::totalUsableSpace() const {
    size_t size = this->scratchBlockSize();
    if (size > 0) {
        size -= kDataStart; // scratchBlockSize reports total block size, not usable size
    }
    for (const Block* b : this->blocks()) {
        size += (b->fSize - kDataStart);
    }
    SkASSERT(size >= this->preallocUsableSpace());
    return size;
}

size_t SkBlockAllocator::totalSpaceInUse() const {
    size_t size = 0;
    for (const Block* b : this->blocks()) {
        size += (b->fCursor - kDataStart);
    }
    SkASSERT(size <= this->totalUsableSpace());
    return size;
}

SkBlockAllocator::Block* SkBlockAllocator::findOwningBlock(const void* p) {
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

void SkBlockAllocator::releaseBlock(Block* block) {
     if (block == &fHead) {
        // Reset the cursor of the head block so that it can be reused if it becomes the new tail
        block->fCursor = kDataStart;
        block->fMetadata = 0;
        block->poisonRange(kDataStart, block->fSize);
        // Unlike in reset(), we don't set the head's next block to null because there are
        // potentially heap-allocated blocks that are still connected to it.
    } else {
        SkASSERT(block->fPrev);
        block->fPrev->fNext = block->fNext;
        if (block->fNext) {
            SkASSERT(fTail != block);
            block->fNext->fPrev = block->fPrev;
        } else {
            SkASSERT(fTail == block);
            fTail = block->fPrev;
        }

        // The released block becomes the new scratch block (if it's bigger), or delete it
        if (this->scratchBlockSize() < block->fSize) {
            SkASSERT(block != fHead.fPrev); // shouldn't already be the scratch block
            if (fHead.fPrev) {
                delete fHead.fPrev;
            }
            block->markAsScratch();
            fHead.fPrev = block;
        } else {
            delete block;
        }
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

void SkBlockAllocator::stealHeapBlocks(SkBlockAllocator* other) {
    Block* toSteal = other->fHead.fNext;
    if (toSteal) {
        // The other's next block connects back to this allocator's current tail, and its new tail
        // becomes the end of other's block linked list.
        SkASSERT(other->fTail != &other->fHead);
        toSteal->fPrev = fTail;
        fTail->fNext = toSteal;
        fTail = other->fTail;
        // The other allocator becomes just its inline head block
        other->fTail = &other->fHead;
        other->fHead.fNext = nullptr;
    } // else no block to steal
}

void SkBlockAllocator::reset() {
    for (Block* b : this->rblocks()) {
        if (b == &fHead) {
            // Reset metadata and cursor, tail points to the head block again
            fTail = b;
            b->fNext = nullptr;
            b->fCursor = kDataStart;
            b->fMetadata = 0;
            // For reset(), but NOT releaseBlock(), the head allocatorMetadata and scratch block
            // are reset/destroyed.
            b->fAllocatorMetadata = 0;
            b->poisonRange(kDataStart, b->fSize);
            this->resetScratchSpace();
        } else {
            delete b;
        }
    }
    SkASSERT(fTail == &fHead && fHead.fNext == nullptr && fHead.fPrev == nullptr &&
             fHead.metadata() == 0 && fHead.fCursor == kDataStart);

    GrowthPolicy gp = static_cast<GrowthPolicy>(fGrowthPolicy);
    fN0 = (gp == GrowthPolicy::kLinear || gp == GrowthPolicy::kExponential) ? 1 : 0;
    fN1 = 1;
}

void SkBlockAllocator::resetScratchSpace() {
    if (fHead.fPrev) {
        delete fHead.fPrev;
        fHead.fPrev = nullptr;
    }
}

void SkBlockAllocator::addBlock(int minSize, int maxSize) {
    SkASSERT(minSize > (int) sizeof(Block) && minSize <= maxSize);

    // Max positive value for uint:23 storage (decltype(fN0) picks up uint64_t, not uint:23).
    static constexpr int kMaxN = (1 << 23) - 1;
    static_assert(2 * kMaxN <= std::numeric_limits<int32_t>::max()); // Growth policy won't overflow

    auto alignAllocSize = [](int size) {
        // Round to a nice boundary since the block isn't maxing out:
        //   if allocSize > 32K, aligns on 4K boundary otherwise aligns on max_align_t, to play
        //   nicely with jeMalloc (from SkArenaAlloc).
        int mask = size > (1 << 15) ? ((1 << 12) - 1) : (kAddressAlign - 1);
        return (size + mask) & ~mask;
    };

    int allocSize;
    void* mem = nullptr;
    if (this->scratchBlockSize() >= minSize) {
        // Activate the scratch block instead of making a new block
        SkASSERT(fHead.fPrev->isScratch());
        allocSize = fHead.fPrev->fSize;
        mem = fHead.fPrev;
        fHead.fPrev = nullptr;
    } else if (minSize < maxSize) {
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
        int sizeIncrement = fBlockIncrement * kAddressAlign;
        if (maxSize / sizeIncrement < nextN1) {
            // The growth policy would overflow, so use the max. We've already confirmed that
            // maxSize will be sufficient for the requested minimumSize
            allocSize = maxSize;
        } else {
            allocSize = std::min(alignAllocSize(std::max(minSize, sizeIncrement * nextN1)),
                                 maxSize);
        }
    } else {
        SkASSERT(minSize == maxSize);
        // Still align on a nice boundary, no max clamping since that would just undo the alignment
        allocSize = alignAllocSize(minSize);
    }

    // Create new block and append to the linked list of blocks in this allocator
    if (!mem) {
        mem = operator new(allocSize);
    }
    fTail->fNext = new (mem) Block(fTail, allocSize);
    fTail = fTail->fNext;
}

#ifdef SK_DEBUG
void SkBlockAllocator::validate() const {
    std::vector<const Block*> blocks;
    const Block* prev = nullptr;
    for (const Block* block : this->blocks()) {
        blocks.push_back(block);

        SkASSERT(kAssignedMarker == block->fSentinel);
        if (block == &fHead) {
            // The head blocks' fPrev may be non-null if it holds a scratch block, but that's not
            // considered part of the linked list
            SkASSERT(!prev && (!fHead.fPrev || fHead.fPrev->isScratch()));
        } else {
            SkASSERT(prev == block->fPrev);
        }
        if (prev) {
            SkASSERT(prev->fNext == block);
        }

        SkASSERT(block->fSize >= (int) sizeof(Block));
        SkASSERT(block->fCursor >= kDataStart);
        SkASSERT(block->fCursor <= block->fSize);

        prev = block;
    }
    SkASSERT(prev == fTail);
    SkASSERT(!blocks.empty());
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
