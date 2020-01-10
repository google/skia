// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(weird_RRect_bug, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    // canvas->scale(2, 2);

    SkPaint p;
    p.setAntiAlias(false);
    p.setStrokeWidth(3);
    p.setStyle(SkPaint::kStroke_Style);

    SkRect r = SkRect::MakeXYWH(20, 20, 20, 20);

    SkRRect rr = SkRRect::MakeRectXY(r, 5, 5);

    canvas->drawRRect(rr, p);
}
}  // END FIDDLE
