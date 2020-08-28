/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAlignedDashPathEffect_DEFINED
#define SkAlignedDashPathEffect_DEFINED

#include "include/core/SkPathEffect.h"

class SK_API SkAlignedDashPathEffect {
public:
    /**
     * An experimental dash path effect that adjusts interval lengths to align
     * dashes to path segments (verbs). The adjustment is per verb, ensuring
     * that the segment begins and ends with half of the first "on" interval.
     * There is no phase for this dash path effect.
     */
    static sk_sp<SkPathEffect> Make(const SkScalar intervals[], int count);
};

#endif
