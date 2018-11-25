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
    sk_OutColor.rgb *= sk_InColor.a;
}

@class {
    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        return input.premul();
    }
}