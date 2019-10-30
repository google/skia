/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CodecBenchPriv_DEFINED
#define CodecBenchPriv_DEFINED

#include "include/core/SkImageInfo.h"

inline const char* color_type_to_str(SkColorType colorType) {
    switch (colorType) {
        case kN32_SkColorType:
            return "N32";
        case kRGB_565_SkColorType:
            return "565";
        case kGray_8_SkColorType:
            return "Gray8";
        case kAlpha_8_SkColorType:
            return "Alpha8";
        default:
            return "Unknown";
    }
}

inline const char* alpha_type_to_str(SkAlphaType alphaType) {
    switch (alphaType) {
        case kOpaque_SkAlphaType:
            return "";
        case kPremul_SkAlphaType:
            return "Premul";
        case kUnpremul_SkAlphaType:
            return "Unpremul";
        default:
            SkASSERT(false);
            return "Unknown";
    }
}

#endif // CodecBenchPriv_DEFINED
