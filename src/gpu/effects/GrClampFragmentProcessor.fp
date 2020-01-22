/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

layout(key) in bool clampToPremul;

@optimizationFlags {
    kConstantOutputForConstantInput_OptimizationFlag |
    kPreservesOpaqueInput_OptimizationFlag
}

void main() {
    @if (clampToPremul) {
        half alpha = saturate(sk_InColor.a);
        sk_OutColor = half4(clamp(sk_InColor.rgb, 0, alpha), alpha);
    } else {
        sk_OutColor = saturate(sk_InColor);
    }
}

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        float clampedAlpha = SkTPin(input.fA, 0.f, 1.f);
        float clampVal = clampToPremul ? clampedAlpha : 1.f;
        return {SkTPin(input.fR, 0.f, clampVal),
                SkTPin(input.fG, 0.f, clampVal),
                SkTPin(input.fB, 0.f, clampVal),
                clampedAlpha};
    }
}

@test(d) {
    return GrClampFragmentProcessor::Make(d->fRandom->nextBool());
}
