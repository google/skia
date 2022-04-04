/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLString.h"
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

std::string BlendExpression(const GrProcessor* processor,
                            GrGLSLUniformHandler* uniformHandler,
                            GrGLSLProgramDataManager::UniformHandle* blendUniform,
                            const char* srcColor,
                            const char* dstColor,
                            SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kSrcOver:
        case SkBlendMode::kDstOver:
        case SkBlendMode::kSrcIn:
        case SkBlendMode::kDstIn:
        case SkBlendMode::kSrcOut:
        case SkBlendMode::kDstOut:
        case SkBlendMode::kSrcATop:
        case SkBlendMode::kDstATop:
        case SkBlendMode::kXor:
        case SkBlendMode::kPlus: {
            const char* blendName;
            *blendUniform = uniformHandler->addUniform(processor, kFragment_GrShaderFlag,
                                                       SkSLType::kHalf4, "blend", &blendName);
            return SkSL::String::printf("blend_porter_duff(%s, %s, %s)",
                                        srcColor, dstColor, blendName);
        }
        case SkBlendMode::kHue:
        case SkBlendMode::kSaturation:
        case SkBlendMode::kLuminosity:
        case SkBlendMode::kColor: {
            const char* blendName;
            *blendUniform = uniformHandler->addUniform(processor, kFragment_GrShaderFlag,
                                                       SkSLType::kHalf2, "blend", &blendName);
            return SkSL::String::printf("blend_hslc(%s, %s, %s.x, bool(%s.y))",
                                        srcColor, dstColor, blendName, blendName);
        }
        default: {
            return SkSL::String::printf("%s(%s, %s)",
                                        BlendFuncName(mode), srcColor, dstColor);
        }
    }
}

int BlendKey(SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kSrcOver:
        case SkBlendMode::kDstOver:
        case SkBlendMode::kSrcIn:
        case SkBlendMode::kDstIn:
        case SkBlendMode::kSrcOut:
        case SkBlendMode::kDstOut:
        case SkBlendMode::kSrcATop:
        case SkBlendMode::kDstATop:
        case SkBlendMode::kXor:
        case SkBlendMode::kPlus:
            return -1;

        case SkBlendMode::kHue:
        case SkBlendMode::kSaturation:
        case SkBlendMode::kLuminosity:
        case SkBlendMode::kColor:
            return -2;

        default:
            return (int)mode;
    }
}

void SetBlendModeUniformData(const GrGLSLProgramDataManager& pdman,
                             GrGLSLProgramDataManager::UniformHandle blendUniform,
                             SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kSrcOver:    pdman.set4f(blendUniform, 1, 0,  0, -1); break;
        case SkBlendMode::kDstOver:    pdman.set4f(blendUniform, 0, 1, -1,  0); break;
        case SkBlendMode::kSrcIn:      pdman.set4f(blendUniform, 0, 0,  1,  0); break;
        case SkBlendMode::kDstIn:      pdman.set4f(blendUniform, 0, 0,  0,  1); break;
        case SkBlendMode::kSrcOut:     pdman.set4f(blendUniform, 0, 0, -1,  0); break;
        case SkBlendMode::kDstOut:     pdman.set4f(blendUniform, 0, 0,  0, -1); break;
        case SkBlendMode::kSrcATop:    pdman.set4f(blendUniform, 0, 0,  1, -1); break;
        case SkBlendMode::kDstATop:    pdman.set4f(blendUniform, 0, 0, -1,  1); break;
        case SkBlendMode::kXor:        pdman.set4f(blendUniform, 0, 0, -1, -1); break;
        case SkBlendMode::kPlus:       pdman.set4f(blendUniform, 1, 1,  0,  0); break;

        case SkBlendMode::kHue:        pdman.set2f(blendUniform, 0, 1); break;
        case SkBlendMode::kSaturation: pdman.set2f(blendUniform, 1, 1); break;
        case SkBlendMode::kColor:      pdman.set2f(blendUniform, 0, 0); break;
        case SkBlendMode::kLuminosity: pdman.set2f(blendUniform, 1, 0); break;

        default:                    /* no uniform data necessary */ break;
    }
}

}  // namespace GrGLSLBlend
