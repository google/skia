// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9b6de4a07b2316228e9340e5a3b82134
REG_FIDDLE(ImageInfo_bytesPerPixel, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        SkImageInfo info = SkImageInfo::Make(1, 1, colorType, kOpaque_SkAlphaType);
        const char* colorTypeStr = SkColorTypeToString(colorType);
        SkDebugf("color: k" "%s" "_SkColorType" "%*s" "bytesPerPixel: %d\n",
                colorTypeStr, 13 - strlen(colorTypeStr), " ",
                info.bytesPerPixel());
    }
}
}  // END FIDDLE
