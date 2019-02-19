/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Null FP - passes the input color through.

@class {
    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        return input;
    }
}

@optimizationFlags { kAll_OptimizationFlags }

void main() {
    sk_OutColor = sk_InColor;
}
