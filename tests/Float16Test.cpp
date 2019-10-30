/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkPixmap.h"
#include "include/private/SkHalf.h"
#include "include/private/SkTo.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkOpts.h"
#include "tests/Test.h"

#include <cmath>

static bool is_denorm(uint16_t h) {
    return (h & 0x7fff) < 0x0400;
}

static bool is_finite(uint16_t h) {
    return (h & 0x7c00) != 0x7c00;
}

DEF_TEST(SkHalfToFloat_finite_ftz, r) {
    for (uint32_t h = 0; h <= 0xffff; h++) {
        if (!is_finite(h)) {
            // _finite_ftz() only works for values that can be represented as a finite half float.
            continue;
        }

        // _finite_ftz() may flush denorms to zero.  0.0f will compare == with both +0.0f and -0.0f.
        float expected  = SkHalfToFloat(h),
              alternate = is_denorm(h) ? 0.0f : expected;

        float actual = SkHalfToFloat_finite_ftz(h)[0];

        REPORTER_ASSERT(r, actual == expected || actual == alternate);
    }
}

DEF_TEST(SkFloatToHalf_finite_ftz, r) {
#if 0
    for (uint64_t bits = 0; bits <= 0xffffffff; bits++) {
#else
    SkRandom rand;
    for (int i = 0; i < 1000000; i++) {
        uint32_t bits = rand.nextU();
#endif
        float f;
        memcpy(&f, &bits, 4);

        uint16_t expected = SkFloatToHalf(f);
        if (!is_finite(expected)) {
            // _finite_ftz() only works for values that can be represented as a finite half float.
            continue;
        }

        uint16_t alternate = expected;
        if (is_denorm(expected)) {
            // _finite_ftz() may flush denorms to zero, and happens to keep the sign bit.
            alternate = std::signbit(f) ? 0x8000 : 0x0000;
        }

        uint16_t actual = SkFloatToHalf_finite_ftz(Sk4f{f})[0];
        // _finite_ftz() may truncate instead of rounding, so it may be one too small.
        REPORTER_ASSERT(r, actual == expected  || actual == expected  - 1 ||
                           actual == alternate || actual == alternate - 1);
    }
}
