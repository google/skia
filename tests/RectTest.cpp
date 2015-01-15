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

DEF_TEST(Rect, reporter) {
    test_stroke_width_clipping(reporter);
}
