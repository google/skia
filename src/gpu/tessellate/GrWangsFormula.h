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
#include "src/gpu/GrVx.h"
#include "src/gpu/tessellate/GrVectorXform.h"

// Wang's formulas for cubics and quadratics (1985) give us the minimum number of evenly spaced (in
// the parametric sense) line segments that a curve must be chopped into in order to guarantee all
// lines stay within a distance of "1/intolerance" pixels from the true curve.
namespace GrWangsFormula {

using grvx::float2, grvx::float4;

// Returns the 4th root of x for use with Wang's formulas below. Since the purpose of these formulas
// is to guide linear approximations, precision is only guaranteed up to ~8 bits and near-zero
// values may be clipped.
SK_ALWAYS_INLINE static float fast_root4(float x) {
#if GRVX_FAST_RSQRT_PRECISION_BITS >= 8
    grvx::float2 x_ = skvx::max(float2(x), float2(1e-2f));
    return grvx::fast_rsqrt(grvx::fast_rsqrt(x_)).lo.val;
#else
    return sqrtf(sqrtf(x));
#endif
}

SK_ALWAYS_INLINE static int root4_log2(float x) {
    return (sk_float_nextlog2(x) + 3) >> 2;  // i.e., "ceil(log2(sqrt(sqrt(f))))
}

// Constant term for the quatratic formula.
constexpr float quadratic_constant(float intolerance) {
    return .25f * intolerance;
}

// Returns Wang's formula for the given quadratic, raised to the 4th power. (Refer to
// GrWangsFormula::quadratic for more comments on the formula.)
// The math is quickest when we calculate this value raised to the 4th power.
SK_ALWAYS_INLINE static float quadratic_pow4(float intolerance, const SkPoint pts[],
                                             const GrVectorXform& vectorXform = GrVectorXform()) {
    using skvx::bit_pun;
    float2 p0 = bit_pun<float2>(pts[0]);
    float2 p1 = bit_pun<float2>(pts[1]);
    float2 p2 = bit_pun<float2>(pts[2]);
    float2 v = grvx::madd<2>(-2, p1, p0 + p2);
    v = vectorXform(v);
    float2 vv = v*v;
    float k = quadratic_constant(intolerance);
    return k*k * (vv[0] + vv[1]);
}

// Returns the minimum number of evenly spaced (in the parametric sense) line segments that the
// quadratic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
SK_ALWAYS_INLINE static float quadratic(float intolerance, const SkPoint pts[],
                                        const GrVectorXform& vectorXform = GrVectorXform()) {
    return fast_root4(quadratic_pow4(intolerance, pts, vectorXform));
}

// Returns the log2 value of Wang's formula for the given quadratic, rounded up to the next int.
SK_ALWAYS_INLINE static int quadratic_log2(float intolerance, const SkPoint pts[],
                                           const GrVectorXform& vectorXform = GrVectorXform()) {
    return root4_log2(quadratic_pow4(intolerance, pts, vectorXform));
}

// Constant term for the cubic formula.
constexpr float cubic_constant(float intolerance) {
    return .75f * intolerance;
}

// Returns Wang's formula for the given cubic, raised to the 4th power. (Refer to
// GrWangsFormula::cubic for more comments on the formula.)
// The math is quickest when we calculate this value raised to the 4th power.
SK_ALWAYS_INLINE static float cubic_pow4(float intolerance, const SkPoint pts[],
                                         const GrVectorXform& vectorXform = GrVectorXform()) {
    float4 p01 = float4::Load(pts);
    float4 p12 = float4::Load(pts + 1);
    float4 p23 = float4::Load(pts + 2);
    float4 v = grvx::madd<4>(-2, p12, p01 + p23);
    v = vectorXform(v);
    float4 vv = v*v;
    float2 m = skvx::max(vv.lo, vv.hi);
    float k = cubic_constant(intolerance);
    return k*k * (m[0] + m[1]);
}

// Returns the minimum number of evenly spaced (in the parametric sense) line segments that the
// cubic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
SK_ALWAYS_INLINE static float cubic(float intolerance, const SkPoint pts[],
                                    const GrVectorXform& vectorXform = GrVectorXform()) {
    return fast_root4(cubic_pow4(intolerance, pts, vectorXform));
}

// Returns the log2 value of Wang's formula for the given cubic, rounded up to the next int.
SK_ALWAYS_INLINE static int cubic_log2(float intolerance, const SkPoint pts[],
                                       const GrVectorXform& vectorXform = GrVectorXform()) {
    return root4_log2(cubic_pow4(intolerance, pts, vectorXform));
}

// Returns the maximum number of line segments a cubic with the given device-space bounding box size
// would ever need to be divided into. This is simply a special case of the cubic formula where we
// maximize its value by placing control points on specific corners of the bounding box.
SK_ALWAYS_INLINE static float worst_case_cubic(float intolerance, float devWidth, float devHeight) {
    float k = cubic_constant(intolerance);
    return sqrtf(2*k * SkVector::Length(devWidth, devHeight));
}

// Returns the maximum log2 number of line segments a cubic with the given device-space bounding box
// size would ever need to be divided into.
SK_ALWAYS_INLINE static int worst_case_cubic_log2(float intolerance, float devWidth,
                                                  float devHeight) {
    float k = cubic_constant(intolerance);
    return root4_log2(4*k*k * (devWidth * devWidth + devHeight * devHeight));
}

}  // namespace GrWangsFormula

#endif
