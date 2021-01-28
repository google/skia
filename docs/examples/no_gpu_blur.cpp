// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(no_gpu_blur, 256, 256, false, 0) {
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
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, 3));
    SkIPoint offset;
    bitmap.extractAlpha(&alpha, &paint, nullptr, &offset);
    paint.setColor(SK_ColorRED);
    canvas->drawImage(bitmap.asImage(), 0, -offset.fY, SkSamplingOptions(), &paint);
    canvas->drawImage(alpha.asImage(), 100 + offset.fX, 0, SkSamplingOptions(), &paint);
}
}  // END FIDDLE
