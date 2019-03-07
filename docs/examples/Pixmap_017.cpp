// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
REG_FIDDLE(Pixmap_017, 256, 256, true, 0) {
// HASH=bf31ee140e2c163c3957276e6d4c4f0c
void draw(SkCanvas* canvas) {
    const char* colors[] = {"Unknown", "Alpha_8", "RGB_565", "ARGB_4444", "RGBA_8888", "RGB_888x",
                            "BGRA_8888", "RGBA_1010102", "RGB_101010x", "Gray_8", "RGBA_F16"};
    SkImageInfo info = SkImageInfo::MakeA8(1, 1);
    for (SkColorType colorType : { kUnknown_SkColorType,   kAlpha_8_SkColorType,
                                   kRGB_565_SkColorType,   kARGB_4444_SkColorType,
                                   kRGBA_8888_SkColorType, kBGRA_8888_SkColorType,
                                   kGray_8_SkColorType,    kRGBA_F16_SkColorType } ) {
        SkPixmap pixmap(info.makeColorType(colorType), nullptr, 4);
        SkDebugf("color: k" "%s" "_SkColorType" "%*s" "bytesPerPixel: %d shiftPerPixel: %d\n",
                colors[colorType], 10 - strlen(colors[colorType]), " ",
                pixmap.info().bytesPerPixel(), pixmap.shiftPerPixel());
    }
}
}
