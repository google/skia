/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkColor.h"
#include "SkHalf.h"
#include "SkPixmap.h"

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

    SkColor4f c4 { 0.5f, 1, 0.5f, 0.25f };
    pm.erase(c4);

    SkPM4f origpm4 = c4.premul();
    for (int y = 0; y < pm.height(); ++y) {
        for (int x = 0; x < pm.width(); ++x) {
            SkPM4f pm4 = SkPM4f::FromF16(pm.addrF16(x, y));
            REPORTER_ASSERT(reporter, eq_within_half_float(origpm4, pm4));
        }
    }
}
