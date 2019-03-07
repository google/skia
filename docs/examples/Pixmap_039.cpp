// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=df4e355c4845350daede833b4fd21ec1
REG_FIDDLE(Pixmap_039, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    std::vector<int32_t> pixels;
    const int width = 256;
    const int height = 64;
    pixels.resize(height * width * 4);
    SkImageInfo srcInfo = SkImageInfo::MakeN32Premul(width, height);
    SkPixmap srcPixmap(srcInfo, (const void*) &pixels.front(), width * 4);
    SkColor  gradColors[] = { 0xFFAA3300, 0x7F881122 };
    SkPoint  gradPoints[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(gradPoints, gradColors, nullptr,
                    SK_ARRAY_COUNT(gradColors), SkShader::kClamp_TileMode));
    SkBitmap bitmap;
    bitmap.installPixels(srcPixmap);
    SkCanvas srcCanvas(bitmap);
    srcCanvas.drawRect(SkRect::MakeWH(width, height), paint);
    canvas->drawBitmap(bitmap, 0, 0);
    std::vector<int32_t> dstPixels;
    dstPixels.resize(height * width * 2);
    SkImageInfo dstInfo = srcInfo.makeColorType(kARGB_4444_SkColorType);
    srcPixmap.readPixels(dstInfo, &dstPixels.front(), width * 2);
    SkPixmap dstPixmap(dstInfo, &dstPixels.front(), width * 2);
    bitmap.installPixels(dstPixmap);
    canvas->drawBitmap(bitmap, 0, 128);
}
}  // END FIDDLE
