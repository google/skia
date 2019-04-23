// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=513afec5795a9504ebf6af5373d16b6b
REG_FIDDLE(Image_MakeRasterCopy, 256, 50, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t storage[][5] = {{ 0xCA, 0xDA, 0xCA, 0xC9, 0xA3 },
                            { 0xAC, 0xA8, 0x89, 0xA7, 0x87 },
                            { 0x9B, 0xB5, 0xE5, 0x95, 0x46 },
                            { 0x90, 0x81, 0xC5, 0x71, 0x33 },
                            { 0x75, 0x55, 0x44, 0x40, 0x30 }};
    SkImageInfo imageInfo = SkImageInfo::Make(5, 5, kGray_8_SkColorType, kOpaque_SkAlphaType);
    SkPixmap pixmap(imageInfo, storage[0], sizeof(storage) / 5);
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    sk_sp<SkImage> image = SkImage::MakeRasterCopy(pixmap);
    *pixmap.writable_addr8(2, 2) = 0x00;
    canvas->scale(10, 10);
    canvas->drawBitmap(bitmap, 0, 0);
    canvas->drawImage(image, 10, 0);
}
}  // END FIDDLE
