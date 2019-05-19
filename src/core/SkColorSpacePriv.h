/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorSpacePriv_DEFINED
#define SkColorSpacePriv_DEFINED

#include <math.h>

#include "include/core/SkColorSpace.h"
#include "include/private/SkFixed.h"

#define SkColorSpacePrintf(...)

// A gamut narrower than sRGB, useful for testing.
static constexpr skcms_Matrix3x3 gNarrow_toXYZD50 = {{
    { 0.190974f,  0.404865f,  0.368380f },
    { 0.114746f,  0.582937f,  0.302318f },
    { 0.032925f,  0.153615f,  0.638669f },
}};

static inline bool color_space_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.01f;
}

// Let's use a stricter version for transfer functions.  Worst case, these are encoded
// in ICC format, which offers 16-bits of fractional precision.
static inline bool transfer_fn_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.001f;
}

static inline bool is_valid_transfer_fn(const skcms_TransferFunction& coeffs) {
    if (SkScalarIsNaN(coeffs.a) || SkScalarIsNaN(coeffs.b) ||
        SkScalarIsNaN(coeffs.c) || SkScalarIsNaN(coeffs.d) ||
        SkScalarIsNaN(coeffs.e) || SkScalarIsNaN(coeffs.f) ||
        SkScalarIsNaN(coeffs.g))
    {
        return false;
    }

    if (coeffs.d < 0.0f) {
        return false;
    }

    if (coeffs.d == 0.0f) {
        // Y = (aX + b)^g + e  for always
        if (0.0f == coeffs.a || 0.0f == coeffs.g) {
            SkColorSpacePrintf("A or G is zero, constant transfer function "
                               "is nonsense");
            return false;
        }
    }

    if (coeffs.d >= 1.0f) {
        // Y = cX + f          for always
        if (0.0f == coeffs.c) {
            SkColorSpacePrintf("C is zero, constant transfer function is "
                               "nonsense");
            return false;
        }
    }

    if ((0.0f == coeffs.a || 0.0f == coeffs.g) && 0.0f == coeffs.c) {
        SkColorSpacePrintf("A or G, and C are zero, constant transfer function "
                           "is nonsense");
        return false;
    }

    if (coeffs.c < 0.0f) {
        SkColorSpacePrintf("Transfer function must be increasing");
        return false;
    }

    if (coeffs.a < 0.0f || coeffs.g < 0.0f) {
        SkColorSpacePrintf("Transfer function must be positive or increasing");
        return false;
    }

    return true;
}

static inline bool is_almost_srgb(const skcms_TransferFunction& coeffs) {
    return transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.a, coeffs.a) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.b, coeffs.b) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.c, coeffs.c) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.d, coeffs.d) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.e, coeffs.e) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.f, coeffs.f) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.g, coeffs.g);
}

static inline bool is_almost_2dot2(const skcms_TransferFunction& coeffs) {
    return transfer_fn_almost_equal(1.0f, coeffs.a) &&
           transfer_fn_almost_equal(0.0f, coeffs.b) &&
           transfer_fn_almost_equal(0.0f, coeffs.e) &&
           transfer_fn_almost_equal(2.2f, coeffs.g) &&
           coeffs.d <= 0.0f;
}

static inline bool is_almost_linear(const skcms_TransferFunction& coeffs) {
    // OutputVal = InputVal ^ 1.0f
    const bool linearExp =
            transfer_fn_almost_equal(1.0f, coeffs.a) &&
            transfer_fn_almost_equal(0.0f, coeffs.b) &&
            transfer_fn_almost_equal(0.0f, coeffs.e) &&
            transfer_fn_almost_equal(1.0f, coeffs.g) &&
            coeffs.d <= 0.0f;

    // OutputVal = 1.0f * InputVal
    const bool linearFn =
            transfer_fn_almost_equal(1.0f, coeffs.c) &&
            transfer_fn_almost_equal(0.0f, coeffs.f) &&
            coeffs.d >= 1.0f;

    return linearExp || linearFn;
}

// Return raw pointers to commonly used SkColorSpaces.
// No need to ref/unref these, but if you do, do it in pairs.
SkColorSpace* sk_srgb_singleton();
SkColorSpace* sk_srgb_linear_singleton();

#endif  // SkColorSpacePriv_DEFINED
