/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkRandom.h"
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

    SkBitSet set1(65536);
    set1.set(12345);
    REPORTER_ASSERT(reporter, set0.test(12345) == false);
    REPORTER_ASSERT(reporter, set1.test(12345) == true);
    REPORTER_ASSERT(reporter, set1.test(22) == false);
    REPORTER_ASSERT(reporter, set0.test(35) == true);

    set0.reset();
    REPORTER_ASSERT(reporter, !set0.findFirst());
    REPORTER_ASSERT(reporter, set0.test(1234) == false);

    set0.set();
    REPORTER_ASSERT(reporter, !set0.findFirstUnset());
    REPORTER_ASSERT(reporter, set0.test(5678) == true);
}

DEF_TEST(BitSetComparisonSizeMatching, reporter) {
    // Verify that bitsets of differing sizes are never equal, even if the contents match.
    SkBitSet bitsetA(123);
    SkBitSet bitsetB(123);
    REPORTER_ASSERT(reporter,   bitsetA == bitsetB);
    REPORTER_ASSERT(reporter, !(bitsetA != bitsetB));

    SkBitSet bitsetC(345);
    REPORTER_ASSERT(reporter,   bitsetA != bitsetC);
    REPORTER_ASSERT(reporter, !(bitsetA == bitsetC));

    SkBitSet bitsetD(67);
    REPORTER_ASSERT(reporter,   bitsetA != bitsetD);
    REPORTER_ASSERT(reporter, !(bitsetA == bitsetD));
}

DEF_TEST(BitSetComparisonBitMatching, reporter) {
    SkRandom random;
    for (int trial = 0; trial < 500; ++trial) {
        // Make two identical bitsets.
        int size = random.nextRangeU(1, 1000);
        SkBitSet bitsetA(size);
        SkBitSet bitsetB(size);

        for (int index = 0; index < size; ++index) {
            if (random.nextBool()) {
                bitsetA.set(index);
                bitsetB.set(index);
            }
        }

        // Verify that the sets are equal.
        REPORTER_ASSERT(reporter,   bitsetA == bitsetB);
        REPORTER_ASSERT(reporter, !(bitsetA != bitsetB));

        // Flip some bits randomly. Always flip an odd number of bits to ensure that our random bit
        // flips don't all cancel each other out.
        int numBitsToFlip = random.nextULessThan(size) | 1;
        for (int index = 0; index < numBitsToFlip; ++index) {
            int bitToFlip = random.nextULessThan(size);
            if (bitsetA.test(bitToFlip)) {
                bitsetA.reset(bitToFlip);
            } else {
                bitsetA.set(bitToFlip);
            }
        }

        // Verify that the sets are no longer equal.
        REPORTER_ASSERT(reporter,   bitsetA != bitsetB);
        REPORTER_ASSERT(reporter, !(bitsetA == bitsetB));
    }
}
