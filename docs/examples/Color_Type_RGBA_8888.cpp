// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=947922a19d59893fe7f9d9ee1954379b
REG_FIDDLE(Color_Type_RGBA_8888, 256, 96, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawBitmap(bitmap, 0, 0);
    auto pack8888 = [](unsigned a, unsigned r, unsigned g, unsigned b) -> uint32_t {
        return (r << 0) | (g << 8) | (b << 16) | (a << 24);
    };
    uint32_t red8888[] = { pack8888(0xFF, 0xFF, 0x0, 0x0), pack8888(0xFF, 0xbb, 0x0, 0x0),
                           pack8888(0xFF, 0x77, 0x0, 0x0), pack8888(0xFF, 0x33, 0x0, 0x0) };
    uint32_t blue8888[] = { pack8888(0xFF, 0x0, 0x0, 0x0FF), pack8888(0xFF, 0x0, 0x0, 0x0bb),
                            pack8888(0xFF, 0x0, 0x0, 0x077), pack8888(0xFF, 0x0, 0x0, 0x033) };
    SkPixmap redPixmap(imageInfo, &red8888, imageInfo.minRowBytes());
    if (bitmap.writePixels(redPixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 2, 2);
    }
    SkPixmap bluePixmap(imageInfo, &blue8888, imageInfo.minRowBytes());
    if (bitmap.writePixels(bluePixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 4, 4);
    }
}
}  // END FIDDLE
