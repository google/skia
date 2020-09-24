/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWangsFormula_DEFINED
#define GrWangsFormula_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkNx.h"
#include "src/gpu/tessellate/GrVectorXform.h"

// Wang's formulas for cubics and quadratics (1985) give us the minimum number of evenly spaced (in
// the parametric sense) line segments that a curve must be chopped into in order to guarantee all
// lines stay within a distance of "1/intolerance" pixels from the true curve.
namespace GrWangsFormula {

SK_ALWAYS_INLINE static float root4(float x) {
    // rsqrt() is quicker than sqrt(), and 1/sqrt(1/sqrt(x)) == sqrt(sqrt(x)).
    return (x != 0) ? sk_float_rsqrt(sk_float_rsqrt(x)) : 0;
}

SK_ALWAYS_INLINE static int ceil_log2_sqrt_sqrt(float x) {
    return (sk_float_nextlog2(x) + 3) >> 2;  // i.e., "ceil(log2(sqrt(sqrt(f))))
}

// Constant term for the quatratic formula.
constexpr float quadratic_k(float intolerance) {
    return .25f * intolerance;
}

// Returns Wang's formula for the given quadratic, raised to the 4th power. (Refer to
// GrWangsFormula::quadratic for more comments on the formula.)
// The math is quickest when we calculate this value raised to the 4th power.
SK_ALWAYS_INLINE static float quadratic_pow4(float intolerance, const SkPoint pts[],
                                             const GrVectorXform& vectorXform = GrVectorXform()) {
    Sk2f p0 = Sk2f::Load(pts);
    Sk2f p1 = Sk2f::Load(pts + 1);
    Sk2f p2 = Sk2f::Load(pts + 2);
    Sk2f v = p0 + p1*-2 + p2;
    v = vectorXform(v);
    Sk2f vv = v*v;
    float k = quadratic_k(intolerance);
    return k*k * (vv[0] + vv[1]);
}

// Returns the minimum number of evenly spaced (in the parametric sense) line segments that the
// quadratic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
SK_ALWAYS_INLINE static float quadratic(float intolerance, const SkPoint pts[],
                                        const GrVectorXform& vectorXform = GrVectorXform()) {
    return root4(quadratic_pow4(intolerance, pts, vectorXform));
}

// Returns the log2 value of Wang's formula for the given quadratic, rounded up to the next int.
SK_ALWAYS_INLINE static int quadratic_log2(float intolerance, const SkPoint pts[],
                                           const GrVectorXform& vectorXform = GrVectorXform()) {
    return ceil_log2_sqrt_sqrt(quadratic_pow4(intolerance, pts, vectorXform));
}

// Constant term for the cubic formula.
constexpr float cubic_k(float intolerance) {
    return .75f * intolerance;
}

// Returns Wang's formula for the given cubic, raised to the 4th power. (Refer to
// GrWangsFormula::cubic for more comments on the formula.)
// The math is quickest when we calculate this value raised to the 4th power.
SK_ALWAYS_INLINE static float cubic_pow4(float intolerance, const SkPoint pts[],
                                         const GrVectorXform& vectorXform = GrVectorXform()) {
    Sk4f p01 = Sk4f::Load(pts);
    Sk4f p12 = Sk4f::Load(pts + 1);
    Sk4f p23 = Sk4f::Load(pts + 2);
    Sk4f v = p01 + p12*-2 + p23;
    v = vectorXform(v);
    Sk4f vv = v*v;
    vv = Sk4f::Max(vv, SkNx_shuffle<2,3,0,1>(vv));
    float k = cubic_k(intolerance);
    return k*k * (vv[0] + vv[1]);
}

// Returns the minimum number of evenly spaced (in the parametric sense) line segments that the
// cubic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
SK_ALWAYS_INLINE static float cubic(float intolerance, const SkPoint pts[],
                                    const GrVectorXform& vectorXform = GrVectorXform()) {
    return root4(cubic_pow4(intolerance, pts, vectorXform));
}

// Returns the log2 value of Wang's formula for the given cubic, rounded up to the next int.
SK_ALWAYS_INLINE static int cubic_log2(float intolerance, const SkPoint pts[],
                                       const GrVectorXform& vectorXform = GrVectorXform()) {
    return ceil_log2_sqrt_sqrt(cubic_pow4(intolerance, pts, vectorXform));
}

// Returns the maximum number of line segments a cubic with the given device-space bounding box size
// would ever need to be divided into. This is simply a special case of the cubic formula where we
// maximize its value by placing control points on specific corners of the bounding box.
SK_ALWAYS_INLINE static float worst_case_cubic(float intolerance, float devWidth, float devHeight) {
    float k = cubic_k(intolerance);
    return SkScalarSqrt(2*k * SkVector::Length(devWidth, devHeight));
}

// Returns the maximum log2 number of line segments a cubic with the given device-space bounding box
// size would ever need to be divided into.
SK_ALWAYS_INLINE static int worst_case_cubic_log2(float intolerance, float devWidth,
                                                  float devHeight) {
    float k = cubic_k(intolerance);
    return ceil_log2_sqrt_sqrt(4*k*k * (devWidth * devWidth + devHeight * devHeight));
}

}  // namespace GrWangsFormula

#endif
