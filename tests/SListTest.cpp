/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTInternalSList.h"
#include "Test.h"

class SListEntry {
public:
    SListEntry* next() { return getSListNext(); }
private:
    SK_DECLARE_INTERNAL_SLIST_INTERFACE(SListEntry);
};

static bool verifyEmptyList(skiatest::Reporter* reporter,
                            const SkTInternalSList<SListEntry>& list,
                            const char* stage) {

    if (!list.isEmpty()) {
        ERRORF(reporter, "%s - List not empty", stage);
        return false;
    }
    if (0 != list.getCount()) {
        ERRORF(reporter, "%s - List count is not zero, %d instead", stage, list.getCount());
        return false;
    }
    if (NULL != list.head()) {
        ERRORF(reporter, "%s - List has elements when empty", stage);
        return false;
    }
    return true;
}

static bool verifyList(skiatest::Reporter* reporter,
                       const SkTInternalSList<SListEntry>& list,
                       const char* stage,
                       SListEntry* start, int count, int step = 1) {
    SListEntry* next = list.head();
    if (list.getCount() != count) {
        ERRORF(reporter, "%s - List was too short, %d instead of %d", stage, list.getCount(), count);
        return false;
    }
    int index = 0;
    for(SListEntry* value = start; index < count; value += step, ++index) {
        if (NULL == next) {
            ERRORF(reporter, "%s - List too short, should be %d", stage, count);
            return false;
        }
        if (next!= value) {
            ERRORF(reporter, "%s - List entries at index %d of %d don't match", stage, index, count);
            return false;
        }
        next = next->next();
    }
    if (NULL != next) {
        ERRORF(reporter, "%s - List too long, should be %d", stage, count);
        return false;
    }
    return true;
}

static void testTInternalSList(skiatest::Reporter* reporter) {
    // Build a test array of data
    static const int testArraySize = 10;
    SListEntry testArray[testArraySize];
    // Basic add remove tests
    SkTInternalSList<SListEntry> list;
    verifyEmptyList(reporter, list, "start");
    // Push values in, testing on the way
    for (int index = 0; index < testArraySize; ++index) {
        list.push(&testArray[index]);
        if (!verifyList(reporter, list, "push", &testArray[index], index+1, -1)) {
            return;
        }
    }
    // Now remove them again
    for (int index = testArraySize - 1; index >= 0; --index) {
        REPORTER_ASSERT(reporter, &testArray[index] == list.pop());
        if ((index != 0) &&
            !verifyList(reporter, list, "pop", &testArray[index-1], index, -1)) {
            return;
        }
    }
    verifyEmptyList(reporter, list, "end");
    // Move between list tests
    for (int index = 0; index < testArraySize; ++index) {
        list.push(&testArray[index]);
    }
    verifyList(reporter, list, "swap", &testArray[testArraySize-1], testArraySize, -1);
    SkTInternalSList<SListEntry> other;
    // Check swap moves the list over unchanged
    other.swap(&list);
    verifyEmptyList(reporter, list, "swap");
    verifyList(reporter, other, "swap", &testArray[testArraySize-1], testArraySize, -1);
    // Check pushAll optimizes to a swap when one of the is empty
    list.pushAll(&other);
    verifyList(reporter, list, "pushAll-empty", &testArray[testArraySize-1], testArraySize, -1);
    verifyEmptyList(reporter, other, "pushAll-empty");
    // Check pushAll when non empty works
    other.push(list.pop());
    other.pushAll(&list);
    verifyEmptyList(reporter, list, "pushAll");
    verifyList(reporter, other, "pushAll", &testArray[0], testArraySize, 1);
}

DEF_TEST(SList, reporter) {
    testTInternalSList(reporter);
}
