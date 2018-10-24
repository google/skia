/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColor.h"
#include "SkColorData.h"
#include "SkColorPriv.h"
#include "SkMathPriv.h"
#include "SkRandom.h"
#include "SkTypes.h"
#include "SkUnPreMultiply.h"
#include "Test.h"

#define GetPackedR16As32(packed)    (SkGetPackedR16(dc) << (8 - SK_R16_BITS))
#define GetPackedG16As32(packed)    (SkGetPackedG16(dc) << (8 - SK_G16_BITS))
#define GetPackedB16As32(packed)    (SkGetPackedB16(dc) << (8 - SK_B16_BITS))

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
    SkRandom r;

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
    SkRandom r;

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

DEF_TEST(Color, reporter) {
    test_premul(reporter);
    //test_interp(reporter);
    test_fast_interp(reporter);
    //test_565blend();
}

#include "GrColor.h"

DEF_GPUTEST(GrColor4s, reporter, /* options */) {
    // Test that GrColor -> GrColor4s -> GrColor round-trips perfectly
    for (unsigned i = 0; i <= 255; ++i) {
        GrColor r = GrColorPackRGBA(i, 0, 0, 0);
        GrColor g = GrColorPackRGBA(0, i, 0, 0);
        GrColor b = GrColorPackRGBA(0, 0, i, 0);
        GrColor a = GrColorPackRGBA(0, 0, 0, i);
        REPORTER_ASSERT(reporter, r == GrColor4s::FromGrColor(r).toGrColor());
        REPORTER_ASSERT(reporter, g == GrColor4s::FromGrColor(g).toGrColor());
        REPORTER_ASSERT(reporter, b == GrColor4s::FromGrColor(b).toGrColor());
        REPORTER_ASSERT(reporter, a == GrColor4s::FromGrColor(a).toGrColor());
        REPORTER_ASSERT(reporter, GrColor4s::FromGrColor(r).isNormalized());
        REPORTER_ASSERT(reporter, GrColor4s::FromGrColor(g).isNormalized());
        REPORTER_ASSERT(reporter, GrColor4s::FromGrColor(b).isNormalized());
        REPORTER_ASSERT(reporter, GrColor4s::FromGrColor(a).isNormalized());
    }

    // Test that floating point values are correctly detected as in/out of range, and that they
    // round-trip to within the limits of the fixed point precision
    float maxErr = 0, worstX = 0, worstRT = 0;
    {
        for (int i = -32768; i <= 32767; ++i) {
            float x = i / 4095.0f;
            float frgba[4] = { x, 0, 0, 0 };
            GrColor4s c4s = GrColor4s::FromFloat4(frgba);
            REPORTER_ASSERT(reporter, c4s.isNormalized() == (x >= 0.0f && x <= 1.0f));
            SkColor4f c4f = c4s.toSkColor4f();
            if (fabsf(c4f.fR - x) > maxErr) {
                maxErr = fabsf(c4f.fR - x);
                worstX = x;
                worstRT = c4f.fR;
            }
        }
    }

    REPORTER_ASSERT(reporter, maxErr < 0.0001f, "maxErr: %f, %f != %f", maxErr, worstX, worstRT);

    // Test clamping of unrepresentable values
    {
        float frgba[4] = { -8.5f, 9.0f, 0, 0 };
        GrColor4s c4s = GrColor4s::FromFloat4(frgba);
        REPORTER_ASSERT(reporter, !c4s.isNormalized());
        SkColor4f c4f = c4s.toSkColor4f();
        REPORTER_ASSERT(reporter, c4f.fR < -8.0f);
        REPORTER_ASSERT(reporter, c4f.fG >  8.0f);
    }
}
