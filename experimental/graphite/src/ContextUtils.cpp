/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ContextUtils.h"

#include "experimental/graphite/src/Uniform.h"
#include "experimental/graphite/src/UniformCache.h"
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

sk_sp<UniformData> make_gradient_uniform_data_common(void* srcs[kNumGradientUniforms]) {
    static constexpr Uniform kUniforms[kNumGradientUniforms] {
            {"colors",  SLType::kFloat4 , kMaxStops },
            {"offsets", SLType::kFloat, kMaxStops },
            {"point0",   SLType::kFloat2 },
            {"point1",   SLType::kFloat2 },
            {"radius0",  SLType::kFloat },
            {"radius1",  SLType::kFloat },
    };

    UniformManager mgr(Layout::kMetal);

    // TODO: Given that, for the sprint, we always know the uniforms we could cache 'dataSize'
    // for each layout and skip the first call.
    size_t dataSize = mgr.writeUniforms(SkSpan<const Uniform>(kUniforms, kNumGradientUniforms),
                                        nullptr, nullptr, nullptr);

    sk_sp<UniformData> result = UniformData::Make(kNumGradientUniforms,
                                                  kUniforms,
                                                  dataSize);

    mgr.writeUniforms(SkSpan<const Uniform>(kUniforms, kNumGradientUniforms),
                      srcs, result->offsets(), result->data());
    return result;
}

sk_sp<UniformData> make_linear_gradient_uniform_data(SkPoint startPoint,
                                                     SkPoint endPoint,
                                                     SkColor4f colors[kMaxStops],
                                                     float offsets[kMaxStops]) {
    float unusedRadii[2] = { 0.0f, 0.0f };
    void* srcs[kNumGradientUniforms] = {
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

    void* srcs[kNumGradientUniforms] = {
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

    void* srcs[kNumGradientUniforms] = {
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

    void* srcs[kNumGradientUniforms] = {
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
    static constexpr int kNumSolidUniforms = 1;
    static constexpr Uniform kUniforms[kNumSolidUniforms] {
        {"color",  SLType::kFloat4 }
    };

    UniformManager mgr(Layout::kMetal);

    size_t dataSize = mgr.writeUniforms(SkSpan<const Uniform>(kUniforms, kNumSolidUniforms),
                                        nullptr, nullptr, nullptr);

    sk_sp<UniformData> result = UniformData::Make(kNumSolidUniforms, kUniforms, dataSize);

    void* srcs[kNumSolidUniforms] = { &color };

    mgr.writeUniforms(SkSpan<const Uniform>(kUniforms, kNumSolidUniforms),
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

std::tuple<Combination, sk_sp<UniformData>> ExtractCombo(UniformCache* cache, const SkPaint& p) {
    Combination result;
    sk_sp<UniformData> uniforms;

    if (auto s = p.getShader()) {
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
            case SkShader::GradientType::kNone_GradientType:
            default:
                result.fShaderType = ShaderCombo::ShaderType::kNone;
                result.fTileMode = SkTileMode::kRepeat;

                uniforms = make_solid_uniform_data(p.getColor4f());
                break;
        }
    } else {
        // Solid colored paint
        result.fShaderType = ShaderCombo::ShaderType::kNone;
        result.fTileMode = SkTileMode::kRepeat;

        uniforms = make_solid_uniform_data(p.getColor4f());
    }

    result.fBlendMode = p.getBlendMode_or(SkBlendMode::kSrcOver);

    sk_sp<UniformData> trueUD = cache->findOrCreate(std::move(uniforms));
    return { result, std::move(trueUD) };
}

} // namespace skgpu
