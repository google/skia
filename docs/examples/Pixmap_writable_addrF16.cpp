// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=7822d78f5cacf5c04267cbbc6c6d0b80
REG_FIDDLE(Pixmap_writable_addrF16, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::Make(1, 2, kRGBA_F16_SkColorType, kPremul_SkAlphaType);
    uint16_t storage[2][4];
    SkPixmap pixmap(info, storage[0], sizeof(uint64_t));
    SkIRect topPixelBounds = {0, 0, 1, 1};
    pixmap.erase({ 0.65f, 0.45f, 0.25f, 1 }, &topPixelBounds);
    SkIRect bottomPixelBounds = {0, 1, 1, 2};
    pixmap.erase({ 0.25f, 0.65f, 0.45f, 1 }, &bottomPixelBounds);
    SkBitmap bitmap;
    canvas->scale(20, 20);
    bitmap.installPixels(pixmap);
    canvas->drawBitmap(bitmap, 0, 0);
    uint16_t* pixel2 = pixmap.writable_addrF16(0, 1);
    for (int i = 0; i < 4; ++i) {
        pixel2[i] = storage[0][i];
    }
    bitmap.installPixels(pixmap);
    canvas->drawBitmap(bitmap, 4, 0);
}
}  // END FIDDLE
