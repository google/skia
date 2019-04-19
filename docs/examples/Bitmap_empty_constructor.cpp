// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6739d14ec0d6a373f2fcadc6b3077fd4
REG_FIDDLE(Bitmap_empty_constructor, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                            "RGBA_F16"};
    SkBitmap bitmap;
    for (int i = 0; i < 2; ++i) {
       SkDebugf("width: %2d  height: %2d", bitmap.width(), bitmap.height());
       SkDebugf("  color: k%s_SkColorType", colors[bitmap.colorType()]);
       SkDebugf("  alpha: k%s_SkAlphaType\n", alphas[bitmap.alphaType()]);
       bitmap.setInfo(SkImageInfo::Make(25, 35, kRGBA_8888_SkColorType, kOpaque_SkAlphaType),
                      0);
    }
}
}  // END FIDDLE
