// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e006bb05cf74ec8d2b3d6adeb5dba11b
REG_FIDDLE(Bitmap_getPixels, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(4, 4, kPremul_SkAlphaType));
    bitmap.allocPixels();
    bitmap.eraseColor(0x00000000);
    void* baseAddr = bitmap.getPixels();
    *(SkPMColor*)baseAddr = 0xFFFFFFFF;
    SkDebugf("bitmap.getColor(0, 1) %c= 0x00000000\n",
              bitmap.getColor(0, 1)  == 0x00000000 ? '=' : '!');
    SkDebugf("bitmap.getColor(0, 0) %c= 0xFFFFFFFF\n",
              bitmap.getColor(0, 0)  == 0xFFFFFFFF ? '=' : '!');
}
}  // END FIDDLE
