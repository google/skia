/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrBlockAllocator.h"
#include "tests/Test.h"

// These tests cover the public API of the GrBlockAllocator, as if it were used in isolation. These
// APIs happen to make use of the underlying protected API, but subclasses that rely on
// GrBlockAllocator should still test their specific interactions with the protected API.

DEF_TEST(GrBlockAllocatorPreallocSize, r) {
    auto pool = GrBlockAllocator::Make(2048);
    REPORTER_ASSERT(r, pool->preallocSize() == 2048);

    // 0 size gets bumped up to the minimum to hold the block allocator and one block
    pool = GrBlockAllocator::Make(0);
    REPORTER_ASSERT(r, pool->preallocSize() >= sizeof(GrBlockAllocator));
    REPORTER_ASSERT(r, pool->avail<1>() == 0);
}

DEF_TEST(GrBlockAllocatorAlloc, r) {
    auto pool = GrBlockAllocator::Make(1024);

    // Assumes the previous pointer was in the same block
    auto validate_ptr = [&r](void* p, int align, void* prevP, int prevSize) {
        uintptr_t pt = reinterpret_cast<uintptr_t>(p);
        uintptr_t prevPT = reinterpret_cast<uintptr_t>(prevP);
        // Matches the requested align
        if (pt % align != 0) {
            SkDebugf("alignment failed for %p at %d\n", p, align);
        }
        REPORTER_ASSERT(r, pt % align == 0);
        // At least prevSize away from the prior pointer, and no more than align - 1 (for padding)
        if (prevP) {
            REPORTER_ASSERT(r, pt >= prevPT + prevSize);
            REPORTER_ASSERT(r, pt <= prevPT + prevSize + align - 1);
        }
    };

    void* p1 = pool->allocate<1>(14);
    validate_ptr(p1, 1, nullptr, 0);

    void* p2 = pool->allocate<2>(24);
    validate_ptr(p2, 2, p1, 14);

    void* p4 = pool->allocate<4>(28);
    validate_ptr(p4, 4, p2, 24);

    void* p8 = pool->allocate<8>(40);
    validate_ptr(p8, 8, p4, 28);

    void* p16 = pool->allocate<16>(64);
    validate_ptr(p16, 16, p8, 40);

    void* p32 = pool->allocate<32>(96);
    validate_ptr(p32, 32, p16, 64);

    // All of these allocations should be in the head block
    REPORTER_ASSERT(r, pool->size() == pool->preallocSize());

    // Requesting an allocation of avail() should not make a new block
    size_t avail = pool->avail<4>();
    void* pAvail = pool->allocate<4>(avail);
    validate_ptr(pAvail, 4, p32, 96);

    // Remaining should be less than the alignment that was requested, and then
    // the next allocation will make a new block
    REPORTER_ASSERT(r, pool->avail<4>() < 4);
    void* pNextBlock = pool->allocate<4>(4);
    validate_ptr(pNextBlock, 4, nullptr, 0);
    REPORTER_ASSERT(r, pool->size() > pool->preallocSize());

    // Allocating more than avail() makes an another block
    size_t currentSize = pool->size();
    void* pTooBig = pool->allocate<4>(pool->avail<4>() * 2);
    validate_ptr(pTooBig, 4, nullptr, 0);
    REPORTER_ASSERT(r, pool->size() > currentSize);

    // Allocating more than the default growth policy (1024 in this case), will fulfill the request
    REPORTER_ASSERT(r, pool->size() - currentSize < 4096);
    currentSize = pool->size();
    void* pReallyTooBig = pool->allocate<4>(4096);
    validate_ptr(pReallyTooBig, 4, nullptr, 0);
    REPORTER_ASSERT(r, pool->size() >= currentSize + 4096);
}

DEF_TEST(GrBlockAllocatorResize, r) {
    auto pool = GrBlockAllocator::Make(1024);

    // Fixed resize from 16 to 32
    void* p = pool->allocate<4>(16);
    REPORTER_ASSERT(r, pool->avail<4>() > 16);
    REPORTER_ASSERT(r, pool->resize(p, 16, 32));

    // Subsequent allocation is 32 bytes ahead of 'p' now, and 'p' cannot be resized further.
    void* pNext = pool->allocate<4>(16);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(pNext) - reinterpret_cast<uintptr_t>(p) == 32);
    REPORTER_ASSERT(r, !pool->resize(p, 32, 48));

    // Confirm that releasing pNext allows 'p' to be resized, and that it can be resized up to avail
    REPORTER_ASSERT(r, pool->release(pNext, 16));
    size_t newSize = 32 + pool->avail<4>();
    REPORTER_ASSERT(r, pool->resize(p, 32, newSize));

    // Confirm that resizing when there's not enough room fails
    REPORTER_ASSERT(r, pool->avail<4>() < newSize);
    REPORTER_ASSERT(r, !pool->resize(p, newSize, 2 * newSize));

    // Confirm that we can shrink 'p' back and then further allocate within the same block
    REPORTER_ASSERT(r, pool->resize(p, newSize, 32));
    pNext = pool->allocate<4>(16);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(pNext) - reinterpret_cast<uintptr_t>(p) == 32);
}

