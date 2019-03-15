// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=311a59931ac340b90f202cd6ac399a0a
REG_FIDDLE(Color_015, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawBitmap(source, 0, 0);
    for (int y = 0; y < 256; ++y) {
       for (int x = 0; x < 256; ++x) {
         SkColor color = source.getColor(x, y);
         SkScalar hsv[3];
         SkColorToHSV(color, hsv);
         hsv[0] = hsv[0] + 90 >= 360 ? hsv[0] - 270 : hsv[0] + 90;
         SkPaint paint;
         paint.setColor(SkHSVToColor((x + y) % 256, hsv));
         canvas->drawRect(SkRect::MakeXYWH(x, y, 1, 1), paint);
      }
    }
}
}  // END FIDDLE
