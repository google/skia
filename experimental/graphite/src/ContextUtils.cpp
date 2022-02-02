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
#include "experimental/graphite/src/UniformManager.h"
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

// TODO: For the sprint we unify all the gradient uniforms into a standard set of 6:
//   kMaxStops colors
//   kMaxStops offsets
//   2 points
//   2 radii
static constexpr int kNumGradientUniforms = 6;
static constexpr SkUniform kGradientUniforms[kNumGradientUniforms] {
        {"colors",   SkSLType::kHalf4 , GradientData::kMaxStops },
        {"offsets",  SkSLType::kFloat, GradientData::kMaxStops },
        {"point0",   SkSLType::kFloat2 },
        {"point1",   SkSLType::kFloat2 },
        {"radius0",  SkSLType::kFloat },
        {"radius1",  SkSLType::kFloat },
};

static constexpr int kNumSolidUniforms = 1;
static constexpr SkUniform kSolidUniforms[kNumSolidUniforms] {
        {"color",  SkSLType::kFloat4 }
};

static const char* kGradientSkSL =
        // TODO: This should use local coords
        "float2 pos = sk_FragCoord.xy;\n"
        "float2 delta = point1 - point0;\n"
        "float2 pt = pos - point0;\n"
        "float t = dot(pt, delta) / dot(delta, delta);\n"
        "float4 result = colors[0];\n"
        "result = mix(result, colors[1],\n"
        "             clamp((t-offsets[0])/(offsets[1]-offsets[0]),\n"
        "                   0, 1));\n"
        "result = mix(result, colors[2],\n"
        "             clamp((t-offsets[1])/(offsets[2]-offsets[1]),\n"
        "                   0, 1));\n"
        "result = mix(result, colors[3],\n"
        "             clamp((t-offsets[2])/(offsets[3]-offsets[2]),\n"
        "             0, 1));\n"
        "outColor = half4(result);\n";

static const char* kSolidColorSkSL = "    outColor = half4(color);\n";

// TODO: kNone is for depth-only draws, so should actually have a fragment output type
// that only defines a [[depth]] attribute but no color calculation.
static const char* kNoneSkSL = "outColor = half4(0.0, 0.0, 1.0, 1.0);\n";

