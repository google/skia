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

SK_ALWAYS_INLINE static float length(const Sk2f& n) {
    Sk2f nn = n*n;
    return std::sqrt(nn[0] + nn[1]);
}

// Constant term for the quatratic formula.
constexpr float quadratic_k(float intolerance) {
    return .25f * intolerance;
}

// Returns the minimum number of evenly spaced (in the parametric sense) line segments that the
// quadratic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
SK_ALWAYS_INLINE static float quadratic(float intolerance, const SkPoint pts[]) {
    Sk2f p0 = Sk2f::Load(pts);
    Sk2f p1 = Sk2f::Load(pts + 1);
    Sk2f p2 = Sk2f::Load(pts + 2);
    float k = quadratic_k(intolerance);
    return SkScalarSqrt(k * length(p0 - p1*2 + p2));
}

// Constant term for the cubic formula.
constexpr float cubic_k(float intolerance) {
    return .75f * intolerance;
}

// Returns the minimum number of evenly spaced (in the parametric sense) line segments that the
// cubic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
SK_ALWAYS_INLINE static float cubic(float intolerance, const SkPoint pts[]) {
    Sk2f p0 = Sk2f::Load(pts);
    Sk2f p1 = Sk2f::Load(pts + 1);
    Sk2f p2 = Sk2f::Load(pts + 2);
    Sk2f p3 = Sk2f::Load(pts + 3);
    float k = cubic_k(intolerance);
    return SkScalarSqrt(k * length(Sk2f::Max((p0 - p1*2 + p2).abs(),
                                             (p1 - p2*2 + p3).abs())));
}

// Returns the maximum number of line segments a cubic with the given device-space bounding box size
// would ever need to be divided into. This is simply a special case of the cubic formula where we
// maximize its value by placing control points on specific corners of the bounding box.
SK_ALWAYS_INLINE static float worst_case_cubic(float intolerance, float devWidth, float devHeight) {
    float k = cubic_k(intolerance);
    return SkScalarSqrt(2*k * SkVector::Length(devWidth, devHeight));
}

SK_ALWAYS_INLINE static int ceil_log2_sqrt_sqrt(float f) {
    return (sk_float_nextlog2(f) + 3) >> 2;  // i.e., "ceil(log2(sqrt(sqrt(f))))
}

// Returns the minimum log2 number of evenly spaced (in the parametric sense) line segments that the
// transformed quadratic must be chopped into in order to guarantee all lines stay within a distance
// of "1/intolerance" pixels from the true curve.
SK_ALWAYS_INLINE static int quadratic_log2(float intolerance, const SkPoint pts[],
                                           const GrVectorXform& vectorXform = GrVectorXform()) {
    Sk2f p0 = Sk2f::Load(pts);
    Sk2f p1 = Sk2f::Load(pts + 1);
    Sk2f p2 = Sk2f::Load(pts + 2);
    Sk2f v = p0 + p1*-2 + p2;
    v = vectorXform(v);
    Sk2f vv = v*v;
    float k = quadratic_k(intolerance);
    float f = k*k * (vv[0] + vv[1]);
    return ceil_log2_sqrt_sqrt(f);
}

// Returns the minimum log2 number of evenly spaced (in the parametric sense) line segments that the
// transformed cubic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
SK_ALWAYS_INLINE static int cubic_log2(float intolerance, const SkPoint pts[],
                                       const GrVectorXform& vectorXform = GrVectorXform()) {
    Sk4f p01 = Sk4f::Load(pts);
    Sk4f p12 = Sk4f::Load(pts + 1);
    Sk4f p23 = Sk4f::Load(pts + 2);
    Sk4f v = p01 + p12*-2 + p23;
    v = vectorXform(v);
    Sk4f vv = v*v;
    vv = Sk4f::Max(vv, SkNx_shuffle<2,3,0,1>(vv));
    float k = cubic_k(intolerance);
    float f = k*k * (vv[0] + vv[1]);
    return ceil_log2_sqrt_sqrt(f);
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
