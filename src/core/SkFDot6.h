/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFDot6_DEFINED
#define SkFDot6_DEFINED

#include "include/core/SkMath.h"
#include "include/core/SkScalar.h"
#include "include/private/SkFixed.h"
#include "include/private/SkTo.h"

typedef int32_t SkFDot6;

/* This uses the magic number approach suggested here:
 * http://stereopsis.com/sree/fpu2006.html and used in
 * _cairo_fixed_from_double. It does banker's rounding
 * (i.e. round to nearest even)
 */
inline SkFDot6 SkScalarRoundToFDot6(SkScalar x, int shift = 0)
{
    union {
        double  fDouble;
        int32_t fBits[2];
    } tmp;
    int fractionalBits = 6 + shift;
    double magic = (1LL << (52 - (fractionalBits))) * 1.5;

    tmp.fDouble = SkScalarToDouble(x) + magic;
#ifdef SK_CPU_BENDIAN
    return tmp.fBits[1];
#else
    return tmp.fBits[0];
#endif
}

#define SK_FDot6One         (64)
#define SK_FDot6Half        (32)

#ifdef SK_DEBUG
    inline SkFDot6 SkIntToFDot6(int x) {
        SkASSERT(SkToS16(x) == x);
        return x << 6;
    }
#else
    #define SkIntToFDot6(x) ((x) << 6)
#endif

#define SkFDot6Floor(x)     ((x) >> 6)
#define SkFDot6Ceil(x)      (((x) + 63) >> 6)
#define SkFDot6Round(x)     (((x) + 32) >> 6)

#define SkFixedToFDot6(x)   ((x) >> 10)

inline SkFixed SkFDot6ToFixed(SkFDot6 x) {
    SkASSERT((SkLeftShift(x, 10) >> 10) == x);

    return SkLeftShift(x, 10);
}

#define SkScalarToFDot6(x)  (SkFDot6)((x) * 64)
#define SkFDot6ToScalar(x)  ((SkScalar)(x) * 0.015625f)
#define SkFDot6ToFloat      SkFDot6ToScalar

inline SkFixed SkFDot6Div(SkFDot6 a, SkFDot6 b) {
    SkASSERT(b != 0);

    if (SkTFitsIn<int16_t>(a)) {
        return SkLeftShift(a, 16) / b;
    } else {
        return SkFixedDiv(a, b);
    }
}

#endif
