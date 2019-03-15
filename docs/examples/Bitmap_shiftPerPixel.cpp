// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=56ede4b7d45c15d5936f81ac3d74f070
REG_FIDDLE(Bitmap_shiftPerPixel, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeA8(1, 1);
    SkBitmap bitmap;
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        bitmap.setInfo(info.makeColorType(colorType));
        const char* colorTypeStr = SkColorTypeToString(colorType);
        SkDebugf("color: k" "%s" "_SkColorType" "%*s" "shiftPerPixel: %d\n",
                colorTypeStr, 14 - strlen(colorTypeStr), " ",
                bitmap.shiftPerPixel());
    }
}
}  // END FIDDLE
