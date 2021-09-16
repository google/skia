/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashPathPriv_DEFINED
#define SkDashPathPriv_DEFINED

#include "include/core/SkPathEffect.h"

namespace SkDashPath {
    /**
     * Calculates the initialDashLength, initialDashIndex, and intervalLength based on the
     * inputed phase and intervals. If adjustedPhase is passed in, then the phase will be
     * adjusted to be between 0 and intervalLength. The result will be stored in adjustedPhase.
     * If adjustedPhase is nullptr then it is assumed phase is already between 0 and intervalLength
     *
     * Caller should have already used ValidDashPath to exclude invalid data.
     */
    void CalcDashParameters(SkScalar phase, const SkScalar intervals[], int32_t count,
                            SkScalar* initialDashLength, int32_t* initialDashIndex,
                            SkScalar* intervalLength, SkScalar* adjustedPhase = nullptr);

    bool FilterDashPath(SkPath* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                        const SkPathEffect::DashInfo& info);

#ifdef SK_BUILD_FOR_FUZZER
    const SkScalar kMaxDashCount = 10000;
#else
    const SkScalar kMaxDashCount = 1000000;
#endif

    /** See comments for InternalFilter */
    enum class StrokeRecApplication {
        kDisallow,
        kAllow,
    };

    /**
     * Caller should have already used ValidDashPath to exclude invalid data. Typically, this leaves
     * the strokeRec unmodified. However, for some simple shapes (e.g. a line) it may directly
     * evaluate the dash and stroke to produce a stroked output path with a fill strokeRec. Passing
     * true for disallowStrokeRecApplication turns this behavior off.
     */
    bool InternalFilter(SkPath* dst, const SkPath& src, SkStrokeRec* rec,
                        const SkRect* cullRect, const SkScalar aIntervals[],
                        int32_t count, SkScalar initialDashLength, int32_t initialDashIndex,
                        SkScalar intervalLength,
                        StrokeRecApplication = StrokeRecApplication::kAllow);

    bool ValidDashPath(SkScalar phase, const SkScalar intervals[], int32_t count);
}  // namespace SkDashPath

#endif
