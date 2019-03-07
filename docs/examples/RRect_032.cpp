// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=da61054322550a2d5ac15114da23bd23
REG_FIDDLE(RRect_032, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkRRect rrect = SkRRect::MakeRectXY({10, 20, 180, 220}, 50, 100);
    for (int index = 0; index < 25; ++index) {
       canvas->drawRRect(rrect, paint);
       rrect.inset(3, 3);
    }
}
}  // END FIDDLE
