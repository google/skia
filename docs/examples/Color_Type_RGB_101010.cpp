// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=92f81aa0459230459600a01e79ccff29
REG_FIDDLE(Color_Type_RGB_101010, 256, 96, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kRGB_101010x_SkColorType, kOpaque_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawBitmap(bitmap, 0, 0);
    auto pack101010x = [](unsigned r, unsigned g, unsigned b) -> uint32_t {
        return (r << 0) | (g << 10) | (b << 20);
    };
    uint32_t redBits[] =  { pack101010x(0x3FF, 0x000, 0x000), pack101010x(0x2ff, 0x000, 0x000),
    pack101010x(0x1ff, 0x000, 0x000), pack101010x(0x0ff, 0x000, 0x000) };
    uint32_t blueBits[] = { pack101010x(0x000, 0x000, 0x3FF), pack101010x(0x000, 0x000, 0x2ff),
    pack101010x(0x000, 0x000, 0x1ff), pack101010x(0x000, 0x000, 0x0ff) };
    if (bitmap.installPixels(imageInfo, (void*) redBits, imageInfo.minRowBytes())) {
        canvas->drawBitmap(bitmap, 2, 2);
    }
    SkPixmap bluePixmap(imageInfo, &blueBits, imageInfo.minRowBytes());
    if (bitmap.installPixels(imageInfo, (void*) blueBits, imageInfo.minRowBytes())) {
        canvas->drawBitmap(bitmap, 4, 4);
    }
}
}  // END FIDDLE
