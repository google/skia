// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=62bc26989c2b4c2a54d516596a71dd97
REG_FIDDLE(Matrix_mapRectScaleTranslate, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkMatrix matrix;
    SkRect rect = {100, 50, 150, 180};
    matrix.setScale(2, .5f, rect.centerX(), rect.centerY());
    SkRect rotated;
    matrix.mapRectScaleTranslate(&rotated, rect);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(rect, paint);
    paint.setColor(SK_ColorRED);
    canvas->drawRect(rotated, paint);
}
}  // END FIDDLE
