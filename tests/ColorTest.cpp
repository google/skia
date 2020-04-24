/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkTypes.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/private/SkColorData.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkMathPriv.h"
#include "tests/Test.h"

DEF_TEST(ColorPremul, reporter) {
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
DEF_TEST(ColorInterp, reporter) {
    SkRandom r;

    U8CPU a0 = 0;
    U8CPU a255 = 255;
    for (int i = 0; i < 200; i++) {
        SkColor colorSrc = r.nextU();
        SkColor colorDst = r.nextU();
        SkPMColor src = SkPreMultiplyColor(colorSrc);
        SkPMColor dst = SkPreMultiplyColor(colorDst);

        if (false) {
            REPORTER_ASSERT(reporter, SkFourByteInterp(src, dst, a0) == dst);
            REPORTER_ASSERT(reporter, SkFourByteInterp(src, dst, a255) == src);
        }
    }
}

DEF_TEST(ColorFastIterp, reporter) {
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
