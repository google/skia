/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

// skcms_internal.h contains APIs shared by skcms' internals and its test tools.
// Please don't use this header from outside the skcms repo.

#include "skcms.h"
#include <stdbool.h>
#include <stdint.h>

#if defined(__cpluscplus)
extern "C" {
#endif

// ~~~~ General Helper Macros ~~~~

    // sizeof(x) will return size_t, which is 32-bit on some machines and 64-bit on others.
    // We have better testing on 64-bit machines, so force 32-bit machines to behave like 64-bit.
    #define SAFE_SIZEOF(x) ((uint64_t)sizeof(x))

    // Please do not use sizeof() directly, and size_t only when required.
    // (We have no way of enforcing these requests...)

    #define ARRAY_COUNT(arr) (int)(SAFE_SIZEOF((arr)) / SAFE_SIZEOF(*(arr)))


// ~~~~ skcms_Curve ~~~~

    // Evaluate an skcms_Curve at x.
    float skcms_eval_curve(const skcms_Curve*, float x);
    float skcms_MaxRoundtripError(const skcms_Curve*, const skcms_TransferFunction*);


// ~~~~ skcms_TransferFunction ~~~~
    bool skcms_TransferFunction_isValid(const skcms_TransferFunction*);

    float skcms_TransferFunction_eval(const skcms_TransferFunction*, float);

    bool skcms_TransferFunction_invert(const skcms_TransferFunction*, skcms_TransferFunction*);

    // Fit c,d,f parameters of an skcms_TransferFunction to the first 2 ≤ L ≤ N
    // evenly-spaced points on an skcms_Curve within a given tolerance, returning L.
    int skcms_fit_linear(const skcms_Curve*, int N, float tol, float* c, float* d, float* f);


// ~~~~ skcms_ICCProfile ~~~~

    bool skcms_GetCHAD(const skcms_ICCProfile* profile, skcms_Matrix3x3* m);

    // 252 of a random shuffle of all possible bytes.
    // 252 is evenly divisible by 3 and 4.  Only 192, 10, 241, and 43 are missing.
    // Used for ICC profile equivalence testing.
    extern const uint8_t skcms_252_random_bytes[252];


// ~~~~ Linear Algebra ~~~~

    typedef struct { float vals[3]; } skcms_Vector3;

    // It is _not_ safe to alias the pointers to invert in-place.
    bool skcms_Matrix3x3_invert(const skcms_Matrix3x3*, skcms_Matrix3x3*);
    skcms_Matrix3x3 skcms_Matrix3x3_concat(const skcms_Matrix3x3* A, const skcms_Matrix3x3* B);

    skcms_Vector3 skcms_MV_mul(const skcms_Matrix3x3*, const skcms_Vector3*);


// ~~~~ Portable Math ~~~~

    static const union {
        uint32_t bits;
        float    f;
    } inf_ = { 0x7f800000 };

    #define INFINITY_ inf_.f

    static inline float floorf_(float x) {
        float roundtrip = (float)((int)x);
        return roundtrip > x ? roundtrip - 1 : roundtrip;
    }

    static inline float fmaxf_(float x, float y) { return x > y ? x : y; }
    static inline float fminf_(float x, float y) { return x < y ? x : y; }
    static inline float fabsf_(float x) { return x < 0 ? -x : x; }

    float log2f_(float);
    float exp2f_(float);
    float powf_(float, float);

    static inline bool isfinitef_(float x) { return 0 == x*0; }


// ~~~~ Transform ~~~~

    #define FOREACH_Op(M) \
        M(noop)           \
        M(load_a8)        \
        M(load_g8)        \
        M(load_4444)      \
        M(load_565)       \
        M(load_888)       \
        M(load_8888)      \
        M(load_1010102)   \
        M(load_161616)    \
        M(load_16161616)  \
        M(load_hhh)       \
        M(load_hhhh)      \
        M(load_fff)       \
        M(load_ffff)      \
        M(swap_rb)        \
        M(clamp)          \
        M(invert)         \
        M(force_opaque)   \
        M(premul)         \
        M(unpremul)       \
        M(matrix_3x3)     \
        M(matrix_3x4)     \
        M(lab_to_xyz)     \
        M(tf_r)           \
        M(tf_g)           \
        M(tf_b)           \
        M(tf_a)           \
        M(table_8_r)      \
        M(table_8_g)      \
        M(table_8_b)      \
        M(table_8_a)      \
        M(table_16_r)     \
        M(table_16_g)     \
        M(table_16_b)     \
        M(table_16_a)     \
        M(clut_3D_8)      \
        M(clut_3D_16)     \
        M(clut_4D_8)      \
        M(clut_4D_16)     \
        M(store_a8)       \
        M(store_g8)       \
        M(store_4444)     \
        M(store_565)      \
        M(store_888)      \
        M(store_8888)     \
        M(store_1010102)  \
        M(store_161616)   \
        M(store_16161616) \
        M(store_hhh)      \
        M(store_hhhh)     \
        M(store_fff)      \
        M(store_ffff)

    typedef enum {
        #define M(op) Op_##op,
        FOREACH_Op(M)
        #undef M
    } Op;

#if defined(__cpluscplus)
}  // extern "C"
#endif
