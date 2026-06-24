/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GainmapTestCommon_DEFINED
#define GainmapTestCommon_DEFINED

#include "include/core/SkColor.h"
#include "include/private/SkGainmapInfo.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>

namespace skiatest {

// Return true if the relative difference between x and y is less than epsilon.
inline bool ApproxEq(float x, float y, float epsilon) {
    float numerator = std::abs(x - y);
    // To avoid being too sensitive around zero, set the minimum denominator to epsilon.
    float denominator = std::max(std::min(std::abs(x), std::abs(y)), epsilon);
    return (numerator / denominator) <= epsilon;
}

inline bool ApproxEq(const SkColor4f& x, const SkColor4f& y, float epsilon) {
    return ApproxEq(x.fR, y.fR, epsilon) && ApproxEq(x.fG, y.fG, epsilon) &&
           ApproxEq(x.fB, y.fB, epsilon);
}

template <typename Reporter>
void ExpectApproxEqInfo(Reporter& r, const SkGainmapInfo& a, const SkGainmapInfo& b) {
    float kEpsilon = 1e-4f;
    REPORTER_ASSERT(r, ApproxEq(a.fGainmapRatioMin, b.fGainmapRatioMin, kEpsilon));
    REPORTER_ASSERT(r, ApproxEq(a.fGainmapRatioMax, b.fGainmapRatioMax, kEpsilon));
    REPORTER_ASSERT(r, ApproxEq(a.fGainmapGamma, b.fGainmapGamma, kEpsilon));
    REPORTER_ASSERT(r, ApproxEq(a.fEpsilonSdr, b.fEpsilonSdr, kEpsilon));
    REPORTER_ASSERT(r, ApproxEq(a.fEpsilonHdr, b.fEpsilonHdr, kEpsilon));
    REPORTER_ASSERT(r, ApproxEq(a.fDisplayRatioSdr, b.fDisplayRatioSdr, kEpsilon));
    REPORTER_ASSERT(r, ApproxEq(a.fDisplayRatioHdr, b.fDisplayRatioHdr, kEpsilon));
    REPORTER_ASSERT(r, a.fType == b.fType);
    REPORTER_ASSERT(r, a.fBaseImageType == b.fBaseImageType);

    REPORTER_ASSERT(r, !!a.fGainmapMathColorSpace == !!b.fGainmapMathColorSpace);
    if (a.fGainmapMathColorSpace && b.fGainmapMathColorSpace) {
        skcms_TransferFunction a_fn;
        skcms_Matrix3x3 a_m;
        a.fGainmapMathColorSpace->transferFn(&a_fn);
        a.fGainmapMathColorSpace->toXYZD50(&a_m);
        skcms_TransferFunction b_fn;
        skcms_Matrix3x3 b_m;
        b.fGainmapMathColorSpace->transferFn(&b_fn);
        b.fGainmapMathColorSpace->toXYZD50(&b_m);

        REPORTER_ASSERT(r, ApproxEq(a_fn.g, b_fn.g, kEpsilon));
        REPORTER_ASSERT(r, ApproxEq(a_fn.a, b_fn.a, kEpsilon));
        REPORTER_ASSERT(r, ApproxEq(a_fn.b, b_fn.b, kEpsilon));
        REPORTER_ASSERT(r, ApproxEq(a_fn.c, b_fn.c, kEpsilon));
        REPORTER_ASSERT(r, ApproxEq(a_fn.d, b_fn.d, kEpsilon));
        REPORTER_ASSERT(r, ApproxEq(a_fn.e, b_fn.e, kEpsilon));
        REPORTER_ASSERT(r, ApproxEq(a_fn.f, b_fn.f, kEpsilon));

        // The round-trip of the color space through the ICC profile loses significant precision.
        // Use a larger epsilon for it.
        const float kMatrixEpsilon = 1e-2f;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                REPORTER_ASSERT(r, ApproxEq(a_m.vals[i][j], b_m.vals[i][j], kMatrixEpsilon));
            }
        }
    }
}

}  // namespace skiatest

#endif  // GainmapTestCommon_DEFINED
