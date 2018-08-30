/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Models two intervals (so 4 colors), that are connected at a specific threshold point.

layout(ctype=GrColor4f) in half4 c0;
layout(ctype=GrColor4f) in half4 c1;
layout(ctype=GrColor4f) in half4 c2;
layout(ctype=GrColor4f) in half4 c3;

// Bias and scale for 0 to threshold
uniform half4 uScale01;
uniform half4 uBias01;

// Bias and scale for threshold to 1
uniform half4 uScale23;
uniform half4 uBias23;

in float threshold;
uniform float uThreshold;

@setData(pdman) {
    // Transform the colors before loading to the GPU so that the shader code is a simple scale+bias
    auto vc0 = Sk4f::Load(c0.fRGBA);
    auto vc1 = Sk4f::Load(c1.fRGBA);
    auto scale = (vc1 - vc0) / threshold;
    pdman.set4f(uBias01, vc0[0], vc0[1], vc0[2], vc0[3]);
    pdman.set4f(uScale01, scale[0], scale[1], scale[2], scale[3]);

    auto vc2 = Sk4f::Load(c2.fRGBA);
    auto vc3 = Sk4f::Load(c3.fRGBA);
    scale = (vc3 - vc2) / (1 - threshold);
    auto bias = vc2 - threshold * scale;
    pdman.set4f(uBias23, bias[0], bias[1], bias[2], bias[3]);
    pdman.set4f(uScale23, scale[0], scale[1], scale[2], scale[3]);

    pdman.set1f(uThreshold, threshold);
}

void main() {
    half t = sk_InColor.x;

    half4 scale, bias;
    if (t < uThreshold) {
        scale = uScale01;
        bias = uBias01;
    } else {
        scale = uScale23;
        bias = uBias23;
    }

    sk_OutColor = t * scale + bias;
}
