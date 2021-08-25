/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlockAllocator.h"
#include "tests/Test.h"

#include <cstring>

using Block = SkBlockAllocator::Block;
using GrowthPolicy = SkBlockAllocator::GrowthPolicy;

class BlockAllocatorTestAccess {
public:
    template<size_t N>
    static size_t ScratchBlockSize(SkSBlockAllocator<N>& pool) {
        return (size_t) pool->scratchBlockSize();
    }
};

// Helper functions for modifying the allocator in a controlled manner
template<size_t N>
static int block_count(const SkSBlockAllocator<N>& pool) {
    int ct = 0;
    for (const Block* b : pool->blocks()) {
        (void) b;
        ct++;
    }
    return ct;
}

template<size_t N>
static Block* get_block(SkSBlockAllocator<N>& pool, int blockIndex) {
    Block* found = nullptr;
    int i = 0;
    for (Block* b: pool->blocks()) {
        if (i == blockIndex) {
            found = b;
            break;
        }
        i++;
    }

    SkASSERT(found != nullptr);
    return found;
}

// SkBlockAllocator holds on to the largest last-released block to reuse for new allocations,
// and this is still counted in its totalSize(). However, it's easier to reason about size - scratch
// in many of these tests.
template<size_t N>
static size_t total_size(SkSBlockAllocator<N>& pool) {
    return pool->totalSize() - BlockAllocatorTestAccess::ScratchBlockSize(pool);
}

template<size_t N>
static size_t add_block(SkSBlockAllocator<N>& pool) {
    size_t currentSize = total_size(pool);
    SkBlockAllocator::Block* current = pool->currentBlock();
    while(pool->currentBlock() == current) {
        pool->template allocate<4>(pool->preallocSize() / 2);
    }
    return total_size(pool) - currentSize;
}

template<size_t N>
static void* alloc_byte(SkSBlockAllocator<N>& pool) {
    auto br = pool->template allocate<1>(1);
    return br.fBlock->ptr(br.fAlignedOffset);
}

DEF_TEST(SkBlockAllocatorPreallocSize, r) {
    // Tests stack/member initialization, option #1 described in doc
    SkBlockAllocator stack{GrowthPolicy::kFixed, 2048};
    SkDEBUGCODE(stack.validate();)

    REPORTER_ASSERT(r, stack.preallocSize() == sizeof(SkBlockAllocator));
    REPORTER_ASSERT(r, stack.preallocUsableSpace() == (size_t) stack.currentBlock()->avail());

    // Tests placement new initialization to increase head block size, option #2
    void* mem = operator new(1024);
    SkBlockAllocator* placement = new (mem) SkBlockAllocator(GrowthPolicy::kLinear, 1024,
                                                             1024 - sizeof(SkBlockAllocator));
    REPORTER_ASSERT(r, placement->preallocSize() == 1024);
    REPORTER_ASSERT(r, placement->preallocUsableSpace() < 1024 &&
                       placement->preallocUsableSpace() >= (1024 - sizeof(SkBlockAllocator)));
    delete placement;

    // Tests inline increased preallocation, option #3
    SkSBlockAllocator<2048> inlined{};
    SkDEBUGCODE(inlined->validate();)
    REPORTER_ASSERT(r, inlined->preallocSize() == 2048);
    REPORTER_ASSERT(r, inlined->preallocUsableSpace() < 2048 &&
                       inlined->preallocUsableSpace() >= (2048 - sizeof(SkBlockAllocator)));
}

