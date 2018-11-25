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
    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        return input.unpremul();
    }
}