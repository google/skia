// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_saveLayerAlpha, 256, 256, false, 0) {
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
