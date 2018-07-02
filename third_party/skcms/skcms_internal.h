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

#ifdef __cplusplus
extern "C" {
#endif

// ~~~~ General Helper Macros ~~~~
    #define ARRAY_COUNT(arr) (int)(sizeof((arr)) / sizeof(*(arr)))

// ~~~~ skcms_TransferFunction ~~~~
    float skcms_TransferFunction_eval  (const skcms_TransferFunction*, float);
    bool  skcms_TransferFunction_invert(const skcms_TransferFunction*, skcms_TransferFunction*);

// ~~~~ skcms_ICCProfile ~~~~
    bool skcms_GetCHAD(const skcms_ICCProfile* profile, skcms_Matrix3x3* m);

    // 252 of a random shuffle of all possible bytes.
    // 252 is evenly divisible by 3 and 4.  Only 192, 10, 241, and 43 are missing.
    // Used for ICC profile equivalence testing.
    extern const uint8_t skcms_252_random_bytes[252];

// ~~~~ Linear Algebra ~~~~
    // It is _not_ safe to alias the pointers to invert in-place.
    bool skcms_Matrix3x3_invert(const skcms_Matrix3x3*, skcms_Matrix3x3*);
    skcms_Matrix3x3 skcms_Matrix3x3_concat(const skcms_Matrix3x3* A, const skcms_Matrix3x3* B);

// ~~~~ Portable Math ~~~~
    static inline float floorf_(float x) {
        float roundtrip = (float)((int)x);
        return roundtrip > x ? roundtrip - 1 : roundtrip;
    }
    static inline float fabsf_(float x) { return x < 0 ? -x : x; }
    float powf_(float, float);

#ifdef __cplusplus
}
#endif
