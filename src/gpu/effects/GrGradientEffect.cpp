/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkVx.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/effects/GrGradientEffect.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/shaders/gradients/SkLinearGradient.h"
#include "src/shaders/gradients/SkRadialGradient.h"
#include "src/shaders/gradients/SkSweepGradient.h"

namespace GrGradientColorizer {

std::unique_ptr<GrFragmentProcessor> SingleInterval(const SkPMColor4f& start,
                                                    const SkPMColor4f& end) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform half4 start;
        uniform half4 end;
        half4 main(float2 coord) {
            // Clamping and/or wrapping was already handled by the parent shader so the output
            // color is a simple lerp.
            return mix(start, end, half(coord.x));
        }
    )");
    return GrSkSLFP::Make(effect, "SingleIntervalColorizer", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kNone,
                          "start", start,
                          "end", end);
}

std::unique_ptr<GrFragmentProcessor> DualInterval(const SkPMColor4f& c0,
                                                  const SkPMColor4f& c1,
                                                  const SkPMColor4f& c2,
                                                  const SkPMColor4f& c3,
                                                  float threshold) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform float4 scale01;
        uniform float4 bias01;
        uniform float4 scale23;
        uniform float4 bias23;
        uniform half threshold;

        half4 main(float2 coord) {
            half t = half(coord.x);

            float4 scale, bias;
            if (t < threshold) {
                scale = scale01;
                bias = bias01;
            } else {
                scale = scale23;
                bias = bias23;
            }

            return half4(t * scale + bias);
        }
    )");

    using sk4f = skvx::Vec<4, float>;

    // Derive scale and biases from the 4 colors and threshold
    auto vc0 = sk4f::Load(c0.vec());
    auto vc1 = sk4f::Load(c1.vec());
    auto scale01 = (vc1 - vc0) / threshold;
    // bias01 = c0

    auto vc2 = sk4f::Load(c2.vec());
    auto vc3 = sk4f::Load(c3.vec());
    auto scale23 = (vc3 - vc2) / (1 - threshold);
    auto bias23 = vc2 - threshold * scale23;

    return GrSkSLFP::Make(effect, "DualIntervalColorizer", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kNone,
                          "scale01", scale01,
                          "bias01", c0,
                          "scale23", scale23,
                          "bias23", bias23,
                          "threshold", threshold);
}

}  // namespace GrGradientColorizer

namespace GrGradientLayout {

static std::unique_ptr<GrFragmentProcessor> apply_matrix(const SkGradientShaderBase& gradient,
                                                         const GrFPArgs& args,
                                                         std::unique_ptr<GrFragmentProcessor> fp) {
    SkMatrix matrix;
    if (!gradient.totalLocalMatrix(args.fPreLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    matrix.postConcat(gradient.getGradientMatrix());
    return GrMatrixEffect::Make(matrix, std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> Linear(const SkLinearGradient& gradient,
                                            const GrFPArgs& args) {
    // We add a tiny delta to t. When gradient stops are set up so that a hard stop in a vertically
    // or horizontally oriented gradient falls exactly at a column or row of pixel centers we can
    // we can get slightly different interpolated t values along the column/row. By adding the delta
    // we will consistently get the color to the "right" of the stop. Of course if the hard stop
    // falls at X.5 - delta then we still could get inconsistent results, but that is much less
    // likely. crbug.com/938592
    // If/when we add filtering of the gradient this can be removed.
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        half4 main(float2 coord) {
            return half4(half(coord.x) + 0.00001, 1, 0, 0); // y = 1 for always valid
        }
    )");
    // The linear gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(effect, "LinearLayout", /*inputFP=*/nullptr,
                             GrSkSLFP::OptFlags::kPreservesOpaqueInput);
    return apply_matrix(gradient, args, std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> Radial(const SkRadialGradient& gradient,
                                            const GrFPArgs& args) {
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        half4 main(float2 coord) {
            return half4(half(length(coord)), 1, 0, 0); // y = 1 for always valid
        }
    )");
    // The radial gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(effect, "RadialLayout", /*inputFP=*/nullptr,
                             GrSkSLFP::OptFlags::kPreservesOpaqueInput);
    return apply_matrix(gradient, args, std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> Sweep(const SkSweepGradient& gradient, const GrFPArgs& args) {
    // On some devices they incorrectly implement atan2(y,x) as atan(y/x). In actuality it is
    // atan2(y,x) = 2 * atan(y / (sqrt(x^2 + y^2) + x)). So to work around this we pass in (sqrt(x^2
    // + y^2) + x) as the second parameter to atan2 in these cases. We let the device handle the
    // undefined behavior of the second paramenter being 0 instead of doing the divide ourselves and
    // using atan instead.
    int useAtanWorkaround =
            args.fContext->priv().caps()->shaderCaps()->atan2ImplementedAsAtanYOverX();
    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform half bias;
        uniform half scale;
        uniform int useAtanWorkaround;

        half4 main(float2 coord) {
            half angle = bool(useAtanWorkaround)
                    ? half(2 * atan(-coord.y, length(coord) - coord.x))
                    : half(atan(-coord.y, -coord.x));

            // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
            half t = (angle * 0.1591549430918 + 0.5 + bias) * scale;
            return half4(t, 1, 0, 0); // y = 1 for always valid
        }
    )");
    // The sweep gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(effect, "SweepLayout", /*inputFP=*/nullptr,
                             GrSkSLFP::OptFlags::kPreservesOpaqueInput,
                             "bias", gradient.getTBias(),
                             "scale", gradient.getTScale(),
                             "useAtanWorkaround", GrSkSLFP::Specialize(useAtanWorkaround));
    return apply_matrix(gradient, args, std::move(fp));
}

}  // namespace GrGradientLayout
