#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=78cea0c4cac205b61ad6f6c982cbd888
REG_FIDDLE(ImageInfo_MakeN32, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32(16, 16, kPremul_SkAlphaType));
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorWHITE);
    SkPaint paint;
    offscreen.drawString("g", 0, 10, paint);
    canvas->scale(8, 8);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
