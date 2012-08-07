
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFDot6_DEFINED
#define SkFDot6_DEFINED

#include "SkMath.h"

typedef int32_t SkFDot6;

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
    SkASSERT((x << 10 >> 10) == x);

    return x << 10;
}

#ifdef SK_SCALAR_IS_FLOAT
    #define SkScalarToFDot6(x)  (SkFDot6)((x) * 64)
#else
    #define SkScalarToFDot6(x)  ((x) >> 10)
#endif

inline SkFixed SkFDot6Div(SkFDot6 a, SkFDot6 b) {
    SkASSERT(b != 0);

    if (a == (int16_t)a) {
        return (a << 16) / b;
    } else {
        return SkFixedDiv(a, b);
    }
}

#endif

