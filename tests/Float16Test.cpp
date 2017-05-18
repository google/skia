/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkAutoPixmapStorage.h"
#include "SkColor.h"
#include "SkHalf.h"
#include "SkOpts.h"
#include "SkPixmap.h"
#include "SkPM4f.h"
#include "SkRandom.h"

#include <cmath>

static bool eq_within_half_float(float a, float b) {
    const float kTolerance = 1.0f / (1 << (8 + 10));

    SkHalf ha = SkFloatToHalf(a);
    SkHalf hb = SkFloatToHalf(b);
    float a2 = SkHalfToFloat(ha);
    float b2 = SkHalfToFloat(hb);
    return fabsf(a2 - b2) <= kTolerance;
}

static bool eq_within_half_float(const SkPM4f& a, const SkPM4f& b) {
    for (int i = 0; i < 4; ++i) {
        if (!eq_within_half_float(a.fVec[i], b.fVec[i])) {
            return false;
        }
    }
    return true;
}

DEF_TEST(color_half_float, reporter) {
    const int w = 100;
    const int h = 100;

    SkImageInfo info = SkImageInfo::Make(w, h, kRGBA_F16_SkColorType, kPremul_SkAlphaType);

    SkAutoPixmapStorage pm;
    pm.alloc(info);
    REPORTER_ASSERT(reporter, pm.getSafeSize() == SkToSizeT(w * h * sizeof(uint64_t)));

    SkColor4f c4 { 1, 0.5f, 0.25f, 0.5f };
    pm.erase(c4);

    SkPM4f origpm4 = c4.premul();
    for (int y = 0; y < pm.height(); ++y) {
        for (int x = 0; x < pm.width(); ++x) {
            SkPM4f pm4 = SkPM4f::FromF16(pm.addrF16(x, y));
            REPORTER_ASSERT(reporter, eq_within_half_float(origpm4, pm4));
        }
    }
}

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
