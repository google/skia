// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=817f95879fadba44baf87ea60e9b595a
REG_FIDDLE(Bitmap_colorSpace, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType,
            SkColorSpace::MakeSRGBLinear()));
    SkColorSpace* colorSpace = bitmap.colorSpace();
    SkDebugf("gammaCloseToSRGB: %s  gammaIsLinear: %s  isSRGB: %s\n",
            colorSpace->gammaCloseToSRGB() ? "true" : "false",
            colorSpace->gammaIsLinear() ? "true" : "false",
            colorSpace->isSRGB() ? "true" : "false");
}
}  // END FIDDLE
