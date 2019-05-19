// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8aa8ca63dff4641dfc6ea8a3c555d59c
REG_FIDDLE(Image_readPixels, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->scale(.5f, .5f);
    const int width = 32;
    const int height = 32;
    std::vector<int32_t> dstPixels;
    dstPixels.resize(height * width * 4);
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    for (int y = 0; y < 512; y += height ) {
        for (int x = 0; x < 512; x += width ) {
            if (image->readPixels(info, &dstPixels.front(), width * 4, x, y)) {
                SkPixmap dstPixmap(info, &dstPixels.front(), width * 4);
                SkBitmap bitmap;
                bitmap.installPixels(dstPixmap);
                canvas->drawBitmap(bitmap, 0, 0);
            }
            canvas->translate(48, 0);
        }
        canvas->translate(-16 * 48, 48);
    }
}
}  // END FIDDLE
