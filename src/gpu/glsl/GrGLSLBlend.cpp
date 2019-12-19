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
    // When and if the SkSL compiler supports inlining we could replace this with
    // out = blend(mode, src, dst) where mode is a literal.
    const char* name;
    switch (mode) {
        case SkBlendMode::kClear:      name = "clear";       break;
        case SkBlendMode::kSrc:        name = "src";         break;
        case SkBlendMode::kDst:        name = "dst";         break;
        case SkBlendMode::kSrcOver:    name = "src_over";    break;
        case SkBlendMode::kDstOver:    name = "dst_over";    break;
        case SkBlendMode::kSrcIn:      name = "src_in";      break;
        case SkBlendMode::kDstIn:      name = "dst_in";      break;
        case SkBlendMode::kSrcOut:     name = "src_out";     break;
        case SkBlendMode::kDstOut:     name = "dst_out";     break;
        case SkBlendMode::kSrcATop:    name = "src_atop";    break;
        case SkBlendMode::kDstATop:    name = "dst_atop";    break;
        case SkBlendMode::kXor:        name = "xor";         break;
        case SkBlendMode::kPlus:       name = "plus";        break;
        case SkBlendMode::kModulate:   name = "modulate";    break;
        case SkBlendMode::kScreen:     name = "screen";      break;
        case SkBlendMode::kOverlay:    name = "overlay";     break;
        case SkBlendMode::kDarken:     name = "darken";      break;
        case SkBlendMode::kLighten:    name = "lighten";     break;
        case SkBlendMode::kColorDodge: name = "color_dodge"; break;
        case SkBlendMode::kColorBurn:  name = "color_burn";  break;
        case SkBlendMode::kHardLight:  name = "hard_light";  break;
        case SkBlendMode::kSoftLight:  name = "soft_light";  break;
        case SkBlendMode::kDifference: name = "difference";  break;
        case SkBlendMode::kExclusion:  name = "exclusion";   break;
        case SkBlendMode::kMultiply:   name = "multiply";    break;
        case SkBlendMode::kHue:        name = "hue";         break;
        case SkBlendMode::kSaturation: name = "saturation";  break;
        case SkBlendMode::kColor:      name = "color";       break;
        case SkBlendMode::kLuminosity: name = "luminosity";  break;
    }
    fsBuilder->codeAppendf("%s = blend_%s(%s, %s);", outColor, name, srcColor, dstColor);
}
