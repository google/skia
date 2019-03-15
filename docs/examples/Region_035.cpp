#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=42bde0ef8c2ee372751428cd6e21c1ca
REG_FIDDLE(Region_035, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setTextSize(128);
    SkPath textPath;
    paint.getTextPath("W", 1, 20, 110, &textPath);
    SkRegion region;
    region.setPath(textPath, SkRegion({0, 0, 256, 256}));
    canvas->drawRegion(region, SkPaint());
    SkIRect iRect = SkIRect::MakeXYWH(frame * 160, 55, 10, 10);
    paint.setColor(region.intersects(iRect) ? SK_ColorBLUE : SK_ColorRED);
    canvas->drawRect(SkRect::Make(iRect), paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
