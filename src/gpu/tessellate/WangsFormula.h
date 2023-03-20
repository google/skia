/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_WangsFormula_DEFINED
#define skgpu_tessellate_WangsFormula_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkString.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkVx.h"
#include "src/gpu/tessellate/Tessellation.h"

#define AI [[maybe_unused]] SK_ALWAYS_INLINE

// Wang's formula gives the minimum number of evenly spaced (in the parametric sense) line segments
// that a bezier curve must be chopped into in order to guarantee all lines stay within a distance
// of "1/precision" pixels from the true curve. Its definition for a bezier curve of degree "n" is
// as follows:
//
//     maxLength = max([length(p[i+2] - 2p[i+1] + p[i]) for (0 <= i <= n-2)])
//     numParametricSegments = sqrt(maxLength * precision * n*(n - 1)/8)
//
// (Goldman, Ron. (2003). 5.6.3 Wang's Formula. "Pyramid Algorithms: A Dynamic Programming Approach
// to Curves and Surfaces for Geometric Modeling". Morgan Kaufmann Publishers.)
namespace skgpu::wangs_formula {

// Returns the value by which to multiply length in Wang's formula. (See above.)
template<int Degree> constexpr float length_term(float precision) {
    return (Degree * (Degree - 1) / 8.f) * precision;
}
template<int Degree> constexpr float length_term_p2(float precision) {
    return ((Degree * Degree) * ((Degree - 1) * (Degree - 1)) / 64.f) * (precision * precision);
}

AI float root4(float x) {
    return sqrtf(sqrtf(x));
}

// Returns nextlog2(sqrt(x)):
//
//   log2(sqrt(x)) == log2(x^(1/2)) == log2(x)/2 == log2(x)/log2(4) == log4(x)
//
AI int nextlog4(float x) {
    return (sk_float_nextlog2(x) + 1) >> 1;
}

// Returns nextlog2(sqrt(sqrt(x))):
//
//   log2(sqrt(sqrt(x))) == log2(x^(1/4)) == log2(x)/4 == log2(x)/log2(16) == log16(x)
//
AI int nextlog16(float x) {
    return (sk_float_nextlog2(x) + 3) >> 2;
}

// Represents the upper-left 2x2 matrix of an affine transform for applying to vectors:
//
//     VectorXform(p1 - p0) == M * float3(p1, 1) - M * float3(p0, 1)
//
class VectorXform {
public:
    AI VectorXform() : fC0{1.0f, 0.f}, fC1{0.f, 1.f} {}
    AI explicit VectorXform(const SkMatrix& m) { *this = m; }
    AI explicit VectorXform(const SkM44& m) { *this = m; }

