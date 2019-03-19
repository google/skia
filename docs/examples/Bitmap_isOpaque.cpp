// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5e76b68bb46d54315eb0c12d83bd6949
REG_FIDDLE(Bitmap_isOpaque, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const int height = 2;
    const int width = 2;
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType));
    for (int index = 0; index < 2; ++index) {
        bitmap.allocPixels();
        bitmap.eraseColor(0x00000000);
        SkDebugf("isOpaque: %s\n", bitmap.isOpaque() ? "true" : "false");
        bitmap.eraseColor(0xFFFFFFFF);
        SkDebugf("isOpaque: %s\n", bitmap.isOpaque() ? "true" : "false");
        bitmap.setInfo(bitmap.info().makeAlphaType(kOpaque_SkAlphaType));
    }
}
}  // END FIDDLE
