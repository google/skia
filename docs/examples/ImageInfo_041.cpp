// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(ImageInfo_041, 256, 256, true, 0) {
// HASH=e47b911f94fc629f756a829e523a2a89
void draw(SkCanvas* canvas) {
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16"};
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        SkImageInfo info = SkImageInfo::Make(1, 1, colorType, kOpaque_SkAlphaType);
        SkDebugf("color: k" "%s" "_SkColorType" "%*s" "shiftPerPixel: %d\n",
                colors[colorType], 14 - strlen(colors[colorType]), " ",
                info.shiftPerPixel());
    }
}
}