    AI VectorXform& operator=(const SkMatrix& m) {
        SkASSERT(!m.hasPerspective());
        fC0 = {m.rc(0,0), m.rc(1,0)};
        fC1 = {m.rc(0,1), m.rc(1,1)};
        return *this;
    }
    AI VectorXform& operator=(const SkM44& m) {
        SkASSERT(m.rc(3,0) == 0.f && m.rc(3,1) == 0.f && m.rc(3,2) == 0.f && m.rc(3,3) == 1.f);
        fC0 = {m.rc(0,0), m.rc(1,0)};
        fC1 = {m.rc(0,1), m.rc(1,1)};
        return *this;
    }
    AI skvx::float2 operator()(skvx::float2 vector) const {
        return fC0 * vector.x() + fC1 * vector.y();
    }
    AI skvx::float4 operator()(skvx::float4 vectors) const {
        return join(fC0 * vectors.x() + fC1 * vectors.y(),
                    fC0 * vectors.z() + fC1 * vectors.w());
    }
private:
    // First and second columns of 2x2 matrix
    skvx::float2 fC0;
    skvx::float2 fC1;
};

// Returns Wang's formula, raised to the 4th power, specialized for a quadratic curve.
AI float quadratic_p4(float precision,
                      skvx::float2 p0, skvx::float2 p1, skvx::float2 p2,
                      const VectorXform& vectorXform = VectorXform()) {
    skvx::float2 v = -2*p1 + p0 + p2;
    v = vectorXform(v);
    skvx::float2 vv = v*v;
    return (vv[0] + vv[1]) * length_term_p2<2>(precision);
}
AI float quadratic_p4(float precision,
                      const SkPoint pts[],
                      const VectorXform& vectorXform = VectorXform()) {
    return quadratic_p4(precision,
                        skvx::bit_pun<skvx::float2>(pts[0]),
                        skvx::bit_pun<skvx::float2>(pts[1]),
                        skvx::bit_pun<skvx::float2>(pts[2]),
                        vectorXform);
}

// Returns Wang's formula specialized for a quadratic curve.
AI float quadratic(float precision,
                   const SkPoint pts[],
                   const VectorXform& vectorXform = VectorXform()) {
    return root4(quadratic_p4(precision, pts, vectorXform));
}

// Returns the log2 value of Wang's formula specialized for a quadratic curve, rounded up to the
// next int.
AI int quadratic_log2(float precision,
                      const SkPoint pts[],
                      const VectorXform& vectorXform = VectorXform()) {
    // nextlog16(x) == ceil(log2(sqrt(sqrt(x))))
    return nextlog16(quadratic_p4(precision, pts, vectorXform));
}

// Returns Wang's formula, raised to the 4th power, specialized for a cubic curve.
AI float cubic_p4(float precision,
                  skvx::float2 p0, skvx::float2 p1, skvx::float2 p2, skvx::float2 p3,
                  const VectorXform& vectorXform = VectorXform()) {
    skvx::float4 p01{p0, p1};
    skvx::float4 p12{p1, p2};
    skvx::float4 p23{p2, p3};
    skvx::float4 v = -2*p12 + p01 + p23;
    v = vectorXform(v);
    skvx::float4 vv = v*v;
    return std::max(vv[0] + vv[1], vv[2] + vv[3]) * length_term_p2<3>(precision);
}
AI float cubic_p4(float precision,
                  const SkPoint pts[],
                  const VectorXform& vectorXform = VectorXform()) {
    return cubic_p4(precision,
                    skvx::bit_pun<skvx::float2>(pts[0]),
                    skvx::bit_pun<skvx::float2>(pts[1]),
                    skvx::bit_pun<skvx::float2>(pts[2]),
                    skvx::bit_pun<skvx::float2>(pts[3]),
                    vectorXform);
}

// Returns Wang's formula specialized for a cubic curve.
AI float cubic(float precision,
               const SkPoint pts[],
               const VectorXform& vectorXform = VectorXform()) {
    return root4(cubic_p4(precision, pts, vectorXform));
}

// Returns the log2 value of Wang's formula specialized for a cubic curve, rounded up to the next
// int.
AI int cubic_log2(float precision,
                  const SkPoint pts[],
                  const VectorXform& vectorXform = VectorXform()) {
    // nextlog16(x) == ceil(log2(sqrt(sqrt(x))))
    return nextlog16(cubic_p4(precision, pts, vectorXform));
}

// Returns the maximum number of line segments a cubic with the given device-space bounding box size
// would ever need to be divided into, raised to the 4th power. This is simply a special case of the
// cubic formula where we maximize its value by placing control points on specific corners of the
// bounding box.
AI float worst_case_cubic_p4(float precision, float devWidth, float devHeight) {
    float kk = length_term_p2<3>(precision);
    return 4*kk * (devWidth * devWidth + devHeight * devHeight);
}

// Returns the maximum number of line segments a cubic with the given device-space bounding box size
// would ever need to be divided into.
AI float worst_case_cubic(float precision, float devWidth, float devHeight) {
    return root4(worst_case_cubic_p4(precision, devWidth, devHeight));
}

// Returns the maximum log2 number of line segments a cubic with the given device-space bounding box
// size would ever need to be divided into.
AI int worst_case_cubic_log2(float precision, float devWidth, float devHeight) {
    // nextlog16(x) == ceil(log2(sqrt(sqrt(x))))
    return nextlog16(worst_case_cubic_p4(precision, devWidth, devHeight));
}

// Returns Wang's formula specialized for a conic curve, raised to the second power.
// Input points should be in projected space.
//
// This is not actually due to Wang, but is an analogue from (Theorem 3, corollary 1):
//   J. Zheng, T. Sederberg. "Estimating Tessellation Parameter Intervals for
//   Rational Curves and Surfaces." ACM Transactions on Graphics 19(1). 2000.
AI float conic_p2(float precision,
                  skvx::float2 p0, skvx::float2 p1, skvx::float2 p2,
                  float w,
                  const VectorXform& vectorXform = VectorXform()) {
    p0 = vectorXform(p0);
    p1 = vectorXform(p1);
    p2 = vectorXform(p2);

    // Compute center of bounding box in projected space
    const skvx::float2 C = 0.5f * (min(min(p0, p1), p2) + max(max(p0, p1), p2));

    // Translate by -C. This improves translation-invariance of the formula,
    // see Sec. 3.3 of cited paper
    p0 -= C;
    p1 -= C;
    p2 -= C;

    // Compute max length
    const float max_len = sqrtf(std::max(dot(p0, p0), std::max(dot(p1, p1), dot(p2, p2))));


    // Compute forward differences
    const skvx::float2 dp = -2*w*p1 + p0 + p2;
    const float dw = fabsf(-2 * w + 2);

    // Compute numerator and denominator for parametric step size of linearization. Here, the
    // epsilon referenced from the cited paper is 1/precision.
    const float rp_minus_1 = std::max(0.f, max_len * precision - 1);
    const float numer = sqrtf(dot(dp, dp)) * precision + rp_minus_1 * dw;
    const float denom = 4 * std::min(w, 1.f);

    // Number of segments = sqrt(numer / denom).
    // This assumes parametric interval of curve being linearized is [t0,t1] = [0, 1].
    // If not, the number of segments is (tmax - tmin) / sqrt(denom / numer).
    return numer / denom;
}
AI float conic_p2(float precision,
                  const SkPoint pts[],
                  float w,
                  const VectorXform& vectorXform = VectorXform()) {
    return conic_p2(precision,
                    skvx::bit_pun<skvx::float2>(pts[0]),
                    skvx::bit_pun<skvx::float2>(pts[1]),
                    skvx::bit_pun<skvx::float2>(pts[2]),
                    w,
                    vectorXform);
}

// Returns the value of Wang's formula specialized for a conic curve.
AI float conic(float tolerance,
               const SkPoint pts[],
               float w,
               const VectorXform& vectorXform = VectorXform()) {
    return sqrtf(conic_p2(tolerance, pts, w, vectorXform));
}

// Returns the log2 value of Wang's formula specialized for a conic curve, rounded up to the next
// int.
AI int conic_log2(float tolerance,
                  const SkPoint pts[],
                  float w,
                  const VectorXform& vectorXform = VectorXform()) {
    // nextlog4(x) == ceil(log2(sqrt(x)))
    return nextlog4(conic_p2(tolerance, pts, w, vectorXform));
}

}  // namespace skgpu::wangs_formula

#undef AI

#endif  // skgpu_tessellate_WangsFormula_DEFINED
