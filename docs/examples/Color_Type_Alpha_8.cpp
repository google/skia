// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=21ae21e4ce53d2018e042dd457997300
REG_FIDDLE(Color_Type_Alpha_8, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kAlpha_8_SkColorType, kOpaque_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    SkPaint orangePaint;
    orangePaint.setARGB(0xFF, 0xFF, 0xA5, 0x00);
    canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &orangePaint);
    uint8_t alpha8[] = { 0xFF, 0xBB, 0x77, 0x33 };
    SkPixmap alphaPixmap(imageInfo, &alpha8, imageInfo.minRowBytes());
    if (bitmap.writePixels(alphaPixmap, 0, 0)) {
        canvas->drawImage(bitmap.asImage(), 2, 2, SkSamplingOptions(), &orangePaint);
    }
}
}  // END FIDDLE
