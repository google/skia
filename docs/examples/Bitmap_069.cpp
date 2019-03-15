// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=b2cbbbbcffb618865d8aae3bc04b2a62
REG_FIDDLE(Bitmap_069, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    const int width = 256;
    const int height = 64;
    SkImageInfo srcInfo = SkImageInfo::MakeN32Premul(width, height);
    SkColor  gradColors[] = { 0xFFAA3300, 0x7F881122 };
    SkPoint  gradPoints[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(gradPoints, gradColors, nullptr,
                    SK_ARRAY_COUNT(gradColors), SkShader::kClamp_TileMode));
    SkBitmap bitmap;
    bitmap.allocPixels(srcInfo);
    SkCanvas srcCanvas(bitmap);
    srcCanvas.drawRect(SkRect::MakeWH(width, height), paint);
    canvas->drawBitmap(bitmap, 0, 0);
    SkImageInfo dstInfo = srcInfo.makeColorType(kARGB_4444_SkColorType);
    std::vector<int16_t> dstPixels;
    dstPixels.resize(height * width);
    bitmap.readPixels(dstInfo, &dstPixels.front(), width * 2, 0, 0);
    SkPixmap dstPixmap(dstInfo, &dstPixels.front(), width * 2);
    bitmap.installPixels(dstPixmap);
    canvas->drawBitmap(bitmap, 0, 64);
}
}  // END FIDDLE
