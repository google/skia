/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Convert RGBA -> HSLA (including unpremul).
//
// Based on work by Sam Hocevar, Emil Persson, and Ian Taylor [1][2][3].  High-level ideas:
//
//   - minimize the number of branches by sorting and computing the hue phase in parallel (vec4s)
//
//   - trade the third sorting branch for a potentially faster std::min and leaving 2nd/3rd
//     channels unsorted (based on the observation that swapping both the channels and the bias sign
//     has no effect under abs)
//
//   - use epsilon offsets for denominators, to avoid explicit zero-checks
//
// An additional trick we employ is deferring premul->unpremul conversion until the very end: the
// alpha factor gets naturally simplified for H and S, and only L requires a dedicated unpremul
// division (so we trade three divs for one).
//
// [1] http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
// [2] http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
// [3] http://www.chilliant.com/rgb2hsv.html

in fragmentProcessor inputFP;

half4 main() {
    half4 c = sample(inputFP);
    half4 p = (c.g < c.b) ? half4(c.bg, -1,  2/3.0)
                          : half4(c.gb,  0, -1/3.0);
    half4 q = (c.r < p.x) ? half4(p.x, c.r, p.yw)
                          : half4(c.r, p.x, p.yz);

    // q.x  -> max channel value
    // q.yz -> 2nd/3rd channel values (unsorted)
    // q.w  -> bias value dependent on max channel selection

    half eps = 0.0001;
    half pmV = q.x;
    half pmC = pmV - min(q.y, q.z);
    half pmL = pmV - pmC * 0.5;
    half   H = abs(q.w + (q.y - q.z) / (pmC * 6 + eps));
    half   S = pmC / (c.a + eps - abs(pmL * 2 - c.a));
    half   L = pmL / (c.a + eps);

    return half4(H, S, L, c.a);
}

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    (kConstantOutputForConstantInput_OptimizationFlag | kPreservesOpaqueInput_OptimizationFlag)
}

@class {
    #include "include/private/SkColorData.h"

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
        SkPMColor4f c = ConstantOutputForConstantInput(this->childProcessor(0), inColor);
        const auto p = (c.fG < c.fB) ? SkPMColor4f{ c.fB, c.fG, -1,  2/3.f }
                                     : SkPMColor4f{ c.fG, c.fB,  0, -1/3.f },
                   q = (c.fR < p[0]) ? SkPMColor4f{ p[0], c.fR, p[1], p[3] }
                                     : SkPMColor4f{ c.fR, p[0], p[1], p[2] };

        const auto eps = 0.0001f, // matching SkSL/ColorMatrix half4 epsilon
                   pmV = q[0],
                   pmC = pmV - std::min(q[1], q[2]),
                   pmL = pmV - pmC * 0.5f,
                     H = std::abs(q[3] + (q[1] - q[2]) / (pmC * 6 + eps)),
                     S = pmC / (c.fA + eps - std::abs(pmL * 2 - c.fA)),
                     L = pmL / (c.fA + eps);

        return { H, S, L, c.fA };
    }
}
