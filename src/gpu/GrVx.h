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
// support. We call this method "madd" to avoid confusion with skvx::fma, which is defined as always
// fused.
template<int N> vec<N> inline madd(vec<N> f, vec<N> m, vec<N> a) {
#if FP_FAST_FMAF
    return skvx::fma(f,m,a);
#else
    return f*m + a;
#endif
}

// Platform-dependent specializations.
#if defined(__SSE__)
    #define GRVX_FAST_RCP_PRECISION_BITS 11
    static inline float4 fast_rcp(float4 x) {
        __m128 x_ = skvx::bit_pun<__m128>(x);
        return skvx::bit_pun<float4>(_mm_rcp_ps(x_));
    }
    static inline float2 fast_rcp(float2 x) {
        return skvx::shuffle<0,1>(fast_rcp(skvx::shuffle<0,1,0,1>(x)));
    }
    #if defined(__AVX__)
        static inline vec<8> fast_rcp(vec<8> x) {
            __m256 x_ = skvx::bit_pun<__m256>(x);
            return skvx::bit_pun<vec<8>>(_mm256_rcp_ps(x_));
        }
    #endif

    #define GRVX_FAST_RSQRT_PRECISION_BITS 11
    static inline float4 fast_rsqrt(float4 x) {
        __m128 x_ = skvx::bit_pun<__m128>(x);
        return skvx::bit_pun<float4>(_mm_rsqrt_ps(x_));
    }
    static inline float2 fast_rsqrt(float2 x) {
        return skvx::shuffle<0,1>(fast_rsqrt(skvx::shuffle<0,1,0,1>(x)));
    }
    #if defined(__AVX__)
        static inline vec<8> fast_rsqrt(vec<8> x) {
            __m256 x_ = skvx::bit_pun<__m256>(x);
            return skvx::bit_pun<vec<8>>(_mm256_rsqrt_ps(x_));
        }
    #endif
#elif defined(__ARM_NEON)
    #define GRVX_FAST_RCP_PRECISION_BITS 8
    static inline float4 fast_rcp(float4 x) {
        float32x4_t x_ = skvx::bit_pun<float32x4_t>(x);
        return skvx::bit_pun<float4>(vrecpeq_f32(x_));
    }
    static inline float2 fast_rcp(float2 x) {
        float32x2_t x_ = skvx::bit_pun<float32x2_t>(x);
        return skvx::bit_pun<float2>(vrecpe_f32(x_));
    }

    #define GRVX_FAST_RSQRT_PRECISION_BITS 8
    static inline float4 fast_rsqrt(float4 x) {
        float32x4_t x_ = skvx::bit_pun<float32x4_t>(x);
        return skvx::bit_pun<float4>(vrsqrteq_f32(x_));
    }
    static inline float2 fast_rsqrt(float2 x) {
        float32x2_t x_ = skvx::bit_pun<float32x2_t>(x);
        return skvx::bit_pun<float2>(vrsqrte_f32(x_));
    }
#endif

#if GRVX_FAST_RCP_PRECISION_BITS
template<int N> inline vec<N> fast_rcp(vec<N> x) {
    vec<N> ret;
    ret.lo = fast_rcp(x.lo);
    ret.hi = fast_rcp(x.hi);
    return ret;
}
template<> inline vec<1> fast_rcp<1>(vec<1> x) {
    return fast_rcp(float2(x.val)).lo.val;
}
#endif

#if GRVX_FAST_RSQRT_PRECISION_BITS
template<int N> inline vec<N> fast_rsqrt(vec<N> x) {
    vec<N> ret;
    ret.lo = fast_rsqrt(x.lo);
    ret.hi = fast_rsqrt(x.hi);
    return ret;
}
template<> inline vec<1> fast_rsqrt<1>(vec<1> x) {
    return fast_rsqrt(float2(x.val)).lo.val;
}
#endif

// Approximates the inverse cosine of x within 1.25 degrees using the rational polynomial:
//
//     acos(x) ~= (bx^3 + ax) / (dx^4 + cx^2 + 1) + pi/2
//
// See: https://stackoverflow.com/a/36387954
//
// The approximation itself is mathematically accurate within 1 degree, but the use of fast_rcp with
// 8 bits of precision bumps our error range out to 1.25 degrees. For a proof of the error range,
// see the "grvx_fast_acos" unit test.
//
// NOTE: This function immediately deviates from pi and 0 outside -1 and 1 respectively. (The
// derivatives are infinite at -1 and 1). So the input must be clamped between -1 and 1.
#define GRVX_FAST_ACOS_MAX_ERROR SkDegreesToRadians(1.25f)
template<int N> inline vec<N> fast_acos(vec<N> x) {
    vec<N> xx = x*x;
    vec<N> a = -0.939115566365855f;
    vec<N> b =  0.9217841528914573f;
    vec<N> c = -1.2845906244690837f;
    vec<N> d =  0.295624144969963174f;
    vec<N> pi_over_2 = 1.5707963267948966f;
    vec<N> numer = madd(b,xx,a);
    vec<N> denom = madd<N>(xx, madd(d,xx,c), 1);
#if (GRVX_FAST_RCP_PRECISION_BITS >= 8)
    return madd(x, numer * fast_rcp(denom), pi_over_2);
#else
    return madd(x, numer/denom, pi_over_2);
#endif
}

// Approximates the angle between a and b within 1.25 degrees (GRVX_FAST_ACOS_MAX_ERROR).
template<int N> vec<N> inline fast_angle_between_vectors(vec<N> ax, vec<N> ay, vec<N> bx, vec<N> by) {
    auto ab_cosTheta = madd(ax, bx, ay*by);
    auto ab_pow2 = madd(ax, ax, ay*ay) * madd(bx, bx, by*by);
    // Unfortunately, not even 11 bits of fast_rsqrt precision is enough here.
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
