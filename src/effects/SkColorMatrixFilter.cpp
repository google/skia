/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkColorMatrixFilter.h"

static SkScalar byte_to_unit_float(U8CPU byte) {
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
    matrix.setScale(byte_to_unit_float(SkColorGetR(mul)),
                    byte_to_unit_float(SkColorGetG(mul)),
                    byte_to_unit_float(SkColorGetB(mul)),
                    1);
    matrix.postTranslate(byte_to_unit_float(SkColorGetR(add)),
                         byte_to_unit_float(SkColorGetG(add)),
                         byte_to_unit_float(SkColorGetB(add)),
                         0);
    return SkColorFilters::Matrix(matrix);
}
