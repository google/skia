// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f02f0110d5605dac6d14dcb8d1d8cb6e
REG_FIDDLE(RRect_inset, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkRRect rrect = SkRRect::MakeRectXY({100, 20, 140, 220}, 50, 100);
    for (int index = 0; index < 25; ++index) {
       canvas->drawRRect(rrect, paint);
       rrect.inset(-3, 3, &rrect);
    }
}
}  // END FIDDLE
