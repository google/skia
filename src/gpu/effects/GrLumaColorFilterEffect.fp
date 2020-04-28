/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

enum class OutputMode {
    kLumaAsAlpha,  //  half4(0, 0, 0, luma)
    kLumaAsRGB     //  half4(luma, luma, luma, alpha)
};

layout(key) in OutputMode mode;

@optimizationFlags {
    kConstantOutputForConstantInput_OptimizationFlag
}

@class {
    #include "include/private/SkColorData.h"

    SkPMColor4f constantOutputForConstantInput(const SkPMColor4f& input) const override {
        float luma = SK_ITU_BT709_LUM_COEFF_R * input.fR +
                     SK_ITU_BT709_LUM_COEFF_G * input.fG +
                     SK_ITU_BT709_LUM_COEFF_B * input.fB;
        luma = SkTPin(luma, 0.0f, 1.0f);
        switch (mode) {
            case OutputMode::kLumaAsAlpha:
                return { 0, 0, 0, luma };
            case OutputMode::kLumaAsRGB:
                return { luma, luma, luma, input.fA };
        }
        SkUNREACHABLE;
    }
}

void main() {
    const half3 SK_ITU_BT709_LUM_COEFF = half3(0.2126, 0.7152, 0.0722);
    half luma = saturate(dot(SK_ITU_BT709_LUM_COEFF, sk_InColor.rgb));
    @switch (mode) {
        case OutputMode::kLumaAsAlpha:
            sk_OutColor = half4(0, 0, 0, luma);
            break;
        case OutputMode::kLumaAsRGB:
            sk_OutColor = half4(luma, luma, luma, sk_InColor.a);
            break;
    }
}
