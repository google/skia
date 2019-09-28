// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=53e00899ef2e00e2096daf7a07d9b059
REG_FIDDLE(Bitmap_getAddr16, 256, 256, true, 3) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap16;
    SkImageInfo dstInfo = SkImageInfo::Make(source.width(), source.height(), kARGB_4444_SkColorType,
                     kPremul_SkAlphaType);
    bitmap16.allocPixels(dstInfo);
    if (source.readPixels(dstInfo, bitmap16.getPixels(), bitmap16.rowBytes(), 0, 0)) {
        uint16_t* row0 = bitmap16.getAddr16(0, 0);
        uint16_t* row1 = bitmap16.getAddr16(0, 1);
        size_t interval = (row1 - row0) * bitmap16.bytesPerPixel();
        SkDebugf("addr interval %c= rowBytes\n", interval == bitmap16.rowBytes() ? '=' : '!');
    }
}
}  // END FIDDLE
