#if 0  // disabled
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(ImageInfo_020, 256, 128, false, 0) {
// HASH=525650a67e19fdd8ca9f72b7eda65174
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(18, 18));
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(15);
    offscreen.drawString("\xF0\x9F\x98\xB8", 1, 15, paint);
    canvas->scale(6, 6);
    canvas->drawBitmap(bitmap, 0, 0);
}
}
#endif  // disabled