DEF_TEST(SkBlockAllocatorAlloc, r) {
    SkSBlockAllocator<1024> pool{};
    SkDEBUGCODE(pool->validate();)

    // Assumes the previous pointer was in the same block
    auto validate_ptr = [&](int align, int size,
                            SkBlockAllocator::ByteRange br,
                            SkBlockAllocator::ByteRange* prevBR) {
        uintptr_t pt = reinterpret_cast<uintptr_t>(br.fBlock->ptr(br.fAlignedOffset));
        // Matches the requested align
        REPORTER_ASSERT(r, pt % align == 0);
        // And large enough
        REPORTER_ASSERT(r, br.fEnd - br.fAlignedOffset >= size);
        // And has enough padding for alignment
        REPORTER_ASSERT(r, br.fAlignedOffset - br.fStart >= 0);
        REPORTER_ASSERT(r, br.fAlignedOffset - br.fStart <= align - 1);
        // And block of the returned struct is the current block of the allocator
        REPORTER_ASSERT(r, pool->currentBlock() == br.fBlock);

        // And make sure that we're past the required end of the previous allocation
        if (prevBR) {
            uintptr_t prevEnd =
                    reinterpret_cast<uintptr_t>(prevBR->fBlock->ptr(prevBR->fEnd - 1));
            REPORTER_ASSERT(r, pt > prevEnd);
        }

        // And make sure that the entire byte range is safe to write into (excluding the dead space
        // between "start" and "aligned offset," which is just padding and is left poisoned)
        std::memset(br.fBlock->ptr(br.fAlignedOffset), 0xFF, br.fEnd - br.fAlignedOffset);
    };

    auto p1 = pool->allocate<1>(14);
    validate_ptr(1, 14, p1, nullptr);

    auto p2 = pool->allocate<2>(24);
    validate_ptr(2, 24, p2, &p1);

    auto p4 = pool->allocate<4>(28);
    validate_ptr(4, 28, p4, &p2);

    auto p8 = pool->allocate<8>(40);
    validate_ptr(8, 40, p8, &p4);

    auto p16 = pool->allocate<16>(64);
    validate_ptr(16, 64, p16, &p8);

    auto p32 = pool->allocate<32>(96);
    validate_ptr(32, 96, p32, &p16);

    // All of these allocations should be in the head block
    REPORTER_ASSERT(r, total_size(pool) == pool->preallocSize());
    SkDEBUGCODE(pool->validate();)

    // Requesting an allocation of avail() should not make a new block
    size_t avail = pool->currentBlock()->avail<4>();
    auto pAvail = pool->allocate<4>(avail);
    validate_ptr(4, avail, pAvail, &p32);

    // Remaining should be less than the alignment that was requested, and then
    // the next allocation will make a new block
    REPORTER_ASSERT(r, pool->currentBlock()->avail<4>() < 4);
    auto pNextBlock = pool->allocate<4>(4);
    validate_ptr(4, 4, pNextBlock, nullptr);
    REPORTER_ASSERT(r, total_size(pool) > pool->preallocSize());

    // Allocating more than avail() makes an another block
    size_t currentSize = total_size(pool);
    size_t bigRequest = pool->currentBlock()->avail<4>() * 2;
    auto pTooBig = pool->allocate<4>(bigRequest);
    validate_ptr(4, bigRequest, pTooBig, nullptr);
    REPORTER_ASSERT(r, total_size(pool) > currentSize);

    // Allocating more than the default growth policy (1024 in this case), will fulfill the request
    REPORTER_ASSERT(r, total_size(pool) - currentSize < 4096);
    currentSize = total_size(pool);
    auto pReallyTooBig = pool->allocate<4>(4096);
    validate_ptr(4, 4096, pReallyTooBig, nullptr);
    REPORTER_ASSERT(r, total_size(pool) >= currentSize + 4096);
    SkDEBUGCODE(pool->validate();)
}

