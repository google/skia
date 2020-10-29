/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVx_DEFINED
#define GrVx_DEFINED

// If more headers are required, then the desired functionality might not belong in this file.
#include "include/private/SkVx.h"

// grvx is Ganesh's addendum to skvx, Skia's SIMD library. Here we introduce functions that are
// approximate and/or have LSB differences from platform to platform (e.g., by using hardware FMAs
// when available). When a function is approximate, its error range is well documented and tested.
namespace grvx {

// Allow floating point contraction. e.g., allow a*x + y to be compiled to a single FMA even though
// it introduces LSB differences on platforms that don't have an FMA instruction.
#if defined(__clang__)
    #pragma STDC FP_CONTRACT ON
#endif

// Use familiar type names from SkSL and GLSL.
template<int N> using vec = skvx::Vec<N, float>;
using float2 = vec<2>;
using float4 = vec<4>;

template<int N> using ivec = skvx::Vec<N, int32_t>;
using int2 = ivec<2>;
using int4 = ivec<4>;

template<int N> using uvec = skvx::Vec<N, uint32_t>;
using uint2 = uvec<2>;
using uint4 = uvec<4>;

// Returns f*m + a. The actual implementation may or may not be fused, depending on hardware
// support. We call this method "fast_fma" to draw attention to the fact that the operation is
// "approximate" (i.e., unfused f*m + a) when it's not supported in hardware.
template<int N> vec<N> inline fast_fma(vec<N> f, vec<N> m, vec<N> a) {
#if FP_FAST_FMAF
    return skvx::fma(f,m,a);
#else
    return f*m + a;
#endif
}

// Approximates the inverse cosine of x within 0.96 degrees using the rational polynomial:
//
//     acos(x) ~= (bx^3 + ax) / (dx^4 + cx^2 + 1) + pi/2
//
// See: https://stackoverflow.com/a/36387954
//
// For a proof of max error, see the "grvx_fast_acos" unit test.
//
// NOTE: This function deviates immediately from pi and 0 outside -1 and 1. (The derivatives are
// infinite at -1 and 1). So the input must still be clamped between -1 and 1.
#define GRVX_FAST_ACOS_MAX_ERROR SkDegreesToRadians(.96f)
template<int N> inline vec<N> fast_acos(vec<N> x) {
    static const vec<N> a = -0.939115566365855f;
    static const vec<N> b =  0.9217841528914573f;
    static const vec<N> c = -1.2845906244690837f;
    static const vec<N> d =  0.295624144969963174f;
    static const vec<N> pi_over_2 = 1.5707963267948966f;
    vec<N> xx = x*x;
    vec<N> numer = fast_fma(b,xx,a);
    vec<N> denom = fast_fma<N>(xx, fast_fma(d,xx,c), 1);
    return fast_fma(x, numer/denom, pi_over_2);
}

// Approximates the angle between a and b within .96 degrees (GRVX_FAST_ACOS_MAX_ERROR).
template<int N> vec<N> inline fast_angle_between_vectors(vec<N> ax, vec<N> ay, vec<N> bx, vec<N> by) {
    auto ab_cosTheta = fast_fma(ax, bx, ay*by);
    auto ab_pow2 = fast_fma(ax, ax, ay*ay) * fast_fma(bx, bx, by*by);
    auto cosTheta = ab_cosTheta / skvx::sqrt(ab_pow2);
    // Clamp cosTheta such that if it is NaN (e.g., if a or b was 0), then we return acos(1) = 0.
    cosTheta = skvx::max(skvx::min(1, cosTheta), -1);
    return fast_acos(cosTheta);
}

static inline float dot(float2 a, float2 b) {
    float2 ab = a*b;
    return ab[0] + ab[1];
}

static inline float cross(float2 a, float2 b) {
    float2 x = a*skvx::shuffle<1,0>(b);
    return x[0] - x[1];
}

#if defined(__clang__)
    #pragma STDC FP_CONTRACT DEFAULT
#endif

};  // namespace grvx

#endif
