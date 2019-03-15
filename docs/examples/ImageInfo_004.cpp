// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7e7c46bb4572e21e13529ff364eb0a9c
REG_FIDDLE(ImageInfo_004, 256, 96, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kRGB_565_SkColorType, kOpaque_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawBitmap(bitmap, 0, 0);
    auto pack565 = [](unsigned r, unsigned g, unsigned b) -> uint16_t {
        return (b << 0) | (g << 5) | (r << 11);
    };
    uint16_t red565[] =  { pack565(0x1F, 0x00, 0x00), pack565(0x17, 0x00, 0x00),
                           pack565(0x0F, 0x00, 0x00), pack565(0x07, 0x00, 0x00) };
    uint16_t blue565[] = { pack565(0x00, 0x00, 0x1F), pack565(0x00, 0x00, 0x17),
                           pack565(0x00, 0x00, 0x0F), pack565(0x00, 0x00, 0x07) };
    SkPixmap redPixmap(imageInfo, &red565, imageInfo.minRowBytes());
    if (bitmap.writePixels(redPixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 2, 2);
    }
    SkPixmap bluePixmap(imageInfo, &blue565, imageInfo.minRowBytes());
    if (bitmap.writePixels(bluePixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 4, 4);
    }
}
}  // END FIDDLE
