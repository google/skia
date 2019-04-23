/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkColorMatrixFilter.h"

static SkScalar byte_to_scale(U8CPU byte) {
    if (0xFF == byte) {
        // want to get this exact
        return 1;
    } else {
        return byte * 0.00392156862745f;
    }
}

sk_sp<SkColorFilter> SkColorMatrixFilter::MakeLightingFilter(SkColor mul, SkColor add) {
    const SkColor opaqueAlphaMask = SK_ColorBLACK;
    // omit the alpha and compare only the RGB values
    if (0 == (add & ~opaqueAlphaMask)) {
        return SkColorFilters::Blend(mul | opaqueAlphaMask, SkBlendMode::kModulate);
    }

    SkColorMatrix matrix;
    matrix.setScale(byte_to_scale(SkColorGetR(mul)),
                    byte_to_scale(SkColorGetG(mul)),
                    byte_to_scale(SkColorGetB(mul)),
                    1);
    matrix.postTranslate255(SkIntToScalar(SkColorGetR(add)),
                         SkIntToScalar(SkColorGetG(add)),
                         SkIntToScalar(SkColorGetB(add)),
                         0);
    return SkColorFilters::Matrix(matrix);
}
