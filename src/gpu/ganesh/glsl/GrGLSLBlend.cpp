/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLString.h"
#include "src/gpu/Blend.h"
#include "src/gpu/ganesh/glsl/GrGLSLBlend.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"

namespace GrGLSLBlend {

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
            return SkSL::String::printf("blend_hslc(%s, %s, bool(%s.x), bool(%s.y))",
                                        srcColor, dstColor, blendName, blendName);
        }
        case SkBlendMode::kOverlay:
        case SkBlendMode::kHardLight: {
            const char* blendName;
            *blendUniform = uniformHandler->addUniform(processor, kFragment_GrShaderFlag,
                                                       SkSLType::kHalf, "blend", &blendName);
            return SkSL::String::printf("blend_overlay(%s, %s, bool(%s))",
                                        srcColor, dstColor, blendName);
        }
        case SkBlendMode::kDarken:
        case SkBlendMode::kLighten: {
            const char* blendName;
            *blendUniform = uniformHandler->addUniform(processor, kFragment_GrShaderFlag,
                                                       SkSLType::kHalf, "blend", &blendName);
            return SkSL::String::printf("blend_darken(%s, %s, %s)", srcColor, dstColor, blendName);
        }
        default: {
            return SkSL::String::printf("%s(%s, %s)",
                                        skgpu::BlendFuncName(mode), srcColor, dstColor);
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

        case SkBlendMode::kOverlay:
        case SkBlendMode::kHardLight:
            return -3;

        case SkBlendMode::kDarken:
        case SkBlendMode::kLighten:
            return -4;

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

        case SkBlendMode::kOverlay:    pdman.set1f(blendUniform, 0); break;
        case SkBlendMode::kHardLight:  pdman.set1f(blendUniform, 1); break;

        case SkBlendMode::kDarken:     pdman.set1f(blendUniform, 1); break;
        case SkBlendMode::kLighten:    pdman.set1f(blendUniform, -1); break;

        default:                    /* no uniform data necessary */ break;
    }
}

}  // namespace GrGLSLBlend
