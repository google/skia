// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9547e74a9d37553a667b913ffd1312dd
REG_FIDDLE(Pixmap_empty_constructor, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
    SkPixmap pixmap;
    for (int i = 0; i < 2; ++i) {
       SkDebugf("width: %2d  height: %2d", pixmap.width(), pixmap.height());
       SkDebugf("  color: k%s_SkColorType", colors[pixmap.colorType()]);
       SkDebugf("  alpha: k%s_SkAlphaType\n", alphas[pixmap.alphaType()]);
       pixmap.reset(SkImageInfo::Make(25, 35, kRGBA_8888_SkColorType, kOpaque_SkAlphaType),
                    nullptr, 0);
    }
}
}  // END FIDDLE
