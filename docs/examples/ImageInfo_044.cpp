// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=818e4e1191e39d2a642902cbf253b399
REG_FIDDLE(ImageInfo_044, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t pixels[][12] = { { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},
                             { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},
                             { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},
                             { 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF},
                             { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                             { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00},
                             { 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},
                             { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00} };
    SkImageInfo imageInfo = SkImageInfo::MakeA8(8, 8);
    SkBitmap bitmap;
    bitmap.installPixels(imageInfo, (void*) pixels, sizeof(pixels[0]));
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawBitmapRect(bitmap, SkRect::MakeWH(8, 8), SkRect::MakeWH(32, 32), &paint);
    size_t offset = imageInfo.computeOffset(2, 3, sizeof(pixels[0]));
    pixels[0][offset] = 0x7F;
    offset = imageInfo.computeOffset(5, 3, sizeof(pixels[0]));
    pixels[0][offset] = 0x7F;
    bitmap.installPixels(imageInfo, (void*) pixels, sizeof(pixels[0]));
    canvas->drawBitmapRect(bitmap, SkRect::MakeWH(8, 8), SkRect::MakeWH(128, 128), &paint);
}
}  // END FIDDLE
