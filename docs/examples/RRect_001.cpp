// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=ad8f5d49edfcee60eddfe2a955b6c5f5
REG_FIDDLE(RRect_copy_const_SkRRect, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect = SkRRect::MakeRect({10, 10, 100, 50});
    SkRRect rrect2(rrect);
    rrect2.inset(20, 20);
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    canvas->drawRRect(rrect, p);
    canvas->drawRRect(rrect2, p);
}
}  // END FIDDLE
