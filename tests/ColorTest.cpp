
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkMathPriv.h"
#include "SkRandom.h"
#include "SkUnPreMultiply.h"

#define GetPackedR16As32(packed)    (SkGetPackedR16(dc) << (8 - SK_R16_BITS))
#define GetPackedG16As32(packed)    (SkGetPackedG16(dc) << (8 - SK_G16_BITS))
#define GetPackedB16As32(packed)    (SkGetPackedB16(dc) << (8 - SK_B16_BITS))

static inline bool S32A_D565_Blend_0(SkPMColor sc, uint16_t dc, U8CPU alpha) {
    unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
    unsigned dr = SkMulS16(SkPacked32ToR16(sc), alpha) + SkMulS16(SkGetPackedR16(dc), dst_scale);
    unsigned dg = SkMulS16(SkPacked32ToG16(sc), alpha) + SkMulS16(SkGetPackedG16(dc), dst_scale);

    unsigned rr = SkDiv255Round(dr);
    unsigned rg = SkDiv255Round(dg);

    if (rr <= 31 && rg <= 63) {
        return true;
    }
    return false;
}

static inline bool S32A_D565_Blend_01(SkPMColor sc, uint16_t dc, U8CPU alpha) {
    unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
    unsigned dr = SkMulS16(SkGetPackedR32(sc), alpha) + SkMulS16(SkGetPackedR16(dc) << 3, dst_scale);
    unsigned dg = SkMulS16(SkGetPackedG32(sc), alpha) + SkMulS16(SkGetPackedG16(dc) << 2, dst_scale);

    unsigned rr = SkDiv255Round(dr) >> 3;
    unsigned rg = SkDiv255Round(dg) >> 2;

    if (rr <= 31 && rg <= 63) {
        return true;
    }
    return false;
}

static inline bool S32A_D565_Blend_02(SkPMColor sc, uint16_t dc, U8CPU alpha) {
    unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
    unsigned dr = SkMulS16(SkGetPackedR32(sc), alpha) + SkMulS16(GetPackedR16As32(dc), dst_scale);
    unsigned dg = SkMulS16(SkGetPackedG32(sc), alpha) + SkMulS16(GetPackedG16As32(dc), dst_scale);
    unsigned db = SkMulS16(SkGetPackedB32(sc), alpha) + SkMulS16(GetPackedB16As32(dc), dst_scale);
    int rc = SkPack888ToRGB16(SkDiv255Round(dr),
                              SkDiv255Round(dg),
                              SkDiv255Round(db));

    unsigned rr = SkGetPackedR16(rc);
    unsigned rg = SkGetPackedG16(rc);

    if (rr <= 31 && rg <= 63) {
        return true;
    }
    return false;
}

static inline bool S32A_D565_Blend_1(SkPMColor sc, uint16_t dc, U8CPU alpha) {
    unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
    unsigned dr = (SkMulS16(SkGetPackedR32(sc), alpha) >> 3) + SkMulS16(SkGetPackedR16(dc), dst_scale);
    unsigned dg = (SkMulS16(SkGetPackedG32(sc), alpha) >> 2) + SkMulS16(SkGetPackedG16(dc), dst_scale);

    unsigned rr = SkDiv255Round(dr);
    unsigned rg = SkDiv255Round(dg);

    if (rr <= 31 && rg <= 63) {
        return true;
    }
    return false;
}

static inline int SkDiv65025Round(int x) {
    return (x + 65025/2) / 65025;
//    return x / 65025;
}
static inline bool S32A_D565_Blend_2(SkPMColor sc, uint16_t dc, U8CPU alpha) {
    unsigned dst_scale = 255*255 - SkGetPackedA32(sc) * alpha;
    alpha *= 255;
    unsigned dr = (SkGetPackedR32(sc) >> 3) * alpha + SkGetPackedR16(dc) * dst_scale;
    unsigned dg = (SkGetPackedG32(sc) >> 2) * alpha + SkGetPackedG16(dc) * dst_scale;

    unsigned rr = SkDiv65025Round(dr);
    unsigned rg = SkDiv65025Round(dg);

    if (rr <= 31 && rg <= 63) {
        return true;
    }
    return false;
}

