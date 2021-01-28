// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(gpu8888diff, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo =
            SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    offscreen.clear(SK_ColorGRAY);
    canvas->drawImage(bitmap.asImage(), 2, 2);
    auto pack8888 = [](unsigned a, unsigned r, unsigned g, unsigned b) -> uint32_t {
        return (a << 24) | (b << 16) | (g << 8) | (r << 0);
    };
    uint32_t red8888[] = {pack8888(0xFF, 0xFF, 0x0, 0x0), pack8888(0xFF, 0xbb, 0x0, 0x0),
                          pack8888(0xFF, 0x99, 0x0, 0x0), pack8888(0xFF, 0x55, 0x0, 0x0)};
    uint32_t blue8888[] = {pack8888(0xFF, 0x0, 0x0, 0x0FF), pack8888(0xFF, 0x0, 0x0, 0x0bb),
                           pack8888(0xFF, 0x0, 0x0, 0x099), pack8888(0xFF, 0x0, 0x0, 0x055)};
    SkPixmap redPixmap(imageInfo, &red8888, 8);
    if (bitmap.writePixels(redPixmap, 0, 0)) {
        canvas->drawImage(bitmap.asImage(), 4, 4);
    }
    SkPixmap bluePixmap(imageInfo, &blue8888, 8);
    if (bitmap.writePixels(bluePixmap, 0, 0)) {
        canvas->drawImage(bitmap.asImage(), 6, 6);
    }
}
}  // END FIDDLE
