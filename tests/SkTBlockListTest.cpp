/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTBlockList.h"
#include "tests/Test.h"

namespace {
struct C {
    C() : fID(-1) { ++gInstCnt; }
    C(int id) : fID(id) { ++gInstCnt; }
    C(C&& c) : C(c.fID) {}
    C(const C& c) : C(c.fID) {}

    C& operator=(C&&) = default;
    C& operator=(const C&) = default;

    ~C() { --gInstCnt; }

    int fID;

    // Under the hood, SkTBlockList and SkBlockAllocator round up to max_align_t. If 'C' was
    // just 4 bytes, that often means the internal blocks can squeeze a few extra instances in. This
    // is fine, but makes predicting a little trickier, so make sure C is a bit bigger.
    int fPadding[4];

    static int gInstCnt;
};
int C::gInstCnt = 0;

struct D {
    int fID;
};

}  // namespace

class TBlockListTestAccess {
public:
    template<int N>
    static size_t ScratchBlockSize(SkTBlockList<C, N>& list) {
        return (size_t) list.fAllocator->scratchBlockSize();
    }

    template<int N>
    static size_t TotalSize(SkTBlockList<C, N>& list) {
        return list.fAllocator->totalSize();
    }
};

// Checks that the allocator has the correct count, etc and that the element IDs are correct.
// Then pops popCnt items and checks again.
template<int N>
static void check_allocator_helper(SkTBlockList<C, N>* allocator, int cnt, int popCnt,
                                   skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, (0 == cnt) == allocator->empty());
    REPORTER_ASSERT(reporter, cnt == allocator->count());
    REPORTER_ASSERT(reporter, cnt == C::gInstCnt);

    int i = 0;
    for (const C& c : allocator->items()) {
        REPORTER_ASSERT(reporter, i == c.fID);
        REPORTER_ASSERT(reporter, allocator->item(i).fID == i);
        ++i;
    }
    REPORTER_ASSERT(reporter, i == cnt);

    if (cnt > 0) {
        REPORTER_ASSERT(reporter, cnt-1 == allocator->back().fID);
    }

    if (popCnt > 0) {
        for (i = 0; i < popCnt; ++i) {
            allocator->pop_back();
        }
        check_allocator_helper(allocator, cnt - popCnt, 0, reporter);
    }
}

template<int N>
static void check_iterator_helper(SkTBlockList<C, N>* allocator,
                                  const std::vector<C*>& expected,
                                  skiatest::Reporter* reporter) {
    const SkTBlockList<C, N>* cAlloc = allocator;
    REPORTER_ASSERT(reporter, (size_t) allocator->count() == expected.size());
    // Forward+const
    int i = 0;
    for (const C& c : cAlloc->items()) {
        REPORTER_ASSERT(reporter, (uintptr_t) &c == (uintptr_t) expected[i]);
        ++i;
    }
    REPORTER_ASSERT(reporter, (size_t) i == expected.size());

    // Forward+non-const
    i = 0;
    for (C& c : allocator->items()) {
        REPORTER_ASSERT(reporter, (uintptr_t) &c == (uintptr_t) expected[i]);
        ++i;
    }
    REPORTER_ASSERT(reporter, (size_t) i == expected.size());

    // Reverse+const
    i = (int) expected.size() - 1;
    for (const C& c : cAlloc->ritems()) {
        REPORTER_ASSERT(reporter, (uintptr_t) &c == (uintptr_t) expected[i]);
        --i;
    }
    REPORTER_ASSERT(reporter, i == -1);

    // Reverse+non-const
    i = (int) expected.size() - 1;
    for (C& c : allocator->ritems()) {
        REPORTER_ASSERT(reporter, (uintptr_t) &c == (uintptr_t) expected[i]);
        --i;
    }
    REPORTER_ASSERT(reporter, i == -1);

    // Also test random access
    for (i = 0; i < allocator->count(); ++i) {
        REPORTER_ASSERT(reporter, (uintptr_t) &allocator->item(i) == (uintptr_t) expected[i]);
        REPORTER_ASSERT(reporter, (uintptr_t) &cAlloc->item(i) == (uintptr_t) expected[i]);
    }
}

