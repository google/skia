// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=33f65524736736fd91802b4198ba6fa8
REG_FIDDLE(ImageInfo_refColorSpace, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info1 = SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType,
            SkColorSpace::MakeSRGBLinear());
    SkImageInfo info2 = SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType,
            info1.refColorSpace());
    SkColorSpace* colorSpace = info2.colorSpace();
    SkDebugf("gammaCloseToSRGB: %s  gammaIsLinear: %s  isSRGB: %s\n",
            colorSpace->gammaCloseToSRGB() ? "true" : "false",
            colorSpace->gammaIsLinear() ? "true" : "false",
            colorSpace->isSRGB() ? "true" : "false");
}
}  // END FIDDLE
