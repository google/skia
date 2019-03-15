// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=086866243bf9e4c14c3b215a2aa69ad9
REG_FIDDLE(Pixmap_036, 256, 72, false, 4) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> pixels;
    pixels.resize(image->height() * image->width() * 4);
    SkPixmap pixmap(SkImageInfo::Make(image->width(), image->height(), kN32_SkColorType,
            image->alphaType()), (const void*) &pixels.front(), image->width() * 4);
    image->readPixels(pixmap, 0, 0);
    for (int y = 0; y < pixmap.height() / 2; ++y) {
        for (int x = 0; x < pixmap.width(); ++x) {
            if ((x & 4) == (y & 4)) {
                *pixmap.writable_addr32(x, y) =
                        *pixmap.writable_addr32(pixmap.width() - x - 1, pixmap.height() - y - 1);
            }
        }
    }
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
