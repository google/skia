/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientColorFilter_DEFINED
#define SkGradientColorFilter_DEFINED

#include "SkColorFilter.h"

/**
 *  t = rCoeff * red + gCoeff * green + bCoeff * blue
 *  t = min(max(t, 0), 1)
 *  result = gradient[t]
 */
class SK_API SkGradientColorFilter {
public:
    static sk_sp<SkColorFilter> Make(float rCoeff, float gCoeff, float bCoeff,
                                     const SkColor colors[], const SkScalar pos[], int count);
};

#endif
