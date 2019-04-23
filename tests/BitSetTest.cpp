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
    REPORTER_ASSERT(reporter, set0.has(0) == false);
    REPORTER_ASSERT(reporter, set0.has(32767) == false);
    REPORTER_ASSERT(reporter, set0.has(65535) == false);

    set0.set(22);
    REPORTER_ASSERT(reporter, set0.has(22) == true);
    set0.set(24);
    REPORTER_ASSERT(reporter, set0.has(24) == true);
    set0.set(35);  // on a different DWORD
    REPORTER_ASSERT(reporter, set0.has(35) == true);
    REPORTER_ASSERT(reporter, set0.has(24) == true);
    REPORTER_ASSERT(reporter, set0.has(35) == true);

    std::vector<unsigned int> data;
    set0.getSetValues([&data](unsigned v) { data.push_back(v); });

    REPORTER_ASSERT(reporter, data.size() == 3);
    REPORTER_ASSERT(reporter, data[0] == 22);
    REPORTER_ASSERT(reporter, data[1] == 24);
    REPORTER_ASSERT(reporter, data[2] == 35);

    SkBitSet set1(65536);
    set1.set(12345);
    REPORTER_ASSERT(reporter, set0.has(12345) == false);
    REPORTER_ASSERT(reporter, set1.has(12345) == true);
    REPORTER_ASSERT(reporter, set1.has(22) == false);
    REPORTER_ASSERT(reporter, set0.has(35) == true);
}
