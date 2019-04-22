#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=812cf26d91b1cdcd2c6b9438a8172518
REG_FIDDLE(Point_distanceToOrigin, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint points[] = { { 60, -110 }, { 90, 10 }, { 120, -110 }, { 180, -50 } };
    const SkPoint origin = {0, 0};
    canvas->translate(30, 140);
    for (auto point : points) {
        canvas->drawLine(origin, point, paint);
        SkAutoCanvasRestore acr(canvas, true);
        SkScalar angle = SkScalarATan2((point.fY - origin.fY), point.fX - origin.fX);
        canvas->rotate(angle * 180 / SK_ScalarPI, origin.fX, origin.fY);
        SkString distance("distance = ");
        distance.appendScalar(point.distanceToOrigin());
        canvas->drawString(distance, origin.fX + 25, origin.fY - 4, paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
