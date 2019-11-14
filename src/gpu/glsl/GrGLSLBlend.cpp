/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

void GrGLSLBlend::AppendMode(GrGLSLFragmentBuilder* fsBuilder, const char* srcColor,
                             const char* dstColor, const char* outColor,
                             SkBlendMode mode) {
    auto modeStr = [](SkBlendMode mode) {
        switch (mode) {
            case SkBlendMode::kClear: return "SkBlendMode::kClear";
            case SkBlendMode::kSrc: return "SkBlendMode::kSrc";
            case SkBlendMode::kDst: return "SkBlendMode::kDst";
            case SkBlendMode::kSrcOver: return "SkBlendMode::kSrcOver";
            case SkBlendMode::kDstOver: return "SkBlendMode::kDstOver";
            case SkBlendMode::kSrcIn: return "SkBlendMode::kSrcIn";
            case SkBlendMode::kDstIn: return "SkBlendMode::kDstIn";
            case SkBlendMode::kSrcOut: return "SkBlendMode::kSrcOut";
            case SkBlendMode::kDstOut: return "SkBlendMode::kDstOut";
            case SkBlendMode::kSrcATop: return "SkBlendMode::kSrcATop";
            case SkBlendMode::kDstATop: return "SkBlendMode::kDstATop";
            case SkBlendMode::kXor: return "SkBlendMode::kXor";
            case SkBlendMode::kPlus: return "SkBlendMode::kPlus";
            case SkBlendMode::kModulate: return "SkBlendMode::kModulate";
            case SkBlendMode::kScreen: return "SkBlendMode::kScreen";
            case SkBlendMode::kOverlay: return "SkBlendMode::kOverlay";
            case SkBlendMode::kDarken: return "SkBlendMode::kDarken";
            case SkBlendMode::kLighten: return "SkBlendMode::kLighten";
            case SkBlendMode::kColorDodge: return "SkBlendMode::kColorDodge";
            case SkBlendMode::kColorBurn: return "SkBlendMode::kColorBurn";
            case SkBlendMode::kHardLight: return "SkBlendMode::kHardLight";
            case SkBlendMode::kSoftLight: return "SkBlendMode::kSoftLight";
            case SkBlendMode::kDifference: return "SkBlendMode::kDifference";
            case SkBlendMode::kExclusion: return "SkBlendMode::kExclusion";
            case SkBlendMode::kMultiply: return "SkBlendMode::kMultiply";
            case SkBlendMode::kHue: return "SkBlendMode::kHue";
            case SkBlendMode::kSaturation: return "SkBlendMode::kSaturation";
            case SkBlendMode::kColor: return "SkBlendMode::kColor";
            case SkBlendMode::kLuminosity: return "SkBlendMode::kLuminosity";
        }
    };
    fsBuilder->codeAppendf("%s = blend(%s, %s, %s);", outColor, modeStr(mode), srcColor, dstColor);
}
