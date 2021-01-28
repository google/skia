// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=9a392b753167cfa849cebeefd5a6e07d
REG_FIDDLE(Pixmap_reset_2, 256, 64, false, 4) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> pixels;
    pixels.resize(image->height() * image->width() * 4);
    SkPixmap pixmap(SkImageInfo::Make(image->width(), image->height(), kN32_SkColorType,
            image->alphaType()), (const void*) &pixels.front(), image->width() * 4);
    image->readPixels(nullptr, pixmap, 0, 0);
    int x = 0;
    for (auto colorType : { kRGBA_8888_SkColorType, kBGRA_8888_SkColorType } ) {
        pixmap.reset(SkImageInfo::Make(image->width(), image->height(), colorType,
                image->alphaType()), (const void*) &pixels.front(), image->width() * 4);
        SkBitmap bitmap;
        bitmap.installPixels(pixmap);
        canvas->drawImage(bitmap.asImage(), x, 0);
        x += 128;
    }
}
}  // END FIDDLE
