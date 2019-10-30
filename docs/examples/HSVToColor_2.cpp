// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d355a17547908cdbc2c38720974b5d11
REG_FIDDLE(HSVToColor_2, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawBitmap(source, 0, 0);
    for (int y = 0; y < 256; ++y) {
       for (int x = 0; x < 256; ++x) {
         SkColor color = source.getColor(x, y);
         SkScalar hsv[3];
         SkColorToHSV(color, hsv);
         hsv[0] = hsv[0] + 90 >= 360 ? hsv[0] - 270 : hsv[0] + 90;
         SkPaint paint;
         paint.setColor(SkHSVToColor(hsv));
         canvas->drawRect(SkRect::MakeXYWH(x, y, 1, 1), paint);
      }
    }
}
}  // END FIDDLE
