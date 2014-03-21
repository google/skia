/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTArray.h"
#include "Test.h"

// Tests the SkTArray<T> class template.

template <bool MEM_COPY>
static void TestTSet_basic(skiatest::Reporter* reporter) {
    SkTArray<int, MEM_COPY> a;

    // Starts empty.
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.count() == 0);

    // { }, add a default constructed element
    a.push_back() = 0;
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.count() == 1);

    // { 0 }, removeShuffle the only element.
    a.removeShuffle(0);
    REPORTER_ASSERT(reporter, a.empty());
    REPORTER_ASSERT(reporter, a.count() == 0);

    // { }, add a default, add a 1, remove first
    a.push_back() = 0;
    REPORTER_ASSERT(reporter, a.push_back() = 1);
    a.removeShuffle(0);
    REPORTER_ASSERT(reporter, !a.empty());
    REPORTER_ASSERT(reporter, a.count() == 1);
    REPORTER_ASSERT(reporter, a[0] == 1);

    // { 1 }, replace with new array
    int b[5] = { 0, 1, 2, 3, 4 };
    a.reset(b, SK_ARRAY_COUNT(b));
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b));
    REPORTER_ASSERT(reporter, a[2] == 2);
    REPORTER_ASSERT(reporter, a[4] == 4);

    // { 0, 1, 2, 3, 4 }, removeShuffle the last
    a.removeShuffle(4);
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b) - 1);
    REPORTER_ASSERT(reporter, a[3] == 3);

    // { 0, 1, 2, 3 }, remove a middle, note shuffle
    a.removeShuffle(1);
    REPORTER_ASSERT(reporter, a.count() == SK_ARRAY_COUNT(b) - 2);
    REPORTER_ASSERT(reporter, a[0] == 0);
    REPORTER_ASSERT(reporter, a[1] == 3);
    REPORTER_ASSERT(reporter, a[2] == 2);

    // {0, 3, 2 }
}

DEF_TEST(TArray, reporter) {
    TestTSet_basic<true>(reporter);
    TestTSet_basic<false>(reporter);
}
