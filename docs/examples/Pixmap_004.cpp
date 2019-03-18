#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=30d70aec4de17c831dba71e03dc9664a
REG_FIDDLE(Pixmap_setColorSpace, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPixmap pixmap;
    sk_sp<SkColorSpace> colorSpace1 = SkColorSpace::MakeRGB(SkColorSpace::kLinear_RenderTargetGamma,
                                                            SkColorSpace::kRec2020_Gamut);
    SkDebugf("is %sunique\n", colorSpace1->unique() ? "" : "not ");
    pixmap.setColorSpace(colorSpace1);
    SkDebugf("is %sunique\n", colorSpace1->unique() ? "" : "not ");
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
