/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTAllocator.h"
#include "tests/Test.h"

namespace {
struct C {
    C() : fID(-1) { ++gInstCnt; }
    C(int id) : fID(id) { ++gInstCnt; }
    C(C&&) = default;
    C& operator=(C&&) = default;
    ~C() { --gInstCnt; }
    int fID;

    static int gInstCnt;
};

int C::gInstCnt = 0;
}

// Checks that the allocator has the correct count, etc and that the element IDs are correct.
// Then pops popCnt items and checks again.
template<int N>
static void check_allocator_helper(GrTAllocator<C, N>* allocator, int cnt, int popCnt,
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
        for (int i = 0; i < popCnt; ++i) {
            allocator->pop_back();
        }
        check_allocator_helper(allocator, cnt - popCnt, 0, reporter);
    }
}

// Adds cnt items to the allocator, tests the cnts and iterators, pops popCnt items and checks
// again. Finally it resets the allocator and checks again.
template<int N>
static void check_allocator(GrTAllocator<C, N>* allocator, int cnt, int popCnt,
                            skiatest::Reporter* reporter) {
    SkASSERT(allocator);
    SkASSERT(allocator->empty());
    for (int i = 0; i < cnt; ++i) {
        // Try both variations of push_back().
        if (i % 1) {
            allocator->push_back(C(i));
        } else {
            allocator->push_back() = C(i);
        }
    }
    check_allocator_helper(allocator, cnt, popCnt, reporter);
    allocator->reset();
    check_allocator_helper(allocator, 0, 0, reporter);
}

template<int N>
static void run_allocator_test(GrTAllocator<C, N>* allocator, skiatest::Reporter* reporter) {
    check_allocator(allocator, 0, 0, reporter);
    check_allocator(allocator, 1, 1, reporter);
    check_allocator(allocator, 2, 2, reporter);
    check_allocator(allocator, 10, 1, reporter);
    check_allocator(allocator, 10, 5, reporter);
    check_allocator(allocator, 10, 10, reporter);
    check_allocator(allocator, 100, 10, reporter);
}

DEF_TEST(GrTAllocator, reporter) {
    // Test combinations of allocators with and without stack storage and with different block
    // sizes.
    GrTAllocator<C> a1(1);
    run_allocator_test(&a1, reporter);

    GrTAllocator<C> a2(2);
    run_allocator_test(&a2, reporter);

    GrTAllocator<C> a5(5);
    run_allocator_test(&a5, reporter);

    GrTAllocator<C, 1> sa1;
    run_allocator_test(&sa1, reporter);

    GrTAllocator<C, 3> sa3;
    run_allocator_test(&sa3, reporter);

    GrTAllocator<C, 4> sa4;
    run_allocator_test(&sa4, reporter);
}
