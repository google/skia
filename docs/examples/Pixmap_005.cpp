// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=febdbfac6cf4cde69837643be2e1f6dd
REG_FIDDLE(Pixmap_005, 256, 128, false, 3) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> pixels;
    pixels.resize(image->height() * image->width() * 4);
    SkPixmap pixmap(SkImageInfo::Make(image->width(), image->height(), kN32_SkColorType,
            image->alphaType()), (const void*) &pixels.front(), image->width() * 4);
    image->readPixels(pixmap, 0, 0);
    SkPixmap inset;
    if (pixmap.extractSubset(&inset, {128, 128, 512, 512})) {
        SkBitmap bitmap;
        bitmap.installPixels(inset);
        canvas->drawBitmap(bitmap, 0, 0);
    }
}
}  // END FIDDLE
