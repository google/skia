// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ab6577df079e6c70511cf2bfc6447b44
REG_FIDDLE(Bitmap_extractAlpha, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap alpha, bitmap;
    bitmap.allocN32Pixels(100, 100);
    SkCanvas offscreen(bitmap);
    offscreen.clear(0);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLUE);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    offscreen.drawCircle(50, 50, 39, paint);
    offscreen.flush();
    bitmap.extractAlpha(&alpha);
    paint.setColor(SK_ColorRED);
    canvas->drawBitmap(bitmap, 0, 0, &paint);
    canvas->drawBitmap(alpha, 100, 0, &paint);
}
}  // END FIDDLE
