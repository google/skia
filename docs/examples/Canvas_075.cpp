// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d3d8ca584134560750b1efa4a4c6e138
REG_FIDDLE(Canvas_075, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkIRect rect = { 64, 48, 192, 160 };
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorYELLOW, SK_ColorMAGENTA } ) {
        paint.setColor(color);
        canvas->drawIRect(rect, paint);
        canvas->rotate(30, 128, 128);
    }
}
}  // END FIDDLE
