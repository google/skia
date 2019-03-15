#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9e0a2de2eb94dba4521d733e73f2bda5
REG_FIDDLE(Point_033, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint lines[][2] = {{{-10, -10}, {90, 30}}, {{0, 0}, {150, 30}}, {{10, 25}, {120, 150}}};
    const SkPoint origin = {30, 160};
    for (auto line : lines) {
        SkPoint a = origin + line[0];
        const SkPoint& b = line[1];
        canvas->drawLine(a, b, paint);
        SkAutoCanvasRestore acr(canvas, true);
        SkScalar angle = SkScalarATan2((b.fY - a.fY), b.fX - a.fX);
        canvas->rotate(angle * 180 / SK_ScalarPI, a.fX, a.fY);
        SkString distance("distance = ");
        distance.appendScalar(SkPoint::Distance(a, b));
        canvas->drawString(distance, a.fX + 25, a.fY - 4, paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
