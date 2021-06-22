/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkVx.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/effects/GrGradientEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"

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
    return GrSkSLFP::Make(effect, "SingleInterval", /*inputFP=*/nullptr, GrSkSLFP::OptFlags::kNone,
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

    return GrSkSLFP::Make(effect, "DualInterval", /*inputFP=*/nullptr, GrSkSLFP::OptFlags::kNone,
                          "scale01", scale01,
                          "bias01", c0,
                          "scale23", scale23,
                          "bias23", bias23,
                          "threshold", threshold);
}

}  // namespace GrGradientColorizer
