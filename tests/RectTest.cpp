/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "src/core/SkRectPriv.h"
#include "tests/Test.h"

static bool has_green_pixels(const SkBitmap& bm) {
    for (int j = 0; j < bm.height(); ++j) {
        for (int i = 0; i < bm.width(); ++i) {
            if (SkColorGetG(bm.getColor(i, j))) {
                return true;
            }
        }
    }

    return false;
}

static void test_stroke_width_clipping(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(100, 10);
    bm.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(bm);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    paint.setColor(0xff00ff00);

    // clip out the left half of our canvas
    canvas.clipRect(SkRect::MakeXYWH(51, 0, 49, 100));

    // no stroke bleed should be visible
    canvas.drawRect(SkRect::MakeWH(44, 100), paint);
    REPORTER_ASSERT(reporter, !has_green_pixels(bm));

    // right stroke edge should bleed into the visible area
    canvas.scale(2, 2);
    canvas.drawRect(SkRect::MakeWH(22, 50), paint);
    REPORTER_ASSERT(reporter, has_green_pixels(bm));
}

static void test_skbug4406(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(10, 10);
    bm.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(bm);
    const SkRect r = { 1.5f, 1, 3.5f, 3 };
    // draw filled green rect first
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(0xff00ff00);
    paint.setStrokeWidth(1);
    paint.setAntiAlias(true);
    canvas.drawRect(r, paint);

    // paint black with stroke rect (that asserts in bug 4406)
    // over the filled rect, it should cover it
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(0xff000000);
    paint.setStrokeWidth(1);
    canvas.drawRect(r, paint);
    REPORTER_ASSERT(reporter, !has_green_pixels(bm));

    // do it again with thinner stroke
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(0xff00ff00);
    paint.setStrokeWidth(1);
    paint.setAntiAlias(true);
    canvas.drawRect(r, paint);
    // paint black with stroke rect (that asserts in bug 4406)
    // over the filled rect, it doesnt cover it completelly with thinner stroke
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(0xff000000);
    paint.setStrokeWidth(0.99f);
    canvas.drawRect(r, paint);
    REPORTER_ASSERT(reporter, has_green_pixels(bm));
}

DEF_TEST(Rect, reporter) {
    test_stroke_width_clipping(reporter);
    test_skbug4406(reporter);
}

DEF_TEST(Rect_grow, reporter) {
    test_stroke_width_clipping(reporter);
    test_skbug4406(reporter);
}

DEF_TEST(Rect_path_nan, reporter) {
    SkRect r = { 0, 0, SK_ScalarNaN, 100 };
    SkPath p;
    p.addRect(r);
    // path normally just jams its bounds to be r, but it must notice that r is non-finite
    REPORTER_ASSERT(reporter, !p.isFinite());
}

DEF_TEST(Rect_largest, reporter) {
    REPORTER_ASSERT(reporter, !SkRectPriv::MakeILarge().isEmpty());
    REPORTER_ASSERT(reporter,  SkRectPriv::MakeILargestInverted().isEmpty());

    REPORTER_ASSERT(reporter, !SkRectPriv::MakeLargest().isEmpty());
    REPORTER_ASSERT(reporter, !SkRectPriv::MakeLargeS32().isEmpty());
    REPORTER_ASSERT(reporter,  SkRectPriv::MakeLargestInverted().isEmpty());
}

/*
 *  Test the setBounds always handles non-finite values correctly:
 *  - setBoundsCheck should return false, and set the rect to all zeros
 *  - setBoundsNoCheck should ensure that rect.isFinite() is false (definitely NOT all zeros)
 */
DEF_TEST(Rect_setbounds, reporter) {
    const SkPoint p0[] = { { SK_ScalarInfinity, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 } };
    const SkPoint p1[] = { { 0, SK_ScalarInfinity }, { 1, 1 }, { 2, 2 }, { 3, 3 } };
    const SkPoint p2[] = { { SK_ScalarNaN, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 } };
    const SkPoint p3[] = { { 0, SK_ScalarNaN }, { 1, 1 }, { 2, 2 }, { 3, 3 } };

    SkRect r;
    const SkRect zeror = { 0, 0, 0, 0 };
    for (const SkPoint* pts : { p0, p1, p2, p3 }) {
        for (int n = 1; n <= 4; ++n) {
            bool isfinite = r.setBoundsCheck(pts, n);
            REPORTER_ASSERT(reporter, !isfinite);
            REPORTER_ASSERT(reporter, r == zeror);

            r.setBoundsNoCheck(pts, n);
            if (r.isFinite())
                r.setBoundsNoCheck(pts, n);
            REPORTER_ASSERT(reporter, !r.isFinite());
        }
    }
}

static float make_big_value(skiatest::Reporter* reporter) {
    // need to make a big value, one that will cause rect.width() to overflow to inf.
    // however, the windows compiler wants about this if it can see the big value inlined.
    // hence, this stupid trick to try to fool their compiler.
    SkASSERT(reporter);
    return reporter ? SK_ScalarMax * 0.75f : 0;
}

DEF_TEST(Rect_center, reporter) {
    // ensure we can compute center even when the width/height might overflow
    const SkScalar big = make_big_value(reporter);
    const SkRect r = { -big, -big, big, big };

    REPORTER_ASSERT(reporter, r.isFinite());
    REPORTER_ASSERT(reporter, SkScalarIsFinite(r.centerX()));
    REPORTER_ASSERT(reporter, SkScalarIsFinite(r.centerY()));
    REPORTER_ASSERT(reporter, !SkScalarIsFinite(r.width()));
    REPORTER_ASSERT(reporter, !SkScalarIsFinite(r.height()));
}

#include "include/core/SkSurface.h"

// Before the fix, this sequence would trigger a release_assert in the Tiler
// in SkBitmapDevice.cpp
DEF_TEST(big_tiled_rect_crbug_927075, reporter) {
    // since part of the regression test allocates a huge buffer, don't bother trying on
    // 32-bit devices (e.g. chromecast) so we avoid them failing to allocated.

    if (sizeof(void*) == 8) {
        const int w = 67108863;
        const int h = 1;
        const auto info = SkImageInfo::MakeN32Premul(w, h);

        auto surf = SkSurface::MakeRaster(info);
        auto canvas = surf->getCanvas();

        const SkRect r = { 257, 213, 67109120, 214 };
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->translate(-r.fLeft, -r.fTop);
        canvas->drawRect(r, paint);
    }
}
