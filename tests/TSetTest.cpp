/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkTSet.h"

// Tests the SkTSet<T> class template.
// Functions that just call SkTDArray are not tested.

static void TestTSet_basic(skiatest::Reporter* reporter) {
    SkTSet<int> set0;
    REPORTER_ASSERT(reporter,  set0.isEmpty());
    REPORTER_ASSERT(reporter, !set0.contains(-1));
    REPORTER_ASSERT(reporter, !set0.contains(0));
    REPORTER_ASSERT(reporter, !set0.contains(1));
    REPORTER_ASSERT(reporter,  set0.count() == 0);

    REPORTER_ASSERT(reporter,  set0.add(0));
    REPORTER_ASSERT(reporter, !set0.isEmpty());
    REPORTER_ASSERT(reporter, !set0.contains(-1));
    REPORTER_ASSERT(reporter,  set0.contains(0));
    REPORTER_ASSERT(reporter, !set0.contains(1));
    REPORTER_ASSERT(reporter,  set0.count() == 1);
    REPORTER_ASSERT(reporter, !set0.add(0));
    REPORTER_ASSERT(reporter,  set0.count() == 1);

#ifdef SK_DEBUG
    set0.validate();
#endif
}

#define COUNT 1732
#define PRIME1 10007
#define PRIME2 1733

// Generates a series of positive unique pseudo-random numbers.
static int f(int i) {
    return (long(i) * PRIME1) % PRIME2;
}

// Will expose contains() too.
static void TestTSet_advanced(skiatest::Reporter* reporter) {
    SkTSet<int> set0;

    for (int i = 0; i < COUNT; i++) {
        REPORTER_ASSERT(reporter, !set0.contains(f(i)));
        if (i > 0) {
            REPORTER_ASSERT(reporter,  set0.contains(f(0)));
            REPORTER_ASSERT(reporter,  set0.contains(f(i / 2)));
            REPORTER_ASSERT(reporter,  set0.contains(f(i - 1)));
        }
        REPORTER_ASSERT(reporter, !set0.contains(f(i)));
        REPORTER_ASSERT(reporter,  set0.count() == i);
        REPORTER_ASSERT(reporter,  set0.add(f(i)));
        REPORTER_ASSERT(reporter,  set0.contains(f(i)));
        REPORTER_ASSERT(reporter,  set0.count() == i + 1);
        REPORTER_ASSERT(reporter, !set0.add(f(i)));
    }

    // Test deterministic output
    for (int i = 0; i < COUNT; i++) {
        REPORTER_ASSERT(reporter, set0[i] == f(i));
    }

    // Test copy constructor too.
    SkTSet<int> set1 = set0;

    REPORTER_ASSERT(reporter, set0.count() == set1.count());
    REPORTER_ASSERT(reporter, !set1.contains(-1000));

    for (int i = 0; i < COUNT; i++) {
        REPORTER_ASSERT(reporter, set1.contains(f(i)));
        REPORTER_ASSERT(reporter, set1[i] == f(i));
    }

    // Test operator= too.
    SkTSet<int> set2;
    set2 = set0;

    REPORTER_ASSERT(reporter, set0.count() == set2.count());
    REPORTER_ASSERT(reporter, !set2.contains(-1000));

    for (int i = 0; i < COUNT; i++) {
        REPORTER_ASSERT(reporter, set2.contains(f(i)));
        REPORTER_ASSERT(reporter, set2[i] == f(i));
    }

#ifdef SK_DEBUG
    set0.validate();
    set1.validate();
    set2.validate();
#endif
}

static void TestTSet_merge(skiatest::Reporter* reporter) {
    SkTSet<int> set;
    SkTSet<int> setOdd;

    for (int i = 0; i < COUNT; i++) {
        REPORTER_ASSERT(reporter, set.add(2 * i));
        REPORTER_ASSERT(reporter, setOdd.add(2 * i + 1));
    }
    // mergeInto returns the number of duplicates. Expected 0.
    REPORTER_ASSERT(reporter, set.mergeInto(setOdd) == 0);
    REPORTER_ASSERT(reporter, set.count() == 2 * COUNT);

    // mergeInto should now find all new numbers duplicate.
    REPORTER_ASSERT(reporter, set.mergeInto(setOdd) == setOdd.count());
    REPORTER_ASSERT(reporter, set.count() == 2 * COUNT);

    for (int i = 0; i < 2 * COUNT; i++) {
        REPORTER_ASSERT(reporter, set.contains(i));
    }

    // check deterministic output
    for (int i = 0; i < COUNT; i++) {
        REPORTER_ASSERT(reporter, set[i] == 2 * i);
        REPORTER_ASSERT(reporter, set[COUNT + i] == 2 * i + 1);
    }

#ifdef SK_DEBUG
    set.validate();
    setOdd.validate();
#endif
}

DEF_TEST(TSet, reporter) {
    TestTSet_basic(reporter);
    TestTSet_advanced(reporter);
    TestTSet_merge(reporter);
}
