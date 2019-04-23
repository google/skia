// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3ac267b08b12dc83c95f91d8dd5d70ee
REG_FIDDLE(ImageInfo_makeColorType, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const int width = 256;
    const int height = 128;
    SkColor pixels[height][width];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int red = SkScalarRoundToInt(255 * SkScalarAbs(SkScalarSin((x * 4 + y) * 0.03f)));
            int blue = SkScalarRoundToInt(255 * SkScalarAbs(SkScalarCos((x * 3 + y) * 0.04f)));
            int green = SkScalarRoundToInt(255 * SkScalarAbs(SkScalarSin((x * 2 + y) * 0.05f)));
            int alpha = SkScalarRoundToInt(255 * SkScalarAbs(SkScalarCos((x * 1 + y) * 0.006f)));
            pixels[y][x] =
                SkColorSetARGB(alpha, red * alpha / 255, green * alpha / 255, blue * alpha / 255);
        }
    }
    SkBitmap bitmap;
    SkImageInfo info = SkImageInfo::Make(width, height, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    bitmap.installPixels(info, (void*) pixels, sizeof(SkColor) * width);
    canvas->drawBitmap(source, 0, 0);
    canvas->drawBitmap(bitmap, 0, 0);
    SkImageInfo rgbaInfo = info.makeColorType(kRGBA_8888_SkColorType);
    bitmap.installPixels(rgbaInfo, (void*) pixels, sizeof(SkColor) * width);
    canvas->drawBitmap(bitmap, 0, 128);
}
}  // END FIDDLE
