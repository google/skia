// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=56ede4b7d45c15d5936f81ac3d74f070
REG_FIDDLE(Bitmap_shiftPerPixel, 256, 256, true, 0) {
static const char* color_type_to_string(SkColorType v) {
    switch (v) {
        case kUnknown_SkColorType: return "Unknown";
        case kAlpha_8_SkColorType: return "Alpha_8";
        case kRGB_565_SkColorType: return "RGB_565";
        case kARGB_4444_SkColorType: return "ARGB_4444";
        case kRGBA_8888_SkColorType: return "RGBA_8888";
        case kRGB_888x_SkColorType: return "RGB_888x";
        case kBGRA_8888_SkColorType: return "BGRA_8888";
        case kRGBA_1010102_SkColorType: return "RGBA_1010102";
        case kRGB_101010x_SkColorType: return "RGB_101010x";
        case kGray_8_SkColorType: return "Gray_8";
        case kRGBA_F16Norm_SkColorType: return "RGBA_F16Norm";
        case kRGBA_F16_SkColorType: return "RGBA_F16";
        case kRGBA_F32_SkColorType: return "RGBA_F32";
    }
    SkASSERT(false); return "";
}

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
        const char* colorTypeStr = color_type_to_string(colorType);
        SkDebugf("color: k" "%s" "_SkColorType" "%*s" "shiftPerPixel: %d\n",
                colorTypeStr, 14 - strlen(colorTypeStr), " ",
                bitmap.shiftPerPixel());
    }
}
}  // END FIDDLE