DEF_TEST(GrBlockAllocatorRelease, r) {
    // NOTE: this test always uses a fixed alignment so that padding doesn't unexpectedly prevent
    // the allocator from fulling recovering bytes. It is expected behavior that the public API
    // cannot reclaim the unknowable padding, but that means when it occurs, subsequent releases
    // will fail because the block position doesn't line up. This is not a real issue in practice
    // since the protected API can be used to recover the padding as well
    auto pool = GrBlockAllocator::Make(1024);

    // Successful allocate and release
    void* p = pool->allocate<8>(32);
    REPORTER_ASSERT(r, pool->release(p, 32));
    // Ensure the above release actually means the next allocation reuses the same space
    void* p2 = pool->allocate<8>(32);
    REPORTER_ASSERT(r, p == p2);

    // Confirm that 'p2' cannot be released if another allocation came after it
    void* p3 = pool->allocate<8>(64);
    (void) p3;
    REPORTER_ASSERT(r, !pool->release(p2, 32));

    // Confirm that 'p4' can be released if 'p5' is released first, and confirm that 'p2' and 'p3'
    // can be released simultaneously (equivalent to 'p3' then 'p2').
    void* p4 = pool->allocate<8>(16);
    void* p5 = pool->allocate<8>(96);
    REPORTER_ASSERT(r, pool->release(p5, 96));
    REPORTER_ASSERT(r, pool->release(p4, 16));
    REPORTER_ASSERT(r, pool->release(p2, 32 + 64));

    // And confirm that passing in the wrong size for the allocation fails
    p = pool->allocate<8>(32);
    REPORTER_ASSERT(r, !pool->release(p, 16));
    REPORTER_ASSERT(r, !pool->release(p, 48));
    REPORTER_ASSERT(r, pool->release(p, 32));
}

DEF_TEST(GrBlockAllocatorRewind, r) {
    // Confirm that a bunch of allocations and then releases in stack order fully goes back to the
    // start of the block (i.e. unwinds the entire stack, and not just the last cursor position)
    auto pool = GrBlockAllocator::Make(1024);

    std::vector<void*> ptrs;
    for (int i = 0; i < 32; ++i) {
        ptrs.push_back(pool->allocate<4>(16));
    }

    // Release everything in reverse order
    for (int i = 31; i >= 0; --i) {
        REPORTER_ASSERT(r, pool->release(ptrs[i], 16));
    }

    // If correct, we've rewound all the way back to the start of the block, so a new allocation
    // will have the same location as ptrs[0]
    REPORTER_ASSERT(r, pool->allocate<4>(16) == ptrs[0]);
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

    auto next_block_size = [](GrBlockAllocator* pool) {
        size_t currentSize = pool->size();
        while(currentSize == pool->size()) {
            pool->allocate<4>(kInitSize / 2);
        }
        return pool->size() - currentSize;
    };

    for (int gp = 0; gp < GrBlockAllocator::kGrowthPolicyCount; ++gp) {
        auto pool = GrBlockAllocator::Make(kInitSize, (GrBlockAllocator::GrowthPolicy) gp);
        REPORTER_ASSERT(r, kExpectedSizes[gp][0] == pool->size());
        for (int i = 1; i < kBlockCount; ++i) {
            REPORTER_ASSERT(r, kExpectedSizes[gp][i] == next_block_size(pool.get()));
        }
    }
}

// These tests cover very specific behavior of the allocator related to releaseBlock() and reset,
// which is relied on by GrMemoryPool, but is easiest to test with a custom subclass.

class TestAllocator : public GrBlockAllocator {
public:
    static constexpr size_t kBlockIncrement = 1024;

    static std::unique_ptr<TestAllocator> Make(GrowthPolicy policy) {
        return GrBlockAllocator::Prealloc<TestAllocator>(kBlockIncrement, policy);
    }

    void operator delete(void* p) { ::operator delete(p); }

