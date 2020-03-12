/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrBlockAllocator.h"
#include "tests/Test.h"

using Block = GrBlockAllocator::Block;

template<int N>
class TestPool {
public:
    static constexpr int kExtra = N - sizeof(GrBlockAllocator);

    TestPool(GrBlockAllocator::GrowthPolicy policy = GrBlockAllocator::GrowthPolicy::kFixed)
            : fPool(policy, N, kExtra) {}

    GrBlockAllocator* allocator() { return &fPool; }

    int blockCount() const {
        int ct = 0;
        for (const Block* b : GrBlockAllocator::Blocks(&fPool)) {
            (void) b;
            ct++;
        }
        return ct;
    }

    const Block* block(int blockIndex) const {
        const Block* found = nullptr;
        int i = 0;
        for (const Block* b: GrBlockAllocator::Blocks(&fPool)) {
            if (i == blockIndex) {
                found = b;
                break;
            }
            i++;
        }

        SkASSERT(found != nullptr);
        return found;
    }
    Block* block(int blockIndex) {
        return const_cast<Block*>(const_cast<const TestPool*>(this)->block(blockIndex));
    }

    size_t addBlock() {
        size_t currentSize = fPool.totalSize();
        while(fPool.totalSize() == currentSize) {
            fPool.allocate<4>(N / 2);
        }
        return fPool.totalSize() - currentSize;
    }

    void* allocByte() {
        auto br = fPool.allocate<1>(1);
        return br.fBlock->ptr(br.fAlignedOffset);
    }

private:
    GrBlockAllocator fPool;
    char fStorage[kExtra];
};

DEF_TEST(GrBlockAllocatorPreallocSize, r) {
    TestPool<2048> poolAndStorage{};
    GrBlockAllocator* pool = poolAndStorage.allocator();
    SkDEBUGCODE(pool->validate();)
    REPORTER_ASSERT(r, pool->preallocSize() == 2048);
    REPORTER_ASSERT(r, pool->preallocUsableSpace() < 2048 &&
                       pool->preallocUsableSpace() >= (2048 - sizeof(GrBlockAllocator)));

    GrBlockAllocator stack{GrBlockAllocator::GrowthPolicy::kFixed, 2048};
    SkDEBUGCODE(stack.validate();)

    REPORTER_ASSERT(r, stack.preallocSize() == sizeof(GrBlockAllocator));
    REPORTER_ASSERT(r, stack.preallocUsableSpace() == (size_t) stack.currentBlock()->avail());
}

DEF_TEST(GrBlockAllocatorAlloc, r) {
    TestPool<1024> poolAndStorage{};
    GrBlockAllocator* pool = poolAndStorage.allocator();
    SkDEBUGCODE(pool->validate();)

    // Assumes the previous pointer was in the same block
    auto validate_ptr = [&](int align, int size,
                            GrBlockAllocator::ByteRange br,
                            GrBlockAllocator::ByteRange* prevBR) {
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
    REPORTER_ASSERT(r, pool->totalSize() == pool->preallocSize());
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
    REPORTER_ASSERT(r, pool->totalSize() > pool->preallocSize());

    // Allocating more than avail() makes an another block
    size_t currentSize = pool->totalSize();
    size_t bigRequest = pool->currentBlock()->avail<4>() * 2;
    auto pTooBig = pool->allocate<4>(bigRequest);
    validate_ptr(4, bigRequest, pTooBig, nullptr);
    REPORTER_ASSERT(r, pool->totalSize() > currentSize);

    // Allocating more than the default growth policy (1024 in this case), will fulfill the request
    REPORTER_ASSERT(r, pool->totalSize() - currentSize < 4096);
    currentSize = pool->totalSize();
    auto pReallyTooBig = pool->allocate<4>(4096);
    validate_ptr(4, 4096, pReallyTooBig, nullptr);
    REPORTER_ASSERT(r, pool->totalSize() >= currentSize + 4096);
    SkDEBUGCODE(pool->validate();)
}

DEF_TEST(GrBlockAllocatorResize, r) {
    TestPool<1024> poolAndStorage{};
    GrBlockAllocator* pool = poolAndStorage.allocator();
    SkDEBUGCODE(pool->validate();)

    // Fixed resize from 16 to 32
    auto p = pool->allocate<4>(16);
    REPORTER_ASSERT(r, p.fBlock->avail<4>() > 16);
    REPORTER_ASSERT(r, p.fBlock->resize(p.fStart, p.fEnd, 32));
    p.fEnd += 16;

    // Subsequent allocation is 32 bytes ahead of 'p' now, and 'p' cannot be resized further.
    auto pNext = pool->allocate<4>(16);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(pNext.fBlock->ptr(pNext.fAlignedOffset)) -
                       reinterpret_cast<uintptr_t>(pNext.fBlock->ptr(p.fAlignedOffset)) == 32);
    REPORTER_ASSERT(r, p.fBlock == pNext.fBlock);
    REPORTER_ASSERT(r, !p.fBlock->resize(p.fStart, p.fEnd, 48));

    // Confirm that releasing pNext allows 'p' to be resized, and that it can be resized up to avail
    REPORTER_ASSERT(r, p.fBlock->release(pNext.fStart, pNext.fEnd));
    size_t newSize = 32 + p.fBlock->avail<4>();
    REPORTER_ASSERT(r, p.fBlock->resize(p.fStart, p.fEnd, newSize));
    p.fEnd = p.fStart + newSize;

    // Confirm that resizing when there's not enough room fails
    REPORTER_ASSERT(r, (size_t) p.fBlock->avail<4>() < newSize);
    REPORTER_ASSERT(r, !p.fBlock->resize(p.fStart, p.fEnd, 2 * newSize));

    // Confirm that we can shrink 'p' back and then further allocate within the same block
    REPORTER_ASSERT(r, p.fBlock->resize(p.fStart, p.fEnd, 32));
    p.fEnd = p.fStart + 32;

    pNext = pool->allocate<4>(16);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(pNext.fBlock->ptr(pNext.fAlignedOffset)) -
                       reinterpret_cast<uintptr_t>(pNext.fBlock->ptr(p.fAlignedOffset)) == 32);
    SkDEBUGCODE(pool->validate();)
}

