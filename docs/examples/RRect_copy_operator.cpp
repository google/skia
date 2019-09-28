// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=52926c98c1cca00606d3ea99f23fea3d
REG_FIDDLE(RRect_copy_operator, 256, 110, false, 0) {
void draw(SkCanvas* canvas) {
    SkRRect rrect = SkRRect::MakeRect({40, 40, 100, 70});
    SkRRect rrect2 = rrect;
    rrect2.inset(-20, -20);
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    canvas->drawRRect(rrect, p);
    canvas->drawRRect(rrect2, p);
}
}  // END FIDDLE
