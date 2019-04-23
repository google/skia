// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4d69b6d9c7726c47c42827d79fc7899c
REG_FIDDLE(RRect_outset, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkRRect rrect = SkRRect::MakeRectXY({100, 20, 140, 220}, 50, 100);
    for (int index = 0; index < 25; ++index) {
       canvas->drawRRect(rrect, paint);
       rrect.outset(-3, 3, &rrect);
    }
}
}  // END FIDDLE
