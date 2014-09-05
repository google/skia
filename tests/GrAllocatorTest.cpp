/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
// This is a GPU-backend specific test
#if SK_SUPPORT_GPU
#include "GrAllocator.h"

namespace {
struct C {
    C() : fID(-1) { ++gInstCnt; }
    C(int id) : fID(id) { ++gInstCnt; }
    ~C() { --gInstCnt; }
    int fID;

    static int gInstCnt;
};

int C::gInstCnt = 0;
}

static void check_allocator_helper(GrTAllocator<C>* allocator, int cnt, int popCnt,
                                   skiatest::Reporter* reporter);

// Adds cnt items to the allocator, tests the cnts and iterators, pops popCnt items and checks
// again. Finally it resets the allocator and checks again.
static void check_allocator(GrTAllocator<C>* allocator, int cnt, int popCnt,
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

// Checks that the allocator has the correct count, etc and that the element IDs are correct.
// Then pops popCnt items and checks again.
static void check_allocator_helper(GrTAllocator<C>* allocator, int cnt, int popCnt,
                                   skiatest::Reporter* reporter) {
    REPORTER_ASSERT(reporter, (0 == cnt) == allocator->empty());
    REPORTER_ASSERT(reporter, cnt == allocator->count());
    REPORTER_ASSERT(reporter, cnt == C::gInstCnt);

    GrTAllocator<C>::Iter iter(allocator);
    for (int i = 0; i < cnt; ++i) {
        REPORTER_ASSERT(reporter, iter.next() && i == iter.get()->fID);
    }
    REPORTER_ASSERT(reporter, !iter.next());
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

DEF_TEST(GrAllocator, reporter) {

    // Test combinations of allocators with and without stack storage and with different block
    // sizes.
    SkTArray<GrTAllocator<C>*> allocators;
    GrTAllocator<C> a1(1);
    allocators.push_back(&a1);
    GrTAllocator<C> a2(2);
    allocators.push_back(&a2);
    GrTAllocator<C> a5(5);
    allocators.push_back(&a5);

    GrSTAllocator<1, C> sa1;
    allocators.push_back(&a1);
    GrSTAllocator<3, C> sa3;
    allocators.push_back(&sa3);
    GrSTAllocator<4, C> sa4;
    allocators.push_back(&sa4);

    for (int i = 0; i < allocators.count(); ++i) {
        check_allocator(allocators[i], 0, 0, reporter);
        check_allocator(allocators[i], 1, 1, reporter);
        check_allocator(allocators[i], 2, 2, reporter);
        check_allocator(allocators[i], 10, 1, reporter);
        check_allocator(allocators[i], 10, 5, reporter);
        check_allocator(allocators[i], 10, 10, reporter);
        check_allocator(allocators[i], 100, 10, reporter);
    }
}

#endif
