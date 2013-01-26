
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkScalarCompare_DEFINED
#define SkScalarCompare_DEFINED

#include "SkFloatBits.h"
#include "SkRect.h"

/** Skia can spend a lot of time just comparing scalars (e.g. quickReject).
    When scalar==fixed, this is very fast, and when scalar==hardware-float, this
    is also reasonable, but if scalar==software-float, then each compare can be
    a function call and take real time. To account for that, we have the flag
    SK_SCALAR_SLOW_COMPARES.

    If this is defined, we have a special trick where we quickly convert floats
    to a 2's compliment form, and then treat them as signed 32bit integers. In
    this form we lose a few subtlties (e.g. NaNs always comparing false) but
    we gain the speed of integer compares.
 */

#ifdef SK_SCALAR_SLOW_COMPARES
    typedef int32_t SkScalarCompareType;
    typedef SkIRect SkRectCompareType;
    #define SkScalarToCompareType(x)    SkScalarAs2sCompliment(x)
#else
    typedef SkScalar SkScalarCompareType;
    typedef SkRect SkRectCompareType;
    #define SkScalarToCompareType(x)    (x)
#endif

#endif
