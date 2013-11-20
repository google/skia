/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTDStackNester.h"

#include "Test.h"
#include "TestClassDef.h"

/**
 *  Test SkTDStackNester<int>::push(). Pushes the current count onto the stack,
 *  and checks that the count has increased by one.
 */
static void test_push(skiatest::Reporter* reporter, SkTDStackNester<int>* nester) {
    SkASSERT(nester);
    const int count = nester->count();
    // test_pop depends on this value.
    nester->push(count);
    REPORTER_ASSERT(reporter, nester->count() == count + 1);
}

/**
 *  Test SkTDStackNester<int>::pop(). Pops the top element off the stack, and
 *  checks that the new count is one smaller, and that the popped element
 *  matches the new count (as was pushed by test_push).
 */
static void test_pop(skiatest::Reporter* reporter, SkTDStackNester<int>* nester) {
    SkASSERT(nester);
    const int count = nester->count();
    // This test should not be called with a count <= 0.
    SkASSERT(count > 0);
    const int top = nester->top();
    int value = -1;
    nester->pop(&value);
    REPORTER_ASSERT(reporter, top == value);
    const int newCount = nester->count();
    REPORTER_ASSERT(reporter, newCount == count - 1);
    // Since test_push always pushes the count prior to the push, value should
    // always be one less than count.
    REPORTER_ASSERT(reporter, newCount == value);
}

/**
 *  Test nest() and unnest(). nest() is called, and it is confirmed that the
 *  count is now zero. Then test_push() is called inc times, followed by a call to
 *  unnest(). After this call, check that the count has returned to the initial count, and
 *  that nestingLevel() has returned to its initial value.
 */
static void test_nest(skiatest::Reporter* reporter, SkTDStackNester<int>* nester, int inc) {
    SkASSERT(nester);
    SkASSERT(inc > 0);
    const int initialCount = nester->count();
    const int initialNesting = nester->nestingLevel();

    nester->nest();
    REPORTER_ASSERT(reporter, nester->count() == 0);
    REPORTER_ASSERT(reporter, nester->nestingLevel() == initialNesting + 1);

    for (int i = 0; i < inc; ++i) {
        test_push(reporter, nester);
    }

    nester->unnest();
    REPORTER_ASSERT(reporter, nester->count() == initialCount);
    REPORTER_ASSERT(reporter, nester->nestingLevel() == initialNesting);
}

class SkTDStackNesterTester {
public:
    static int GetSlotCount() {
        return SkTDStackNester<int>::kSlotCount;
    }
};

static void test_stack_nester(skiatest::Reporter* reporter) {
    SkTDStackNester<int> nester;
    int count = nester.count();
    REPORTER_ASSERT(reporter, 0 == count);
    REPORTER_ASSERT(reporter, nester.nestingLevel() == 0);
    REPORTER_ASSERT(reporter, nester.empty());

    // Test nesting (with arbitrary number of pushes) from the beginning.
    test_nest(reporter, &nester, 3);

    const int slotCount = SkTDStackNesterTester::GetSlotCount();

    // Test pushing beyond the boundary of the first Rec.
    for (; count < 2 * slotCount; ++count) {
        if (3 == count) {
            // Test nesting (an arbitrary number of pushes) early on.
            test_nest(reporter, &nester, 7);
        } else if (slotCount - 4 == count) {
            // Test nesting across the boundary of a Rec.
            test_nest(reporter, &nester, 6);
        }
        test_push(reporter, &nester);
    }

    // Pop everything off the stack except for the last one, to confirm
    // that the destructor handles a remaining object.
    while (nester.count() > 1) {
        test_pop(reporter, &nester);
    }
}

DEF_TEST(TDStackNester, reporter) {
    test_stack_nester(reporter);
}
