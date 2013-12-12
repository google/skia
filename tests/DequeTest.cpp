/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkDeque.h"

static void assert_count(skiatest::Reporter* reporter, const SkDeque& deq, int count) {
    if (0 == count) {
        REPORTER_ASSERT(reporter, deq.empty());
        REPORTER_ASSERT(reporter, 0 == deq.count());
        REPORTER_ASSERT(reporter, sizeof(int) == deq.elemSize());
        REPORTER_ASSERT(reporter, NULL == deq.front());
        REPORTER_ASSERT(reporter, NULL == deq.back());
    } else {
        REPORTER_ASSERT(reporter, !deq.empty());
        REPORTER_ASSERT(reporter, count == deq.count());
        REPORTER_ASSERT(reporter, sizeof(int) == deq.elemSize());
        REPORTER_ASSERT(reporter, NULL != deq.front());
        REPORTER_ASSERT(reporter, NULL != deq.back());
        if (1 == count) {
            REPORTER_ASSERT(reporter, deq.back() == deq.front());
        } else {
            REPORTER_ASSERT(reporter, deq.back() != deq.front());
        }
    }
}

static void assert_iter(skiatest::Reporter* reporter, const SkDeque& deq,
                        int max, int min) {
    // test forward iteration
    SkDeque::Iter iter(deq, SkDeque::Iter::kFront_IterStart);
    void* ptr;

    int value = max;
    while (NULL != (ptr = iter.next())) {
        REPORTER_ASSERT(reporter, value == *(int*)ptr);
        value -= 1;
    }
    REPORTER_ASSERT(reporter, value+1 == min);

    // test reverse iteration
    iter.reset(deq, SkDeque::Iter::kBack_IterStart);

    value = min;
    while (NULL != (ptr = iter.prev())) {
        REPORTER_ASSERT(reporter, value == *(int*)ptr);
        value += 1;
    }
    REPORTER_ASSERT(reporter, value-1 == max);

    // test mixed iteration
    iter.reset(deq, SkDeque::Iter::kFront_IterStart);

    value = max;
    // forward iteration half-way
    for (int i = 0; i < deq.count()/2 && NULL != (ptr = iter.next()); i++) {
        REPORTER_ASSERT(reporter, value == *(int*)ptr);
        value -= 1;
    }
    // then back down w/ reverse iteration
    while (NULL != (ptr = iter.prev())) {
        REPORTER_ASSERT(reporter, value == *(int*)ptr);
        value += 1;
    }
    REPORTER_ASSERT(reporter, value-1 == max);
}

// This helper is intended to only give the unit test access to SkDeque's
// private numBlocksAllocated method
class DequeUnitTestHelper {
public:
    int fNumBlocksAllocated;

    DequeUnitTestHelper(const SkDeque& deq) {
        fNumBlocksAllocated = deq.numBlocksAllocated();
    }
};

static void assert_blocks(skiatest::Reporter* reporter,
                          const SkDeque& deq,
                          int allocCount) {
    DequeUnitTestHelper helper(deq);

    if (0 == deq.count()) {
        REPORTER_ASSERT(reporter, 1 == helper.fNumBlocksAllocated);
    } else {
        int expected = (deq.count() + allocCount - 1) / allocCount;
        // A block isn't freed in the deque when it first becomes empty so
        // sometimes an extra block lingers around
        REPORTER_ASSERT(reporter,
            expected == helper.fNumBlocksAllocated ||
            expected+1 == helper.fNumBlocksAllocated);
    }
}

static void TestSub(skiatest::Reporter* reporter, int allocCount) {
    SkDeque deq(sizeof(int), allocCount);
    int i;

    // test pushing on the front

    assert_count(reporter, deq, 0);
    for (i = 1; i <= 10; i++) {
        *(int*)deq.push_front() = i;
    }
    assert_count(reporter, deq, 10);
    assert_iter(reporter, deq, 10, 1);
    assert_blocks(reporter, deq, allocCount);

    for (i = 0; i < 5; i++) {
        deq.pop_front();
    }
    assert_count(reporter, deq, 5);
    assert_iter(reporter, deq, 5, 1);
    assert_blocks(reporter, deq, allocCount);

    for (i = 0; i < 5; i++) {
        deq.pop_front();
    }
    assert_count(reporter, deq, 0);
    assert_blocks(reporter, deq, allocCount);

    // now test pushing on the back

    for (i = 10; i >= 1; --i) {
        *(int*)deq.push_back() = i;
    }
    assert_count(reporter, deq, 10);
    assert_iter(reporter, deq, 10, 1);
    assert_blocks(reporter, deq, allocCount);

    for (i = 0; i < 5; i++) {
        deq.pop_back();
    }
    assert_count(reporter, deq, 5);
    assert_iter(reporter, deq, 10, 6);
    assert_blocks(reporter, deq, allocCount);

    for (i = 0; i < 5; i++) {
        deq.pop_back();
    }
    assert_count(reporter, deq, 0);
    assert_blocks(reporter, deq, allocCount);

    // now test pushing/popping on both ends

    *(int*)deq.push_front() = 5;
    *(int*)deq.push_back() = 4;
    *(int*)deq.push_front() = 6;
    *(int*)deq.push_back() = 3;
    *(int*)deq.push_front() = 7;
    *(int*)deq.push_back() = 2;
    *(int*)deq.push_front() = 8;
    *(int*)deq.push_back() = 1;
    assert_count(reporter, deq, 8);
    assert_iter(reporter, deq, 8, 1);
    assert_blocks(reporter, deq, allocCount);
}

DEF_TEST(Deque, reporter) {
    // test it once with the default allocation count
    TestSub(reporter, 1);
    // test it again with a generous allocation count
    TestSub(reporter, 10);
}
