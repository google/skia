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

#define SK_SUPPORT_LEGACY_GRADIENT_PRECISION

#ifdef SK_SUPPORT_LEGACY_GRADIENT_PRECISION
    #define SkGradFixed             SkFixed
    #define SkScalarToGradFixed     SkScalarToFixed
    #define SkFixedToGradFixed(x)   (x)
    #define SkGradFixedToFixed(x)   (x)
    #define kFracMax_SkGradFixed    0xFFFF
#else
    #define SkGradFixed             SkFixed3232
    #define SkScalarToGradFixed     SkScalarToFixed3232
    #define SkFixedToGradFixed      SkFixedToFixed3232
    #define SkGradFixedToFixed(x)   (SkFixed)((x) >> 16)
    #define kFracMax_SkGradFixed    0xFFFFFFFFLL
#endif

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

private:
    void initFor1(SkGradFixed fx);
};

#endif
