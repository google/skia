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
    auto info = skgpu::GetReducedBlendModeInfo(mode);
    if (info.fUniformData.empty()) {
        return SkSL::String::printf("%s(%s, %s)", info.fFunction, srcColor, dstColor);
    }

    SkSLType skslType = (SkSLType)((int)SkSLType::kHalf + info.fUniformData.size() - 1);
    SkASSERT(skslType >= SkSLType::kHalf && skslType <= SkSLType::kHalf4);

    const char* blendUniName;
    *blendUniform = uniformHandler->addUniform(processor, kFragment_GrShaderFlag,
                                               skslType, "blend", &blendUniName);
    return SkSL::String::printf("%s(%s, %s, %s)", info.fFunction, blendUniName, srcColor, dstColor);
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
    auto info = skgpu::GetReducedBlendModeInfo(mode);
    switch (info.fUniformData.size()) {
        case 0:
            /* no uniform data necessary */
            break;
        case 1:
            pdman.set1f(blendUniform, info.fUniformData[0]);
            break;
        case 2:
            pdman.set2f(blendUniform, info.fUniformData[0], info.fUniformData[1]);
            break;
        case 3:
            pdman.set3f(blendUniform, info.fUniformData[0], info.fUniformData[1],
                                      info.fUniformData[2]);
            break;
        case 4:
            pdman.set4f(blendUniform, info.fUniformData[0], info.fUniformData[1],
                                      info.fUniformData[2], info.fUniformData[3]);
            break;
    }
}

}  // namespace GrGLSLBlend
