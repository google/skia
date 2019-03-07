// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=0424f67ebc2858e8fd04ae3367b115ff
REG_FIDDLE(Color_008, 256, 128, false, 1) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> srcPixels;
    srcPixels.resize(source.height() * source.rowBytes());
    SkPixmap pixmap(SkImageInfo::MakeN32Premul(source.width(), source.height()),
                    &srcPixels.front(), source.rowBytes());
    source.readPixels(pixmap, 0, 0);
    for (int y = 0; y < source.height(); ++y) {
        for (int x = 0; x < source.width(); ++x) {
            SkPMColor pixel = srcPixels[y * source.width() + x];
            const SkColor color = SkUnPreMultiply::PMColorToColor(pixel);
            if (SkColorGetA(color) == SK_AlphaOPAQUE) {
                srcPixels[y * source.width() + x] = SK_ColorGREEN;
            }
        }
    }
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