DEF_TEST(SkBlockAllocatorResize, r) {
    SkSBlockAllocator<1024> pool{};
    SkDEBUGCODE(pool->validate();)

    // Fixed resize from 16 to 32
    SkBlockAllocator::ByteRange p = pool->allocate<4>(16);
    REPORTER_ASSERT(r, p.fBlock->avail<4>() > 16);
    REPORTER_ASSERT(r, p.fBlock->resize(p.fStart, p.fEnd, 16));
    p.fEnd += 16;

    std::memset(p.fBlock->ptr(p.fAlignedOffset), 0x11, p.fEnd - p.fAlignedOffset);

    // Subsequent allocation is 32 bytes ahead of 'p' now, and 'p' cannot be resized further.
    auto pNext = pool->allocate<4>(16);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(pNext.fBlock->ptr(pNext.fAlignedOffset)) -
                       reinterpret_cast<uintptr_t>(pNext.fBlock->ptr(p.fAlignedOffset)) == 32);
    REPORTER_ASSERT(r, p.fBlock == pNext.fBlock);
    REPORTER_ASSERT(r, !p.fBlock->resize(p.fStart, p.fEnd, 48));

    // Confirm that releasing pNext allows 'p' to be resized, and that it can be resized up to avail
    REPORTER_ASSERT(r, p.fBlock->release(pNext.fStart, pNext.fEnd));
    int fillBlock = p.fBlock->avail<4>();
    REPORTER_ASSERT(r, p.fBlock->resize(p.fStart, p.fEnd, fillBlock));
    p.fEnd += fillBlock;

    std::memset(p.fBlock->ptr(p.fAlignedOffset), 0x22, p.fEnd - p.fAlignedOffset);

    // Confirm that resizing when there's not enough room fails
    REPORTER_ASSERT(r, p.fBlock->avail<4>() < fillBlock);
    REPORTER_ASSERT(r, !p.fBlock->resize(p.fStart, p.fEnd, fillBlock));

    // Confirm that we can shrink 'p' back to 32 bytes and then further allocate again
    int shrinkTo32 = p.fStart - p.fEnd + 32;
    REPORTER_ASSERT(r, p.fBlock->resize(p.fStart, p.fEnd, shrinkTo32));
    p.fEnd += shrinkTo32;
    REPORTER_ASSERT(r, p.fEnd - p.fStart == 32);

    std::memset(p.fBlock->ptr(p.fAlignedOffset), 0x33, p.fEnd - p.fAlignedOffset);

    pNext = pool->allocate<4>(16);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(pNext.fBlock->ptr(pNext.fAlignedOffset)) -
                       reinterpret_cast<uintptr_t>(pNext.fBlock->ptr(p.fAlignedOffset)) == 32);
    SkDEBUGCODE(pool->validate();)

    // Confirm that we can't shrink past the start of the allocation, but we can shrink it to 0
    int shrinkTo0 = pNext.fStart - pNext.fEnd;
#ifndef SK_DEBUG
    // Only test for false on release builds; a negative size should assert on debug builds
    REPORTER_ASSERT(r, !pNext.fBlock->resize(pNext.fStart, pNext.fEnd, shrinkTo0 - 1));
#endif
    REPORTER_ASSERT(r, pNext.fBlock->resize(pNext.fStart, pNext.fEnd, shrinkTo0));
}

DEF_TEST(SkBlockAllocatorRelease, r) {
    SkSBlockAllocator<1024> pool{};
    SkDEBUGCODE(pool->validate();)

    // Successful allocate and release
    auto p = pool->allocate<8>(32);
    REPORTER_ASSERT(r, pool->currentBlock()->release(p.fStart, p.fEnd));
    // Ensure the above release actually means the next allocation reuses the same space
    auto p2 = pool->allocate<8>(32);
    REPORTER_ASSERT(r, p.fStart == p2.fStart);

    // Confirm that 'p2' cannot be released if another allocation came after it
    auto p3 = pool->allocate<8>(64);
    (void) p3;
    REPORTER_ASSERT(r, !p2.fBlock->release(p2.fStart, p2.fEnd));

    // Confirm that 'p4' can be released if 'p5' is released first, and confirm that 'p2' and 'p3'
    // can be released simultaneously (equivalent to 'p3' then 'p2').
    auto p4 = pool->allocate<8>(16);
    auto p5 = pool->allocate<8>(96);
    REPORTER_ASSERT(r, p5.fBlock->release(p5.fStart, p5.fEnd));
    REPORTER_ASSERT(r, p4.fBlock->release(p4.fStart, p4.fEnd));
    REPORTER_ASSERT(r, p2.fBlock->release(p2.fStart, p3.fEnd));

    // And confirm that passing in the wrong size for the allocation fails
    p = pool->allocate<8>(32);
    REPORTER_ASSERT(r, !p.fBlock->release(p.fStart, p.fEnd - 16));
    REPORTER_ASSERT(r, !p.fBlock->release(p.fStart, p.fEnd + 16));
    REPORTER_ASSERT(r, p.fBlock->release(p.fStart, p.fEnd));
    SkDEBUGCODE(pool->validate();)
}

DEF_TEST(SkBlockAllocatorRewind, r) {
    // Confirm that a bunch of allocations and then releases in stack order fully goes back to the
    // start of the block (i.e. unwinds the entire stack, and not just the last cursor position)
    SkSBlockAllocator<1024> pool{};
    SkDEBUGCODE(pool->validate();)

    std::vector<SkBlockAllocator::ByteRange> ptrs;
    ptrs.reserve(32); // silence clang-tidy performance warning
    for (int i = 0; i < 32; ++i) {
        ptrs.push_back(pool->allocate<4>(16));
    }

    // Release everything in reverse order
    SkDEBUGCODE(pool->validate();)
    for (int i = 31; i >= 0; --i) {
        auto br = ptrs[i];
        REPORTER_ASSERT(r, br.fBlock->release(br.fStart, br.fEnd));
    }

    // If correct, we've rewound all the way back to the start of the block, so a new allocation
    // will have the same location as ptrs[0]
    SkDEBUGCODE(pool->validate();)
    REPORTER_ASSERT(r, pool->allocate<4>(16).fStart == ptrs[0].fStart);
}

