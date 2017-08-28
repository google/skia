/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRGeometry.h"

#include "GrTypes.h"
#include "SkPoint.h"
#include "SkNx.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

// We convert between SkPoint and Sk2f freely throughout this file.
GR_STATIC_ASSERT(SK_SCALAR_IS_FLOAT);
GR_STATIC_ASSERT(2 * sizeof(float) == sizeof(SkPoint));
GR_STATIC_ASSERT(0 == offsetof(SkPoint, fX));

static inline Sk2f normalize(const Sk2f& n) {
    Sk2f nn = n*n;
    return n * (nn + SkNx_shuffle<1,0>(nn)).rsqrt();
}

static inline float dot(const Sk2f& a, const Sk2f& b) {
    float product[2];
    (a * b).store(product);
    return product[0] + product[1];
}

// Returns whether the (convex) curve segment is monotonic with respect to [endPt - startPt].
static inline bool is_convex_curve_monotonic(const Sk2f& startPt, const Sk2f& startTan,
                                             const Sk2f& endPt, const Sk2f& endTan) {
    Sk2f v = endPt - startPt;
    float dot0 = dot(startTan, v);
    float dot1 = dot(endTan, v);

    // A small, negative tolerance handles floating-point error in the case when one tangent
    // approaches 0 length, meaning the (convex) curve segment is effectively a flat line.
    float tolerance = -std::max(std::abs(dot0), std::abs(dot1)) * SK_ScalarNearlyZero;
    return dot0 >= tolerance && dot1 >= tolerance;
}

static inline Sk2f lerp(const Sk2f& a, const Sk2f& b, const Sk2f& t) {
    return SkNx_fma(t, b - a, a);
}

bool GrCCPRChopMonotonicQuadratics(const SkPoint& startPt, const SkPoint& controlPt,
                                   const SkPoint& endPt, SkPoint dst[5]) {
    Sk2f p0 = Sk2f::Load(&startPt);
    Sk2f p1 = Sk2f::Load(&controlPt);
    Sk2f p2 = Sk2f::Load(&endPt);

    Sk2f tan0 = p1 - p0;
    Sk2f tan1 = p2 - p1;
    // This should almost always be this case for well-behaved curves in the real world.
    if (is_convex_curve_monotonic(p0, tan0, p2, tan1)) {
        return false;
    }

    // Chop the curve into two segments with equal curvature. To do this we find the T value whose
    // tangent is perpendicular to the vector that bisects tan0 and -tan1.
    Sk2f n = normalize(tan0) - normalize(tan1);

    // This tangent can be found where (dQ(t) dot n) = 0:
    //
    //   0 = (dQ(t) dot n) = | 2*t  1 | * | p0 - 2*p1 + p2 | * | n |
    //                                    | -2*p0 + 2*p1   |   | . |
    //
    //                     = | 2*t  1 | * | tan1 - tan0 | * | n |
    //                                    | 2*tan0      |   | . |
    //
    //                     = 2*t * ((tan1 - tan0) dot n) + (2*tan0 dot n)
    //
    //   t = (tan0 dot n) / ((tan0 - tan1) dot n)
    Sk2f dQ1n = (tan0 - tan1) * n;
    Sk2f dQ0n = tan0 * n;
    Sk2f t = (dQ0n + SkNx_shuffle<1,0>(dQ0n)) / (dQ1n + SkNx_shuffle<1,0>(dQ1n));
    t = Sk2f::Min(Sk2f::Max(t, 0), 1); // Clamp for FP error.

    Sk2f p01 = SkNx_fma(t, tan0, p0);
    Sk2f p12 = SkNx_fma(t, tan1, p1);
    Sk2f p012 = lerp(p01, p12, t);

    p0.store(&dst[0]);
    p01.store(&dst[1]);
    p012.store(&dst[2]);
    p12.store(&dst[3]);
    p2.store(&dst[4]);

    return true;
}
