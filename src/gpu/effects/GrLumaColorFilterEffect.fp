/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@optimizationFlags {
    kConstantOutputForConstantInput_OptimizationFlag
}

@class {
    #include "include/private/SkColorData.h"

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        float luma = SK_ITU_BT709_LUM_COEFF_R * input.fR +
                     SK_ITU_BT709_LUM_COEFF_G * input.fG +
                     SK_ITU_BT709_LUM_COEFF_B * input.fB;
        return { 0, 0, 0, SkTPin(luma, 0.0f, 1.0f) };
    }
}

void main() {
    const half3 SK_ITU_BT709_LUM_COEFF = half3(0.2126, 0.7152, 0.0722);
    half luma = saturate(dot(SK_ITU_BT709_LUM_COEFF, sk_InColor.rgb));
    sk_OutColor = half4(0, 0, 0, luma);
}
