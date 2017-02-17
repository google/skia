/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define SkColorSpacePrintf(...)

static inline bool color_space_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.01f;
}

static inline bool is_zero_to_one(float v) {
    return (0.0f <= v) && (v <= 1.0f);
}

static inline bool is_valid_transfer_fn(const SkColorSpaceTransferFn& coeffs) {
    if (SkScalarIsNaN(coeffs.fA) || SkScalarIsNaN(coeffs.fB) ||
        SkScalarIsNaN(coeffs.fC) || SkScalarIsNaN(coeffs.fD) ||
        SkScalarIsNaN(coeffs.fE) || SkScalarIsNaN(coeffs.fF) ||
        SkScalarIsNaN(coeffs.fG))
    {
        return false;
    }

    if (!is_zero_to_one(coeffs.fD)) {
        return false;
    }

    if (coeffs.fD == 0.0f) {
        // Y = (aX + b)^g + c  for always
        if (0.0f == coeffs.fA || 0.0f == coeffs.fG) {
            SkColorSpacePrintf("A or G is zero, constant transfer function "
                               "is nonsense");
            return false;
        }
    }

    if (coeffs.fD >= 1.0f) {
        // Y = eX + f          for always
        if (0.0f == coeffs.fE) {
            SkColorSpacePrintf("E is zero, constant transfer function is "
                               "nonsense");
            return false;
        }
    }

    if ((0.0f == coeffs.fA || 0.0f == coeffs.fG) && 0.0f == coeffs.fC) {
        SkColorSpacePrintf("A or G, and E are zero, constant transfer function "
                           "is nonsense");
        return false;
    }

    if (coeffs.fC < 0.0f) {
        SkColorSpacePrintf("Transfer function must be increasing");
        return false;
    }

    if (coeffs.fA < 0.0f || coeffs.fG < 0.0f) {
        SkColorSpacePrintf("Transfer function must be positive or increasing");
        return false;
    }

    return true;
}

static inline bool is_almost_srgb(const SkColorSpaceTransferFn& coeffs) {
    return color_space_almost_equal(1.0f / 1.055f,   coeffs.fA) &&
           color_space_almost_equal(0.055f / 1.055f, coeffs.fB) &&
           color_space_almost_equal(1.0f / 12.92f,   coeffs.fC) &&
           color_space_almost_equal(0.04045f,        coeffs.fD) &&
           color_space_almost_equal(0.00000f,        coeffs.fE) &&
           color_space_almost_equal(0.00000f,        coeffs.fF) &&
           color_space_almost_equal(2.40000f,        coeffs.fG);
}

static inline bool is_almost_2dot2(const SkColorSpaceTransferFn& coeffs) {
    return color_space_almost_equal(1.0f, coeffs.fA) &&
           color_space_almost_equal(0.0f, coeffs.fB) &&
           color_space_almost_equal(0.0f, coeffs.fC) &&
           color_space_almost_equal(0.0f, coeffs.fD) &&
           color_space_almost_equal(0.0f, coeffs.fE) &&
           color_space_almost_equal(0.0f, coeffs.fF) &&
           color_space_almost_equal(2.2f, coeffs.fG);
}
