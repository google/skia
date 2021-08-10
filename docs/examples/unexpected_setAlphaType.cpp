// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(unexpected_setAlphaType, 256, 256, true, 0) {
static const char* alphatype_name(SkAlphaType at) {
    switch (at) {
        case kUnknown_SkAlphaType:  return "Unknown";
        case kOpaque_SkAlphaType:   return "Opaque";
        case kPremul_SkAlphaType:   return "Premul";
        case kUnpremul_SkAlphaType: return "Unpremul";
    }
    SkASSERT(false);
    return "unexpected alphatype";
}
static const char* colortype_name(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:            return "Unknown";
        case kAlpha_8_SkColorType:            return "Alpha_8";
        case kA16_unorm_SkColorType:          return "Alpha_16";
        case kA16_float_SkColorType:          return "A16_float";
        case kRGB_565_SkColorType:            return "RGB_565";
        case kARGB_4444_SkColorType:          return "ARGB_4444";
        case kRGBA_8888_SkColorType:          return "RGBA_8888";
        case kSRGBA_8888_SkColorType:         return "SRGBA_8888";
        case kRGB_888x_SkColorType:           return "RGB_888x";
        case kBGRA_8888_SkColorType:          return "BGRA_8888";
        case kRGBA_1010102_SkColorType:       return "RGBA_1010102";
        case kRGB_101010x_SkColorType:        return "RGB_101010x";
        case kBGRA_1010102_SkColorType:       return "BGRA_1010102";
        case kBGR_101010x_SkColorType:        return "BGR_101010x";
        case kGray_8_SkColorType:             return "Gray_8";
        case kRGBA_F16Norm_SkColorType:       return "RGBA_F16Norm";
        case kRGBA_F16_SkColorType:           return "RGBA_F16";
        case kRGBA_F32_SkColorType:           return "RGBA_F32";
        case kR8G8_unorm_SkColorType:         return "R8G8_unorm";
        case kR16G16_unorm_SkColorType:       return "R16G16_unorm";
        case kR16G16_float_SkColorType:       return "R16G16_float";
        case kR16G16B16A16_unorm_SkColorType: return "R16G16B16A16_unorm";
    }
    SkASSERT(false);
    return "unexpected colortype";
}
void draw(SkCanvas* canvas) {
    static const SkAlphaType kAlphaTypes[] =
        {kUnknown_SkAlphaType, kOpaque_SkAlphaType, kPremul_SkAlphaType, kUnpremul_SkAlphaType};
    static const SkColorType kColorTypes[] =
         {kUnknown_SkColorType, kAlpha_8_SkColorType, kRGB_565_SkColorType,
          kARGB_4444_SkColorType, kRGBA_8888_SkColorType, kRGB_888x_SkColorType,
          kBGRA_8888_SkColorType, kRGBA_1010102_SkColorType, kRGB_101010x_SkColorType,
          kGray_8_SkColorType, kRGBA_F16_SkColorType};
    SkBitmap bitmap;
    SkDebugf("%16s Canonical    Unknown           Opaque            Premul            "
             "Unpremul\n", " ");
    for (SkColorType colorType : kColorTypes) {
        for (SkAlphaType canonicalAlphaType : kAlphaTypes) {
            SkColorTypeValidateAlphaType(colorType, kUnknown_SkAlphaType, &canonicalAlphaType);
            SkDebugf("%15s %10s ", colortype_name(colorType), alphatype_name(canonicalAlphaType));
            for (SkAlphaType alphaType : kAlphaTypes) {
                bitmap.setInfo(SkImageInfo::Make(4, 4, colorType, canonicalAlphaType));
                bool result = bitmap.setAlphaType(alphaType);
                SkDebugf("%s %s    ", result ? "true " : "false",
                         alphatype_name(bitmap.alphaType()));
            }
            SkDebugf("\n");
        }
    }
}
}  // END FIDDLE
