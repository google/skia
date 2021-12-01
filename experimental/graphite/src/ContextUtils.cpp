/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include <string>
#include "experimental/graphite/src/DrawList.h" // TODO: split PaintParams out into their own header
#include "experimental/graphite/src/DrawTypes.h"
#include "experimental/graphite/src/Uniform.h"
#include "experimental/graphite/src/UniformManager.h"
#include "include/core/SkPaint.h"

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
static constexpr Uniform kGradientUniforms[kNumGradientUniforms] {
        {"colors",  SLType::kHalf4 , kMaxStops },
        {"offsets", SLType::kFloat, kMaxStops },
        {"point0",   SLType::kFloat2 },
        {"point1",   SLType::kFloat2 },
        {"radius0",  SLType::kFloat },
        {"radius1",  SLType::kFloat },
};

static constexpr int kNumSolidUniforms = 1;
static constexpr Uniform kSolidUniforms[kNumSolidUniforms] {
        {"color",  SLType::kFloat4 }
};

static const char* kGradientMSL =
        // TODO: This should use local coords
        "float2 pos = interpolated.position.xy;\n"
        "float2 delta = uniforms.point1 - uniforms.point0;\n"
        "float2 pt = pos - uniforms.point0;\n"
        "float t = dot(pt, delta) / dot(delta, delta);\n"
        "float4 result = uniforms.colors[0];\n"
        "result = mix(result, uniforms.colors[1],\n"
        "             clamp((t-uniforms.offsets[0])/(uniforms.offsets[1]-uniforms.offsets[0]),\n"
        "                   0, 1));\n"
        "result = mix(result, uniforms.colors[2],\n"
        "             clamp((t-uniforms.offsets[1])/(uniforms.offsets[2]-uniforms.offsets[1]),\n"
        "                   0, 1));\n"
        "result = mix(result, uniforms.colors[3],\n"
        "             clamp((t-uniforms.offsets[2])/(uniforms.offsets[3]-uniforms.offsets[2]),\n"
        "             0, 1));\n"
        "out.color = result;\n";

static const char* kSolidColorMSL = "out.color = float4(uniforms.color);\n";

// TODO: kNone is for depth-only draws, so should actually have a fragment output type
// that only defines a [[depth]] attribute but no color calculation.
static const char* kNoneMSL = "out.color float4(0.0, 0.0, 1.0, 1.0);\n";

sk_sp<UniformData> make_gradient_uniform_data_common(const void* srcs[kNumGradientUniforms]) {
    UniformManager mgr(Layout::kMetal);

    // TODO: Given that, for the sprint, we always know the uniforms we could cache 'dataSize'
    // for each layout and skip the first call.
    size_t dataSize = mgr.writeUniforms(SkSpan<const Uniform>(kGradientUniforms,
                                                              kNumGradientUniforms),
                                        nullptr, nullptr, nullptr);

    sk_sp<UniformData> result = UniformData::Make(kNumGradientUniforms,
                                                  kGradientUniforms,
                                                  dataSize);

    mgr.writeUniforms(SkSpan<const Uniform>(kGradientUniforms, kNumGradientUniforms),
                      srcs, result->offsets(), result->data());
    return result;
}

