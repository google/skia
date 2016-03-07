/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

DEF_SIMPLE_GM(bigrect, canvas, 35, 35) {
    static const SkScalar kBig = SkFloatToScalar(5e10f);

    SkPaint outlinePaint;
    outlinePaint.setColor(SK_ColorBLUE);
    outlinePaint.setStyle(SkPaint::kStroke_Style);
    outlinePaint.setStrokeWidth(0);

    SkPaint outOfBoundsPaint;
    outOfBoundsPaint.setColor(SK_ColorRED);
    outOfBoundsPaint.setStyle(SkPaint::kStroke_Style);
    outOfBoundsPaint.setStrokeWidth(0);

    // Looks like this:
    // +-+-+--+-+------+
    // | | |  | |  +---+
    // | +-+  | |  +---+
    // |      | |      |
    // +------+-+------+
    // +------+-+------+
    // |      | |      |
    // +---+  | |  +-+ |
    // +---+  | |  | | |
    // +------+-+--+-+-+

    SkRect tl = SkRect::MakeLTRB(SkIntToScalar(5),
                                 -kBig,
                                 SkIntToScalar(10),
                                 SkIntToScalar(10));
    canvas->drawRect(tl, outlinePaint);

    SkRect tr = SkRect::MakeLTRB(SkIntToScalar(25),
                                 SkIntToScalar(5),
                                 kBig,
                                 SkIntToScalar(10));
    canvas->drawRect(tr, outlinePaint);

    SkRect br = SkRect::MakeLTRB(SkIntToScalar(25),
                                 SkIntToScalar(25),
                                 SkIntToScalar(30),
                                 kBig);
    canvas->drawRect(br, outlinePaint);

    SkRect bl = SkRect::MakeLTRB(-kBig,
                                 SkIntToScalar(25),
                                 SkIntToScalar(10),
                                 SkIntToScalar(30));
    canvas->drawRect(bl, outlinePaint);

    SkRect horiz = SkRect::MakeLTRB(-kBig,
                                    SkIntToScalar(15),
                                    kBig,
                                    SkIntToScalar(20));
    canvas->drawRect(horiz, outlinePaint);

    SkRect vert = SkRect::MakeLTRB(SkIntToScalar(15),
                                   -kBig,
                                   SkIntToScalar(20),
                                   kBig);
    canvas->drawRect(vert, outlinePaint);

    SkRect leftBorder = SkRect::MakeLTRB(-2, -1, 0, 35);
    canvas->drawRect(leftBorder, outlinePaint);

    SkRect topBorder = SkRect::MakeLTRB(-1, -2, 35, 0);
    canvas->drawRect(topBorder, outlinePaint);

    SkRect rightBorder = SkRect::MakeLTRB(34, -1, 36, 35);
    canvas->drawRect(rightBorder, outlinePaint);

    SkRect bottomBorder = SkRect::MakeLTRB(-1, 34, 35, 36);
    canvas->drawRect(bottomBorder, outlinePaint);

    SkRect outOfBounds = SkRect::MakeLTRB(-1, -1, 35, 35);
    canvas->drawRect(outOfBounds, outOfBoundsPaint);
}
