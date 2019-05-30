/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(tracked) in uniform half4x4 m;
layout(tracked) in uniform half4 v;
layout(key) in bool unpremulInput;
layout(key) in bool clampOutput;
layout(key) in bool premulOutput;


@optimizationFlags {
    kNone_OptimizationFlags
}

void main() {
    @if (unpremulInput) {
        // The max() is to guard against 0 / 0 during unpremul when the incoming color is
        // transparent black.
        half nonZeroAlpha = max(sk_InColor.a, 0.00001);
        sk_InColor = half4(sk_InColor.rgb / nonZeroAlpha, nonZeroAlpha);
    }
    sk_OutColor = m * sk_InColor + v;
    @if (clampOutput) {
        sk_OutColor = saturate(sk_OutColor);
    }
    @if (premulOutput) {
        sk_OutColor.rgb *= sk_OutColor.a;
    }
}

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        SkColor4f color;
        if (unpremulInput) {
            color = input.unpremul();
        } else {
            color.fR = input.fR;
            color.fG = input.fG;
            color.fB = input.fB;
            color.fA = input.fA;
        }
        if (clampOutput) {
            color.fR = SkTPin(color.fR, 0.f, 1.f);
            color.fG = SkTPin(color.fG, 0.f, 1.f);
            color.fB = SkTPin(color.fB, 0.f, 1.f);
            color.fA = SkTPin(color.fA, 0.f, 1.f);
        }
        if (premulOutput) {
            return color.premul();
        } else {
            return {color.fR, color.fG, color.fB, color.fA};
        }
    }
}

@test(d) {
    return nullptr;
}
