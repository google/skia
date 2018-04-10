/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

// skcms.h contains the entire public API for skcms.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// A row-major 3x3 matrix (ie vals[row][col])
typedef struct {
    float vals[3][3];
} skcms_Matrix3x3;

// A row-major 3x4 matrix (ie vals[row][col])
typedef struct {
    float vals[3][4];
} skcms_Matrix3x4;

// A transfer function mapping encoded values to linear values,
// represented by this 7-parameter piecewise function:
//
//   linear = sign(encoded) *  (c*|encoded| + f)       , 0 <= |encoded| < d
//          = sign(encoded) * ((a*|encoded| + b)^g + e), d <= |encoded|
//
// (A simple gamma transfer function sets g to gamma and a to 1.)
typedef struct {
    float g, a,b,c,d,e,f;
} skcms_TransferFunction;

// Unified representation of 'curv' or 'para' tag data, or a 1D table from 'mft1' or 'mft2'
typedef struct {
    union {
        struct {
            uint32_t alias_of_table_entries;
            skcms_TransferFunction parametric;
        };
        struct {
            uint32_t table_entries;
            const uint8_t* table_8;
            const uint8_t* table_16;
        };
    };
} skcms_Curve;

typedef struct {
    // Optional: N 1D curves, followed by an N-dimensional CLUT.
    // If input_channels == 0, these curves and CLUT are skipped,
    // Otherwise, input_channels must be in [1, 4].
    uint32_t        input_channels;
    skcms_Curve     input_curves[4];
    uint8_t         grid_points[4];
    const uint8_t*  grid_8;
    const uint8_t*  grid_16;

    // Optional: 3 1D curves, followed by a color matrix.
    // If matrix_channels == 0, these curves and matrix are skipped,
    // Otherwise, matrix_channels must be 3.
    uint32_t        matrix_channels;
    skcms_Curve     matrix_curves[3];
    skcms_Matrix3x4 matrix;

    // Required: 3 1D curves. Always present, and output_channels must be 3.
    uint32_t        output_channels;
    skcms_Curve     output_curves[3];
} skcms_A2B;

typedef struct {
    const uint8_t* buffer;

    uint32_t size;
    uint32_t data_color_space;
    uint32_t pcs;
    uint32_t tag_count;

    // skcms_Parse() will set commonly-used fields for you when possible:

    // If we can parse red, green and blue transfer curves from the profile,
    // trc will be set to those three curves, and has_trc will be true.
    bool                   has_trc;
    skcms_Curve            trc[3];

    // If this profile's gamut can be represented by a 3x3 transform to XYZD50,
    // skcms_Parse() sets toXYZD50 to that transform and has_toXYZD50 to true.
    bool                   has_toXYZD50;
    skcms_Matrix3x3        toXYZD50;

    // If the profile has a valid A2B0 tag, skcms_Parse() sets A2B to that data,
    // and has_A2B to true.
    bool                   has_A2B;
    skcms_A2B              A2B;
} skcms_ICCProfile;

// Parse an ICC profile and return true if possible, otherwise return false.
// The buffer is not copied, it must remain valid as long as the skcms_ICCProfile
// will be used.
bool skcms_Parse(const void*, size_t, skcms_ICCProfile*);

bool skcms_ApproximateCurve(const skcms_Curve* curve, skcms_TransferFunction* approx,
                            float* max_error);

// A specialized approximation for transfer functions with gamma between 1 and 3.
//     f(x) = Ax^3 + Bx^2 + (1-A-B)x
typedef struct {
    float A,B;
} skcms_TF13;

bool skcms_ApproximateCurve13(const skcms_Curve* curve, skcms_TF13* approx, float* max_error);

typedef struct {
    uint32_t       signature;
    uint32_t       type;
    uint32_t       size;
    const uint8_t* buf;
} skcms_ICCTag;

void skcms_GetTagByIndex    (const skcms_ICCProfile*, uint32_t idx, skcms_ICCTag*);
bool skcms_GetTagBySignature(const skcms_ICCProfile*, uint32_t sig, skcms_ICCTag*);

typedef enum {
    skcms_PixelFormat_RGB_565,
    skcms_PixelFormat_BGR_565,

    skcms_PixelFormat_RGB_888,
    skcms_PixelFormat_BGR_888,
    skcms_PixelFormat_RGBA_8888,
    skcms_PixelFormat_BGRA_8888,

    skcms_PixelFormat_RGBA_1010102,
    skcms_PixelFormat_BGRA_1010102,

    skcms_PixelFormat_RGB_161616,     // Big-endian.  Pointers must be 16-bit aligned.
    skcms_PixelFormat_BGR_161616,
    skcms_PixelFormat_RGBA_16161616,
    skcms_PixelFormat_BGRA_16161616,

    skcms_PixelFormat_RGB_hhh,        // 1-5-10 half-precision float.
    skcms_PixelFormat_BGR_hhh,        // Pointers must be 16-bit aligned.
    skcms_PixelFormat_RGBA_hhhh,
    skcms_PixelFormat_BGRA_hhhh,

    skcms_PixelFormat_RGB_fff,        // 1-8-23 single-precision float (the normal kind).
    skcms_PixelFormat_BGR_fff,        // Pointers must be 32-bit aligned.
    skcms_PixelFormat_RGBA_ffff,
    skcms_PixelFormat_BGRA_ffff,
} skcms_PixelFormat;

// We always store any alpha channel linearly.  In the chart below, tf-1() is the inverse
// transfer function for the given color profile (applying the transfer function linearizes).

// We treat opaque as a strong requirement, not just a performance hint: we will ignore
// any source alpha and treat it as 1.0, and will make sure that any destination alpha
// channel is filled with the equivalent of 1.0.

// When premultiplying and/or using a non-linear transfer function, it's important
// that we know the order the operations are applied.  If you're used to working
// with non-color-managed drawing systems, PremulAsEncoded is probably the "premul"
// you're looking for; if you want linear blending, PremulLinear is the choice for you.

typedef enum {
    skcms_AlphaFormat_Opaque,          // alpha is always opaque
                                       //   tf-1(r),   tf-1(g),   tf-1(b),   1.0
    skcms_AlphaFormat_Unpremul,        // alpha and color are unassociated
                                       //   tf-1(r),   tf-1(g),   tf-1(b),   a
    skcms_AlphaFormat_PremulAsEncoded, // premultiplied while encoded
                                       //   tf-1(r)*a, tf-1(g)*a, tf-1(b)*a, a
    skcms_AlphaFormat_PremulLinear,    // premultiplied while linear
                                       //   tf-1(r*a), tf-1(g*a), tf-1(b*a), a
} skcms_AlphaFormat;

// Convert npixels pixels from src format and color profile to dst format and color profile
// and return true, otherwise return false.  It is safe to alias dst == src if dstFmt == srcFmt.
bool skcms_Transform(const void*             src,
                     skcms_PixelFormat       srcFmt,
                     skcms_AlphaFormat       srcAlpha,
                     const skcms_ICCProfile* srcProfile,
                     void*                   dst,
                     skcms_PixelFormat       dstFmt,
                     skcms_AlphaFormat       dstAlpha,
                     const skcms_ICCProfile* dstProfile,
                     size_t                  npixels);

#ifdef __cplusplus
}
#endif
