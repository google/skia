// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=bf31ee140e2c163c3957276e6d4c4f0c
REG_FIDDLE(Pixmap_shiftPerPixel, 256, 256, true, 0) {
const char* color_type(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return "Unknown";
        case kAlpha_8_SkColorType:      return "Alpha_8";
        case kRGB_565_SkColorType:      return "RGB_565";
        case kARGB_4444_SkColorType:    return "ARGB_4444";
        case kRGBA_8888_SkColorType:    return "RGBA_8888";
        case kRGB_888x_SkColorType:     return "RGB_888x";
        case kBGRA_8888_SkColorType:    return "BGRA_8888";
        case kRGBA_1010102_SkColorType: return "RGBA_1010102";
        case kRGB_101010x_SkColorType:  return "RGB_101010x";
        case kGray_8_SkColorType:       return "Gray_8";
        case kRGBA_F16Norm_SkColorType: return "RGBA_F16Norm";
        case kRGBA_F16_SkColorType:     return "RGBA_F16";
        case kRGBA_F32_SkColorType:     return "RGBA_F32";
        default: SkASSERT(false); return nullptr;
    }
}
void draw(SkCanvas* canvas) {
    SkImageInfo info = SkImageInfo::MakeA8(1, 1);
    for (SkColorType colorType : { kUnknown_SkColorType,   kAlpha_8_SkColorType,
                                   kRGB_565_SkColorType,   kARGB_4444_SkColorType,
                                   kRGBA_8888_SkColorType, kBGRA_8888_SkColorType,
                                   kGray_8_SkColorType,    kRGBA_F16_SkColorType } ) {
        SkPixmap pixmap(info.makeColorType(colorType), nullptr, 4);
        SkDebugf("color: k" "%s" "_SkColorType" "%*s" "bytesPerPixel: %d shiftPerPixel: %d\n",
                color_type(colorType), (int)(10 - strlen(color_type(colorType))), " ",
                pixmap.info().bytesPerPixel(), pixmap.shiftPerPixel());
    }
}
}  // END FIDDLE