DEF_TEST(SkBlockAllocatorGrowthPolicy, r) {
    static constexpr int kInitSize = 128;
    static constexpr int kBlockCount = 5;
    static constexpr size_t kExpectedSizes[SkBlockAllocator::kGrowthPolicyCount][kBlockCount] = {
        // kFixed -> kInitSize per block
        { kInitSize, kInitSize, kInitSize, kInitSize, kInitSize },
        // kLinear -> (block ct + 1) * kInitSize for next block
        { kInitSize, 2 * kInitSize, 3 * kInitSize, 4 * kInitSize, 5 * kInitSize },
        // kFibonacci -> 1, 1, 2, 3, 5 * kInitSize for the blocks
        { kInitSize, kInitSize, 2 * kInitSize, 3 * kInitSize, 5 * kInitSize },
        // kExponential -> 1, 2, 4, 8, 16 * kInitSize for the blocks
        { kInitSize, 2 * kInitSize, 4 * kInitSize, 8 * kInitSize, 16 * kInitSize },
    };

    for (int gp = 0; gp < SkBlockAllocator::kGrowthPolicyCount; ++gp) {
        SkSBlockAllocator<kInitSize> pool{(GrowthPolicy) gp};
        SkDEBUGCODE(pool->validate();)

        REPORTER_ASSERT(r, kExpectedSizes[gp][0] == total_size(pool));
        for (int i = 1; i < kBlockCount; ++i) {
            REPORTER_ASSERT(r, kExpectedSizes[gp][i] == add_block(pool));
        }

        SkDEBUGCODE(pool->validate();)
    }
}

DEF_TEST(SkBlockAllocatorReset, r) {
    static constexpr int kBlockIncrement = 1024;

    SkSBlockAllocator<kBlockIncrement> pool{GrowthPolicy::kLinear};
    SkDEBUGCODE(pool->validate();)

    void* firstAlloc = alloc_byte(pool);

    // Add several blocks
    add_block(pool);
    add_block(pool);
    add_block(pool);
    SkDEBUGCODE(pool->validate();)

    REPORTER_ASSERT(r, block_count(pool) == 4); // 3 added plus the implicit head

    get_block(pool, 0)->setMetadata(2);

    // Reset and confirm that there's only one block, a new allocation matches 'firstAlloc' again,
    // and new blocks are sized based on a reset growth policy.
    pool->reset();
    SkDEBUGCODE(pool->validate();)

    REPORTER_ASSERT(r,block_count(pool) == 1);
    REPORTER_ASSERT(r, pool->preallocSize() == pool->totalSize());
    REPORTER_ASSERT(r, get_block(pool, 0)->metadata() == 0);

    REPORTER_ASSERT(r, firstAlloc == alloc_byte(pool));
    REPORTER_ASSERT(r, 2 * kBlockIncrement == add_block(pool));
    REPORTER_ASSERT(r, 3 * kBlockIncrement == add_block(pool));
    SkDEBUGCODE(pool->validate();)
}

