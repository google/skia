#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=60a08f3ce75374fc815384616d114df7
REG_FIDDLE(Point_Normalize, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint lines[][2] = { {{  30, 110 }, { 190,  30 }},
                                 {{  30, 220 }, { 120, 140 }}};
    for (auto line : lines) {
        canvas->drawLine(line[0], line[1], paint);
        SkVector vector = line[1] - line[0];
        SkScalar priorLength = SkPoint::Normalize(&vector);
        SkVector rotate90 = { -vector.fY, vector.fX };
        rotate90 *= 10.f;
        canvas->drawLine(line[0] - rotate90, line[0] + rotate90, paint);
        canvas->drawLine(line[1] - rotate90, line[1] + rotate90, paint);
        SkString length("length = ");
        length.appendScalar(priorLength);
        canvas->drawString(length, line[0].fX + 25, line[0].fY - 4, paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
