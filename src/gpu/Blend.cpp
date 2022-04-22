/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/Blend.h"

#include "include/core/SkBlendMode.h"

namespace skgpu {

const char* BlendFuncName(SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kClear:      return "blend_clear";
        case SkBlendMode::kSrc:        return "blend_src";
        case SkBlendMode::kDst:        return "blend_dst";
        case SkBlendMode::kSrcOver:    return "blend_src_over";
        case SkBlendMode::kDstOver:    return "blend_dst_over";
        case SkBlendMode::kSrcIn:      return "blend_src_in";
        case SkBlendMode::kDstIn:      return "blend_dst_in";
        case SkBlendMode::kSrcOut:     return "blend_src_out";
        case SkBlendMode::kDstOut:     return "blend_dst_out";
        case SkBlendMode::kSrcATop:    return "blend_src_atop";
        case SkBlendMode::kDstATop:    return "blend_dst_atop";
        case SkBlendMode::kXor:        return "blend_xor";
        case SkBlendMode::kPlus:       return "blend_plus";
        case SkBlendMode::kModulate:   return "blend_modulate";
        case SkBlendMode::kScreen:     return "blend_screen";
        case SkBlendMode::kOverlay:    return "blend_overlay";
        case SkBlendMode::kDarken:     return "blend_darken";
        case SkBlendMode::kLighten:    return "blend_lighten";
        case SkBlendMode::kColorDodge: return "blend_color_dodge";
        case SkBlendMode::kColorBurn:  return "blend_color_burn";
        case SkBlendMode::kHardLight:  return "blend_hard_light";
        case SkBlendMode::kSoftLight:  return "blend_soft_light";
        case SkBlendMode::kDifference: return "blend_difference";
        case SkBlendMode::kExclusion:  return "blend_exclusion";
        case SkBlendMode::kMultiply:   return "blend_multiply";
        case SkBlendMode::kHue:        return "blend_hue";
        case SkBlendMode::kSaturation: return "blend_saturation";
        case SkBlendMode::kColor:      return "blend_color";
        case SkBlendMode::kLuminosity: return "blend_luminosity";
    }
    SkUNREACHABLE;
}

ReducedBlendModeInfo GetReducedBlendModeInfo(SkBlendMode mode) {
    static constexpr float kSrcOver[]    = {1, 0,  0, -1};
    static constexpr float kDstOver[]    = {0, 1, -1,  0};
    static constexpr float kSrcIn[]      = {0, 0,  1,  0};
    static constexpr float kDstIn[]      = {0, 0,  0,  1};
    static constexpr float kSrcOut[]     = {0, 0, -1,  0};
    static constexpr float kDstOut[]     = {0, 0,  0, -1};
    static constexpr float kSrcATop[]    = {0, 0,  1, -1};
    static constexpr float kDstATop[]    = {0, 0, -1,  1};
    static constexpr float kXor[]        = {0, 0, -1, -1};
    static constexpr float kPlus[]       = {1, 1,  0,  0};

    static constexpr float kHue[]        = {0, 1};
    static constexpr float kSaturation[] = {1, 1};
    static constexpr float kColor[]      = {0, 0};
    static constexpr float kLuminosity[] = {1, 0};

    static constexpr float kOverlay[]    = {0};
    static constexpr float kHardLight[]  = {1};

    static constexpr float kDarken[]     = {1};
    static constexpr float kLighten[]    = {-1};

    switch (mode) {
        case SkBlendMode::kSrcOver:    return {"blend_porter_duff", SkMakeSpan(kSrcOver)};
        case SkBlendMode::kDstOver:    return {"blend_porter_duff", SkMakeSpan(kDstOver)};
        case SkBlendMode::kSrcIn:      return {"blend_porter_duff", SkMakeSpan(kSrcIn)};
        case SkBlendMode::kDstIn:      return {"blend_porter_duff", SkMakeSpan(kDstIn)};
        case SkBlendMode::kSrcOut:     return {"blend_porter_duff", SkMakeSpan(kSrcOut)};
        case SkBlendMode::kDstOut:     return {"blend_porter_duff", SkMakeSpan(kDstOut)};
        case SkBlendMode::kSrcATop:    return {"blend_porter_duff", SkMakeSpan(kSrcATop)};
        case SkBlendMode::kDstATop:    return {"blend_porter_duff", SkMakeSpan(kDstATop)};
        case SkBlendMode::kXor:        return {"blend_porter_duff", SkMakeSpan(kXor)};
        case SkBlendMode::kPlus:       return {"blend_porter_duff", SkMakeSpan(kPlus)};

        case SkBlendMode::kHue:        return {"blend_hslc", SkMakeSpan(kHue)};
        case SkBlendMode::kSaturation: return {"blend_hslc", SkMakeSpan(kSaturation)};
        case SkBlendMode::kColor:      return {"blend_hslc", SkMakeSpan(kColor)};
        case SkBlendMode::kLuminosity: return {"blend_hslc", SkMakeSpan(kLuminosity)};

        case SkBlendMode::kOverlay:    return {"blend_overlay", SkMakeSpan(kOverlay)};
        case SkBlendMode::kHardLight:  return {"blend_overlay", SkMakeSpan(kHardLight)};

        case SkBlendMode::kDarken:     return {"blend_darken", SkMakeSpan(kDarken)};
        case SkBlendMode::kLighten:    return {"blend_darken", SkMakeSpan(kLighten)};

        default:                       return {BlendFuncName(mode), {}};
    }
}

}  // namespace skgpu
