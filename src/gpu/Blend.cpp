/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/Blend.h"

#include "include/core/SkBlendMode.h"

#ifdef SK_DEBUG
#include "include/core/SkString.h"
#endif

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

SkSpan<const float> GetPorterDuffBlendConstants(SkBlendMode mode) {
    // See sksl_gpu.sksl's blend_porter_duff function for explanation of values
    static constexpr float kClear[]      = {0,  0,  0,  0};
    static constexpr float kSrc[]        = {1,  0,  0,  0};
    static constexpr float kDst[]        = {0,  1,  0,  0};
    static constexpr float kSrcOver[]    = {1,  1,  0, -1};
    static constexpr float kDstOver[]    = {1,  1, -1,  0};
    static constexpr float kSrcIn[]      = {0,  0,  1,  0};
    static constexpr float kDstIn[]      = {0,  0,  0,  1};
    static constexpr float kSrcOut[]     = {1,  0, -1,  0};
    static constexpr float kDstOut[]     = {0,  1,  0, -1};
    static constexpr float kSrcATop[]    = {0,  1,  1, -1};
    static constexpr float kDstATop[]    = {1,  0, -1,  1};
    static constexpr float kXor[]        = {1,  1, -1, -1};

    switch (mode) {
        case SkBlendMode::kClear:      return SkSpan(kClear);
        case SkBlendMode::kSrc:        return SkSpan(kSrc);
        case SkBlendMode::kDst:        return SkSpan(kDst);
        case SkBlendMode::kSrcOver:    return SkSpan(kSrcOver);
        case SkBlendMode::kDstOver:    return SkSpan(kDstOver);
        case SkBlendMode::kSrcIn:      return SkSpan(kSrcIn);
        case SkBlendMode::kDstIn:      return SkSpan(kDstIn);
        case SkBlendMode::kSrcOut:     return SkSpan(kSrcOut);
        case SkBlendMode::kDstOut:     return SkSpan(kDstOut);
        case SkBlendMode::kSrcATop:    return SkSpan(kSrcATop);
        case SkBlendMode::kDstATop:    return SkSpan(kDstATop);
        case SkBlendMode::kXor:        return SkSpan(kXor);
        default:                       return {};
    }
}

ReducedBlendModeInfo GetReducedBlendModeInfo(SkBlendMode mode) {
    static constexpr float kHue[]        = {0, 1};
    static constexpr float kSaturation[] = {1, 1};
    static constexpr float kColor[]      = {0, 0};
    static constexpr float kLuminosity[] = {1, 0};

    static constexpr float kOverlay[]    = {0};
    static constexpr float kHardLight[]  = {1};

    static constexpr float kDarken[]     = {1};
    static constexpr float kLighten[]    = {-1};

    // This switch must be kept in sync with BlendKey() in src/ganesh/glsl/GrGLSLBlend.cpp.
    switch (mode) {
        // Clear/src/dst are intentionally omitted; using the built-in blend_xxxxx functions is
        // preferable, since that gives us an opportunity to eliminate the src/dst entirely.

        case SkBlendMode::kSrcOver:
        case SkBlendMode::kDstOver:
        case SkBlendMode::kSrcIn:
        case SkBlendMode::kDstIn:
        case SkBlendMode::kSrcOut:
        case SkBlendMode::kDstOut:
        case SkBlendMode::kSrcATop:
        case SkBlendMode::kDstATop:
        case SkBlendMode::kXor:        return {"blend_porter_duff",
                                               GetPorterDuffBlendConstants(mode)};

        case SkBlendMode::kHue:        return {"blend_hslc", SkSpan(kHue)};
        case SkBlendMode::kSaturation: return {"blend_hslc", SkSpan(kSaturation)};
        case SkBlendMode::kColor:      return {"blend_hslc", SkSpan(kColor)};
        case SkBlendMode::kLuminosity: return {"blend_hslc", SkSpan(kLuminosity)};

        case SkBlendMode::kOverlay:    return {"blend_overlay", SkSpan(kOverlay)};
        case SkBlendMode::kHardLight:  return {"blend_overlay", SkSpan(kHardLight)};

        case SkBlendMode::kDarken:     return {"blend_darken", SkSpan(kDarken)};
        case SkBlendMode::kLighten:    return {"blend_darken", SkSpan(kLighten)};

        default:                       return {BlendFuncName(mode), {}};
    }
}

