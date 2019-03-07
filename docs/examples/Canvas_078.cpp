// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=90fed1bb11efb43aada94113338c63d8
REG_FIDDLE(Canvas_078, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRect outer = {30, 40, 210, 220};
    SkRect radii = {30, 50, 70, 90 };
    SkRRect rRect;
    rRect.setNinePatch(outer, radii.fLeft, radii.fTop, radii.fRight, radii.fBottom);
    canvas->drawRRect(rRect, paint);
    paint.setColor(SK_ColorWHITE);
    canvas->drawLine(outer.fLeft + radii.fLeft, outer.fTop,
                     outer.fLeft + radii.fLeft, outer.fBottom, paint);
    canvas->drawLine(outer.fRight - radii.fRight, outer.fTop,
                     outer.fRight - radii.fRight, outer.fBottom, paint);
    canvas->drawLine(outer.fLeft,  outer.fTop + radii.fTop,
                     outer.fRight, outer.fTop + radii.fTop, paint);
    canvas->drawLine(outer.fLeft,  outer.fBottom - radii.fBottom,
                     outer.fRight, outer.fBottom - radii.fBottom, paint);
}
}  // END FIDDLE
