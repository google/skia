// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(f16to8888drawImageBug, 256, 256, false, 0) {
// f16to8888drawImageBug
void draw(SkCanvas* canvas) {
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGBLinear();

    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_F16_SkColorType, kPremul_SkAlphaType, colorSpace);
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(imageInfo);
    SkPaint p;
    surface->getCanvas()->drawRect(SkRect::MakeXYWH(20, 20, 40, 40), p);

    sk_sp<SkColorSpace> colorSpace2 = SkColorSpace::MakeSRGB();

    SkImageInfo imageInfo2 =
            SkImageInfo::Make(100, 100, kN32_SkColorType, kPremul_SkAlphaType, colorSpace2);
    sk_sp<SkSurface> surface2 = SkSurface::MakeRaster(imageInfo2);
    surface2->getCanvas()->drawImage(surface->makeImageSnapshot(), 0, 0);

    canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
    canvas->drawImage(surface2->makeImageSnapshot(), 50, 0);
}
}  // END FIDDLE
