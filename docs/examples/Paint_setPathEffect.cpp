// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Paint_setPathEffect, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setPathEffect(SkDiscretePathEffect::Make(3, 5));
    canvas->drawRect(SkRect::MakeXYWH(40, 40, 175, 175), paint);
}
}  // END FIDDLE
