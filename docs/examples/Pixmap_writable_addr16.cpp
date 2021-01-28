// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=6da54774f6432b46b47ea9013c15f280
REG_FIDDLE(Pixmap_writable_addr16, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint16_t storage[][5] = {{ 0xCABF, 0xDABE, 0xCA9D, 0xC96C, 0xA39B },
                             { 0xACEE, 0xA87C, 0x893A, 0x4779, 0x8708 },
                             { 0x4B7C, 0x255B, 0x2559, 0x2557, 0x4656 },
                             { 0x9099, 0x8128, 0x2557, 0x4124, 0x3323 },
                             { 0x7547, 0x5505, 0x4434, 0x2012, 0x0000 }};
    SkImageInfo imageInfo = SkImageInfo::Make(5, 5, kARGB_4444_SkColorType, kPremul_SkAlphaType);
    SkPixmap pixmap(imageInfo, storage[0], sizeof(storage) / 5);
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    canvas->scale(10, 10);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    *pixmap.writable_addr16(2, 2) = 0x000F;
    bitmap.installPixels(pixmap);
    canvas->drawImage(bitmap.asImage(), 10, 0);
}
}  // END FIDDLE
