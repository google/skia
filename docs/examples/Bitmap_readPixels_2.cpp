// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e9f70cbc9827097449a386ec7a8a8188
REG_FIDDLE(Bitmap_readPixels_2, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> srcPixels;
    srcPixels.resize(source.height() * source.rowBytes());
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            SkPixmap pixmap(SkImageInfo::MakeN32Premul(source.width() / 4, source.height() / 4),
                    &srcPixels.front() + x * source.height() * source.width() / 4 +
                    y * source.width() / 4, source.rowBytes());
            source.readPixels(pixmap, x * source.width() / 4, y * source.height() / 4);
        }
    }
    canvas->scale(.5f, .5f);
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeN32Premul(source.width(), source.height()),
                             &srcPixels.front(), source.rowBytes());
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
