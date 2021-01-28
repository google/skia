// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(f16to8888busted, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    // Create the linear-rgb color space and the image info
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeSRGBLinear();
    SkImageInfo info = SkImageInfo::Make(100, 100, SkColorType::kRGBA_F16_SkColorType,
                                         SkAlphaType::kPremul_SkAlphaType, colorSpace);

    sk_sp<SkSurface> offscreen = SkSurface::MakeRaster(info);
    SkPaint paint;
    offscreen->getCanvas()->drawRect(SkRect::MakeXYWH(25, 25, 50, 50), paint);

    // Take a snapshot from surface and draw it on the canvas
    sk_sp<SkImage> img = offscreen->makeImageSnapshot();
    canvas->drawImageRect(img, SkRect::MakeWH(100, 100), SkSamplingOptions());
}
}  // END FIDDLE
