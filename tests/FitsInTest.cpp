/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkTFitsIn.h"
#include "tests/Test.h"

#include <limits>

#define TEST(S, s, D, expected) REPORTER_ASSERT(reporter, (SkTFitsIn<D>((S)(s)) == (expected)))

enum TestEnum_t : uint8_t {
    kFoo,
    kBar,
    kBaz,
};

DEF_TEST(FitsIn, reporter) {
    TEST(uint16_t, 257, int8_t, false);

    TEST(int32_t,  1, int8_t, true);
    TEST(int32_t, -1, int8_t, true);
    TEST(int32_t,  (int32_t)(std::numeric_limits<int8_t>::max)(),    int8_t, true);
    TEST(int32_t, ((int32_t)(std::numeric_limits<int8_t>::max)())+1, int8_t, false);
    TEST(int32_t,  (int32_t)(std::numeric_limits<int8_t>::min)(),    int8_t, true);
    TEST(int32_t, (int32_t)((std::numeric_limits<int8_t>::min)())-1, int8_t, false);

    TEST(int32_t,  1, uint8_t, true);
    TEST(int32_t, -1, uint8_t, false);
    TEST(int32_t,  (int32_t)(std::numeric_limits<uint8_t>::max)(),    uint8_t, true);
    TEST(int32_t, ((int32_t)(std::numeric_limits<uint8_t>::max)())+1, uint8_t, false);
    TEST(int32_t,  0, uint8_t, true);
    TEST(int32_t, -1, uint8_t, false);
    TEST(int32_t, -127, uint8_t, false);
    TEST(int32_t, -128, uint8_t, false);

    TEST(uint8_t, 2, TestEnum_t, true);
    TEST(TestEnum_t, kBar, uint8_t, true);

    TEST(int32_t, 1000, int8_t, false);
    TEST(int32_t, 1000, uint8_t, false);

    TEST(int32_t, 1, int32_t, true);
    TEST(int32_t, -1, int32_t, true);
    TEST(int32_t, 1, uint32_t, true);
    TEST(int32_t, -1, uint32_t, false);

    TEST(int32_t, 1, int64_t, true);
    TEST(int32_t, -1, int64_t, true);
    TEST(int32_t, 1, uint64_t, true);
    TEST(int32_t, -1, uint64_t, false);

    TEST(uint32_t, 1, int8_t, true);
    TEST(uint32_t, 1, uint8_t, true);
    TEST(uint32_t, 1, int32_t, true);
    TEST(uint32_t, 1, uint32_t, true);
    TEST(uint32_t, 1, int64_t, true);
    TEST(uint32_t, 1, uint64_t, true);

    TEST(uint32_t, (std::numeric_limits<uint32_t>::max)(), int8_t, false);
    TEST(uint32_t, (std::numeric_limits<uint32_t>::max)(), uint8_t, false);
    TEST(uint32_t, (std::numeric_limits<uint32_t>::max)(), int32_t, false);
    TEST(uint32_t, (std::numeric_limits<uint32_t>::max)(), uint32_t, true);
    TEST(uint32_t, (std::numeric_limits<uint32_t>::max)(), int64_t, true);
    TEST(uint32_t, (std::numeric_limits<uint32_t>::max)(), uint64_t, true);

    TEST(uint64_t, 1, int8_t, true);
    TEST(uint64_t, 1, uint8_t, true);
    TEST(uint64_t, 1, int32_t, true);
    TEST(uint64_t, 1, uint32_t, true);
    TEST(uint64_t, 1, int64_t, true);
    TEST(uint64_t, 1, uint64_t, true);

    // Uncommenting the following should cause compile failures.
    //TEST(float, 1, uint64_t, true);
}
