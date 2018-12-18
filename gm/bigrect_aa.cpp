/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

DEF_SIMPLE_GM(bigrect_aa, canvas, 500, 500) {
    // Draw a very wide rectangle, a very tall rectangle, and a rectangle at 45 degrees. The
    // dimensions of the rectangles are such that it strains 16-bit floating point representation
    // used in the GPU GrFillRectOp's edge distance vertex attributes.

    static const SkRect kBigRect = { 0.25f, 0.5f, 33000.f, 100.f };

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setAntiAlias(true);

    canvas->save();
    canvas->translate(10.1f, 10.4f);
    canvas->drawRect(kBigRect, paint);
    canvas->restore();

    canvas->save();
    canvas->translate(50.1f, 120.6f);
    canvas->rotate(90.f);
    canvas->drawRect(kBigRect, paint);
    canvas->restore();

    canvas->save();
    canvas->translate(160.2f, 120.2f);
    canvas->rotate(45.f);
    canvas->drawRect(kBigRect, paint);
    canvas->restore();
}
