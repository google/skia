#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8363ab179447ee4b827679e20d3d81eb
REG_FIDDLE(Point_length, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint points[] = { { 90, 30 }, { 120, 150 }, { 150, 30 }, { 210, 90 } };
    const SkPoint origin = {30, 140};
    for (auto point : points) {
        canvas->drawLine(origin, point, paint);
        SkAutoCanvasRestore acr(canvas, true);
        SkScalar angle = SkScalarATan2((point.fY - origin.fY), point.fX - origin.fX);
        canvas->rotate(angle * 180 / SK_ScalarPI, origin.fX, origin.fY);
        SkString length("length = ");
        length.appendScalar(point.length());
        canvas->drawString(length, origin.fX + 25, origin.fY - 4, paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
