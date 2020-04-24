#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c98773d8b4509969d78cb8121e4b77f6
REG_FIDDLE(Point_Length, 256, 192, false, 0) {
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
        length.appendScalar(SkPoint::Length(point.fX, point.fY));
        canvas->drawString(length, origin.fX + 25, origin.fY - 4, paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
