#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e3899c2715c332bfc7648d5f2b9eefc6
REG_FIDDLE(Region_contains, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath xPath;
    paint.getTextPath("X", 1, 20, 110, &xPath);
    SkRegion xRegion;
    xRegion.setPath(xPath, SkRegion({0, 0, 256, 256}));
    canvas->drawRegion(xRegion, paint);
    for (int y = 0; y < 128; y += 8) {
        for (int x = 0; x < 128; x += 8) {
           paint.setColor(xRegion.contains(x, y) ? SK_ColorWHITE : SK_ColorRED);
           canvas->drawPoint(x, y, paint);
        }
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
