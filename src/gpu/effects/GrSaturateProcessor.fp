/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


@optimizationFlags {
    kConstantOutputForConstantInput_OptimizationFlag |
    kPreservesOpaqueInput_OptimizationFlag
}

void main() { sk_OutColor = saturate(sk_InColor); }

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        return {SkTPin(input.fR, 0.f, 1.f),
                SkTPin(input.fG, 0.f, 1.f),
                SkTPin(input.fB, 0.f, 1.f),
                SkTPin(input.fA, 0.f, 1.f)};
    }
}

@test(d) { return GrSaturateProcessor::Make(); }
