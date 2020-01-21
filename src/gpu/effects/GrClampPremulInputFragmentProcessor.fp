/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@optimizationFlags {
    kPreservesOpaqueInput_OptimizationFlag | kConstantOutputForConstantInput_OptimizationFlag
}

void main() {
    sk_OutColor = half4(clamp(sk_InColor.rgb, 0, sk_InColor.a), sk_InColor.a);
}

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        return {
            SkTPin(input.fR, 0.f, input.fA),
            SkTPin(input.fG, 0.f, input.fA),
            SkTPin(input.fB, 0.f, input.fA),
            input.fA
        };
    }
}
