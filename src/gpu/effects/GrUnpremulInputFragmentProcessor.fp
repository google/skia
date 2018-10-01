/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@optimizationFlags {
    kPreservesOpaqueInput_OptimizationFlag | kConstantOutputForConstantInput_OptimizationFlag
}

void main() {
    sk_OutColor = sk_InColor;
    half invAlpha = sk_InColor.a <= 0 ? 0 : 1 / sk_InColor.a;
    sk_OutColor.rgb *= invAlpha;
}

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        SkColor4f upm = input.unpremul();
        return { upm.fR, upm.fG, upm.fB, upm.fA };
    }
}