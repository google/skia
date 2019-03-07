// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6e2a8c9358b34aebd2ec586815fe9d3a
REG_FIDDLE(Bitmap_048, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t storage[][5] = {{ 0xCA, 0xDA, 0xCA, 0xC9, 0xA3 },
                            { 0xAC, 0xA8, 0x89, 0x47, 0x87 },
                            { 0x4B, 0x25, 0x25, 0x25, 0x46 },
                            { 0x90, 0x81, 0x25, 0x41, 0x33 },
                            { 0x75, 0x55, 0x44, 0x20, 0x00 }};
    SkImageInfo imageInfo = SkImageInfo::Make(5, 5, kGray_8_SkColorType, kOpaque_SkAlphaType);
    SkPixmap pixmap(imageInfo, storage[0], sizeof(storage) / 5);
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    canvas->scale(10, 10);
    canvas->drawBitmap(bitmap, 0, 0);
    *pixmap.writable_addr8(2, 2) = 0xFF;
    bitmap.installPixels(pixmap);
    canvas->drawBitmap(bitmap, 10, 0);
}
}  // END FIDDLE
