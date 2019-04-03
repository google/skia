// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=af3adcbea7b58bf90298ca5e0ea93030
REG_FIDDLE(Bitmap_setAlphaType, 256, 256, true, 0) {
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
    SkBitmap bitmap;
    SkAlphaType alphaTypes[] = { kUnknown_SkAlphaType, kOpaque_SkAlphaType, kPremul_SkAlphaType,
    kUnpremul_SkAlphaType
                               };
    SkDebugf("%18s%15s%17s%18s%19s\n", "Canonical", "Unknown", "Opaque", "Premul", "Unpremul");
    for (SkColorType colorType : {
    kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
    kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
    kGray_8_SkColorType, kRGBA_F16_SkColorType
                                 } ) {
        for (SkAlphaType canonicalAlphaType : alphaTypes) {
            SkColorTypeValidateAlphaType(colorType, kUnknown_SkAlphaType, &canonicalAlphaType );
            SkDebugf("%12s %9s  ", color_type_to_string(colorType),
                    alpha_type_to_string(canonicalAlphaType));
            for (SkAlphaType alphaType : alphaTypes) {
                bitmap.setInfo(SkImageInfo::Make(4, 4, colorType, canonicalAlphaType));
                bool result = bitmap.setAlphaType(alphaType);
                SkDebugf("%s %s    ", result ? "true " : "false",
                        alpha_type_to_string(bitmap.alphaType()));
            }
            SkDebugf("\n");
        }
    }
}
}  // END FIDDLE
