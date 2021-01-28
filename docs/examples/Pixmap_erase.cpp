// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=a0cdbafed4786788cc90681e7b294234
REG_FIDDLE(Pixmap_erase, 256, 50, false, 0) {
void draw(SkCanvas* canvas) {
    uint32_t storage[2];
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 2);
    SkPixmap pixmap(info, storage, info.minRowBytes());
    pixmap.erase(SK_ColorBLUE, {0, 0, 1, 1});
    pixmap.erase(SK_ColorRED, {0, 1, 1, 2});
    SkBitmap bitmap;
    canvas->scale(20, 20);
    bitmap.installPixels(pixmap);
    canvas->drawImage(bitmap.asImage(), 0, 0);
}
}  // END FIDDLE
