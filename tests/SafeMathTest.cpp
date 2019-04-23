/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSafeMath.h"
#include "tests/Test.h"

DEF_TEST(SafeMath, r) {
    size_t max = std::numeric_limits<size_t>::max();

    {
        size_t halfMax = max >> 1;
        size_t halfMaxPlus1 = halfMax + 1;
        SkSafeMath safe;
        REPORTER_ASSERT(r, safe.add(halfMax, halfMax) == 2 * halfMax);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.add(halfMax, halfMaxPlus1) == max);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.add(max, 1) == 0);
        REPORTER_ASSERT(r, !safe);
    }

    {
        SkSafeMath safe;
        (void) safe.add(max, max);
        REPORTER_ASSERT(r, !safe);
    }

    {
        size_t bits = (sizeof(size_t) * 8);
        size_t halfBits = bits / 2;
        size_t sqrtMax = max >> halfBits;
        size_t sqrtMaxPlus1 = sqrtMax + 1;
        SkSafeMath safe;
        REPORTER_ASSERT(r, safe.mul(sqrtMax, sqrtMax) == sqrtMax * sqrtMax);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.mul(sqrtMax, sqrtMaxPlus1) == sqrtMax << halfBits);
        REPORTER_ASSERT(r, safe);
        REPORTER_ASSERT(r, safe.mul(sqrtMaxPlus1, sqrtMaxPlus1) == 0);
        REPORTER_ASSERT(r, !safe);
    }

    {
        SkSafeMath safe;
        (void) safe.mul(max, max);
        REPORTER_ASSERT(r, !safe);
    }
}
