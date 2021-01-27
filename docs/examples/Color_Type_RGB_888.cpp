// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4260d6cc15db2c60c07f6fdc8d9ae425
REG_FIDDLE(Color_Type_RGB_888, 256, 96, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kRGB_888x_SkColorType, kOpaque_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    auto pack888 = [](unsigned r, unsigned g, unsigned b) -> uint32_t {
        return (r << 0) | (g << 8) | (b << 16);
    };
    uint32_t red888[] =  { pack888(0xFF, 0x00, 0x00), pack888(0xbb, 0x00, 0x00),
        pack888(0x77, 0x00, 0x00), pack888(0x33, 0x00, 0x00) };
    uint32_t blue888[] = { pack888(0x00, 0x00, 0xFF), pack888(0x00, 0x00, 0xbb),
        pack888(0x00, 0x00, 0x77), pack888(0x00, 0x00, 0x33) };
    if (bitmap.installPixels(imageInfo, (void*) red888, imageInfo.minRowBytes())) {
        canvas->drawImage(bitmap.asImage(), 2, 2);
    }
    if (bitmap.installPixels(imageInfo, (void*) blue888, imageInfo.minRowBytes())) {
        canvas->drawImage(bitmap.asImage(), 4, 4);
    }
}
}  // END FIDDLE
