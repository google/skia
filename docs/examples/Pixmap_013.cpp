// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3421bb20a302d563832ba7bb45e0cc58
REG_FIDDLE(Pixmap_colorSpace, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPixmap pixmap(SkImageInfo::MakeN32(16, 32, kPremul_SkAlphaType,
            SkColorSpace::MakeSRGBLinear()), nullptr, 64);
    SkColorSpace* colorSpace = pixmap.colorSpace();
    SkDebugf("gammaCloseToSRGB: %s  gammaIsLinear: %s  isSRGB: %s\n",
            colorSpace->gammaCloseToSRGB() ? "true" : "false",
            colorSpace->gammaIsLinear() ? "true" : "false",
            colorSpace->isSRGB() ? "true" : "false");
}
}  // END FIDDLE
