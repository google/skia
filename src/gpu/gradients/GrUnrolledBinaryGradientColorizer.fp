/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Unrolled gradient code supporting up to 8 intervals that produces code
// targeting a specific interval count.

// Assumed to be between 1 and 8.
layout(key) in int intervalCount;

// With the current hardstop detection threshold of 0.00024, the maximum scale and bias values
// will be on the order of 4k (since they divide by dt). That is well outside the precision
// capabilities of half floats, which can lead to inaccurate gradient calculations
layout(ctype=SkPMColor4f) in uniform float4 scale0_1;
layout(ctype=SkPMColor4f, when=intervalCount > 1) in uniform float4 scale2_3;
layout(ctype=SkPMColor4f, when=intervalCount > 2) in uniform float4 scale4_5;
layout(ctype=SkPMColor4f, when=intervalCount > 3) in uniform float4 scale6_7;
layout(ctype=SkPMColor4f, when=intervalCount > 4) in uniform float4 scale8_9;
layout(ctype=SkPMColor4f, when=intervalCount > 5) in uniform float4 scale10_11;
layout(ctype=SkPMColor4f, when=intervalCount > 6) in uniform float4 scale12_13;
layout(ctype=SkPMColor4f, when=intervalCount > 7) in uniform float4 scale14_15;

layout(ctype=SkPMColor4f) in uniform float4 bias0_1;
layout(ctype=SkPMColor4f, when=intervalCount > 1) in uniform float4 bias2_3;
layout(ctype=SkPMColor4f, when=intervalCount > 2) in uniform float4 bias4_5;
layout(ctype=SkPMColor4f, when=intervalCount > 3) in uniform float4 bias6_7;
layout(ctype=SkPMColor4f, when=intervalCount > 4) in uniform float4 bias8_9;
layout(ctype=SkPMColor4f, when=intervalCount > 5) in uniform float4 bias10_11;
layout(ctype=SkPMColor4f, when=intervalCount > 6) in uniform float4 bias12_13;
layout(ctype=SkPMColor4f, when=intervalCount > 7) in uniform float4 bias14_15;

// The 7 threshold positions that define the boundaries of the 8 intervals (excluding t = 0, and t =
// 1) are packed into two half4's instead of having up to 7 separate scalar uniforms. For low
// interval counts, the extra components are ignored in the shader, but the uniform simplification
// is worth it. It is assumed thresholds are provided in increasing value, mapped as:
//  - thresholds1_7.x = boundary between (0,1) and (2,3) -> 1_2
//  -              .y = boundary between (2,3) and (4,5) -> 3_4
//  -              .z = boundary between (4,5) and (6,7) -> 5_6
//  -              .w = boundary between (6,7) and (8,9) -> 7_8
//  - thresholds9_13.x = boundary between (8,9) and (10,11) -> 9_10
//  -               .y = boundary between (10,11) and (12,13) -> 11_12
//  -               .z = boundary between (12,13) and (14,15) -> 13_14
//  -               .w = unused
in uniform half4 thresholds1_7;
in uniform half4 thresholds9_13;

void main() {
    half t = sk_InColor.x;

    float4 scale, bias;
    // Explicit binary search for the proper interval that t falls within. The interval count
    // checks are converted into constant expressions in the C++ generated SkSL, which are then
    // optimized to the minimal number of branches for the specific interval count.

    // thresholds1_7.w is mid point for intervals (0,7) and (8,15)
    if (intervalCount <= 4 || t < thresholds1_7.w) {
        // thresholds1_7.y is mid point for intervals (0,3) and (4,7)
        if (intervalCount <= 2 || t < thresholds1_7.y) {
            // thresholds1_7.x is mid point for intervals (0,1) and (2,3)
            if (intervalCount <= 1 || t < thresholds1_7.x) {
                scale = scale0_1;
                bias = bias0_1;
            } else {
                scale = scale2_3;
                bias = bias2_3;
            }
        } else {
            // thresholds1_7.z is mid point for intervals (4,5) and (6,7)
            if (intervalCount <= 3 || t < thresholds1_7.z) {
                scale = scale4_5;
                bias = bias4_5;
            } else {
                scale = scale6_7;
                bias = bias6_7;
            }
        }
    } else {
        // thresholds9_13.y is mid point for intervals (8,11) and (12,15)
        if (intervalCount <= 6 || t < thresholds9_13.y) {
            // thresholds9_13.x is mid point for intervals (8,9) and (10,11)
            if (intervalCount <= 5 || t < thresholds9_13.x) {
                // interval 8-9
                scale = scale8_9;
                bias = bias8_9;
            } else {
                // interval 10-11
                scale = scale10_11;
                bias = bias10_11;
            }
        } else {
            // thresholds9_13.z is mid point for intervals (12,13) and (14,15)
            if (intervalCount <= 7 || t < thresholds9_13.z) {
                // interval 12-13
                scale = scale12_13;
                bias = bias12_13;
            } else {
                // interval 14-15
                scale = scale14_15;
                bias = bias14_15;
            }
        }
    }

    sk_OutColor = half4(t * scale + bias);
}

