// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b7bc91ff16c9b9351b2a127f35394b82
REG_FIDDLE(Canvas_destructor, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(200, 200));
    {
        SkCanvas offscreen(bitmap);
        SkPaint paint;
        SkFont font(nullptr, 100);
        offscreen.drawString("ABC", 20, 160, font, paint);
        SkRect layerBounds = SkRect::MakeXYWH(32, 32, 192, 192);
        offscreen.saveLayerAlpha(&layerBounds, 128);
        offscreen.clear(SK_ColorWHITE);
        offscreen.drawString("DEF", 20, 160, font, paint);
    }
    canvas->drawBitmap(bitmap, 0, 0, nullptr);
}
}  // END FIDDLE
