// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=838202e0d49cad2eb3eeb495834f6d63
REG_FIDDLE(Pixmap_erase_2, 256, 50, false, 0) {
void draw(SkCanvas* canvas) {
    uint32_t storage[2];
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 2);
    SkPixmap pixmap(info, storage, info.minRowBytes());
    pixmap.erase(SK_ColorBLUE);
    SkBitmap bitmap;
    canvas->scale(20, 20);
    bitmap.installPixels(pixmap);
    canvas->drawImage(bitmap.asImage(), 0, 0);
}
}  // END FIDDLE
