// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=de9be45255e48fae445c916a41063abc
REG_FIDDLE(Bitmap_swap, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    auto debugster = [](const char* prefix, const SkBitmap& b) -> void {
        const char* alphas[] = {"Unknown", "Opaque", "Premul", "Unpremul"};
        const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                                "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16Norm",
                                "RGBA_F16"};
        SkDebugf("%s width:%d height:%d colorType:k%s_SkColorType alphaType:k%s_SkAlphaType\n",
                 prefix, b.width(), b.height(), colors[b.colorType()], alphas[b.alphaType()]);
    };
    SkBitmap one, two;
    if (!one.tryAllocPixels(
            SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kOpaque_SkAlphaType))) {
        return;
    }
    if (!two.tryAllocPixels(
            SkImageInfo::Make(2, 2, kBGRA_8888_SkColorType, kPremul_SkAlphaType))) {
        return;
    }
    for (int index = 0; index < 2; ++index) {
       debugster("one", one);
       debugster("two", two);
       one.swap(two);
    }
}
}  // END FIDDLE
