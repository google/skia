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

layout(ctype=GrColor4f, tracked) in uniform half4 scale0_1;
layout(ctype=GrColor4f, tracked, when=intervalCount > 1) in uniform half4 scale2_3;
layout(ctype=GrColor4f, tracked, when=intervalCount > 2) in uniform half4 scale4_5;
layout(ctype=GrColor4f, tracked, when=intervalCount > 3) in uniform half4 scale6_7;
layout(ctype=GrColor4f, tracked, when=intervalCount > 4) in uniform half4 scale8_9;
layout(ctype=GrColor4f, tracked, when=intervalCount > 5) in uniform half4 scale10_11;
layout(ctype=GrColor4f, tracked, when=intervalCount > 6) in uniform half4 scale12_13;
layout(ctype=GrColor4f, tracked, when=intervalCount > 7) in uniform half4 scale14_15;

layout(ctype=GrColor4f, tracked) in uniform half4 bias0_1;
layout(ctype=GrColor4f, tracked, when=intervalCount > 1) in uniform half4 bias2_3;
layout(ctype=GrColor4f, tracked, when=intervalCount > 2) in uniform half4 bias4_5;
layout(ctype=GrColor4f, tracked, when=intervalCount > 3) in uniform half4 bias6_7;
layout(ctype=GrColor4f, tracked, when=intervalCount > 4) in uniform half4 bias8_9;
layout(ctype=GrColor4f, tracked, when=intervalCount > 5) in uniform half4 bias10_11;
layout(ctype=GrColor4f, tracked, when=intervalCount > 6) in uniform half4 bias12_13;
layout(ctype=GrColor4f, tracked, when=intervalCount > 7) in uniform half4 bias14_15;

// Code assumes these are in increasing value
layout(when=intervalCount > 1) in uniform half threshold1_2;
layout(when=intervalCount > 2) in uniform half threshold3_4;
layout(when=intervalCount > 3) in uniform half threshold5_6;
layout(when=intervalCount > 4) in uniform half threshold7_8;
layout(when=intervalCount > 5) in uniform half threshold9_10;
layout(when=intervalCount > 6) in uniform half threshold11_12;
layout(when=intervalCount > 7) in uniform half threshold13_14;

void main() {
    half t = sk_InColor.x;

    half4 scale, bias;
    // Explicit binary search for the proper interval that t falls within. The interval count
    // checks are converted into constant expressions in the C++ generated SkSL, which are then
    // optimized to the minimal number of branches for the specific interval count.
    if (intervalCount <= 4 || t < threshold7_8) {
        if (intervalCount <= 2 || t < threshold3_4) {
            if (intervalCount <= 1 || t < threshold1_2) {
                // interval 0-1
                scale = scale0_1;
                bias = bias0_1;
            } else {
                // interval 2-3
                scale = scale2_3;
                bias = bias2_3;
            }
        } else {
            if (intervalCount <= 3 || t < threshold5_6) {
                // interval 4-5
                scale = scale4_5;
                bias = bias4_5;
            } else {
                // interval 6-7
                scale = scale6_7;
                bias = bias6_7;
            }
        }
    } else {
        if (intervalCount <= 6 || t < threshold11_12) {
            if (intervalCount <= 5 || t < threshold9_10) {
                // interval 8-9
                scale = scale8_9;
                bias = bias8_9;
            } else {
                // interval 10-11
                scale = scale10_11;
                bias = bias10_11;
            }
        } else {
            if (intervalCount <= 7 || t < threshold13_14) {
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

    sk_OutColor = t * scale + bias;
}

//////////////////////////////////////////////////////////////////////////////

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const GrColor4f* colors,
                                                     const SkScalar* positions,
                                                     int count);
}

@cppEnd {
    static const int kMaxIntervals = 8;
    std::unique_ptr<GrFragmentProcessor> GrUnrolledBinaryGradientColorizer::Make(
            const GrColor4f* colors, const SkScalar* positions, int count) {
        // Depending on how the positions resolve into hard stops or regular stops, the number of
        // intervals specified by the number of colors/positions can change. For instance, a plain
        // 3 color gradient is two intervals, but a 4 color gradient with a hard stop is also
        // two intervals. At the most extreme end, an 8 interval gradient made entirely of hard
        // stops has 16 colors.

        if (count > 16) {
            // Definitely cannot represent this gradient configuration
            return nullptr;
        }

        // FIXME would it benefit the raster implementation to have this same representation?
        // Then all gradients are computed from scale*t + bias; and could then be computed once and
        // stored within the SkGradientShader instance instead of being calculated each time a
        // GrFragmentProcessor is instantiated. The dual and single interval shaders are just
        // specializations of this.
        GrColor4f scales[kMaxIntervals];
        GrColor4f biases[kMaxIntervals];
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

            auto c0 = Sk4f::Load(colors[i].fRGBA);
            auto c1 = Sk4f::Load(colors[i + 1].fRGBA);

            auto scale = (c1 - c0) / dt;
            auto bias = c0 - t0 * scale;

            scale.store(scales + intervalCount);
            bias.store(biases + intervalCount);
            thresholds[intervalCount] = t1;
            intervalCount++;
        }

        // FIXME Would it make sense to have very similar shaders that have lower power-of-two
        // maximum inputs? That way if the interval count <= 4 it is not allocating
        // member storage for the unused intervals. Since there is already a specialized 1 and 2
        // interval shader, this would only apply to gradients that have 3 or 4 intervals (since 5,
        // 6, or 7 intervals still goes to this shader). That provides a savings of up to four
        // intervals = 8 GrColor4fs = 128B extra per GrFragmentProcessor instance.
        return std::unique_ptr<GrFragmentProcessor>(new GrUnrolledBinaryGradientColorizer(
                intervalCount, scales[0], scales[1], scales[2], scales[3], scales[4], scales[5],
                scales[6], scales[7], biases[0], biases[1], biases[2], biases[3], biases[4],
                biases[5], biases[6], biases[7], thresholds[0], thresholds[1], thresholds[2],
                thresholds[3], thresholds[4], thresholds[5], thresholds[6]));
    }
}
