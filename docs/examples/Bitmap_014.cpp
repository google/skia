// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cb028b7931da85b949ad0953b9711f9f
REG_FIDDLE(Bitmap_014, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap1, bitmap2;
    bitmap1.setInfo(SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType,
            SkColorSpace::MakeSRGBLinear()));
    bitmap2.setInfo(SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType,
            bitmap1.refColorSpace()));
    SkColorSpace* colorSpace = bitmap2.colorSpace();
    SkDebugf("gammaCloseToSRGB: %s  gammaIsLinear: %s  isSRGB: %s\n",
            colorSpace->gammaCloseToSRGB() ? "true" : "false",
            colorSpace->gammaIsLinear() ? "true" : "false",
            colorSpace->isSRGB() ? "true" : "false");
}
}  // END FIDDLE
