/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


// Based on work by Sam Hocevar, Emil Persson, and Ian Taylor [1][2][3].
//
// [1] http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
// [2] http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
// [3] http://www.chilliant.com/rgb2hsv.html

void main() {
    half3   hsl = sk_InColor.rgb;
    half3 kBias = half3(0, 2/3.0, 1/3.0);
    half3   rgb = saturate(abs(fract(hsl.xxx + kBias) * 6 - 3) - 1);
    half      C = (1 - abs(2 * hsl.z - 1)) * hsl.y;

    sk_OutColor = saturate(half4((rgb - 0.5) * C + hsl.z, sk_InColor.a));
    sk_OutColor.rgb *= sk_OutColor.a;
}

@optimizationFlags {
    kConstantOutputForConstantInput_OptimizationFlag
}

@class {
    #include "include/private/SkColorData.h"
    #include "include/private/SkNx.h"

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& c) const override {
        const auto H = c[0],
                   S = c[1],
                   L = c[2],
                   C = (1 - std::abs(2 * L - 1)) * S;

        const auto p = H + Sk4f(0, 2/3.f, 1/3.f, 0),
                   q = ((p - p.floor()) * 6 - 3).abs() - 1,
                 RGB = C * (Sk4f::Min(Sk4f::Max(q, 0), 1) - 0.5f) + L,
               pmRGB = Sk4f::Min(Sk4f::Max(RGB, 0), 1) * c.fA;

        return { pmRGB[0], pmRGB[1], pmRGB[2], c.fA };
    }
}
