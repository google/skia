// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=de14d8d30e4a7b6462103d0e0dd96b0b
REG_FIDDLE(Pixmap_037, 256, 40, false, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::Make(3, 3, kRGBA_F16_SkColorType, kPremul_SkAlphaType);
    uint64_t storage[9];
    SkPixmap pixmap(info, storage, 3 * sizeof(uint64_t));
    SkColor4f c4 { 1, 0.45f, 0.25f, 0.65f };
    pixmap.erase(c4);
    SkBitmap bitmap;
    canvas->scale(10, 10);
    bitmap.installPixels(pixmap);
    canvas->drawBitmap(bitmap, 0, 0);
    *pixmap.writable_addr64(1, 1) |= 0x00ff000000000000LL;
    bitmap.installPixels(pixmap);
    canvas->drawBitmap(bitmap, 10, 0);
}
}  // END FIDDLE