    int blockCount() const {
        int ct = 0;
        for (Block b : Blocks(this)) {
            (void) b;
            ct++;
        }
        return ct;
    }

    void releaseBlock(int blockIndex) {
        Block toRelease = reinterpret_cast<Block>(nullptr);
        int i = 0;
        for (Block b: Blocks(this)) {
            if (i == blockIndex) {
                toRelease = b;
                break;
            }
            i++;
        }

        SkASSERT(toRelease != reinterpret_cast<Block>(nullptr));
        this->GrBlockAllocator::releaseBlock(toRelease);
    }

    size_t addBlock() {
        size_t currentSize = this->size();
        while(this->size() == currentSize) {
            this->GrBlockAllocator::allocate<4>(512);
        }
        return this->size() - currentSize;
    }

    int* blockData(int blockIndex) {
        int i = 0;
        for (Block b: Blocks(this)) {
            if (i == blockIndex) {
                return BlockData(b);
            }
            i++;
        }
        SkASSERT(false);
        return nullptr;
    }

    void* allocByte() {
        return this->GrBlockAllocator::allocate<1>(1);
    }

    // Make these public (but can't just use 'using' for template declarations)
    template<size_t kAlign, typename Meta, typename... MetaArgs>
    void* allocateMeta(int size, Meta** metaPtr, MetaArgs... args) {
        return this->GrBlockAllocator::allocate<kAlign>(size, metaPtr,
                                                        std::forward<MetaArgs>(args)...);
    }

    template<size_t kAlign, typename Meta>
    Meta* metadata(void* p) {
        return GrBlockAllocator::Metadata<kAlign, Meta>(p);
    }

    void validate() const {
#ifdef SK_DEBUG
        this->GrBlockAllocator::validate();
#endif
    }

private:
    friend class GrBlockAllocator;

    TestAllocator(void* headBlock, int blockSize, GrowthPolicy policy)
            : GrBlockAllocator(headBlock, blockSize, policy) {}
};

DEF_TEST(GrBlockAllocatorReset, r) {
    auto pool = TestAllocator::Make(GrBlockAllocator::GrowthPolicy::kLinear);
    pool->validate();
    void* firstAlloc = pool->allocByte();

    // Add several blocks
    pool->addBlock();
    pool->addBlock();
    pool->addBlock();
    pool->validate();

    REPORTER_ASSERT(r, pool->blockCount() == 4); // 3 added plus the implicit head

    (*pool->blockData(0)) = 2;

    // Reset and confirm that there's only one block, a new allocation matches 'firstAlloc' again,
    // and new blocks are sized based on a reset growth policy.
    pool->reset();
    pool->validate();
    REPORTER_ASSERT(r, pool->blockCount() == 1);
    REPORTER_ASSERT(r, pool->preallocSize() == pool->size());
    REPORTER_ASSERT(r, *pool->blockData(0) == 0);

    REPORTER_ASSERT(r, firstAlloc == pool->allocByte());
    REPORTER_ASSERT(r, 2 * TestAllocator::kBlockIncrement == pool->addBlock());
    REPORTER_ASSERT(r, 3 * TestAllocator::kBlockIncrement == pool->addBlock());
    pool->validate();
}

