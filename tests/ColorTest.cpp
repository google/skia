/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkMathPriv.h"
#include "SkRandom.h"
#include "SkUnPreMultiply.h"

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
