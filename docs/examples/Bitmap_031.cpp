// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9df1baa17658fbd0c419780f26fd854f
REG_FIDDLE(Bitmap_031, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::Make(2, 2, kN32_SkColorType, kPremul_SkAlphaType));
    for (int index = 0; index < 2; ++index) {
        bitmap.allocPixels();
        bitmap.eraseColor(0x00000000);
        SkDebugf("computeIsOpaque: %s\n", SkBitmap::ComputeIsOpaque(bitmap) ? "true" : "false");
        bitmap.eraseColor(0xFFFFFFFF);
        SkDebugf("computeIsOpaque: %s\n", SkBitmap::ComputeIsOpaque(bitmap) ? "true" : "false");
        bitmap.setInfo(bitmap.info().makeAlphaType(kOpaque_SkAlphaType));
    }
}
}  // END FIDDLE