#ifdef SK_DEBUG

namespace {

const char *equation_string(skgpu::BlendEquation eq) {
    switch (eq) {
        case skgpu::BlendEquation::kAdd:             return "add";
        case skgpu::BlendEquation::kSubtract:        return "subtract";
        case skgpu::BlendEquation::kReverseSubtract: return "reverse_subtract";
        case skgpu::BlendEquation::kScreen:          return "screen";
        case skgpu::BlendEquation::kOverlay:         return "overlay";
        case skgpu::BlendEquation::kDarken:          return "darken";
        case skgpu::BlendEquation::kLighten:         return "lighten";
        case skgpu::BlendEquation::kColorDodge:      return "color_dodge";
        case skgpu::BlendEquation::kColorBurn:       return "color_burn";
        case skgpu::BlendEquation::kHardLight:       return "hard_light";
        case skgpu::BlendEquation::kSoftLight:       return "soft_light";
        case skgpu::BlendEquation::kDifference:      return "difference";
        case skgpu::BlendEquation::kExclusion:       return "exclusion";
        case skgpu::BlendEquation::kMultiply:        return "multiply";
        case skgpu::BlendEquation::kHSLHue:          return "hsl_hue";
        case skgpu::BlendEquation::kHSLSaturation:   return "hsl_saturation";
        case skgpu::BlendEquation::kHSLColor:        return "hsl_color";
        case skgpu::BlendEquation::kHSLLuminosity:   return "hsl_luminosity";
        case skgpu::BlendEquation::kIllegal:
            SkASSERT(false);
            return "<illegal>";
    }

    SkUNREACHABLE;
}

const char *coeff_string(skgpu::BlendCoeff coeff) {
    switch (coeff) {
        case skgpu::BlendCoeff::kZero:    return "zero";
        case skgpu::BlendCoeff::kOne:     return "one";
        case skgpu::BlendCoeff::kSC:      return "src_color";
        case skgpu::BlendCoeff::kISC:     return "inv_src_color";
        case skgpu::BlendCoeff::kDC:      return "dst_color";
        case skgpu::BlendCoeff::kIDC:     return "inv_dst_color";
        case skgpu::BlendCoeff::kSA:      return "src_alpha";
        case skgpu::BlendCoeff::kISA:     return "inv_src_alpha";
        case skgpu::BlendCoeff::kDA:      return "dst_alpha";
        case skgpu::BlendCoeff::kIDA:     return "inv_dst_alpha";
        case skgpu::BlendCoeff::kConstC:  return "const_color";
        case skgpu::BlendCoeff::kIConstC: return "inv_const_color";
        case skgpu::BlendCoeff::kS2C:     return "src2_color";
        case skgpu::BlendCoeff::kIS2C:    return "inv_src2_color";
        case skgpu::BlendCoeff::kS2A:     return "src2_alpha";
        case skgpu::BlendCoeff::kIS2A:    return "inv_src2_alpha";
        case skgpu::BlendCoeff::kIllegal:
            SkASSERT(false);
            return "<illegal>";
    }

    SkUNREACHABLE;
}

} // anonymous namespace

SkString BlendInfo::dump() const {
    SkString out;
    out.printf("writes_color(%d) equation(%s) src_coeff(%s) dst_coeff:(%s) const(0x%08x)",
               fWritesColor, equation_string(fEquation), coeff_string(fSrcBlend),
               coeff_string(fDstBlend), fBlendConstant.toBytes_RGBA());
    return out;
}

#endif // SK_DEBUG

}  // namespace skgpu
