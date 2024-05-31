// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_scale, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkRect rect = { 10, 20, 60, 120 };
    canvas->translate(20, 20);
    canvas->drawRect(rect, paint);
    canvas->scale(2, .5f);
    paint.setColor(SK_ColorGRAY);
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
