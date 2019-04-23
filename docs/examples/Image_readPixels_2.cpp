// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b77a73c4baa63a4a8e2a4fdd96144d0b
REG_FIDDLE(Image_readPixels_2, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> srcPixels;
    int rowBytes = image->width() * 4;
    int quarterWidth = image->width() / 4;
    int quarterHeight = image->height() / 4;
    srcPixels.resize(image->height() * rowBytes);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            SkPixmap pixmap(SkImageInfo::MakeN32Premul(quarterWidth, quarterHeight),
                    &srcPixels.front() + x * image->height() * quarterWidth +
                    y * quarterWidth, rowBytes);
            image->readPixels(pixmap, x * quarterWidth, y * quarterHeight);
        }
    }
    canvas->scale(.5f, .5f);
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeN32Premul(image->width(), image->height()),
                             &srcPixels.front(), rowBytes);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