// Adds cnt items to the allocator, tests the cnts and iterators, pops popCnt items and checks
// again. Finally it resets the allocator and checks again.
template<int N>
static void check_allocator(SkTBlockList<C, N>* allocator, int cnt, int popCnt,
                            skiatest::Reporter* reporter) {
    enum ItemInitializer : int {
        kCopyCtor,
        kMoveCtor,
        kCopyAssign,
        kMoveAssign,
        kEmplace,
    };
    static constexpr int kInitCount = (int) kEmplace + 1;

    SkASSERT(allocator);
    SkASSERT(allocator->empty());
    std::vector<C*> items;
    for (int i = 0; i < cnt; ++i) {
        switch((ItemInitializer) (i % kInitCount)) {
            case kCopyCtor:
                allocator->push_back(C(i));
                break;
            case kMoveCtor:
                allocator->push_back(std::move(C(i)));
                break;
            case kCopyAssign:
                allocator->push_back() = C(i);
                break;
            case kMoveAssign:
                allocator->push_back() = std::move(C(i));
                break;
            case kEmplace:
                allocator->emplace_back(i);
                break;
        }
        items.push_back(&allocator->back());
    }
    check_iterator_helper(allocator, items, reporter);
    check_allocator_helper(allocator, cnt, popCnt, reporter);
    allocator->reset();
    check_iterator_helper(allocator, {}, reporter);
    check_allocator_helper(allocator, 0, 0, reporter);
}

template<int N>
static void run_allocator_test(SkTBlockList<C, N>* allocator, skiatest::Reporter* reporter) {
    check_allocator(allocator, 0, 0, reporter);
    check_allocator(allocator, 1, 1, reporter);
    check_allocator(allocator, 2, 2, reporter);
    check_allocator(allocator, 10, 1, reporter);
    check_allocator(allocator, 10, 5, reporter);
    check_allocator(allocator, 10, 10, reporter);
    check_allocator(allocator, 100, 10, reporter);
}

template<int N1, int N2>
static void run_concat_test(skiatest::Reporter* reporter, int aCount, int bCount) {

    SkTBlockList<C, N1> listA;
    SkTBlockList<C, N2> listB;

    for (int i = 0; i < aCount; ++i) {
        listA.emplace_back(i);
    }
    for (int i = 0; i < bCount; ++i) {
        listB.emplace_back(aCount + i);
    }

    REPORTER_ASSERT(reporter, listA.count() == aCount && listB.count() == bCount);
    REPORTER_ASSERT(reporter, C::gInstCnt == aCount + bCount);

    // Concatenate B into A and verify.
    listA.concat(std::move(listB));
    REPORTER_ASSERT(reporter, listA.count() == aCount + bCount);
    // SkTBlockList guarantees the moved list is empty, but clang-tidy doesn't know about it;
    // in practice we won't really be using moved lists so this won't pollute our main code base
    // with lots of warning disables.
    REPORTER_ASSERT(reporter, listB.count() == 0); // NOLINT(bugprone-use-after-move)
    REPORTER_ASSERT(reporter, C::gInstCnt == aCount + bCount);

    int i = 0;
    for (const C& item : listA.items()) {
        // By construction of A and B originally, the concatenated id sequence is continuous
        REPORTER_ASSERT(reporter, i == item.fID);
        i++;
    }
    REPORTER_ASSERT(reporter, i == (aCount + bCount));
}

template<int N1, int N2>
static void run_concat_trivial_test(skiatest::Reporter* reporter, int aCount, int bCount) {
    static_assert(std::is_trivially_copyable<D>::value);

    // This is similar to run_concat_test(), except since D is trivial we can't verify the instant
    // counts that are tracked via ctor/dtor.
    SkTBlockList<D, N1> listA;
    SkTBlockList<D, N2> listB;

    for (int i = 0; i < aCount; ++i) {
        listA.push_back({i});
    }
    for (int i = 0; i < bCount; ++i) {
        listB.push_back({aCount + i});
    }

    REPORTER_ASSERT(reporter, listA.count() == aCount && listB.count() == bCount);
    // Concatenate B into A and verify.
    listA.concat(std::move(listB));
    REPORTER_ASSERT(reporter, listA.count() == aCount + bCount);
    REPORTER_ASSERT(reporter, listB.count() == 0); // NOLINT(bugprone-use-after-move): see above

    int i = 0;
    for (const D& item : listA.items()) {
        // By construction of A and B originally, the concatenated id sequence is continuous
        REPORTER_ASSERT(reporter, i == item.fID);
        i++;
    }
    REPORTER_ASSERT(reporter, i == (aCount + bCount));
}

