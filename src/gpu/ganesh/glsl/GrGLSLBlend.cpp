/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/glsl/GrGLSLBlend.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/Blend.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"
#include "src/sksl/SkSLString.h"

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
    // This switch must be kept in sync with GetReducedBlendModeInfo() in src/gpu/Blend.cpp.
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
            return -1;  // blend_porter_duff

        case SkBlendMode::kHue:
        case SkBlendMode::kSaturation:
        case SkBlendMode::kLuminosity:
        case SkBlendMode::kColor:
            return -2;  // blend_hslc

        case SkBlendMode::kOverlay:
        case SkBlendMode::kHardLight:
            return -3;  // blend_overlay

        case SkBlendMode::kDarken:
        case SkBlendMode::kLighten:
            return -4;  // blend_darken

        default:
            return (int)mode;  // uses a dedicated SkSL blend function
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
