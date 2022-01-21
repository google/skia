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

namespace {

// TODO: For the sprint we only support 4 stops in the gradients
static constexpr int kMaxStops = 4;
// TODO: For the sprint we unify all the gradient uniforms into a standard set of 6:
//   kMaxStops colors
//   kMaxStops offsets
//   2 points
//   2 radii
static constexpr int kNumGradientUniforms = 6;
static constexpr SkUniform kGradientUniforms[kNumGradientUniforms] {
        {"colors",   SkSLType::kHalf4 , kMaxStops },
        {"offsets",  SkSLType::kFloat, kMaxStops },
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

sk_sp<SkUniformData> make_gradient_uniform_data_common(const void* srcs[kNumGradientUniforms]) {
    UniformManager mgr(Layout::kMetal);

    // TODO: Given that, for the sprint, we always know the uniforms we could cache 'dataSize'
    // for each layout and skip the first call.
    size_t dataSize = mgr.writeUniforms(SkSpan<const SkUniform>(kGradientUniforms,
                                                                kNumGradientUniforms),
                                        nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(kNumGradientUniforms,
                                                      kGradientUniforms,
                                                      dataSize);

    mgr.writeUniforms(SkSpan<const SkUniform>(kGradientUniforms, kNumGradientUniforms),
                      srcs, result->offsets(), result->data());
    return result;
}

sk_sp<SkUniformData> make_linear_gradient_uniform_data(SkPoint startPoint,
                                                       SkPoint endPoint,
                                                       SkColor4f colors[kMaxStops],
                                                       float offsets[kMaxStops]) {
    float unusedRadii[2] = { 0.0f, 0.0f };
    const void* srcs[kNumGradientUniforms] = {
            colors,
            offsets,
            &startPoint,
            &endPoint,
            &unusedRadii[0],
            &unusedRadii[1],
    };

    return make_gradient_uniform_data_common(srcs);
};

sk_sp<SkUniformData> make_radial_gradient_uniform_data(SkPoint point,
                                                       float radius,
                                                       SkColor4f colors[kMaxStops],
                                                       float offsets[kMaxStops]) {
    SkPoint unusedPoint = {0.0f, 0.0f};
    float unusedRadius = 0.0f;

    const void* srcs[kNumGradientUniforms] = {
            colors,
            offsets,
            &point,
            &unusedPoint,
            &radius,
            &unusedRadius,
    };

    return make_gradient_uniform_data_common(srcs);
};

sk_sp<SkUniformData> make_sweep_gradient_uniform_data(SkPoint point,
                                                      SkColor4f colors[kMaxStops],
                                                      float offsets[kMaxStops]) {
    SkPoint unusedPoint = {0.0f, 0.0f};
    float unusedRadii[2] = {0.0f, 0.0f};

    const void* srcs[kNumGradientUniforms] = {
            colors,
            offsets,
            &point,
            &unusedPoint,
            &unusedRadii[0],
            &unusedRadii[1],
    };

    return make_gradient_uniform_data_common(srcs);
};

sk_sp<SkUniformData> make_conical_gradient_uniform_data(SkPoint point0,
                                                        SkPoint point1,
                                                        float radius0,
                                                        float radius1,
                                                        SkColor4f colors[kMaxStops],
                                                        float offsets[kMaxStops]) {

    const void* srcs[kNumGradientUniforms] = {
            colors,
            offsets,
            &point0,
            &point1,
            &radius0,
            &radius1,
    };

    return make_gradient_uniform_data_common(srcs);
};

void to_color4fs(int numColors, SkColor colors[kMaxStops], SkColor4f color4fs[kMaxStops]) {
    SkASSERT(numColors >= 2 && numColors <= kMaxStops);

    int i;
    for (i = 0; i < numColors; ++i) {
        color4fs[i] = SkColor4f::FromColor(colors[i]);
    }
    for ( ; i < kMaxStops; ++i) {
        color4fs[i] = color4fs[numColors-1];
    }
}

void expand_stops(int numStops, float offsets[kMaxStops]) {
    SkASSERT(numStops >= 2 && numStops <= kMaxStops);

    for (int i = numStops ; i < kMaxStops; ++i) {
        offsets[i] = offsets[numStops-1];
    }
}

sk_sp<SkUniformData> make_solid_uniform_data(SkColor4f color) {
    UniformManager mgr(Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(SkSpan<const SkUniform>(kSolidUniforms, kNumSolidUniforms),
                                        nullptr, nullptr, nullptr);

    sk_sp<SkUniformData> result = SkUniformData::Make(kNumSolidUniforms, kSolidUniforms, dataSize);

    const void* srcs[kNumSolidUniforms] = { &color };

    mgr.writeUniforms(SkSpan<const SkUniform>(kSolidUniforms, kNumSolidUniforms),
                      srcs, result->offsets(), result->data());
    return result;
}

} // anonymous namespace

std::tuple<SkUniquePaintParamsID, sk_sp<SkUniformData>> ExtractPaintData(Context* context,
                                                                         const PaintParams& p) {
    SkPaintParamsKey key;
    sk_sp<SkUniformData> uniforms;

    auto dict = context->priv().shaderCodeDictionary();

    // TODO: add UniformData generation to PaintParams::toKey and use it here
    if (auto s = p.shader()) {
        SkColor colors[kMaxStops];
        SkColor4f color4fs[kMaxStops];
        float offsets[kMaxStops];
        SkShader::GradientInfo gradInfo;

        gradInfo.fColorCount = kMaxStops;
        gradInfo.fColors = colors;
        gradInfo.fColorOffsets = offsets;

        SkShader::GradientType type = s->asAGradient(&gradInfo);
        if (gradInfo.fColorCount > kMaxStops) {
            type = SkShader::GradientType::kNone_GradientType;
        }

        switch (type) {
            case SkShader::kLinear_GradientType: {
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               { type, gradInfo.fTileMode, gradInfo.fColorCount });

                uniforms = make_linear_gradient_uniform_data(gradInfo.fPoint[0],
                                                             gradInfo.fPoint[1],
                                                             color4fs,
                                                             offsets);
            } break;
            case SkShader::kRadial_GradientType: {
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               { type, gradInfo.fTileMode, gradInfo.fColorCount });

                uniforms =  make_radial_gradient_uniform_data(gradInfo.fPoint[0],
                                                              gradInfo.fRadius[0],
                                                              color4fs,
                                                              offsets);
            } break;
            case SkShader::kSweep_GradientType:
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               { type, gradInfo.fTileMode, gradInfo.fColorCount });

                uniforms = make_sweep_gradient_uniform_data(gradInfo.fPoint[0],
                                                            color4fs,
                                                            offsets);
                break;
            case SkShader::GradientType::kConical_GradientType:
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                GradientShaderBlocks::AddToKey(SkBackend::kGraphite,
                                               &key,
                                               { type, gradInfo.fTileMode, gradInfo.fColorCount });

                uniforms = make_conical_gradient_uniform_data(gradInfo.fPoint[0],
                                                              gradInfo.fPoint[1],
                                                              gradInfo.fRadius[0],
                                                              gradInfo.fRadius[1],
                                                              color4fs,
                                                              offsets);
                break;
            case SkShader::GradientType::kColor_GradientType:
                // TODO: The solid color gradient type should use its color, not
                // the paint color
            case SkShader::GradientType::kNone_GradientType:
            default:
                SolidColorShaderBlock::AddToKey(SkBackend::kGraphite, &key);

                uniforms = make_solid_uniform_data(p.color());
                break;
        }
    } else {
        // Solid colored paint
        SolidColorShaderBlock::AddToKey(SkBackend::kGraphite, &key);

        uniforms = make_solid_uniform_data(p.color());
    }

