// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=33a360c3404ac21db801943336843d8e
REG_FIDDLE(ImageInfo_005, 256, 96, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kARGB_4444_SkColorType, kPremul_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawBitmap(bitmap, 0, 0);
    auto pack4444 = [](unsigned a, unsigned r, unsigned g, unsigned b) -> uint16_t {
        return (a << 0) | (b << 4) | (g << 8) | (r << 12);
    };
    uint16_t red4444[] =  { pack4444(0xF, 0xF, 0x0, 0x0), pack4444(0xF, 0xb, 0x0, 0x0),
                            pack4444(0xF, 0x7, 0x0, 0x0), pack4444(0xF, 0x3, 0x0, 0x0) };
    uint16_t blue4444[] = { pack4444(0xF, 0x0, 0x0, 0xF), pack4444(0xF, 0x0, 0x0, 0xb),
                            pack4444(0xF, 0x0, 0x0, 0x7), pack4444(0xF, 0x0, 0x0, 0x3) };
    SkPixmap redPixmap(imageInfo, &red4444, imageInfo.minRowBytes());
    if (bitmap.writePixels(redPixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 2, 2);
    }
    SkPixmap bluePixmap(imageInfo, &blue4444, imageInfo.minRowBytes());
    if (bitmap.writePixels(bluePixmap, 0, 0)) {
        canvas->drawBitmap(bitmap, 4, 4);
    }
}
}  // END FIDDLE
