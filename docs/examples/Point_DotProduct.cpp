#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=496db0131a003162faba7d7f98b30340
REG_FIDDLE(Point_DotProduct, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkVector vectors[][2] = {{{50, 2}, {-14, 20}}, {{0, 50}, {-50, 0}}, {{-20, 25}, {25, -20}},
                             {{-20, -24}, {-24, -20}}};
    SkPoint center[] = {{32, 32}, {160, 32}, {32, 160}, {160, 160}};
    paint.setStrokeWidth(2);
    for (size_t i = 0; i < 4; ++i) {
        canvas->drawLine(center[i], center[i] + vectors[i][0], paint);
        canvas->drawLine(center[i], center[i] + vectors[i][1], paint);
        SkString str;
        str.printf("dot = %g", SkPoint::DotProduct(vectors[i][0], vectors[i][1]));
        canvas->drawString(str, center[i].fX, center[i].fY, paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
