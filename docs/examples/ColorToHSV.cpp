// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=1e0370f12c8aab5b84f9e824074f1e5a
REG_FIDDLE(ColorToHSV, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawBitmap(source, 0, 0);
    for (int y = 0; y < 256; ++y) {
       for (int x = 0; x < 256; ++x) {
         SkScalar hsv[3];
         SkColorToHSV(source.getColor(x, y), hsv);
         hsv[1] = 1 - hsv[1];
         SkPaint paint;
         paint.setColor(SkHSVToColor(hsv));
         canvas->drawRect(SkRect::MakeXYWH(x, y, 1, 1), paint);
      }
    }
}
}  // END FIDDLE