    if (p.blender()) {
        as_BB(p.blender())->addToKey(dict, SkBackend::kGraphite, &key);
    } else {
        BlendModeBlock::AddToKey(SkBackend::kGraphite, &key, SkBlendMode::kSrcOver);
    }

    auto entry = context->priv().shaderCodeDictionary()->findOrCreate(key);

    return { entry->uniqueID(), std::move(uniforms) };
}

SkSpan<const SkUniform> GetUniforms(CodeSnippetID snippetID) {
    switch (snippetID) {
        case CodeSnippetID::kDepthStencilOnlyDraw:
            return {nullptr, 0};
        case CodeSnippetID::kLinearGradientShader:
            return SkMakeSpan(kGradientUniforms, kNumGradientUniforms);
        case CodeSnippetID::kSolidColorShader:
        case CodeSnippetID::kRadialGradientShader:
        case CodeSnippetID::kSweepGradientShader:
        case CodeSnippetID::kConicalGradientShader:
        default:
            return SkMakeSpan(kSolidUniforms, kNumSolidUniforms);
    }
}

const char* GetShaderSkSL(CodeSnippetID snippetID) {
    switch (snippetID) {
        case CodeSnippetID::kDepthStencilOnlyDraw:
            return kNoneSkSL;
        case CodeSnippetID::kLinearGradientShader:
            return kGradientSkSL;
        case CodeSnippetID::kSolidColorShader:
        case CodeSnippetID::kRadialGradientShader:
        case CodeSnippetID::kSweepGradientShader:
        case CodeSnippetID::kConicalGradientShader:
        default:
            return kSolidColorSkSL;
    }
}

} // namespace skgpu