DEF_TEST(GrBlockAllocatorRelease, r) {
    TestPool<1024> poolAndStorage{};
    GrBlockAllocator* pool = poolAndStorage.allocator();
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

DEF_TEST(GrBlockAllocatorRewind, r) {
    // Confirm that a bunch of allocations and then releases in stack order fully goes back to the
    // start of the block (i.e. unwinds the entire stack, and not just the last cursor position)
    TestPool<1024> poolAndStorage{};
    GrBlockAllocator* pool = poolAndStorage.allocator();
    SkDEBUGCODE(pool->validate();)

    std::vector<GrBlockAllocator::ByteRange> ptrs;
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

DEF_TEST(GrBlockAllocatorGrowthPolicy, r) {
    static constexpr int kInitSize = 128;
    static constexpr int kBlockCount = 5;
    static constexpr size_t kExpectedSizes[GrBlockAllocator::kGrowthPolicyCount][kBlockCount] = {
        // kFixed -> kInitSize per block
        { kInitSize, kInitSize, kInitSize, kInitSize, kInitSize },
        // kLinear -> (block ct + 1) * kInitSize for next block
        { kInitSize, 2 * kInitSize, 3 * kInitSize, 4 * kInitSize, 5 * kInitSize },
        // kFibonacci -> 1, 1, 2, 3, 5 * kInitSize for the blocks
        { kInitSize, kInitSize, 2 * kInitSize, 3 * kInitSize, 5 * kInitSize },
        // kExponential -> 1, 2, 4, 8, 16 * kInitSize for the blocks
        { kInitSize, 2 * kInitSize, 4 * kInitSize, 8 * kInitSize, 16 * kInitSize },
    };

    for (int gp = 0; gp < GrBlockAllocator::kGrowthPolicyCount; ++gp) {
        TestPool<kInitSize> pool{(GrBlockAllocator::GrowthPolicy) gp};
        SkDEBUGCODE(pool.allocator()->validate();)

        REPORTER_ASSERT(r, kExpectedSizes[gp][0] == pool.allocator()->totalSize());
        for (int i = 1; i < kBlockCount; ++i) {
            REPORTER_ASSERT(r, kExpectedSizes[gp][i] == pool.addBlock());
        }

        SkDEBUGCODE(pool.allocator()->validate();)
    }
}

DEF_TEST(GrBlockAllocatorReset, r) {
    static constexpr int kBlockIncrement = 1024;

    TestPool<kBlockIncrement> pool{GrBlockAllocator::GrowthPolicy::kLinear};
    SkDEBUGCODE(pool.allocator()->validate();)

    void* firstAlloc = pool.allocByte();

    // Add several blocks
    pool.addBlock();
    pool.addBlock();
    pool.addBlock();
    SkDEBUGCODE(pool.allocator()->validate();)

    REPORTER_ASSERT(r, pool.blockCount() == 4); // 3 added plus the implicit head

    pool.block(0)->setMetadata(2);

    // Reset and confirm that there's only one block, a new allocation matches 'firstAlloc' again,
    // and new blocks are sized based on a reset growth policy.
    pool.allocator()->reset();
    SkDEBUGCODE(pool.allocator()->validate();)

    REPORTER_ASSERT(r, pool.blockCount() == 1);
    REPORTER_ASSERT(r, pool.allocator()->preallocSize() == pool.allocator()->totalSize());
    REPORTER_ASSERT(r, pool.block(0)->metadata() == 0);

    REPORTER_ASSERT(r, firstAlloc == pool.allocByte());
    REPORTER_ASSERT(r, 2 * kBlockIncrement == pool.addBlock());
    REPORTER_ASSERT(r, 3 * kBlockIncrement == pool.addBlock());
    SkDEBUGCODE(pool.allocator()->validate();)
}

DEF_TEST(GrBlockAllocatorReleaseBlock, r) {
    // This loops over all growth policies to make sure that the incremental releases update the
    // sequence correctly for each policy.
    for (int gp = 0; gp < GrBlockAllocator::kGrowthPolicyCount; ++gp) {
        TestPool<1024> pool{(GrBlockAllocator::GrowthPolicy) gp};
        SkDEBUGCODE(pool.allocator()->validate();)

        void* firstAlloc = pool.allocByte();

        size_t b1Size = pool.allocator()->totalSize();
        size_t b2Size = pool.addBlock();
        size_t b3Size = pool.addBlock();
        size_t b4Size = pool.addBlock();
        SkDEBUGCODE(pool.allocator()->validate();)

        pool.block(0)->setMetadata(1);
        pool.block(1)->setMetadata(2);
        pool.block(2)->setMetadata(3);
        pool.block(3)->setMetadata(4);

        // Remove the 3 added blocks, but always remove the i = 1 to test intermediate removal (and
        // on the last iteration, will test tail removal).
        REPORTER_ASSERT(r, pool.allocator()->totalSize() == b1Size + b2Size + b3Size + b4Size);
        pool.allocator()->releaseBlock(pool.block(1));
        REPORTER_ASSERT(r, pool.blockCount() == 3);
        REPORTER_ASSERT(r, pool.block(1)->metadata() == 3);
        REPORTER_ASSERT(r, pool.allocator()->totalSize() == b1Size + b3Size + b4Size);

        pool.allocator()->releaseBlock(pool.block(1));
        REPORTER_ASSERT(r, pool.blockCount() == 2);
        REPORTER_ASSERT(r, pool.block(1)->metadata() == 4);
        REPORTER_ASSERT(r, pool.allocator()->totalSize() == b1Size + b4Size);

        pool.allocator()->releaseBlock(pool.block(1));
        REPORTER_ASSERT(r, pool.blockCount() == 1);
        REPORTER_ASSERT(r, pool.allocator()->totalSize() == b1Size);

        // Since we're back to just the head block, if we add a new block, the growth policy should
        // match the original sequence instead of continuing with "b5Size'"
        size_t size = pool.addBlock();
        REPORTER_ASSERT(r, size/*pool->addBlock()*/ == b2Size);
        pool.allocator()->releaseBlock(pool.block(1));

        // Explicitly release the head block and confirm it's reset
        pool.allocator()->releaseBlock(pool.block(0));
        REPORTER_ASSERT(r, pool.allocator()->totalSize() == pool.allocator()->preallocSize());
        REPORTER_ASSERT(r, pool.blockCount() == 1);
        REPORTER_ASSERT(r, firstAlloc == pool.allocByte());
        REPORTER_ASSERT(r, pool.block(0)->metadata() == 0); // metadata reset too

        // Confirm that if we have > 1 block, but release the head block we can still access the
        // others
        pool.addBlock();
        pool.addBlock();
        pool.allocator()->releaseBlock(pool.block(0));
        REPORTER_ASSERT(r, pool.blockCount() == 3);
        SkDEBUGCODE(pool.allocator()->validate();)
    }
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

DEF_TEST(GrBlockAllocatorMetadata, r) {
    TestPool<1024> poolAndStorage{GrBlockAllocator::GrowthPolicy::kFixed};
    GrBlockAllocator* pool = poolAndStorage.allocator();
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
