/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDashPathPriv_DEFINED
#define SkDashPathPriv_DEFINED

#include "include/core/SkPathEffect.h"
#include "include/core/SkSpan.h"
#include "src/core/SkPathEffectBase.h"

namespace SkDashPath {
/**
 * Calculates the initialDashLength, initialDashIndex, and intervalLength based on the
 * inputed phase and intervals. If adjustedPhase is passed in, then the phase will be
 * adjusted to be between 0 and intervalLength. The result will be stored in adjustedPhase.
 * If adjustedPhase is nullptr then it is assumed phase is already between 0 and intervalLength
 *
 * Caller should have already used ValidDashPath to exclude invalid data.
 */
void CalcDashParameters(SkScalar phase, SkSpan<const SkScalar> intervals,
                        SkScalar* initialDashLength, size_t* initialDashIndex,
                        SkScalar* intervalLength, SkScalar* adjustedPhase = nullptr);

bool FilterDashPath(SkPathBuilder* dst, const SkPath& src, SkStrokeRec*, const SkRect*,
                    const SkPathEffectBase::DashInfo& info);

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
    bool InternalFilter(SkPathBuilder* dst, const SkPath& src, SkStrokeRec* rec,
                        const SkRect* cullRect, SkSpan<const SkScalar> aIntervals,
                        SkScalar initialDashLength, int32_t initialDashIndex,
                        SkScalar intervalLength, SkScalar startPhase,
                        StrokeRecApplication = StrokeRecApplication::kAllow);

    bool ValidDashPath(SkScalar phase, SkSpan<const SkScalar> intervals);

}  // namespace SkDashPath

#endif