template<int N>
static void run_reserve_test(skiatest::Reporter* reporter) {
    constexpr int kItemsPerBlock = N + 4; // Make this a number > 1, even if N starting items == 1

    SkTBlockList<C, N> list(kItemsPerBlock);
    size_t initialSize = TBlockListTestAccess::TotalSize(list);
    // Should be able to add N instances of T w/o changing size from initialSize
    for (int i = 0; i < N; ++i) {
        list.push_back(C(i));
    }
    REPORTER_ASSERT(reporter, initialSize == TBlockListTestAccess::TotalSize(list));

    // Reserve room for 2*kItemsPerBlock items
    list.reserve(2 * kItemsPerBlock);
    REPORTER_ASSERT(reporter, list.count() == N); // count shouldn't change though

    size_t reservedSize = TBlockListTestAccess::TotalSize(list);
    REPORTER_ASSERT(reporter, reservedSize >= initialSize + 2 * kItemsPerBlock * sizeof(C));
    for (int i = 0; i < 2 * kItemsPerBlock; ++i) {
        list.push_back(C(i));
    }
    REPORTER_ASSERT(reporter, reservedSize == TBlockListTestAccess::TotalSize(list));

    // Make the next block partially fully (N > 0 but < kItemsPerBlock)
    for (int i = 0; i < N; ++i) {
        list.push_back(C(i));
    }

    // Reserve room again for 2*kItemsPerBlock, but reserve should automatically take account of the
    // (kItemsPerBlock-N) that are still available in the active block
    list.reserve(2 * kItemsPerBlock);
    int extraReservedCount = kItemsPerBlock + N;
    // Because SkTBlockList normally allocates blocks in fixed sizes, and extraReservedCount >
    // items-per-block, it will always use that size and not that of the growth policy.
    REPORTER_ASSERT(reporter, TBlockListTestAccess::ScratchBlockSize(list) >=
                                       extraReservedCount * sizeof(C));

    reservedSize = TBlockListTestAccess::TotalSize(list);
    for (int i = 0; i < 2 * kItemsPerBlock; ++i) {
        list.push_back(C(i));
    }
    REPORTER_ASSERT(reporter, reservedSize == TBlockListTestAccess::TotalSize(list));

    // If we reserve a count < items-per-block, it will use the fixed size from the growth policy.
    list.reserve(2);
    REPORTER_ASSERT(reporter, TBlockListTestAccess::ScratchBlockSize(list) >=
                                       kItemsPerBlock * sizeof(C));

    // Ensure the reservations didn't initialize any more D's than anticipated
    int expectedInstanceCount = 2 * (N + 2 * kItemsPerBlock);
    REPORTER_ASSERT(reporter, expectedInstanceCount == C::gInstCnt);

    list.reset();
    REPORTER_ASSERT(reporter, 0 == C::gInstCnt);
}

DEF_TEST(SkTBlockList, reporter) {
    // Test combinations of allocators with and without stack storage and with different block sizes
    SkTBlockList<C> a1(1);
    run_allocator_test(&a1, reporter);

    SkTBlockList<C> a2(2);
    run_allocator_test(&a2, reporter);

    SkTBlockList<C> a5(5);
    run_allocator_test(&a5, reporter);

    SkTBlockList<C, 1> sa1;
    run_allocator_test(&sa1, reporter);

    SkTBlockList<C, 3> sa3;
    run_allocator_test(&sa3, reporter);

    SkTBlockList<C, 4> sa4;
    run_allocator_test(&sa4, reporter);

    run_reserve_test<1>(reporter);
    run_reserve_test<2>(reporter);
    run_reserve_test<3>(reporter);
    run_reserve_test<4>(reporter);
    run_reserve_test<5>(reporter);

    run_concat_test<1, 1>(reporter, 10, 10);
    run_concat_test<5, 1>(reporter, 50, 10);
    run_concat_test<1, 5>(reporter, 10, 50);
    run_concat_test<5, 5>(reporter, 100, 100);

    run_concat_trivial_test<1, 1>(reporter, 10, 10);
    run_concat_trivial_test<5, 1>(reporter, 50, 10);
    run_concat_trivial_test<1, 5>(reporter, 10, 50);
    run_concat_trivial_test<5, 5>(reporter, 100, 100);
}
