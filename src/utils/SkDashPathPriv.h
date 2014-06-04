/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashPathPriv_DEFINED
#define SkDashPathPriv_DEFINED

#include "SkPathEffect.h"

namespace SkDashPath {
    /*
     * Calculates the initialDashLength, initialDashIndex, and intervalLength based on the
     * inputed phase and intervals. If adjustedPhase is passed in, then the phase will be
     * adjusted to be between 0 and intervalLength. The result will be stored in adjustedPhase.
     * If adjustedPhase is NULL then it is assumed phase is already between 0 and intervalLength
     */
    void CalcDashParameters(SkScalar phase, const SkScalar intervals[], int32_t count,
                            SkScalar* initialDashLength, int32_t* initialDashIndex,
                            SkScalar* intervalLength, SkScalar* adjustedPhase = NULL);

    bool FilterDashPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                        const SkScalar aIntervals[], int32_t count, SkScalar initialDashLength,
                        int32_t initialDashIndex, SkScalar intervalLength);
    
    bool FilterDashPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                        const SkPathEffect::DashInfo& info);
}

#endif