DEF_TEST(SkBlockAllocatorReleaseBlock, r) {
    // This loops over all growth policies to make sure that the incremental releases update the
    // sequence correctly for each policy.
    for (int gp = 0; gp < SkBlockAllocator::kGrowthPolicyCount; ++gp) {
        SkSBlockAllocator<1024> pool{(GrowthPolicy) gp};
        SkDEBUGCODE(pool->validate();)

        void* firstAlloc = alloc_byte(pool);

        size_t b1Size = total_size(pool);
        size_t b2Size = add_block(pool);
        size_t b3Size = add_block(pool);
        size_t b4Size = add_block(pool);
        SkDEBUGCODE(pool->validate();)

        get_block(pool, 0)->setMetadata(1);
        get_block(pool, 1)->setMetadata(2);
        get_block(pool, 2)->setMetadata(3);
        get_block(pool, 3)->setMetadata(4);

        // Remove the 3 added blocks, but always remove the i = 1 to test intermediate removal (and
        // on the last iteration, will test tail removal).
        REPORTER_ASSERT(r, total_size(pool) == b1Size + b2Size + b3Size + b4Size);
        pool->releaseBlock(get_block(pool, 1));
        REPORTER_ASSERT(r, block_count(pool) == 3);
        REPORTER_ASSERT(r, get_block(pool, 1)->metadata() == 3);
        REPORTER_ASSERT(r, total_size(pool) == b1Size + b3Size + b4Size);

        pool->releaseBlock(get_block(pool, 1));
        REPORTER_ASSERT(r, block_count(pool) == 2);
        REPORTER_ASSERT(r, get_block(pool, 1)->metadata() == 4);
        REPORTER_ASSERT(r, total_size(pool) == b1Size + b4Size);

        pool->releaseBlock(get_block(pool, 1));
        REPORTER_ASSERT(r, block_count(pool) == 1);
        REPORTER_ASSERT(r, total_size(pool) == b1Size);

        // Since we're back to just the head block, if we add a new block, the growth policy should
        // match the original sequence instead of continuing with "b5Size'"
        pool->resetScratchSpace();
        size_t size = add_block(pool);
        REPORTER_ASSERT(r, size == b2Size);
        pool->releaseBlock(get_block(pool, 1));

        // Explicitly release the head block and confirm it's reset
        pool->releaseBlock(get_block(pool, 0));
        REPORTER_ASSERT(r, total_size(pool) == pool->preallocSize());
        REPORTER_ASSERT(r, block_count(pool) == 1);
        REPORTER_ASSERT(r, firstAlloc == alloc_byte(pool));
        REPORTER_ASSERT(r, get_block(pool, 0)->metadata() == 0); // metadata reset too

        // Confirm that if we have > 1 block, but release the head block we can still access the
        // others
        add_block(pool);
        add_block(pool);
        pool->releaseBlock(get_block(pool, 0));
        REPORTER_ASSERT(r, block_count(pool) == 3);
        SkDEBUGCODE(pool->validate();)
    }
}

DEF_TEST(SkBlockAllocatorIterateAndRelease, r) {
    SkSBlockAllocator<256> pool;

    pool->headBlock()->setMetadata(1);
    add_block(pool);
    add_block(pool);
    add_block(pool);

    // Loop forward and release the blocks
    int releaseCount = 0;
    for (auto* b : pool->blocks()) {
        pool->releaseBlock(b);
        releaseCount++;
    }
    REPORTER_ASSERT(r, releaseCount == 4);
    // pool should have just the head block, but was reset
    REPORTER_ASSERT(r, pool->headBlock()->metadata() == 0);
    REPORTER_ASSERT(r, block_count(pool) == 1);

    // Add more blocks
    pool->headBlock()->setMetadata(1);
    add_block(pool);
    add_block(pool);
    add_block(pool);

    // Loop in reverse and release the blocks
    releaseCount = 0;
    for (auto* b : pool->rblocks()) {
        pool->releaseBlock(b);
        releaseCount++;
    }
    REPORTER_ASSERT(r, releaseCount == 4);
    // pool should have just the head block, but was reset
    REPORTER_ASSERT(r, pool->headBlock()->metadata() == 0);
    REPORTER_ASSERT(r, block_count(pool) == 1);
}

