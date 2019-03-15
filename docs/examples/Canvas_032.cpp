// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8ab88d86fb438856cc48d6e2f08a6e24
REG_FIDDLE(Canvas_032, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawCircle(50, 50, 50, paint);
    canvas->saveLayerAlpha(nullptr, 128);
    paint.setColor(SK_ColorBLUE);
    canvas->drawCircle(100, 50, 50, paint);
    paint.setColor(SK_ColorGREEN);
    paint.setAlpha(128);
    canvas->drawCircle(75, 90, 50, paint);
    canvas->restore();
}
}  // END FIDDLE
