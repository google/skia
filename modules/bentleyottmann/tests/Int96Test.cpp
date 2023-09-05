// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Int96.h"
#include "tests/Test.h"

#include <cstdint>
#include <limits>

using namespace bentleyottmann;

DEF_TEST(BO_Int96Basic, reporter) {
    {
        int32_t t = 0;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 0 && z.lo == 0);
    }
    {
        int64_t t = 0;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 0 && z.lo == 0);
    }
    {
        int32_t t = -1;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -1 && z.lo == 0xFFFFFFFF);
    }
    {
        int64_t t = -1;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -1 && z.lo == 0xFFFFFFFF);
    }
    {
        int32_t t = 3;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 0 && z.lo == 3);
    }
    {
        int64_t t = 3;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 0 && z.lo == 3);
    }
    {
        int32_t t = -3;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -1 && z.lo == (uint32_t)-3);
    }
    {
        int64_t t = -3;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -1 && z.lo == (uint32_t)-3);
    }

    {
        int64_t t = 1ll << 32;
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == 1 && z.lo == 0);
    }
    {
        // -2 << 32 -- without the warnings.
        int64_t t = -(2ll << 32);
        Int96 z = Int96::Make(t);
        REPORTER_ASSERT(reporter, z.hi == -2 && z.lo == 0);
    }
}

[[maybe_unused]]
static int64_t interesting64[] = {-std::numeric_limits<int64_t>::max(),
                                  -std::numeric_limits<int64_t>::max() + 1,
                                  (int64_t) -std::numeric_limits<int32_t>::max() - 1,
                                  (int64_t) -std::numeric_limits<int32_t>::max(),
                                  (int64_t) -std::numeric_limits<int32_t>::max() + 1,
                                  -2,
                                  -1,
                                  0,
                                  1,
                                  2,
                                  (int64_t) std::numeric_limits<int32_t>::max() - 1,
                                  (int64_t) std::numeric_limits<int32_t>::max(),
                                  (int64_t) std::numeric_limits<int32_t>::max() + 1,
                                  std::numeric_limits<int64_t>::max() - 1,
                                  std::numeric_limits<int64_t>::max()};

DEF_TEST(BO_Int96Less, reporter) {
#if (defined(__clang__) || defined(__GNUC__)) && defined(__SIZEOF_INT128__)
    for (auto a : interesting64) {
        for (auto b : interesting64) {
            __int128 a128 = a,
                    b128 = b;
            bool l128 = a128 < b128,
                 g128 = b128 < a128;

            Int96 a96 = Int96::Make(a),
                  b96 = Int96::Make(b);
            bool l96 = a96 < b96,
                 g96 = b96 < a96;

            REPORTER_ASSERT(reporter, l128 == l96);
            REPORTER_ASSERT(reporter, g128 == g96);

        }
    }
#endif
}

DEF_TEST(BO_Int96Add, reporter) {
#if (defined(__clang__) || defined(__GNUC__)) && defined(__SIZEOF_INT128__)
    for (auto a : interesting64) {
        for (auto b : interesting64) {
            __int128 a128 = a,
                     b128 = b,
                     r128 = a128 + b128;

            Int96 a96 = Int96::Make(a),
                  b96 = Int96::Make(b),
                  r96 = a96 + b96;

            // Explicitly check the low bits.
            REPORTER_ASSERT(reporter, r96.lo == (r128 & 0xFFFFFFFF));

            // Build a __int128 from an Int96.
            __int128 hi128 = r96.hi,
                     lo128 = r96.lo,
                     all128 = hi128 * 0x1'0000'0000 + lo128;
            REPORTER_ASSERT(reporter, r128 == all128);
        }
    }
#endif
}

DEF_TEST(BO_Int96Mult, reporter) {
#if (defined(__clang__) || defined(__GNUC__)) && defined(__SIZEOF_INT128__)
    int32_t interesting32[] = {-std::numeric_limits<int32_t>::max(),
                               -std::numeric_limits<int32_t>::max() + 1,
                               -2,
                               -1,
                               0,
                               1,
                               2,
                               std::numeric_limits<int32_t>::max() - 1,
                               std::numeric_limits<int32_t>::max()};

    for (auto i64 : interesting64) {
        for (auto i32 : interesting32) {
            __int128 a128 = i64,
                     b128 = i32,
                     r128 = a128 * b128;

            Int96 r96 = multiply(i64, i32);

            // Explicitly check the low bits.
            REPORTER_ASSERT(reporter, r96.lo == (r128 & 0xFFFFFFFF));

            // Build a __int128 from an Int96.
            __int128 hi128 = r96.hi,
                     lo128 = r96.lo,
                     all128 = hi128 * 0x1'0000'0000 + lo128;
            REPORTER_ASSERT(reporter, r128 == all128);
        }
    }
#endif
}
