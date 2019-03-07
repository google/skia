// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d84fce292d86c7d9ef37ae2d179c03c7
REG_FIDDLE(Point_013, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint lines[][2] = { {{  30, 110 }, { 190,  30 }},
                                 {{ 120, 140 }, {  30, 220 }}};
    for (auto line : lines) {
        canvas->drawLine(line[0], line[1], paint);
        SkVector vector = line[1] - line[0];
        if (vector.normalize()) {
            SkVector rotate90 = { -vector.fY, vector.fX };
            rotate90 *= 10.f;
            canvas->drawLine(line[0] - rotate90, line[0] + rotate90, paint);
            canvas->drawLine(line[1] - rotate90, line[1] + rotate90, paint);
        }
    }
}
}  // END FIDDLE