static inline void test_565blend() {
    int total_failures = 0;
    for (int global_alpha = 0; global_alpha <= 255; ++global_alpha) {
        int failures = 0;
        int total = 0;
        for (int src_a = 0; src_a <= 255; ++src_a) {
            for (int src_c = 0; src_c <= src_a; ++src_c) {
                SkPMColor sc = SkPackARGB32(src_a, src_c, src_c, src_c);
                for (int dst_r = 0; dst_r <= 31; ++dst_r) {
                    for (int dst_g = 0; dst_g <= 63; ++dst_g) {
                        uint16_t dc = SkPackRGB16(dst_r, dst_g, dst_r);
                        failures += !S32A_D565_Blend_0(sc, dc, global_alpha);
                        total += 1;
                    }
                }
            }
        }
        SkDebugf("global_alpha=%d failures=%d total=%d %g\n", global_alpha, failures, total, failures * 100.0 / total);
        total_failures += failures;
    }
    SkDebugf("total failures %d\n", total_failures);
}

static inline void test_premul(skiatest::Reporter* reporter) {
    for (int a = 0; a <= 255; a++) {
        for (int x = 0; x <= 255; x++) {
            SkColor c0 = SkColorSetARGB(a, x, x, x);
            SkPMColor p0 = SkPreMultiplyColor(c0);

            SkColor c1 = SkUnPreMultiply::PMColorToColor(p0);
            SkPMColor p1 = SkPreMultiplyColor(c1);

            // we can't promise that c0 == c1, since c0 -> p0 is a many to one
            // function, however, we can promise that p0 -> c1 -> p1 : p0 == p1
            REPORTER_ASSERT(reporter, p0 == p1);

            {
                int ax = SkMulDiv255Ceiling(x, a);
                REPORTER_ASSERT(reporter, ax <= a);
            }
        }
    }
}

/**
  This test fails: SkFourByteInterp does *not* preserve opaque destinations.
  SkAlpha255To256 implemented as (alpha + 1) is faster than
  (alpha + (alpha >> 7)), but inaccurate, and Skia intends to phase it out.
*/
/*
static void test_interp(skiatest::Reporter* reporter) {
    SkMWCRandom r;

    U8CPU a0 = 0;
    U8CPU a255 = 255;
    for (int i = 0; i < 200; i++) {
        SkColor colorSrc = r.nextU();
        SkColor colorDst = r.nextU();
        SkPMColor src = SkPreMultiplyColor(colorSrc);
        SkPMColor dst = SkPreMultiplyColor(colorDst);

        REPORTER_ASSERT(reporter, SkFourByteInterp(src, dst, a0) == dst);
        REPORTER_ASSERT(reporter, SkFourByteInterp(src, dst, a255) == src);
    }
}
*/

static inline void test_fast_interp(skiatest::Reporter* reporter) {
    SkMWCRandom r;

    U8CPU a0 = 0;
    U8CPU a255 = 255;
    for (int i = 0; i < 200; i++) {
        SkColor colorSrc = r.nextU();
        SkColor colorDst = r.nextU();
        SkPMColor src = SkPreMultiplyColor(colorSrc);
        SkPMColor dst = SkPreMultiplyColor(colorDst);

        REPORTER_ASSERT(reporter, SkFastFourByteInterp(src, dst, a0) == dst);
        REPORTER_ASSERT(reporter, SkFastFourByteInterp(src, dst, a255) == src);
    }
}

static void TestColor(skiatest::Reporter* reporter) {
    test_premul(reporter);
    //test_interp(reporter);
    test_fast_interp(reporter);
//    test_565blend();
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Color", ColorTestClass, TestColor)
