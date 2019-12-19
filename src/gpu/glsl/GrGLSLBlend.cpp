/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

namespace GrGLSLBlend {

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

void AppendMode(GrGLSLFragmentBuilder* fsBuilder,
                const char* srcColor,
                const char* dstColor,
                const char* outColor,
                SkBlendMode mode) {
    fsBuilder->codeAppendf("%s = %s(%s, %s);", outColor, BlendFuncName(mode), srcColor, dstColor);
}

}  // namespace GrGLSLBlend
