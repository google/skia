/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkPaint.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "Test.h"

#include <cmath>
#include <cstdlib>

static inline void assert_color(skiatest::Reporter* reporter,
                                SkColor expected, SkColor actual, int tolerance) {
    REPORTER_ASSERT(reporter, abs((int)(SkColorGetA(expected) - SkColorGetA(actual))) <= tolerance);
    REPORTER_ASSERT(reporter, abs((int)(SkColorGetR(expected) - SkColorGetR(actual))) <= tolerance);
    REPORTER_ASSERT(reporter, abs((int)(SkColorGetG(expected) - SkColorGetG(actual))) <= tolerance);
    REPORTER_ASSERT(reporter, abs((int)(SkColorGetB(expected) - SkColorGetB(actual))) <= tolerance);
}

static inline void assert_color(skiatest::Reporter* reporter, SkColor expected, SkColor actual) {
    const int TOLERANCE = 1;
    assert_color(reporter, expected, actual, TOLERANCE);
}

/**
 * This test case is a mirror of the Android CTS tests for MatrixColorFilter
 * found in the android.graphics.ColorMatrixColorFilterTest class.
 */
static inline void test_colorMatrixCTS(skiatest::Reporter* reporter) {

    SkBitmap bitmap;
    bitmap.allocN32Pixels(1,1);

    SkCanvas canvas(bitmap);
    SkPaint paint;

    SkScalar blueToCyan[20] = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
    paint.setColorFilter(SkColorFilters::MatrixRowMajor255(blueToCyan));

    paint.setColor(SK_ColorBLUE);
    canvas.drawPoint(0, 0, paint);
    assert_color(reporter, SK_ColorCYAN, bitmap.getColor(0, 0));

    paint.setColor(SK_ColorGREEN);
    canvas.drawPoint(0, 0, paint);
    assert_color(reporter, SK_ColorGREEN, bitmap.getColor(0, 0));

    paint.setColor(SK_ColorRED);
    canvas.drawPoint(0, 0, paint);
    assert_color(reporter, SK_ColorRED, bitmap.getColor(0, 0));

    // color components are clipped, not scaled
    paint.setColor(SK_ColorMAGENTA);
    canvas.drawPoint(0, 0, paint);
    assert_color(reporter, SK_ColorWHITE, bitmap.getColor(0, 0));

    SkScalar transparentRedAddBlue[20] = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 64.0f,
           -0.5f, 0.0f, 0.0f, 1.0f, 0.0f
    };
    paint.setColorFilter(SkColorFilters::MatrixRowMajor255(transparentRedAddBlue));
    bitmap.eraseColor(SK_ColorTRANSPARENT);

    paint.setColor(SK_ColorRED);
    canvas.drawPoint(0, 0, paint);
    assert_color(reporter, SkColorSetARGB(128, 255, 0, 64), bitmap.getColor(0, 0), 2);

    paint.setColor(SK_ColorCYAN);
    canvas.drawPoint(0, 0, paint);
    // blue gets clipped
    assert_color(reporter, SK_ColorCYAN, bitmap.getColor(0, 0));

    // change array to filter out green
    REPORTER_ASSERT(reporter, 1.0f == transparentRedAddBlue[6]);
    transparentRedAddBlue[6] = 0.0f;

    // check that changing the array has no effect
    canvas.drawPoint(0, 0, paint);
    assert_color(reporter, SK_ColorCYAN, bitmap.getColor(0, 0));

    // create a new filter with the changed matrix
    paint.setColorFilter(SkColorFilters::MatrixRowMajor255(transparentRedAddBlue));
    canvas.drawPoint(0, 0, paint);
    assert_color(reporter, SK_ColorBLUE, bitmap.getColor(0, 0));
}

DEF_TEST(ColorMatrix, reporter) {
    test_colorMatrixCTS(reporter);
}


DEF_TEST(ColorMatrix_clamp_while_unpremul, r) {
    // This matrix does green += 255/255 and alpha += 32/255.  We want to test
    // that if we pass it opaque alpha and small red and blue values, red and
    // blue stay unchanged, not pumped up by that ~1.12 intermediate alpha.
    SkScalar m[] = {
        1, 0, 0, 0, 0,
        0, 1, 0, 0, 255,
        0, 0, 1, 0, 0,
        0, 0, 0, 1, 32,
    };
    auto filter = SkColorFilters::MatrixRowMajor255(m);

    SkColor filtered = filter->filterColor(0xff0a0b0c);
    REPORTER_ASSERT(r, SkColorGetA(filtered) == 0xff);
    REPORTER_ASSERT(r, SkColorGetR(filtered) == 0x0a);
    REPORTER_ASSERT(r, SkColorGetG(filtered) == 0xff);
    REPORTER_ASSERT(r, SkColorGetB(filtered) == 0x0c);
}
