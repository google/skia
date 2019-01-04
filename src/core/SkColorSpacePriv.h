/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkColorSpacePriv_DEFINED
#define SkColorSpacePriv_DEFINED

#include <math.h>

#include "SkColorSpace.h"
#include "SkFixed.h"

#define SkColorSpacePrintf(...)

// A gamut narrower than sRGB, useful for testing.
static constexpr skcms_Matrix3x3 gNarrow_toXYZD50 = {{
    { 0.190974f,  0.404865f,  0.368380f },
    { 0.114746f,  0.582937f,  0.302318f },
    { 0.032925f,  0.153615f,  0.638669f },
}};

static inline void to_xyz_d50(SkMatrix44* toXYZD50, SkColorSpace::Gamut gamut) {
    switch (gamut) {
        case SkColorSpace::kSRGB_Gamut:
            toXYZD50->set3x3RowMajorf(&SkNamedGamut::kSRGB.vals[0][0]);
            break;
        case SkColorSpace::kAdobeRGB_Gamut:
            toXYZD50->set3x3RowMajorf(&SkNamedGamut::kAdobeRGB.vals[0][0]);
            break;
        case SkColorSpace::kDCIP3_D65_Gamut:
            toXYZD50->set3x3RowMajorf(&SkNamedGamut::kDCIP3.vals[0][0]);
            break;
        case SkColorSpace::kRec2020_Gamut:
            toXYZD50->set3x3RowMajorf(&SkNamedGamut::kRec2020.vals[0][0]);
            break;
    }
}

static inline bool color_space_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.01f;
}

// Let's use a stricter version for transfer functions.  Worst case, these are encoded
// in ICC format, which offers 16-bits of fractional precision.
static inline bool transfer_fn_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.001f;
}

static inline bool is_zero_to_one(float v) {
    // Because we allow a value just barely larger than 1, the client can use an
    // entirely linear transfer function.
    return (0.0f <= v) && (v <= nextafterf(1.0f, 2.0f));
}

static inline bool is_valid_transfer_fn(const SkColorSpaceTransferFn& coeffs) {
    if (SkScalarIsNaN(coeffs.fA) || SkScalarIsNaN(coeffs.fB) ||
        SkScalarIsNaN(coeffs.fC) || SkScalarIsNaN(coeffs.fD) ||
        SkScalarIsNaN(coeffs.fE) || SkScalarIsNaN(coeffs.fF) ||
        SkScalarIsNaN(coeffs.fG))
    {
        return false;
    }

    if (coeffs.fD < 0.0f) {
        return false;
    }

    if (coeffs.fD == 0.0f) {
        // Y = (aX + b)^g + e  for always
        if (0.0f == coeffs.fA || 0.0f == coeffs.fG) {
            SkColorSpacePrintf("A or G is zero, constant transfer function "
                               "is nonsense");
            return false;
        }
    }

    if (coeffs.fD >= 1.0f) {
        // Y = cX + f          for always
        if (0.0f == coeffs.fC) {
            SkColorSpacePrintf("C is zero, constant transfer function is "
                               "nonsense");
            return false;
        }
    }

    if ((0.0f == coeffs.fA || 0.0f == coeffs.fG) && 0.0f == coeffs.fC) {
        SkColorSpacePrintf("A or G, and C are zero, constant transfer function "
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
    return transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.a, coeffs.fA) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.b, coeffs.fB) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.c, coeffs.fC) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.d, coeffs.fD) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.e, coeffs.fE) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.f, coeffs.fF) &&
           transfer_fn_almost_equal(SkNamedTransferFn::kSRGB.g, coeffs.fG);
}

static inline bool is_almost_2dot2(const SkColorSpaceTransferFn& coeffs) {
    return transfer_fn_almost_equal(1.0f, coeffs.fA) &&
           transfer_fn_almost_equal(0.0f, coeffs.fB) &&
           transfer_fn_almost_equal(0.0f, coeffs.fE) &&
           transfer_fn_almost_equal(2.2f, coeffs.fG) &&
           coeffs.fD <= 0.0f;
}

static inline bool is_almost_linear(const SkColorSpaceTransferFn& coeffs) {
    // OutputVal = InputVal ^ 1.0f
    const bool linearExp =
            transfer_fn_almost_equal(1.0f, coeffs.fA) &&
            transfer_fn_almost_equal(0.0f, coeffs.fB) &&
            transfer_fn_almost_equal(0.0f, coeffs.fE) &&
            transfer_fn_almost_equal(1.0f, coeffs.fG) &&
            coeffs.fD <= 0.0f;

    // OutputVal = 1.0f * InputVal
    const bool linearFn =
            transfer_fn_almost_equal(1.0f, coeffs.fC) &&
            transfer_fn_almost_equal(0.0f, coeffs.fF) &&
            coeffs.fD >= 1.0f;

    return linearExp || linearFn;
}

// Return raw pointers to commonly used SkColorSpaces.
// No need to ref/unref these, but if you do, do it in pairs.
SkColorSpace* sk_srgb_singleton();
SkColorSpace* sk_srgb_linear_singleton();

#endif  // SkColorSpacePriv_DEFINED
