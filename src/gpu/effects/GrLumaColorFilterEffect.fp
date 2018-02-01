/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

@class {
    #include "SkColorData.h"

    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        float luma = SK_ITU_BT709_LUM_COEFF_R * input.fRGBA[0] +
                     SK_ITU_BT709_LUM_COEFF_G * input.fRGBA[1] +
                     SK_ITU_BT709_LUM_COEFF_B * input.fRGBA[2];
        return GrColor4f(0, 0, 0, luma);
    }
}

void main() {
    const half3 SK_ITU_BT709_LUM_COEFF = half3(0.2126, 0.7152, 0.0722);
    half luma = dot(SK_ITU_BT709_LUM_COEFF, sk_InColor.rgb);
    sk_OutColor = half4(0, 0, 0, luma);
}