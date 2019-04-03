// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=befac1c29ed21507d367e4d824383a04
REG_FIDDLE(ColorTypeValidateAlphaType, 256, 640, false, 0) {
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

static const char* alpha_type_to_string(SkAlphaType v) {
    switch (v) {
        case kUnknown_SkAlphaType: return "Unknown";
        case kOpaque_SkAlphaType: return "Opaque";
        case kPremul_SkAlphaType: return "Premul";
        case kUnpremul_SkAlphaType: return "Unpremul";
    }
    SkASSERT(false); return "";
}

void draw(SkCanvas* canvas) {
    SkAlphaType alphaTypes[] = { kUnknown_SkAlphaType, kOpaque_SkAlphaType, kPremul_SkAlphaType,
    kUnpremul_SkAlphaType
                               };
    SkPaint paint;
    SkFont font(SkTypeface::MakeFromName("monospace", SkFontStyle()), 10);
    int y = 15;
    canvas->drawString("   colorType   alphaType  canonical", 10, y, font, paint);
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        for (SkAlphaType alphaType : alphaTypes) {
            SkAlphaType canonicalAlphaType  = kUnknown_SkAlphaType;
            bool result = SkColorTypeValidateAlphaType(colorType, alphaType, &canonicalAlphaType);
            SkString string;
            string.printf("%13s %10s %10s", color_type_to_string(colorType),
                          alpha_type_to_string(alphaType),
                     result ? alpha_type_to_string(canonicalAlphaType) : "------  ");
            canvas->drawString(string, 10, y += 14, font, paint);
        }
    }
}
}  // END FIDDLE