DEF_TEST(GrBlockAllocatorReleaseBlock, r) {
    // This loops over all growth policies to make sure that the incremental releases update the
    // sequence correctly for each policy.
    for (int gp = 0; gp < GrBlockAllocator::kGrowthPolicyCount; ++gp) {
        auto pool = TestAllocator::Make((GrBlockAllocator::GrowthPolicy) gp);
        pool->validate();

        void* firstAlloc = pool->allocByte();

        size_t b1Size = pool->size();
        size_t b2Size = pool->addBlock();
        size_t b3Size = pool->addBlock();
        size_t b4Size = pool->addBlock();
        pool->validate();

        *pool->blockData(0) = 1;
        *pool->blockData(1) = 2;
        *pool->blockData(2) = 3;
        *pool->blockData(3) = 4;

        // Remove the 3 added blocks, but always remove the i = 1 to test intermediate removal (and
        // on the last iteration, will test tail removal).
        REPORTER_ASSERT(r, pool->size() == b1Size + b2Size + b3Size + b4Size);
        pool->releaseBlock(1);
        REPORTER_ASSERT(r, pool->blockCount() == 3);
        REPORTER_ASSERT(r, *pool->blockData(1) == 3);
        REPORTER_ASSERT(r, pool->size() == b1Size + b3Size + b4Size);

        pool->releaseBlock(1);
        REPORTER_ASSERT(r, pool->blockCount() == 2);
        REPORTER_ASSERT(r, *pool->blockData(1) == 4);
        REPORTER_ASSERT(r, pool->size() == b1Size + b4Size);

        pool->releaseBlock(1);
        REPORTER_ASSERT(r, pool->blockCount() == 1);
        REPORTER_ASSERT(r, pool->size() == b1Size);

        // Since we're back to just the head block, if we add a new block, the growth policy should
        // match the original sequence instead of continuing with "b5Size'"
        size_t size = pool->addBlock();
        REPORTER_ASSERT(r, size/*pool->addBlock()*/ == b2Size);
        pool->releaseBlock(1);

        // Explicitly release the head block and confirm it's reset
        pool->releaseBlock(0);
        REPORTER_ASSERT(r, pool->size() == pool->preallocSize());
        REPORTER_ASSERT(r, pool->blockCount() == 1);
        REPORTER_ASSERT(r, firstAlloc == pool->allocByte());
        REPORTER_ASSERT(r, *pool->blockData(0) == 0); // metadata reset too

        // Confirm that if we have > 1 block, but release the head block we can still access the
        // others
        pool->addBlock();
        pool->addBlock();
        pool->releaseBlock(0);
        REPORTER_ASSERT(r, pool->blockCount() == 3);
        pool->validate();
    }
}

// These tests ensure that the built-in allocation metadata utilities work as expected.
struct TestMeta {
    int fX1;
    int fX2;

    TestMeta(uintptr_t block, int offset, int size, skiatest::Reporter* r, int userSize)
            : fX1(1), fX2(2) {
        REPORTER_ASSERT(r, (block + offset) % alignof(TestMeta) == 0);
        // size accounts for both the metadata and the user allocation, and padding
        REPORTER_ASSERT(r, size >= (int) (sizeof(TestMeta) + userSize));
    }
};
struct alignas(32) TestMetaBig {
    int fX1;
    int fX2;

    TestMetaBig(uintptr_t block, int offset, int size, skiatest::Reporter* r, int userSize)
            : fX1(1), fX2(2) {
        REPORTER_ASSERT(r, (block + offset) % alignof(TestMetaBig) == 0);
        // size accounts for both the metadata and the user allocation
        REPORTER_ASSERT(r, size >= (int) (sizeof(TestMetaBig) + userSize));
    }
};
DEF_TEST(GrBlockAllocatorMetadata, r) {
    auto pool = TestAllocator::Make(GrBlockAllocator::GrowthPolicy::kFixed);
    pool->validate();

    // Allocation where alignment of user data > alignment of metadata
    TestMeta* meta = nullptr;
    void* p1 = pool->allocateMeta<8, TestMeta>(16, &meta, r, 16);
    pool->validate();

    // Confirm that the metadata ctor was run
    REPORTER_ASSERT(r, meta != nullptr);
    REPORTER_ASSERT(r, meta->fX1 == 1 && meta->fX2 == 2);
    // Confirm alignment for both pointers
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(meta) % alignof(TestMeta) == 0);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(p1) % 8 == 0);
    // Access fields
    meta->fX1 = 2;
    meta->fX2 = 5;

    // Repeat, but for metadata that has a larger alignment than the allocation
    TestMetaBig* metaBig = nullptr;
    void* p2 = pool->allocateMeta<8, TestMetaBig>(16, &metaBig, r, 16);
    pool->validate();

    REPORTER_ASSERT(r, metaBig != nullptr);
    REPORTER_ASSERT(r, metaBig->fX1 == 1 && metaBig->fX2 == 2);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(metaBig) % alignof(TestMetaBig) == 0);
    REPORTER_ASSERT(r, reinterpret_cast<uintptr_t>(p2) % 8 == 0);
    // Access fields
    metaBig->fX1 = 3;
    metaBig->fX2 = 6;

    // Recover metadata from 'p1' and 'p2', and ensure it matches what was assigned
    TestMeta* meta2 = pool->metadata<8, TestMeta>(p1);
    TestMetaBig* metaBig2 = pool->metadata<8, TestMetaBig>(p2);

    REPORTER_ASSERT(r, meta == meta2);
    REPORTER_ASSERT(r, metaBig == metaBig2);
    REPORTER_ASSERT(r, meta2->fX1 == 2 && meta2->fX2 == 5);
    REPORTER_ASSERT(r, metaBig2->fX1 == 3 && metaBig2->fX2 == 6);
}