sk_sp<UniformData> make_linear_gradient_uniform_data(SkPoint startPoint,
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

sk_sp<UniformData> make_radial_gradient_uniform_data(SkPoint point,
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

sk_sp<UniformData> make_sweep_gradient_uniform_data(SkPoint point,
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

sk_sp<UniformData> make_conical_gradient_uniform_data(SkPoint point0,
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

sk_sp<UniformData> make_solid_uniform_data(SkColor4f color) {
    UniformManager mgr(Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(SkSpan<const Uniform>(kSolidUniforms, kNumSolidUniforms),
                                        nullptr, nullptr, nullptr);

    sk_sp<UniformData> result = UniformData::Make(kNumSolidUniforms, kSolidUniforms, dataSize);

    const void* srcs[kNumSolidUniforms] = { &color };

    mgr.writeUniforms(SkSpan<const Uniform>(kSolidUniforms, kNumSolidUniforms),
                      srcs, result->offsets(), result->data());
    return result;
}

} // anonymous namespace

sk_sp<UniformData> UniformData::Make(int count,
                                     const Uniform* uniforms,
                                     size_t dataSize) {
    // TODO: the offsets and data should just be allocated right after UniformData in an arena
    uint32_t* offsets = new uint32_t[count];
    char* data = new char[dataSize];

    return sk_sp<UniformData>(new UniformData(count, uniforms, offsets, data, dataSize));
}

std::tuple<Combination, sk_sp<UniformData>> ExtractCombo(const PaintParams& p) {
    Combination result;
    sk_sp<UniformData> uniforms;

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
        // TODO(robertphillips): Remove once gradient MSL compiles
        type = SkShader::GradientType::kNone_GradientType;

        switch (type) {
            case SkShader::kLinear_GradientType: {
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                result.fShaderType = ShaderCombo::ShaderType::kLinearGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = make_linear_gradient_uniform_data(gradInfo.fPoint[0],
                                                             gradInfo.fPoint[1],
                                                             color4fs,
                                                             offsets);
            } break;
            case SkShader::kRadial_GradientType: {
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                result.fShaderType = ShaderCombo::ShaderType::kRadialGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms =  make_radial_gradient_uniform_data(gradInfo.fPoint[0],
                                                              gradInfo.fRadius[0],
                                                              color4fs,
                                                              offsets);
            } break;
            case SkShader::kSweep_GradientType:
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                result.fShaderType = ShaderCombo::ShaderType::kSweepGradient;
                result.fTileMode = gradInfo.fTileMode;

                uniforms = make_sweep_gradient_uniform_data(gradInfo.fPoint[0],
                                                            color4fs,
                                                            offsets);
                break;
            case SkShader::GradientType::kConical_GradientType:
                to_color4fs(gradInfo.fColorCount, colors, color4fs);
                expand_stops(gradInfo.fColorCount, offsets);

                result.fShaderType = ShaderCombo::ShaderType::kConicalGradient;
                result.fTileMode = gradInfo.fTileMode;

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
                result.fShaderType = ShaderCombo::ShaderType::kSolidColor;
                result.fTileMode = SkTileMode::kClamp;

                uniforms = make_solid_uniform_data(p.color());
                break;
        }
    } else {
        // Solid colored paint
        result.fShaderType = ShaderCombo::ShaderType::kSolidColor;
        result.fTileMode = SkTileMode::kClamp;

        uniforms = make_solid_uniform_data(p.color());
    }

    result.fBlendMode = p.blendMode();
    return { result, std::move(uniforms) };
}

SkSpan<const Uniform> GetUniforms(ShaderCombo::ShaderType shaderType) {
    switch (shaderType) {
        case ShaderCombo::ShaderType::kLinearGradient:
        case ShaderCombo::ShaderType::kRadialGradient:
        case ShaderCombo::ShaderType::kSweepGradient:
        case ShaderCombo::ShaderType::kConicalGradient:
            // TODO(robertphillips): return gradient uniforms when MSL is ready
            // return SkMakeSpan(kGradientUniforms, kNumGradientUniforms);
            return SkMakeSpan(kSolidUniforms, kNumSolidUniforms);
        case ShaderCombo::ShaderType::kNone:
            return {nullptr, 0};
        default:
            return SkMakeSpan(kSolidUniforms, kNumSolidUniforms);
    }
}

const char* GetShaderMSL(ShaderCombo::ShaderType shaderType) {
    switch (shaderType) {
        case ShaderCombo::ShaderType::kLinearGradient:
        case ShaderCombo::ShaderType::kRadialGradient:
        case ShaderCombo::ShaderType::kSweepGradient:
        case ShaderCombo::ShaderType::kConicalGradient:
            // TODO(robertphillips): return gradient MSL when ready
            (void) kGradientMSL;
            return kSolidColorMSL;
        case ShaderCombo::ShaderType::kNone:
            return kNoneMSL;
        case ShaderCombo::ShaderType::kSolidColor:
        default:
            return kSolidColorMSL;
    }
}

} // namespace skgpu