sk_sp<SkUniformData> make_gradient_uniform_data_common(SkSpan<const SkUniform> uniforms,
                                                       const void* srcs[kNumGradientUniforms]) {
    UniformManager mgr(Layout::kMetal);

    // TODO: Given that, for the sprint, we always know the uniforms we could cache 'dataSize'
    // for each layout and skip the first call.
    size_t dataSize = mgr.writeUniforms(uniforms, nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(uniforms, dataSize);

    mgr.writeUniforms(result->uniforms(), srcs, result->offsets(), result->data());
    return result;
}

sk_sp<SkUniformData> make_linear_gradient_uniform_data(SkPoint startPoint,
                                                       SkPoint endPoint,
                                                       SkColor4f colors[GradientData::kMaxStops],
                                                       float offsets[GradientData::kMaxStops]) {
    static constexpr size_t kExpectedNumUniforms = 6;

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kLinearGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    float unusedRadii[2] = { 0.0f, 0.0f };
    const void* srcs[kExpectedNumUniforms] = {
            colors,
            offsets,
            &startPoint,
            &endPoint,
            &unusedRadii[0],
            &unusedRadii[1],
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_radial_gradient_uniform_data(SkPoint point,
                                                       float radius,
                                                       SkColor4f colors[GradientData::kMaxStops],
                                                       float offsets[GradientData::kMaxStops]) {
    static constexpr size_t kExpectedNumUniforms = 6;

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kRadialGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    SkPoint unusedPoint = {0.0f, 0.0f};
    float unusedRadius = 0.0f;

    const void* srcs[kExpectedNumUniforms] = {
            colors,
            offsets,
            &point,
            &unusedPoint,
            &radius,
            &unusedRadius,
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_sweep_gradient_uniform_data(SkPoint point,
                                                      SkColor4f colors[GradientData::kMaxStops],
                                                      float offsets[GradientData::kMaxStops]) {
    static constexpr size_t kExpectedNumUniforms = 6;

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kSweepGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    SkPoint unusedPoint = {0.0f, 0.0f};
    float unusedRadii[2] = {0.0f, 0.0f};

    const void* srcs[kExpectedNumUniforms] = {
            colors,
            offsets,
            &point,
            &unusedPoint,
            &unusedRadii[0],
            &unusedRadii[1],
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_conical_gradient_uniform_data(SkPoint point0,
                                                        SkPoint point1,
                                                        float radius0,
                                                        float radius1,
                                                        SkColor4f colors[GradientData::kMaxStops],
                                                        float offsets[GradientData::kMaxStops]) {
    static constexpr size_t kExpectedNumUniforms = 6;

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kConicalGradientShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    const void* srcs[kExpectedNumUniforms] = {
            colors,
            offsets,
            &point0,
            &point1,
            &radius0,
            &radius1,
    };

    return make_gradient_uniform_data_common(uniforms, srcs);
};

sk_sp<SkUniformData> make_solid_uniform_data(SkColor4f color) {
    static constexpr size_t kExpectedNumUniforms = 1;

    SkSpan<const SkUniform> uniforms = skgpu::GetUniforms(CodeSnippetID::kSolidColorShader);
    SkASSERT(uniforms.size() == kExpectedNumUniforms);

    UniformManager mgr(Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(uniforms, nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(uniforms, dataSize);

    const void* srcs[kExpectedNumUniforms] = { &color };

    mgr.writeUniforms(result->uniforms(), srcs, result->offsets(), result->data());
    return result;
}

} // anonymous namespace

std::tuple<SkUniquePaintParamsID, std::unique_ptr<SkUniformBlock>> ExtractPaintData(
        SkShaderCodeDictionary* dictionary, const PaintParams& p) {
    SkPaintParamsKey key;
    sk_sp<SkUniformData> uniforms;

    std::unique_ptr<SkUniformBlock> block = std::make_unique<SkUniformBlock>();

    // TODO: add UniformData generation to PaintParams::toKey and use it here
    if (auto s = p.shader()) {
        SkColor colors[GradientData::kMaxStops];
        float offsets[GradientData::kMaxStops];
        SkShader::GradientInfo gradInfo;

        gradInfo.fColorCount = GradientData::kMaxStops;
        gradInfo.fColors = colors;
        gradInfo.fColorOffsets = offsets;

        SkShader::GradientType type = s->asAGradient(&gradInfo);
        if (gradInfo.fColorCount > GradientData::kMaxStops) {
            type = SkShader::GradientType::kNone_GradientType;
        }

        GradientData data(type, gradInfo.fPoint, gradInfo.fRadius,
                          gradInfo.fTileMode, gradInfo.fColorCount,
                          colors, offsets);

        switch (type) {
            case SkShader::kLinear_GradientType: {
                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               block.get(),
                                               data);

                // TODO: move this into GradientShaderBlocks::AddToKey
                uniforms = make_linear_gradient_uniform_data(data.fPoints[0],
                                                             data.fPoints[1],
                                                             data.fColor4fs,
                                                             data.fOffsets);
            } break;
            case SkShader::kRadial_GradientType: {
                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               block.get(),
                                               data);

                // TODO: move this into GradientShaderBlocks::AddToKey
                uniforms = make_radial_gradient_uniform_data(data.fPoints[0],
                                                             data.fRadii[0],
                                                             data.fColor4fs,
                                                             data.fOffsets);
            } break;
            case SkShader::kSweep_GradientType:
                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               block.get(),
                                               data);

                // TODO: move this into GradientShaderBlocks::AddToKey
                uniforms = make_sweep_gradient_uniform_data(data.fPoints[0],
                                                            data.fColor4fs,
                                                            data.fOffsets);
                break;
            case SkShader::GradientType::kConical_GradientType:
                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               block.get(),
                                               data);

                // TODO: move this into GradientShaderBlocks::AddToKey
                uniforms = make_conical_gradient_uniform_data(data.fPoints[0],
                                                              data.fPoints[1],
                                                              data.fRadii[0],
                                                              data.fRadii[1],
                                                              data.fColor4fs,
                                                              data.fOffsets);
                break;
            case SkShader::GradientType::kColor_GradientType: [[fallthrough]];
                // TODO: The solid color gradient type should use its color, not
                // the paint color
            case SkShader::GradientType::kNone_GradientType:  [[fallthrough]];
            default:
                SolidColorShaderBlock::AddToKey(SkBackend::kGraphite, &key, block.get(), p.color());

                // TODO: move this into SolidColorShaderBlock::AddToKey
                uniforms = make_solid_uniform_data(p.color());
                break;
        }
    } else {
        // Solid colored paint
        SolidColorShaderBlock::AddToKey(SkBackend::kGraphite, &key, block.get(), p.color());

        // TODO: move this into SolidColorShaderBlock::AddToKey
        uniforms = make_solid_uniform_data(p.color());
    }

    if (p.blender()) {
        as_BB(p.blender())->addToKey(dictionary, SkBackend::kGraphite, &key, block.get());
    } else {
        BlendModeBlock::AddToKey(SkBackend::kGraphite, &key, block.get(), SkBlendMode::kSrcOver);
    }

    auto entry = dictionary->findOrCreate(key);

    block->add(std::move(uniforms));

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
        case CodeSnippetID::kSolidColorShader:
        default:
            return SkMakeSpan(kSolidUniforms, kNumSolidUniforms);
    }
}

const char* GetShaderSkSL(CodeSnippetID snippetID) {
    switch (snippetID) {
        case CodeSnippetID::kDepthStencilOnlyDraw:
            return kNoneSkSL;
        case CodeSnippetID::kLinearGradientShader: [[fallthrough]];
        case CodeSnippetID::kRadialGradientShader: [[fallthrough]];
        case CodeSnippetID::kSweepGradientShader:  [[fallthrough]];
        case CodeSnippetID::kConicalGradientShader:
            return kGradientSkSL;
        case CodeSnippetID::kSolidColorShader:
        default:
            return kSolidColorSkSL;
    }
}

} // namespace skgpu
