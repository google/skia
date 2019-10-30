// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c4620df2eaba447b581688d3100053b1
REG_FIDDLE(RRect_setNinePatch, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRRect rrect;
    rrect.setNinePatch({30, 10, 100, 60}, 10, 20, 20, 10);
    canvas->drawRRect(rrect, paint);
    paint.setColor(SK_ColorWHITE);
    const SkRect r = rrect.getBounds();
    canvas->drawLine(r.fLeft, r.fTop + rrect.radii(SkRRect::kUpperLeft_Corner).fY,
                     r.fRight, r.fTop + rrect.radii(SkRRect::kUpperRight_Corner).fY, paint);
    canvas->drawLine(r.fLeft, r.fBottom - rrect.radii(SkRRect::kLowerLeft_Corner).fY,
                     r.fRight, r.fBottom - rrect.radii(SkRRect::kLowerRight_Corner).fY, paint);
    canvas->drawLine(r.fLeft + rrect.radii(SkRRect::kUpperLeft_Corner).fX, r.fTop,
                     r.fLeft + rrect.radii(SkRRect::kLowerLeft_Corner).fX, r.fBottom, paint);
    canvas->drawLine(r.fRight - rrect.radii(SkRRect::kUpperRight_Corner).fX, r.fTop,
                     r.fRight - rrect.radii(SkRRect::kLowerRight_Corner).fX, r.fBottom, paint);
}
}  // END FIDDLE
