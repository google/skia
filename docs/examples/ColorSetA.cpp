// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=18f6f376f771f5ffa56d5e5b2ebd20fb
REG_FIDDLE(ColorSetA, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawBitmap(source, 0, 0);
    for (int y = 0; y < 256; y += 16) {
       for (int x = 0; x < 256; x += 16) {
         SkColor color = source.getColor(x + 8, y + 8);
         SkPaint paint;
         paint.setColor(SkColorSetA(color, x + y));
         canvas->drawRect(SkRect::MakeXYWH(x, y, 16, 16), paint);
      }
    }
}
}  // END FIDDLE
