/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(ctype=SkMatrix44, tracked) in uniform half4x4 m;
layout(ctype=SkVector4, tracked) in uniform half4 v;
layout(key) in bool unpremulInput;
layout(key) in bool clampRGBOutput;
layout(key) in bool premulOutput;
layout(key) in bool hslaMatrix;

@optimizationFlags {
    kConstantOutputForConstantInput_OptimizationFlag
}


void main() {
    half4 inputColor = sk_InColor;
    @if (unpremulInput) {
        // The max() is to guard against 0 / 0 during unpremul when the incoming color is
        // transparent black.
        half nonZeroAlpha = max(inputColor.a, 0.0001);
        inputColor = half4(inputColor.rgb / nonZeroAlpha, nonZeroAlpha);
    }

    @if (hslaMatrix) {
        half3 c = half3(inputColor.r, inputColor.g, inputColor.b);
        half4 K = half4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        half4 p = mix(half4(c.bg, K.wz), half4(c.gb, K.xy), step(c.b, c.g));
        half4 q = mix(half4(p.xyw, c.r), half4(c.r, p.yzx), step(p.x, c.r));

        half d = q.x - min(q.w, q.y);
        //half e = 1.0e-10;
        half e = 0.0001;
        half3 hsv = half3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);

        inputColor.r = hsv.x;
        inputColor.b = hsv.z * (1 - hsv.y * 0.5);
        half cmin = min(inputColor.b, 1 - inputColor.b);
        inputColor.g = (cmin != 0) ? (hsv.z - inputColor.b) / cmin : 0;
    }

    sk_OutColor = m * inputColor + v;

    @if (hslaMatrix) {
        half3   hsl = sk_OutColor.rgb;
        half3 kBias = half3(0, 2.0/3, 1.0/3);
        half3     p = abs(fract(hsl.xxx + kBias.xyz) * 6 - 3);
        half3   rgb = saturate(p - 1);
        half      C = (1 - abs(2 * hsl.z - 1)) * hsl.y;

        sk_OutColor.rgb = (rgb - 0.5) * C + hsl.z;
    }

    @if (false && hslaMatrix) {
        half h = sk_OutColor.r;
        half v = sk_OutColor.b + sk_OutColor.g * min(sk_OutColor.b, 1 - sk_OutColor.b);
        half s = (v == 0) ? 0 : (2 - 2 * sk_OutColor.b / v);
        half3 c = half3(h, s, v);

        half4 K = half4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        half3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        half3 rgb = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        sk_OutColor = half4(rgb.r, rgb.g, rgb.b, sk_OutColor.a);
    }
    @if (clampRGBOutput) {
        sk_OutColor = saturate(sk_OutColor);
    } else {
        sk_OutColor.a = saturate(sk_OutColor.a);
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
        m.mapScalars(color.vec());
        color.fR += v.fData[0];
        color.fG += v.fData[1];
        color.fB += v.fData[2];
        color.fA += v.fData[3];
        color.fA = SkTPin(color.fA, 0.f, 1.f);
        if (clampRGBOutput) {
            color.fR = SkTPin(color.fR, 0.f, 1.f);
            color.fG = SkTPin(color.fG, 0.f, 1.f);
            color.fB = SkTPin(color.fB, 0.f, 1.f);
        }
        if (premulOutput) {
            return color.premul();
        } else {
            return {color.fR, color.fG, color.fB, color.fA};
        }
    }
}

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(const float matrix[20],
                                                     bool unpremulInput,
                                                     bool clampRGBOutput,
                                                     bool premulOutput,
                                                     bool hslaMatrix = false) {
        SkMatrix44 m44;
        m44.set4x4(
                matrix[0], matrix[5], matrix[10], matrix[15],
                matrix[1], matrix[6], matrix[11], matrix[16],
                matrix[2], matrix[7], matrix[12], matrix[17],
                matrix[3], matrix[8], matrix[13], matrix[18]
        );
        auto v4 = SkVector4(matrix[4], matrix[9], matrix[14], matrix[19]);
        return std::unique_ptr<GrFragmentProcessor>(
                new GrColorMatrixFragmentProcessor(m44, v4, unpremulInput, clampRGBOutput,
                                                   premulOutput, hslaMatrix));
    }
}

@test(d) {
    float m[20];
    for (int i = 0; i < 20; ++i) {
        m[i] = d->fRandom->nextRangeScalar(-10.f, 10.f);
    }
    bool unpremul = d->fRandom->nextBool();
    bool clampRGB = d->fRandom->nextBool();
    bool premul = d->fRandom->nextBool();
    bool hsla = d->fRandom->nextBool();
    return Make(m, unpremul, clampRGB, premul, hsla);
}
