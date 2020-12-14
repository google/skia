/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor? inputFP;

@optimizationFlags {
    (inputFP ? ProcessorOptimizationFlags(inputFP.get()) : kAll_OptimizationFlags) &
            kConstantOutputForConstantInput_OptimizationFlag
}

@class {
    #include "include/private/SkColorData.h"

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& inColor) const override {
        SkPMColor4f input = ConstantOutputForConstantInput(this->childProcessor(0), inColor);
        float luma = SK_ITU_BT709_LUM_COEFF_R * input.fR +
                     SK_ITU_BT709_LUM_COEFF_G * input.fG +
                     SK_ITU_BT709_LUM_COEFF_B * input.fB;
        return { 0, 0, 0, SkTPin(luma, 0.0f, 1.0f) };
    }
}

half4 main() {
    half4 inputColor = sample(inputFP);
    const half3 SK_ITU_BT709_LUM_COEFF = half3(0.2126, 0.7152, 0.0722);
    half luma = saturate(dot(SK_ITU_BT709_LUM_COEFF, inputColor.rgb));
    return luma.000x;
}
