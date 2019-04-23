// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a6575a49467ce8d28bb01cc7638fa04d
REG_FIDDLE(Anti_Alias, 512, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(50, 50);
    SkCanvas offscreen(bitmap);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    for (bool antialias : { false, true }) {
        paint.setColor(antialias ? SK_ColorRED : SK_ColorBLUE);
        paint.setAntiAlias(antialias);
        bitmap.eraseColor(0);
        offscreen.drawLine(5, 5, 15, 30, paint);
        canvas->drawLine(5, 5, 15, 30, paint);
        canvas->save();
        canvas->scale(10, 10);
        canvas->drawBitmap(bitmap, antialias ? 12 : 0, 0);
        canvas->restore();
        canvas->translate(15, 0);
    }
}
}  // END FIDDLE
