/*
 * Copyright 2017 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkSafeMath.h"
#include "tests/Test.h"

#include <cstddef>
#include <limits>

DEF_TEST(SafeMath_SizeT_Success, r) {
    constexpr size_t max = std::numeric_limits<size_t>::max();
    constexpr size_t halfMax = max >> 1;
    constexpr size_t halfMaxPlus1 = halfMax + 1;
    SkSafeMath safe;

    REPORTER_ASSERT(r, safe.add(halfMax, halfMax) == 2 * halfMax);
    REPORTER_ASSERT(r, safe.add(halfMax, halfMaxPlus1) == max);
    REPORTER_ASSERT(r, safe.ok());

    constexpr size_t bits = (sizeof(size_t) * 8);
    constexpr size_t halfBits = bits / 2;
    constexpr size_t sqrtMax = max >> halfBits;
    constexpr size_t sqrtMaxPlus1 = sqrtMax + 1;
    safe = {};
    REPORTER_ASSERT(r, safe.mul(sqrtMax, sqrtMax) == sqrtMax * sqrtMax);
    REPORTER_ASSERT(r, safe.mul(sqrtMax, sqrtMaxPlus1) == sqrtMax << halfBits);
    REPORTER_ASSERT(r, safe.ok());
}

DEF_TEST(SafeMath_SizeT_Failure, r) {
    constexpr size_t max = std::numeric_limits<size_t>::max();
    SkSafeMath safe;

    (void)safe.add(max, 1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.add(max, max);
    REPORTER_ASSERT(r, !safe.ok());

    constexpr size_t bits = (sizeof(size_t) * 8);
    constexpr size_t halfBits = bits / 2;
    constexpr size_t sqrtMax = max >> halfBits;
    constexpr size_t sqrtMaxPlus1 = sqrtMax + 1;
    safe = {};
    (void)safe.mul(sqrtMaxPlus1, sqrtMaxPlus1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.mul(max, max);
    REPORTER_ASSERT(r, !safe.ok());
}

DEF_TEST(SafeMath_Int_Success, r) {
    constexpr int maxInt = std::numeric_limits<int>::max();
    constexpr int minInt = std::numeric_limits<int>::min();
    SkSafeMath safe;

    REPORTER_ASSERT(r, safe.addInt(maxInt, 0) == maxInt);
    REPORTER_ASSERT(r, safe.addInt(minInt, 1) == minInt + 1);
    REPORTER_ASSERT(r, safe.subInt(minInt, 0) == minInt);
    REPORTER_ASSERT(r, safe.subInt(-1, minInt) == maxInt);
    REPORTER_ASSERT(r, safe.mulInt(maxInt, 1) == maxInt);
    REPORTER_ASSERT(r, safe.mulInt(maxInt, 0) == 0);
    REPORTER_ASSERT(r, safe.divInt(6, 3) == 2);
    REPORTER_ASSERT(r, safe.modInt(6, 3) == 0);
    REPORTER_ASSERT(r, safe.modInt(9, 4) == 1);
    REPORTER_ASSERT(r, safe.ok());
}

DEF_TEST(SafeMath_Int_Failure, r) {
    constexpr int maxInt = std::numeric_limits<int>::max();
    constexpr int minInt = std::numeric_limits<int>::min();
    constexpr int sqrtMaxInt = 46340;  // floor(sqrt(std::numeric_limits<int>::max()))
    SkSafeMath safe;

    (void)safe.addInt(maxInt, 1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.addInt(minInt, -1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.subInt(minInt, 1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.subInt(maxInt, -1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.mulInt(maxInt, 2);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.mulInt(sqrtMaxInt + 1, sqrtMaxInt + 1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.mulInt(maxInt, -2);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.mulInt(minInt, 2);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.mulInt(minInt, minInt);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.mulInt(minInt, -1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.divInt(6, 0);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.divInt(minInt, -1);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.modInt(6, 0);
    REPORTER_ASSERT(r, !safe.ok());

    safe = {};
    (void)safe.modInt(minInt, -1);
    REPORTER_ASSERT(r, !safe.ok());
}
