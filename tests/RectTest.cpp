/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkRect.h"
#include "Test.h"

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
