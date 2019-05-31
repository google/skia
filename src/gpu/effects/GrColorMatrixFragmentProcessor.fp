/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(ctype=SkMatrix44, tracked) in uniform half4x4 m;
layout(ctype=SkRect, tracked) in uniform half4 v;
layout(key) in bool unpremulInput;
layout(key) in bool clampRGBOutput;
layout(key) in bool premulOutput;

@optimizationFlags {
    kConstantOutputForConstantInput_OptimizationFlag
}

void main() {
    half4 inputColor = sk_InColor;
    @if (unpremulInput) {
        // The max() is to guard against 0 / 0 during unpremul when the incoming color is
        // transparent black.
        half nonZeroAlpha = max(inputColor.a, 0.00001);
        inputColor = half4(inputColor.rgb / nonZeroAlpha, nonZeroAlpha);
    }
    sk_OutColor = m * inputColor + v;
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
        color.fR += v.fLeft;
        color.fG += v.fTop;
        color.fB += v.fRight;
        color.fA += v.fBottom;
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
    static std::unique_ptr<GrFragmentProcessor> Make(const float matrix[20], bool unpremulInput, bool clampRGBOutput, bool premulOutput) {
        const float m[] {
                matrix[0],  matrix[1],  matrix[2],  matrix[3],
                matrix[5],  matrix[6],  matrix[7],  matrix[8],
                matrix[10], matrix[11], matrix[12], matrix[13],
                matrix[15], matrix[16], matrix[17], matrix[18]
        };
        SkMatrix44 m44;
        m44.setRowMajorf(m);
        SkRect v4 = SkRect::MakeLTRB(matrix[4], matrix[9], matrix[14], matrix[19]);
        return std::unique_ptr<GrFragmentProcessor>(new GrColorMatrixFragmentProcessor(m44, v4, unpremulInput, clampRGBOutput, premulOutput));
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
    return Make(m, unpremul, clampRGB, premul);
}
