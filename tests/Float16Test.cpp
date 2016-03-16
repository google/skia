/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkColor.h"
#include "SkHalf.h"
#include "SkOpts.h"
#include "SkPixmap.h"
#include "SkPM4f.h"
#include "SkRandom.h"

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

DEF_TEST(float_to_half, reporter) {
    const float    fs[] = {    1.0,    2.0,    3.0,    4.0,    5.0,    6.0,    7.0 };
    const uint16_t hs[] = { 0x3c00, 0x4000, 0x4200, 0x4400, 0x4500, 0x4600, 0x4700 };

    uint16_t hscratch[7];
    SkOpts::float_to_half(hscratch, fs, 7);
    REPORTER_ASSERT(reporter, 0 == memcmp(hscratch, hs, sizeof(hs)));

    float fscratch[7];
    SkOpts::half_to_float(fscratch, hs, 7);
    REPORTER_ASSERT(reporter, 0 == memcmp(fscratch, fs, sizeof(fs)));
}

static uint32_t u(float f) {
    uint32_t x;
    memcpy(&x, &f, 4);
    return x;
}

DEF_TEST(HalfToFloat_01, r) {
    for (uint16_t h = 0; h < 0x8000; h++) {
        float f = SkHalfToFloat(h);
        if (f >= 0 && f <= 1) {
            float got = SkHalfToFloat_01(h)[0];
            if (got != f) {
                SkDebugf("0x%04x -> 0x%08x (%g), want 0x%08x (%g)\n",
                        h,
                        u(got), got,
                        u(f), f);
            }
            REPORTER_ASSERT(r, SkHalfToFloat_01(h)[0] == f);
            REPORTER_ASSERT(r, SkFloatToHalf_01(SkHalfToFloat_01(h)) == h);
        }
    }
}

DEF_TEST(FloatToHalf_01, r) {
#if 0
    for (uint32_t bits = 0; bits < 0x80000000; bits++) {
#else
    SkRandom rand;
    for (int i = 0; i < 1000000; i++) {
        uint32_t bits = rand.nextU();
#endif
        float f;
        memcpy(&f, &bits, 4);
        if (f >= 0 && f <= 1) {
            uint16_t h1 = (uint16_t)SkFloatToHalf_01(Sk4f(f,0,0,0)),
                     h2 = SkFloatToHalf(f);
            bool ok = (h1 == h2 || h1 == h2-1);
            REPORTER_ASSERT(r, ok);
            if (!ok) {
                SkDebugf("%08x (%d) -> %04x (%d), want %04x (%d)\n",
                         bits, bits>>23, h1, h1>>10, h2, h2>>10);
                break;
            }
        }
    }
}
