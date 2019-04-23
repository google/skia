/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

// Draws big rects with clip (0, 0, 35, 35). The size of the rects is given by big.
static void draw_big_rect(SkCanvas* canvas, SkScalar big, const SkPaint& rectPaint) {
    // Looks like this:
    // +--+-+----+-+----+
    // |  | |    | |    |
    // |--+-+----+-+----+
    // |--+-+----+-+----+
    // |  | |    | |    |
    // |  | |    +-+    |
    // +--+-+--+     +--+
    // +--+-+--+     +--+
    // |  | |    +-+    |
    // |  | |    | |    |
    // +--+-+----+-+----+

    canvas->clipRect({0, 0, 35, 35});

    // Align to pixel boundaries.
    canvas->translate(0.5, 0.5);

    SkRect horiz = SkRect::MakeLTRB(-big, 5, big, 10);
    canvas->drawRect(horiz, rectPaint);

    SkRect vert = SkRect::MakeLTRB(5, -big, 10, big);
    canvas->drawRect(vert, rectPaint);

    SkRect fromLeft = SkRect::MakeLTRB(-big, 20, 17, 25);
    canvas->drawRect(fromLeft, rectPaint);

    SkRect fromTop = SkRect::MakeLTRB(20, -big, 25, 17);
    canvas->drawRect(fromTop, rectPaint);

    SkRect fromRight = SkRect::MakeLTRB(28, 20, big, 25);
    canvas->drawRect(fromRight, rectPaint);

    SkRect fromBottom = SkRect::MakeLTRB(20, 28, 25, big);
    canvas->drawRect(fromBottom, rectPaint);

    SkRect leftBorder = SkRect::MakeLTRB(-2, -1, 0, 35);
    canvas->drawRect(leftBorder, rectPaint);

    SkRect topBorder = SkRect::MakeLTRB(-1, -2, 35, 0);
    canvas->drawRect(topBorder, rectPaint);

    SkRect rightBorder = SkRect::MakeLTRB(34, -1, 36, 35);
    canvas->drawRect(rightBorder, rectPaint);

    SkRect bottomBorder = SkRect::MakeLTRB(-1, 34, 35, 36);
    canvas->drawRect(bottomBorder, rectPaint);

    SkPaint outOfBoundsPaint;
    outOfBoundsPaint.setColor(SK_ColorRED);
    outOfBoundsPaint.setStyle(SkPaint::kStroke_Style);
    outOfBoundsPaint.setStrokeWidth(0);

    SkRect outOfBounds = SkRect::MakeLTRB(-1, -1, 35, 35);
    canvas->drawRect(outOfBounds, outOfBoundsPaint);
}

DEF_SIMPLE_GM(bigrect, canvas, 325, 125) {
    // Test with sizes:
    //   - reasonable size (for comparison),
    //   - outside the range of int32, and
    //   - outside the range of SkFixed.
    static const SkScalar sizes[] = {SkIntToScalar(100), 5e10f, 1e6f};

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) {
            canvas->save();
            canvas->translate(SkIntToScalar(i*40+5), SkIntToScalar(j*40+5));

            SkPaint paint;
            paint.setColor(SK_ColorBLUE);
            // These are the three parameters that affect the behavior of SkDraw::drawRect.
            if (i & 1) {
                paint.setStyle(SkPaint::kFill_Style);
            } else {
                paint.setStyle(SkPaint::kStroke_Style);
            }
            if (i & 2) {
                paint.setStrokeWidth(1);
            } else {
                paint.setStrokeWidth(0);
            }
            if (i & 4) {
                paint.setAntiAlias(true);
            } else {
                paint.setAntiAlias(false);
            }

            const SkScalar big = SkFloatToScalar(sizes[j]);
            draw_big_rect(canvas, big, paint);
            canvas->restore();
        }
    }
}
