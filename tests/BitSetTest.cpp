/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkBitSet.h"
#include "tests/Test.h"

#include <vector>

DEF_TEST(BitSet, reporter) {
    SkBitSet set0(65536);
    REPORTER_ASSERT(reporter, set0.size() == 65536);
    REPORTER_ASSERT(reporter, set0.test(0) == false);
    REPORTER_ASSERT(reporter, set0.test(32767) == false);
    REPORTER_ASSERT(reporter, set0.test(65535) == false);
    REPORTER_ASSERT(reporter, !set0.findFirst());

    set0.set(22);
    REPORTER_ASSERT(reporter, set0.test(22) == true);
    REPORTER_ASSERT(reporter, set0.findFirst());
    REPORTER_ASSERT(reporter, *set0.findFirst() == 22);
    set0.set(24);
    REPORTER_ASSERT(reporter, set0.test(24) == true);
    REPORTER_ASSERT(reporter, *set0.findFirst() == 22);
    set0.set(35);  // on a different DWORD
    REPORTER_ASSERT(reporter, set0.test(35) == true);
    REPORTER_ASSERT(reporter, *set0.findFirst() == 22);
    REPORTER_ASSERT(reporter, set0.test(24) == true);
    REPORTER_ASSERT(reporter, set0.test(35) == true);
    set0.set(21);
    REPORTER_ASSERT(reporter, set0.test(21) == true);
    REPORTER_ASSERT(reporter, *set0.findFirst() == 21);
    set0.reset(21);
    REPORTER_ASSERT(reporter, set0.test(21) == false);
    REPORTER_ASSERT(reporter, *set0.findFirst() == 22);

    std::vector<unsigned int> data;
    set0.forEachSetIndex([&data](unsigned v) { data.push_back(v); });

    REPORTER_ASSERT(reporter, data.size() == 3);
    REPORTER_ASSERT(reporter, data[0] == 22);
    REPORTER_ASSERT(reporter, data[1] == 24);
    REPORTER_ASSERT(reporter, data[2] == 35);

    SkBitSet set1(24680);
    set1.set(12345);
    REPORTER_ASSERT(reporter, set1.size() == 24680);
    REPORTER_ASSERT(reporter, set0.test(12345) == false);
    REPORTER_ASSERT(reporter, set1.test(12345) == true);
    REPORTER_ASSERT(reporter, set1.test(22) == false);
    REPORTER_ASSERT(reporter, set0.test(35) == true);

    // Test swap().
    set0.swap(set1);
    REPORTER_ASSERT(reporter, set0.size() == 24680);
    REPORTER_ASSERT(reporter, set1.size() == 65536);
    REPORTER_ASSERT(reporter, set1.test(12345) == false);
    REPORTER_ASSERT(reporter, set0.test(12345) == true);
    REPORTER_ASSERT(reporter, set0.test(22) == false);
    REPORTER_ASSERT(reporter, set1.test(35) == true);

    // Test lopping off bits and shrink/grow mechanics with resize().
    SkBitSet set2(100);
    for (int index = 0; index < 100; ++index) {
        set2.set(index);
    }
    REPORTER_ASSERT(reporter, !set2.findFirstUnset());

    for (int tempSize = 99; tempSize >= 0; --tempSize) {
        set2.resize(tempSize);
        REPORTER_ASSERT(reporter, set2.size() == size_t(tempSize));
        REPORTER_ASSERT(reporter, !set2.findFirstUnset());

        set2.resize(100);
        REPORTER_ASSERT(reporter, set2.size() == 100);
        REPORTER_ASSERT(reporter, *set2.findFirstUnset() == size_t(tempSize));
        for (int testIdx = tempSize; testIdx < 100; ++testIdx) {
            REPORTER_ASSERT(reporter, !set2.test(testIdx));
        }
    }
}
