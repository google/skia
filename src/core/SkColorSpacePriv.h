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
#include "src/core/SkVM_fwd.h"

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

// NOTE: All of this logic is copied from skcms.cc, and needs to be kept in sync.

// Most transfer functions we work with are sRGBish.
// For exotic HDR transfer functions, we encode them using a tf.g that makes no sense,
// and repurpose the other fields to hold the parameters of the HDR functions.
enum TFKind { Bad_TF, sRGBish_TF, PQish_TF, HLGish_TF, HLGinvish_TF };

static inline TFKind classify_transfer_fn(const skcms_TransferFunction& tf) {
    if (tf.g < 0 && (int)tf.g == tf.g) {
        // TODO: sanity checks for PQ/HLG like we do for sRGBish.
        switch ((int)tf.g) {
            case -PQish_TF:     return PQish_TF;
            case -HLGish_TF:    return HLGish_TF;
            case -HLGinvish_TF: return HLGinvish_TF;
        }
        return Bad_TF;
    }

    // Basic sanity checks for sRGBish transfer functions.
    if (sk_float_isfinite(tf.a + tf.b + tf.c + tf.d + tf.e + tf.f + tf.g)
            // a,c,d,g should be non-negative to make any sense.
            && tf.a >= 0
            && tf.c >= 0
            && tf.d >= 0
            && tf.g >= 0
            // Raising a negative value to a fractional tf->g produces complex numbers.
            && tf.a * tf.d + tf.b >= 0) {
        return sRGBish_TF;
    }

    return Bad_TF;
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

skvm::Color sk_program_transfer_fn(skvm::Builder*, skvm::Uniforms*,
                                   const skcms_TransferFunction&, skvm::Color);

// Return raw pointers to commonly used SkColorSpaces.
// No need to ref/unref these, but if you do, do it in pairs.
SkColorSpace* sk_srgb_singleton();
SkColorSpace* sk_srgb_linear_singleton();

#endif  // SkColorSpacePriv_DEFINED
