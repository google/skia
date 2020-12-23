/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;
layout(ctype=SkM44, tracked) in uniform half4x4 m;
layout(ctype=SkV4, tracked) in uniform half4 v;
layout(key) in bool unpremulInput;
layout(key) in bool clampRGBOutput;
layout(key) in bool premulOutput;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    kConstantOutputForConstantInput_OptimizationFlag
}

half4 main() {
    half4 color = sample(inputFP);
    @if (unpremulInput) {
        color = unpremul(color);
    }
    color = m * color + v;
    @if (clampRGBOutput) {
        color = saturate(color);
    } else {
        color.a = saturate(color.a);
    }
    @if (premulOutput) {
        color.rgb *= color.a;
    }
    return color;
}

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
        SkPMColor4f input = ConstantOutputForConstantInput(this->childProcessor(0), inColor);
        SkColor4f color;
        if (unpremulInput) {
            color = input.unpremul();
        } else {
            color.fR = input.fR;
            color.fG = input.fG;
            color.fB = input.fB;
            color.fA = input.fA;
        }
        auto v4 = m.map(color.fR, color.fG, color.fB, color.fA) + v;
        color = {v4.x, v4.y, v4.z, v4.w};
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
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                     const float matrix[20], bool unpremulInput,
                                                     bool clampRGBOutput, bool premulOutput) {
        SkM44 m44(
            matrix[ 0], matrix[ 1], matrix[ 2], matrix[ 3],
            matrix[ 5], matrix[ 6], matrix[ 7], matrix[ 8],
            matrix[10], matrix[11], matrix[12], matrix[13],
            matrix[15], matrix[16], matrix[17], matrix[18]
        );
        SkV4 v4 = {matrix[4], matrix[9], matrix[14], matrix[19]};
        return std::unique_ptr<GrFragmentProcessor>(new GrColorMatrixFragmentProcessor(
            std::move(inputFP), m44, v4, unpremulInput, clampRGBOutput, premulOutput));
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
    return Make(d->inputFP(), m, unpremul, clampRGB, premul);
}
