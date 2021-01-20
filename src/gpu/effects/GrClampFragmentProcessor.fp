/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor inputFP;
layout(key) in bool clampToPremul;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
    (kConstantOutputForConstantInput_OptimizationFlag |
     kPreservesOpaqueInput_OptimizationFlag)
}

half4 clampedPM(half4 inputColor) {
    half alpha = saturate(inputColor.a);
    return half4(clamp(inputColor.rgb, 0, alpha), alpha);
}

half4 main() {
    half4 inputColor = sample(inputFP);
    return clampToPremul ? clampedPM(inputColor) : saturate(inputColor);
}

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
        SkPMColor4f input = ConstantOutputForConstantInput(this->childProcessor(0), inColor);
        float clampedAlpha = SkTPin(input.fA, 0.f, 1.f);
        float clampVal = clampToPremul ? clampedAlpha : 1.f;
        return {SkTPin(input.fR, 0.f, clampVal),
                SkTPin(input.fG, 0.f, clampVal),
                SkTPin(input.fB, 0.f, clampVal),
                clampedAlpha};
    }
}

@test(d) {
    return GrClampFragmentProcessor::Make(d->inputFP(), d->fRandom->nextBool());
}
