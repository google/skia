// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=182ef48ab5e04ba3578496fda8d9fa36
REG_FIDDLE(Canvas_clipRRect, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0x8055aaff);
    SkRRect oval;
    oval.setOval({10, 20, 90, 100});
    canvas->clipRRect(oval, SkClipOp::kIntersect, true);
    canvas->drawCircle(70, 100, 60, paint);
}
}  // END FIDDLE
