/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include <string>
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/DrawTypes.h"
#include "experimental/graphite/src/PaintParams.h"
#include "include/core/SkPaint.h"
#include "include/private/SkShaderCodeDictionary.h"
#include "include/private/SkUniquePaintParamsID.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkUniform.h"
#include "src/core/SkUniformData.h"

namespace skgpu {

using GradientData = GradientShaderBlocks::GradientData;

namespace {

//--------------------------------------------------------------------------------------------------
// TODO: For the sprint we unify all the gradient uniforms into a standard set of 6:
//   kMaxStops colors
//   kMaxStops offsets
//   2 points
//   2 radii
static constexpr int kNumGradientUniforms = 6;
static constexpr SkUniform kGradientUniforms[kNumGradientUniforms] = {
        { "colors",  SkSLType::kHalf4, GradientData::kMaxStops },
        { "offsets", SkSLType::kFloat, GradientData::kMaxStops },
        { "point0",  SkSLType::kFloat2 },
        { "point1",  SkSLType::kFloat2 },
        { "radius0", SkSLType::kFloat },
        { "radius1", SkSLType::kFloat },
};

static const char *kLinearGradient4Name = "linear_grad_4_shader";
static const char *kLinearGradient4SkSL =
        // TODO: This should use local coords
        "half4 linear_grad_4_shader() {\n"
        "    float2 pos = sk_FragCoord.xy;\n"
        "    float2 delta = point1 - point0;\n"
        "    float2 pt = pos - point0;\n"
        "    float t = dot(pt, delta) / dot(delta, delta);\n"
        "    float4 result = colors[0];\n"
        "    result = mix(result, colors[1],\n"
        "                 clamp((t-offsets[0])/(offsets[1]-offsets[0]),\n"
        "                       0, 1));\n"
        "    result = mix(result, colors[2],\n"
        "                 clamp((t-offsets[1])/(offsets[2]-offsets[1]),\n"
        "                       0, 1));\n"
        "    result = mix(result, colors[3],\n"
        "                 clamp((t-offsets[2])/(offsets[3]-offsets[2]),\n"
        "                 0, 1));\n"
        "    return half4(result);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumSolidUniforms = 1;
static constexpr SkUniform kSolidUniforms[kNumSolidUniforms] = {
        { "color", SkSLType::kFloat4 }
};

static const char* kSolidColorName = "solid_shader";
static const char* kSolidColorSkSL =
        "half4 solid_shader() {\n"
        "    return half4(color);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
static constexpr int kNumImageUniforms = 0;
static constexpr SkUniform kImageUniforms[kNumImageUniforms] = {
};

static const char* kImageName = "image_shader";
static const char* kImageSkSL =
        "half4 image_shader() {\n"
        "    float r = fract(abs(sk_FragCoord.x/10.0));\n"
        "    return half4(r, 0.0, 0.0, 1.0);\n"
        "}\n";

//--------------------------------------------------------------------------------------------------
// TODO: kNone is for depth-only draws, so should actually have a fragment output type
// that only defines a [[depth]] attribute but no color calculation.
static const char* kNoneName = "none";
static const char* kNoneSkSL = "outColor = half4(0.0, 0.0, 1.0, 1.0);\n";

} // anonymous namespace

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> ExtractPaintData(
        SkShaderCodeDictionary* dictionary,
        const PaintParams& p) {

    SkPaintParamsKey key;
    std::unique_ptr<SkUniformBlock> block = std::make_unique<SkUniformBlock>();

    p.toKey(dictionary, SkBackend::kGraphite, &key, block.get());

    auto entry = dictionary->findOrCreate(key);

    return { entry->uniqueID(), std::move(block) };
}

SkSpan<const SkUniform> GetUniforms(CodeSnippetID snippetID) {
    switch (snippetID) {
        case CodeSnippetID::kDepthStencilOnlyDraw:
            return {nullptr, 0};
        case CodeSnippetID::kLinearGradientShader: [[fallthrough]];
        case CodeSnippetID::kRadialGradientShader: [[fallthrough]];
        case CodeSnippetID::kSweepGradientShader:  [[fallthrough]];
        case CodeSnippetID::kConicalGradientShader:
            return SkMakeSpan(kGradientUniforms, kNumGradientUniforms);
        case CodeSnippetID::kImageShader:
            return SkMakeSpan(kImageUniforms, kNumImageUniforms);
        case CodeSnippetID::kSolidColorShader:     [[fallthrough]];
        default:
            return SkMakeSpan(kSolidUniforms, kNumSolidUniforms);
    }
}

// TODO: move this to the dictionary
std::tuple<const char*, const char*> GetShaderSkSL(CodeSnippetID snippetID) {
    switch (snippetID) {
        case CodeSnippetID::kDepthStencilOnlyDraw:
            return { kNoneName, kNoneSkSL};
        case CodeSnippetID::kLinearGradientShader: [[fallthrough]];
        case CodeSnippetID::kRadialGradientShader: [[fallthrough]];
        case CodeSnippetID::kSweepGradientShader:  [[fallthrough]];
        case CodeSnippetID::kConicalGradientShader:
            return { kLinearGradient4Name, kLinearGradient4SkSL };
        case CodeSnippetID::kImageShader:
            return { kImageName, kImageSkSL };
        case CodeSnippetID::kSolidColorShader:     [[fallthrough]];
        default:
            return { kSolidColorName, kSolidColorSkSL };
    }
}




} // namespace skgpu
