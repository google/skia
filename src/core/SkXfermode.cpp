/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"

#if defined(SK_GANESH)
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrCustomXfermode.h"
#include "src/gpu/ganesh/effects/GrPorterDuffXferProcessor.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

const char* SkBlendMode_Name(SkBlendMode bm) {
    switch (bm) {
        case SkBlendMode::kClear:      return "Clear";
        case SkBlendMode::kSrc:        return "Src";
        case SkBlendMode::kDst:        return "Dst";
        case SkBlendMode::kSrcOver:    return "SrcOver";
        case SkBlendMode::kDstOver:    return "DstOver";
        case SkBlendMode::kSrcIn:      return "SrcIn";
        case SkBlendMode::kDstIn:      return "DstIn";
        case SkBlendMode::kSrcOut:     return "SrcOut";
        case SkBlendMode::kDstOut:     return "DstOut";
        case SkBlendMode::kSrcATop:    return "SrcATop";
        case SkBlendMode::kDstATop:    return "DstATop";
        case SkBlendMode::kXor:        return "Xor";
        case SkBlendMode::kPlus:       return "Plus";
        case SkBlendMode::kModulate:   return "Modulate";
        case SkBlendMode::kScreen:     return "Screen";

        case SkBlendMode::kOverlay:    return "Overlay";
        case SkBlendMode::kDarken:     return "Darken";
        case SkBlendMode::kLighten:    return "Lighten";
        case SkBlendMode::kColorDodge: return "ColorDodge";
        case SkBlendMode::kColorBurn:  return "ColorBurn";
        case SkBlendMode::kHardLight:  return "HardLight";
        case SkBlendMode::kSoftLight:  return "SoftLight";
        case SkBlendMode::kDifference: return "Difference";
        case SkBlendMode::kExclusion:  return "Exclusion";
        case SkBlendMode::kMultiply:   return "Multiply";

        case SkBlendMode::kHue:        return "Hue";
        case SkBlendMode::kSaturation: return "Saturation";
        case SkBlendMode::kColor:      return "Color";
        case SkBlendMode::kLuminosity: return "Luminosity";
    }
    SkUNREACHABLE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(SK_GANESH)
const GrXPFactory* SkBlendMode_AsXPFactory(SkBlendMode mode) {
    if (SkBlendMode_AsCoeff(mode, nullptr, nullptr)) {
        const GrXPFactory* result = GrPorterDuffXPFactory::Get(mode);
        SkASSERT(result);
        return result;
    }

    SkASSERT(GrCustomXfermode::IsSupportedMode(mode));
    return GrCustomXfermode::Get(mode);
}
#endif