DEF_TEST(SkBlockAllocatorScratchBlockReserve, r) {
    SkSBlockAllocator<256> pool;

    size_t added = add_block(pool);
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) == 0);
    size_t total = pool->totalSize();
    pool->releaseBlock(pool->currentBlock());

    // Total size shouldn't have changed, the released block should become scratch
    REPORTER_ASSERT(r, pool->totalSize() == total);
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) == added);

    // But a reset definitely deletes any scratch block
    pool->reset();
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) == 0);

    // Reserving more than what's available adds a scratch block, and current block remains avail.
    size_t avail = pool->currentBlock()->avail();
    size_t reserve = avail + 1;
    pool->reserve(reserve);
    REPORTER_ASSERT(r, (size_t) pool->currentBlock()->avail() == avail);
    // And rounds up to the fixed size of this pool's growth policy
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) >= reserve &&
                       BlockAllocatorTestAccess::ScratchBlockSize(pool) % 256 == 0);

    // Allocating more than avail activates the scratch block (so totalSize doesn't change)
    size_t preAllocTotalSize = pool->totalSize();
    pool->allocate<1>(avail + 1);
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) == 0);
    REPORTER_ASSERT(r, pool->totalSize() == preAllocTotalSize);

    // When reserving less than what's still available in the current block, no scratch block is
    // added.
    pool->reserve(pool->currentBlock()->avail());
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) == 0);

    // Unless checking available bytes is disabled
    pool->reserve(pool->currentBlock()->avail(), SkBlockAllocator::kIgnoreExistingBytes_Flag);
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) > 0);

    // If kIgnoreGrowthPolicy is specified, the new scratch block should not have been updated to
    // follow the size (which in this case is a fixed 256 bytes per block).
    pool->resetScratchSpace();
    pool->reserve(32, SkBlockAllocator::kIgnoreGrowthPolicy_Flag);
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) > 0 &&
                       BlockAllocatorTestAccess::ScratchBlockSize(pool) < 256);

    // When requesting an allocation larger than the current block and the scratch block, a new
    // block is added, and the scratch block remains scratch.
    SkBlockAllocator::Block* oldTail = pool->currentBlock();
    avail = oldTail->avail();
    size_t scratchAvail = 2 * avail;
    pool->reserve(scratchAvail);
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) >= scratchAvail);

    // This allocation request is higher than oldTail's available, and the scratch size so we
    // should add a new block and scratch size should stay the same.
    scratchAvail = BlockAllocatorTestAccess::ScratchBlockSize(pool);
    pool->allocate<1>(scratchAvail + 1);
    REPORTER_ASSERT(r, pool->currentBlock() != oldTail);
    REPORTER_ASSERT(r, BlockAllocatorTestAccess::ScratchBlockSize(pool) == scratchAvail);
}

DEF_TEST(SkBlockAllocatorStealBlocks, r) {
    SkSBlockAllocator<256> poolA;
    SkSBlockAllocator<128> poolB;

    add_block(poolA);
    add_block(poolA);
    add_block(poolA);

    add_block(poolB);
    add_block(poolB);

    char* bAlloc = (char*) alloc_byte(poolB);
    *bAlloc = 't';

    const SkBlockAllocator::Block* allocOwner = poolB->findOwningBlock(bAlloc);

    REPORTER_ASSERT(r, block_count(poolA) == 4);
    REPORTER_ASSERT(r, block_count(poolB) == 3);

    size_t aSize = poolA->totalSize();
    size_t bSize = poolB->totalSize();
    size_t theftSize = bSize - poolB->preallocSize();

    // This steal should move B's 2 heap blocks to A, bringing A to 6 and B to just its head
    poolA->stealHeapBlocks(poolB.allocator());
    REPORTER_ASSERT(r, block_count(poolA) == 6);
    REPORTER_ASSERT(r, block_count(poolB) == 1);
    REPORTER_ASSERT(r, poolB->preallocSize() == poolB->totalSize());
    REPORTER_ASSERT(r, poolA->totalSize() == aSize + theftSize);

    REPORTER_ASSERT(r, *bAlloc == 't');
    REPORTER_ASSERT(r, (uintptr_t) poolA->findOwningBlock(bAlloc) == (uintptr_t) allocOwner);
    REPORTER_ASSERT(r, !poolB->findOwningBlock(bAlloc));

    // Redoing the steal now that B is just a head block should be a no-op
    poolA->stealHeapBlocks(poolB.allocator());
    REPORTER_ASSERT(r, block_count(poolA) == 6);
    REPORTER_ASSERT(r, block_count(poolB) == 1);
}

// These tests ensure that the allocation padding mechanism works as intended
struct TestMeta {
    int fX1;
    int fX2;
};
struct alignas(32) TestMetaBig {
    int fX1;
    int fX2;
};

