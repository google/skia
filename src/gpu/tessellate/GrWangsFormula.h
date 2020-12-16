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

// Wang's formula gives the minimum number of evenly spaced (in the parametric sense) line segments
// that a bezier curve must be chopped into in order to guarantee all lines stay within a distance
// of "1/intolerance" pixels from the true curve. Its definition for a bezier curve of degree "n" is
// as follows:
//
//     maxLength = max([length(p[i+2] - 2p[i+1] + p[i]) for (0 <= i <= n-2)])
//     numParametricSegments = sqrt(maxLength * intolerance * n*(n - 1)/8)
//
// (Goldman, Ron. (2003). 5.6.3 Wang's Formula. "Pyramid Algorithms: A Dynamic Programming Approach
// to Curves and Surfaces for Geometric Modeling". Morgan Kaufmann Publishers.)
namespace GrWangsFormula {

// Returns the value by which to multiply length in Wang's formula. (See above.)
template<int Degree> constexpr float length_term(float intolerance) {
    return (Degree * (Degree - 1) / 8.f) * intolerance;
}
template<int Degree> constexpr float length_term_pow2(float intolerance) {
    return ((Degree * Degree) * ((Degree - 1) * (Degree - 1)) / 64.f) * (intolerance * intolerance);
}

SK_ALWAYS_INLINE static float root4(float x) {
    return sqrtf(sqrtf(x));
}

// Returns nextlog2(sqrt(sqrt(x))):
//
//   log2(sqrt(sqrt(x))) == log2(x^(1/4)) == log2(x)/4 == log2(x)/log2(16) == log16(x)
//
SK_ALWAYS_INLINE static int nextlog16(float x) {
    return (sk_float_nextlog2(x) + 3) >> 2;
}

// Returns Wang's formula, raised to the 4th power, specialized for a quadratic curve.
SK_ALWAYS_INLINE static float quadratic_pow4(float intolerance, const SkPoint pts[],
                                             const GrVectorXform& vectorXform = GrVectorXform()) {
    using grvx::float2, skvx::bit_pun;
    float2 p0 = bit_pun<float2>(pts[0]);
    float2 p1 = bit_pun<float2>(pts[1]);
    float2 p2 = bit_pun<float2>(pts[2]);
    float2 v = grvx::fast_madd<2>(-2, p1, p0) + p2;
    v = vectorXform(v);
    float2 vv = v*v;
    return (vv[0] + vv[1]) * length_term_pow2<2>(intolerance);
}

// Returns Wang's formula specialized for a quadratic curve.
SK_ALWAYS_INLINE static float quadratic(float intolerance, const SkPoint pts[],
                                        const GrVectorXform& vectorXform = GrVectorXform()) {
    return root4(quadratic_pow4(intolerance, pts, vectorXform));
}

// Returns the log2 value of Wang's formula specialized for a quadratic curve, rounded up to the
// next int.
SK_ALWAYS_INLINE static int quadratic_log2(float intolerance, const SkPoint pts[],
                                           const GrVectorXform& vectorXform = GrVectorXform()) {
    // nextlog16(x) == ceil(log2(sqrt(sqrt(x))))
    return nextlog16(quadratic_pow4(intolerance, pts, vectorXform));
}

// Returns Wang's formula, raised to the 4th power, specialized for a cubic curve.
SK_ALWAYS_INLINE static float cubic_pow4(float intolerance, const SkPoint pts[],
                                         const GrVectorXform& vectorXform = GrVectorXform()) {
    using grvx::float4;
    float4 p01 = float4::Load(pts);
    float4 p12 = float4::Load(pts + 1);
    float4 p23 = float4::Load(pts + 2);
    float4 v = grvx::fast_madd<4>(-2, p12, p01) + p23;
    v = vectorXform(v);
    float4 vv = v*v;
    return std::max(vv[0] + vv[1], vv[2] + vv[3]) * length_term_pow2<3>(intolerance);
}

// Returns Wang's formula specialized for a cubic curve.
SK_ALWAYS_INLINE static float cubic(float intolerance, const SkPoint pts[],
                                    const GrVectorXform& vectorXform = GrVectorXform()) {
    return root4(cubic_pow4(intolerance, pts, vectorXform));
}

// Returns the log2 value of Wang's formula specialized for a cubic curve, rounded up to the next
// int.
SK_ALWAYS_INLINE static int cubic_log2(float intolerance, const SkPoint pts[],
                                       const GrVectorXform& vectorXform = GrVectorXform()) {
    // nextlog16(x) == ceil(log2(sqrt(sqrt(x))))
    return nextlog16(cubic_pow4(intolerance, pts, vectorXform));
}

// Returns the maximum number of line segments a cubic with the given device-space bounding box size
// would ever need to be divided into. This is simply a special case of the cubic formula where we
// maximize its value by placing control points on specific corners of the bounding box.
SK_ALWAYS_INLINE static float worst_case_cubic(float intolerance, float devWidth, float devHeight) {
    float k = length_term<3>(intolerance);
    return sqrtf(2*k * SkVector::Length(devWidth, devHeight));
}

// Returns the maximum log2 number of line segments a cubic with the given device-space bounding box
// size would ever need to be divided into.
SK_ALWAYS_INLINE static int worst_case_cubic_log2(float intolerance, float devWidth,
                                                  float devHeight) {
    float kk = length_term_pow2<3>(intolerance);
    // nextlog16(x) == ceil(log2(sqrt(sqrt(x))))
    return nextlog16(4*kk * (devWidth * devWidth + devHeight * devHeight));
}

// Returns Wang's fomrula specialized for a conic curve. This is not actually due to Wang,
// but is an analogue. Input points should be in projected space.
//
// Formula and proof from:
//   J. Zheng, T. Sederberg. "Estimating Tessellation Parameter Intervals for
//   Rational Curves and Surfaces." ACM Transactions on Graphics 19(1). 2000.
// See Thm 3, Corollary 1.
SK_ALWAYS_INLINE static float conic(float intolerance, const SkPoint pts[], float w,
                                    const GrVectorXform& vectorXform = GrVectorXform()) {
    using grvx::float2;
    using grvx::float4;

    float2 p0 = float2::Load(pts), p1 = float2::Load(pts + 1), p2 = float2::Load(pts + 2);

    // Compute center of bounding box in projected space
    float2 C;
    {
        float2 min = p0, max = p0;
        min = skvx::min(skvx::min(min, p1), p2);
        max = skvx::max(skvx::max(max, p1), p2);
        C = 0.5f * (min + max);
    }

    // Translate by -C
    p0 -= C;
    p1 -= C;
    p2 -= C;

    // Compute max length
    float max_len;
    {
        float4 sqd_lens = float4({grvx::dot(p0, p0), grvx::dot(p1, p1), grvx::dot(p2, p2), 0});
        max_len = sqrtf(skvx::max(sqd_lens));
    }

    // Compute forward differences
    const float2 dp = grvx::fast_madd<2>(-2 * w, p1, p0) + p2;
    const float dw = std::abs(1 - 2 * w + 1);

    // Compute delta = parametric step size of linearization
    const float eps = 1.f / intolerance;
    const float r_minus_eps = std::max(0.f, max_len - eps);
    const float min_w = std::min(w, 1.f);
    const float numer = 4 * min_w * eps;
    const float denom = sqrtf(grvx::dot(dp, dp)) + r_minus_eps * dw;
    const float delta = sqrtf(numer / denom);

    constexpr float tmin = 0, tmax = 1;
    return (tmax - tmin) / delta;
}

}  // namespace GrWangsFormula

#endif
