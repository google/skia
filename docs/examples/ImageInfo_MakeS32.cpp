// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=de418ccb42471d1589508ef3955f8c53
REG_FIDDLE(ImageInfo_MakeS32, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    const int width = 256;
    const int height = 32;
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    SkColor  gradColors[] = { 0xFFAA0055, 0xFF11CC88 };
    SkPoint  gradPoints[] = { { 0, 0 }, { width, 0 } };
    SkPaint gradPaint;
    gradPaint.setShader(SkGradientShader::MakeLinear(gradPoints, gradColors, nullptr,
                    SK_ARRAY_COUNT(gradColors), SkTileMode::kClamp));
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType));
    SkCanvas offScreen(bitmap);
    offScreen.drawRect(SkRect::MakeWH(width, height), gradPaint);
    canvas->drawBitmap(bitmap, 0, 0);
    bitmap.allocPixels(SkImageInfo::MakeS32(width, height, kPremul_SkAlphaType));
    SkCanvas sRGBOffscreen(bitmap);
    sRGBOffscreen.drawRect(SkRect::MakeWH(width, height), gradPaint);
    canvas->drawBitmap(bitmap, 0, 48);
    SkBitmap noColorSpaceBitmap;
    noColorSpaceBitmap.setInfo(SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType));
    noColorSpaceBitmap.setPixels(bitmap.getAddr(0, 0));
    canvas->drawBitmap(noColorSpaceBitmap, 0, 96);
}
}  // END FIDDLE