DEF_TEST(SkBlockAllocatorMetadata, r) {
    SkSBlockAllocator<1024> pool{};
    SkDEBUGCODE(pool->validate();)

    // Allocation where alignment of user data > alignment of metadata
    SkASSERT(alignof(TestMeta) < 16);
    auto p1 = pool->allocate<16, sizeof(TestMeta)>(16);
    SkDEBUGCODE(pool->validate();)

    REPORTER_ASSERT(r, p1.fAlignedOffset - p1.fStart >= (int) sizeof(TestMeta));
    TestMeta* meta = static_cast<TestMeta*>(p1.fBlock->ptr(p1.fAlignedOffset - sizeof(TestMeta)));
    // Confirm alignment for both pointers
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(meta) % alignof(TestMeta) == 0);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(p1.fBlock->ptr(p1.fAlignedOffset)) % 16 == 0);
    // Access fields to make sure 'meta' matches compilers expectations...
    meta->fX1 = 2;
    meta->fX2 = 5;

    // Repeat, but for metadata that has a larger alignment than the allocation
    SkASSERT(alignof(TestMetaBig) == 32);
    auto p2 = pool->allocate<alignof(TestMetaBig), sizeof(TestMetaBig)>(16);
    SkDEBUGCODE(pool->validate();)

    REPORTER_ASSERT(r, p2.fAlignedOffset - p2.fStart >= (int) sizeof(TestMetaBig));
    TestMetaBig* metaBig = static_cast<TestMetaBig*>(
            p2.fBlock->ptr(p2.fAlignedOffset - sizeof(TestMetaBig)));
    // Confirm alignment for both pointers
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(metaBig) % alignof(TestMetaBig) == 0);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(p2.fBlock->ptr(p2.fAlignedOffset)) % 16 == 0);
    // Access fields
    metaBig->fX1 = 3;
    metaBig->fX2 = 6;

    // Ensure metadata values persist after allocations
    REPORTER_ASSERT(r, meta->fX1 == 2 && meta->fX2 == 5);
    REPORTER_ASSERT(r, metaBig->fX1 == 3 && metaBig->fX2 == 6);
}

DEF_TEST(SkBlockAllocatorAllocatorMetadata, r) {
    SkSBlockAllocator<256> pool{};
    SkDEBUGCODE(pool->validate();)

    REPORTER_ASSERT(r, pool->metadata() == 0); // initial value

    pool->setMetadata(4);
    REPORTER_ASSERT(r, pool->metadata() == 4);

    // Releasing the head block doesn't change the allocator's metadata (even though that's where
    // it is stored).
    pool->releaseBlock(pool->headBlock());
    REPORTER_ASSERT(r, pool->metadata() == 4);

    // But resetting the whole allocator brings things back to as if it were newly constructed
    pool->reset();
    REPORTER_ASSERT(r, pool->metadata() == 0);
}

template<size_t Align, size_t Padding>
static void run_owning_block_test(skiatest::Reporter* r, SkBlockAllocator* pool) {
    auto br = pool->allocate<Align, Padding>(1);

    void* userPtr = br.fBlock->ptr(br.fAlignedOffset);
    void* metaPtr = br.fBlock->ptr(br.fAlignedOffset - Padding);

    Block* block = pool->owningBlock<Align, Padding>(userPtr, br.fStart);
    REPORTER_ASSERT(r, block == br.fBlock);

    block = pool->owningBlock<Align>(metaPtr, br.fStart);
    REPORTER_ASSERT(r, block == br.fBlock);

    block = reinterpret_cast<Block*>(reinterpret_cast<uintptr_t>(userPtr) - br.fAlignedOffset);
    REPORTER_ASSERT(r, block == br.fBlock);
}

template<size_t Padding>
static void run_owning_block_tests(skiatest::Reporter* r, SkBlockAllocator* pool) {
    run_owning_block_test<1, Padding>(r, pool);
    run_owning_block_test<2, Padding>(r, pool);
    run_owning_block_test<4, Padding>(r, pool);
    run_owning_block_test<8, Padding>(r, pool);
    run_owning_block_test<16, Padding>(r, pool);
    run_owning_block_test<32, Padding>(r, pool);
    run_owning_block_test<64, Padding>(r, pool);
    run_owning_block_test<128, Padding>(r, pool);
}

DEF_TEST(SkBlockAllocatorOwningBlock, r) {
    SkSBlockAllocator<1024> pool{};
    SkDEBUGCODE(pool->validate();)

    run_owning_block_tests<1>(r, pool.allocator());
    run_owning_block_tests<2>(r, pool.allocator());
    run_owning_block_tests<4>(r, pool.allocator());
    run_owning_block_tests<8>(r, pool.allocator());
    run_owning_block_tests<16>(r, pool.allocator());
    run_owning_block_tests<32>(r, pool.allocator());

    // And some weird numbers
    run_owning_block_tests<3>(r, pool.allocator());
    run_owning_block_tests<9>(r, pool.allocator());
    run_owning_block_tests<17>(r, pool.allocator());
}
