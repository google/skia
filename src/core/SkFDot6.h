/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFDot6_DEFINED
#define SkFDot6_DEFINED

#include "SkFixed.h"
#include "SkScalar.h"
#include "SkMath.h"

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
    inline SkFDot6 SkIntToFDot6(S16CPU x) {
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

    if (a == (int16_t)a) {
        return SkLeftShift(a, 16) / b;
    } else {
        return SkFixedDiv(a, b);
    }
}

#include "SkFDot6Constants.h"

class QuickFDot6Inverse {
public:
    inline static SkFixed Lookup(SkFDot6 x) {
        SkASSERT(SkAbs32(x) < kInverseTableSize);
        return gFDot6INVERSE[kInverseTableSize + x];
    }
};

static inline SkFixed QuickSkFDot6Div(SkFDot6 a, SkFDot6 b) {
    const int kMinBits = 3;  // abs(b) should be at least (1 << kMinBits) for quick division
    const int kMaxBits = 31; // Number of bits available in signed int
    // Given abs(b) <= (1 << kMinBits), the inverse of abs(b) is at most 1 << (22 - kMinBits) in
    // SkFixed format. Hence abs(a) should be less than kMaxAbsA
    const int kMaxAbsA = 1 << (kMaxBits - (22 - kMinBits));
    SkFDot6 abs_a = SkAbs32(a);
    SkFDot6 abs_b = SkAbs32(b);
    if (abs_b >= (1 << kMinBits) && abs_b < kInverseTableSize && abs_a < kMaxAbsA) {
        SkASSERT((int64_t)a * QuickFDot6Inverse::Lookup(b) <= SK_MaxS32
                && (int64_t)a * QuickFDot6Inverse::Lookup(b) >= SK_MinS32);
        SkFixed ourAnswer = (a * QuickFDot6Inverse::Lookup(b)) >> 6;
        #ifdef SK_DEBUG
        SkFixed directAnswer = SkFDot6Div(a, b);
        SkASSERT(
            (directAnswer == 0 && ourAnswer == 0) ||
            SkFixedDiv(SkAbs32(directAnswer - ourAnswer), SkAbs32(directAnswer)) <= 1 << 10
        );
        #endif
        return ourAnswer;
    } else {
        return SkFDot6Div(a, b);
    }
}

#endif
