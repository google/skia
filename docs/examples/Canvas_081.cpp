// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=841229e25ca9dfb68bd0dc4dfff356eb
REG_FIDDLE(Canvas_081, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawCircle(128, 128, 90, paint);
    paint.setColor(SK_ColorWHITE);
    canvas->drawCircle(86, 86, 20, paint);
    canvas->drawCircle(160, 76, 20, paint);
    canvas->drawCircle(140, 150, 35, paint);
}
}  // END FIDDLE