//////////////////////////////////////////////////////////////////////////////

@class {
    static const int kMaxColorCount = 16;
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const SkPMColor4f* colors,
                                                     const SkScalar* positions,
                                                     int count);
}

@cppEnd {
    static const int kMaxIntervals = 8;
    std::unique_ptr<GrFragmentProcessor> GrUnrolledBinaryGradientColorizer::Make(
            const SkPMColor4f* colors, const SkScalar* positions, int count) {
        // Depending on how the positions resolve into hard stops or regular stops, the number of
        // intervals specified by the number of colors/positions can change. For instance, a plain
        // 3 color gradient is two intervals, but a 4 color gradient with a hard stop is also
        // two intervals. At the most extreme end, an 8 interval gradient made entirely of hard
        // stops has 16 colors.

        if (count > kMaxColorCount) {
            // Definitely cannot represent this gradient configuration
            return nullptr;
        }

        // The raster implementation also uses scales and biases, but since they must be calculated
        // after the dst color space is applied, it limits our ability to cache their values.
        SkPMColor4f scales[kMaxIntervals];
        SkPMColor4f biases[kMaxIntervals];
        SkScalar thresholds[kMaxIntervals];

        int intervalCount = 0;

        for (int i = 0; i < count - 1; i++) {
            if (intervalCount >= kMaxIntervals) {
                // Already reached kMaxIntervals, and haven't run out of color stops so this
                // gradient cannot be represented by this shader.
                return nullptr;
            }

            SkScalar t0 = positions[i];
            SkScalar t1 = positions[i + 1];
            SkScalar dt = t1 - t0;
            // If the interval is empty, skip to the next interval. This will automatically create
            // distinct hard stop intervals as needed. It also protects against malformed gradients
            // that have repeated hard stops at the very beginning that are effectively unreachable.
            if (SkScalarNearlyZero(dt)) {
                continue;
            }

            auto c0 = Sk4f::Load(colors[i].vec());
            auto c1 = Sk4f::Load(colors[i + 1].vec());

            auto scale = (c1 - c0) / dt;
            auto bias = c0 - t0 * scale;

            scale.store(scales + intervalCount);
            bias.store(biases + intervalCount);
            thresholds[intervalCount] = t1;
            intervalCount++;
        }

        // For isEqual to make sense, set the unused values to something consistent
        for (int i = intervalCount; i < kMaxIntervals; i++) {
            scales[i] = SK_PMColor4fTRANSPARENT;
            biases[i] = SK_PMColor4fTRANSPARENT;
            thresholds[i] = 0.0;
        }

        return std::unique_ptr<GrFragmentProcessor>(new GrUnrolledBinaryGradientColorizer(
                intervalCount, scales[0], scales[1], scales[2], scales[3], scales[4], scales[5],
                scales[6], scales[7], biases[0], biases[1], biases[2], biases[3], biases[4],
                biases[5], biases[6], biases[7],
                SkRect::MakeLTRB(thresholds[0], thresholds[1], thresholds[2], thresholds[3]),
                SkRect::MakeLTRB(thresholds[4], thresholds[5], thresholds[6], 0.0)));
    }
}
