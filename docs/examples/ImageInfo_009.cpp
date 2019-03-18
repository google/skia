// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1282dc1127ce1b0061544619ae4de0f0
REG_FIDDLE(Color_Type_RGBA_1010102, 256, 96, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kRGBA_1010102_SkColorType, kOpaque_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawBitmap(bitmap, 0, 0);
    auto pack1010102 = [](unsigned r, unsigned g, unsigned b, unsigned a) -> uint32_t {
        return (r << 0) | (g << 10) | (b << 20) | (a << 30);
    };
    uint32_t redBits[] =  { pack1010102(0x3FF, 0x000, 0x000, 0x3),
                            pack1010102(0x2ff, 0x000, 0x000, 0x3),
                            pack1010102(0x1ff, 0x000, 0x000, 0x3),
                            pack1010102(0x0ff, 0x000, 0x000, 0x3) };
    uint32_t blueBits[] = { pack1010102(0x000, 0x000, 0x3FF, 0x3),
                            pack1010102(0x000, 0x000, 0x2ff, 0x3),
                            pack1010102(0x000, 0x000, 0x1ff, 0x3),
                            pack1010102(0x000, 0x000, 0x0ff, 0x3) };
    if (bitmap.installPixels(imageInfo, (void*) redBits, imageInfo.minRowBytes())) {
        canvas->drawBitmap(bitmap, 2, 2);
    }
    SkPixmap bluePixmap(imageInfo, &blueBits, imageInfo.minRowBytes());
    if (bitmap.installPixels(imageInfo, (void*) blueBits, imageInfo.minRowBytes())) {
        canvas->drawBitmap(bitmap, 4, 4);
    }
}
}  // END FIDDLE
