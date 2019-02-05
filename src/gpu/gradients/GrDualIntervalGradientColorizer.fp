/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Models two intervals (so 4 colors), that are connected at a specific threshold point.

// Bias and scale for 0 to threshold
layout(ctype=SkPMColor4f, tracked) in uniform float4 scale01;
layout(ctype=SkPMColor4f, tracked) in uniform float4 bias01;

// Bias and scale for threshold to 1
layout(ctype=SkPMColor4f, tracked) in uniform float4 scale23;
layout(ctype=SkPMColor4f, tracked) in uniform float4 bias23;

layout(tracked) in uniform half threshold;

void main() {
    half t = sk_InColor.x;

    float4 scale, bias;
    if (t < threshold) {
        scale = scale01;
        bias = bias01;
    } else {
        scale = scale23;
        bias = bias23;
    }

    sk_OutColor = half4(t * scale + bias);
}

//////////////////////////////////////////////////////////////////////////////

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const SkPMColor4f& c0, const SkPMColor4f& c1,
                                                     const SkPMColor4f& c2, const SkPMColor4f& c3,
                                                     float threshold);
}

@cppEnd {
    std::unique_ptr<GrFragmentProcessor> GrDualIntervalGradientColorizer::Make(
            const SkPMColor4f& c0, const SkPMColor4f& c1, const SkPMColor4f& c2, const SkPMColor4f& c3, float threshold) {
        // Derive scale and biases from the 4 colors and threshold
        auto vc0 = Sk4f::Load(c0.vec());
        auto vc1 = Sk4f::Load(c1.vec());
        auto scale01 = (vc1 - vc0) / threshold;
        // bias01 = c0

        auto vc2 = Sk4f::Load(c2.vec());
        auto vc3 = Sk4f::Load(c3.vec());
        auto scale23 = (vc3 - vc2) / (1 - threshold);
        auto bias23 = vc2 - threshold * scale23;

        return std::unique_ptr<GrFragmentProcessor>(new GrDualIntervalGradientColorizer(
                { scale01[0], scale01[1], scale01[2], scale01[3] }, c0,
                { scale23[0], scale23[1], scale23[2], scale23[3] },
                { bias23[0], bias23[1], bias23[2], bias23[3] }, threshold));
    }
}
