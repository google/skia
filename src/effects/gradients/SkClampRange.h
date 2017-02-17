/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkClampRange_DEFINED
#define SkClampRange_DEFINED

#include "SkFixed.h"
#include "SkScalar.h"

#define SkGradFixed               SkFixed3232

// We want the largest 32.32 value representable as a float. (float)0x7FFFFFFF
// becomes too big, due to limited mantissa on the float and its rounding rules, so
// we have to manually compute the next smaller value (aka nextafter).

// #define SkGradFixedMaxScalar nextafterf(SkFixed3232ToFloat(SkFixed3232Max), 0)
// #define SkGradFixedMinScalar nextafterf(SkFixed3232ToFloat(SkFixed3232Min), 0)
#define SkGradFixedMaxScalar      ( 2147483520.0f)
#define SkGradFixedMinScalar      (-2147483520.0f)
#define SkScalarPinToGradFixed(x) SkScalarToFixed3232(SkTPin(x,                   \
                                                             SkGradFixedMinScalar,\
                                                             SkGradFixedMaxScalar))
#define SkFixedToGradFixed(x)     SkFixedToFixed3232(x)
#define SkGradFixedToFixed(x)     (SkFixed)((x) >> 16)
#define kFracMax_SkGradFixed      0xFFFFFFFFLL

/**
 *  Iteration fixed fx by dx, clamping as you go to [0..kFracMax_SkGradFixed], this class
 *  computes the (up to) 3 spans there are:
 *
 *  range0: use constant value V0
 *  range1: iterate as usual fx += dx
 *  range2: use constant value V1
 */
struct SkClampRange {
    int fCount0;    // count for fV0
    int fCount1;    // count for interpolating (fV0...fV1)
    int fCount2;    // count for fV1
    SkGradFixed fFx1;   // initial fx value for the fCount1 range.
                    // only valid if fCount1 > 0
    int fV0, fV1;

    void init(SkGradFixed fx, SkGradFixed dx, int count, int v0, int v1);

    void validate(int count) const {
#ifdef SK_DEBUG
        SkASSERT(fCount0 >= 0);
        SkASSERT(fCount1 >= 0);
        SkASSERT(fCount2 >= 0);
        SkASSERT(fCount0 + fCount1 + fCount2 == count);
#endif
    }

private:
    void initFor1(SkGradFixed fx);
};

#endif
