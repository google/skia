/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVx_DEFINED
#define GrVx_DEFINED

#include "include/core/SkTypes.h"
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

// Use familiar type names and functions from SkSL and GLSL.
template<int N> using vec = skvx::Vec<N, float>;
using float2 = vec<2>;
using float4 = vec<4>;

template<int N> using ivec = skvx::Vec<N, int32_t>;
using int2 = ivec<2>;
using int4 = ivec<4>;

template<int N> using uvec = skvx::Vec<N, uint32_t>;
using uint2 = uvec<2>;
using uint4 = uvec<4>;

static SK_ALWAYS_INLINE float dot(float2 a, float2 b) {
    float2 ab = a*b;
    return ab[0] + ab[1];
}

static SK_ALWAYS_INLINE float cross(float2 a, float2 b) {
    float2 x = a*skvx::shuffle<1,0>(b);
    return x[0] - x[1];
}

// Returns f*m + a. The actual implementation may or may not be fused, depending on hardware
// support. We call this method "fast_madd" to draw attention to the fact that the operation may
// give different results on different platforms.
template<int N> SK_ALWAYS_INLINE vec<N> fast_madd(vec<N> f, vec<N> m, vec<N> a) {
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
// For a proof of max error, see the "grvx_approx_acos" unit test.
//
// NOTE: This function deviates immediately from pi and 0 outside -1 and 1. (The derivatives are
// infinite at -1 and 1). So the input must still be clamped between -1 and 1.
#define GRVX_APPROX_ACOS_MAX_ERROR SkDegreesToRadians(.96f)
template<int N> SK_ALWAYS_INLINE vec<N> approx_acos(vec<N> x) {
    constexpr static float a = -0.939115566365855f;
    constexpr static float b =  0.9217841528914573f;
    constexpr static float c = -1.2845906244690837f;
    constexpr static float d =  0.295624144969963174f;
    constexpr static float pi_over_2 = 1.5707963267948966f;
    vec<N> xx = x*x;
    vec<N> numer = fast_madd<N>(b,xx,a);
    vec<N> denom = fast_madd<N>(xx, fast_madd<N>(d,xx,c), 1);
    return fast_madd<N>(x, numer/denom, pi_over_2);
}

// Approximates the angle between vectors a and b within .96 degrees (GRVX_FAST_ACOS_MAX_ERROR).
// a (and b) represent "N" (Nx2/2) 2d vectors in SIMD, with the x values found in a.lo, and the
// y values in a.hi.
//
// Due to fp32 overflow, this method is only valid for magnitudes in the range (2^-31, 2^31)
// exclusive. Results are undefined if the inputs fall outside this range.
//
// NOTE: If necessary, we can extend our valid range to 2^(+/-63) by normalizing a and b separately.
// i.e.: "cosTheta = dot(a,b) / sqrt(dot(a,a)) / sqrt(dot(b,b))".
template<int Nx2>
SK_ALWAYS_INLINE vec<Nx2/2> approx_angle_between_vectors(vec<Nx2> a, vec<Nx2> b) {
    auto aa=a*a, bb=b*b, ab=a*b;
    auto cosTheta = (ab.lo + ab.hi) / skvx::sqrt((aa.lo + aa.hi) * (bb.lo + bb.hi));
    // Clamp cosTheta such that if it is NaN (e.g., if a or b was 0), then we return acos(1) = 0.
    cosTheta = skvx::max(skvx::min(1, cosTheta), -1);
    return approx_acos(cosTheta);
}

// De-interleaving load of 4 vectors.
//
// WARNING: These are really only supported well on NEON. Consider restructuring your data before
// resorting to these methods.
template<typename T>
SK_ALWAYS_INLINE void strided_load4(const T* v, skvx::Vec<1,T>& a, skvx::Vec<1,T>& b,
                                    skvx::Vec<1,T>& c, skvx::Vec<1,T>& d) {
    a.val = v[0];
    b.val = v[1];
    c.val = v[2];
    d.val = v[3];
}
template<int N, typename T>
SK_ALWAYS_INLINE typename std::enable_if<N >= 2, void>::type
strided_load4(const T* v, skvx::Vec<N,T>& a, skvx::Vec<N,T>& b, skvx::Vec<N,T>& c,
              skvx::Vec<N,T>& d) {
    strided_load4(v, a.lo, b.lo, c.lo, d.lo);
    strided_load4(v + 4*(N/2), a.hi, b.hi, c.hi, d.hi);
}
#if !defined(SKNX_NO_SIMD)
#if defined(__ARM_NEON)
#define IMPL_LOAD4_TRANSPOSED(N, T, VLD) \
template<> \
SK_ALWAYS_INLINE void strided_load4(const T* v, skvx::Vec<N,T>& a, skvx::Vec<N,T>& b, \
                                    skvx::Vec<N,T>& c, skvx::Vec<N,T>& d) { \
    auto mat = VLD(v); \
    a = skvx::bit_pun<skvx::Vec<N,T>>(mat.val[0]); \
    b = skvx::bit_pun<skvx::Vec<N,T>>(mat.val[1]); \
    c = skvx::bit_pun<skvx::Vec<N,T>>(mat.val[2]); \
    d = skvx::bit_pun<skvx::Vec<N,T>>(mat.val[3]); \
}
IMPL_LOAD4_TRANSPOSED(2, uint32_t, vld4_u32);
IMPL_LOAD4_TRANSPOSED(4, uint16_t, vld4_u16);
IMPL_LOAD4_TRANSPOSED(8, uint8_t, vld4_u8);
IMPL_LOAD4_TRANSPOSED(2, int32_t, vld4_s32);
IMPL_LOAD4_TRANSPOSED(4, int16_t, vld4_s16);
IMPL_LOAD4_TRANSPOSED(8, int8_t, vld4_s8);
IMPL_LOAD4_TRANSPOSED(2, float, vld4_f32);
IMPL_LOAD4_TRANSPOSED(4, uint32_t, vld4q_u32);
IMPL_LOAD4_TRANSPOSED(8, uint16_t, vld4q_u16);
IMPL_LOAD4_TRANSPOSED(16, uint8_t, vld4q_u8);
IMPL_LOAD4_TRANSPOSED(4, int32_t, vld4q_s32);
IMPL_LOAD4_TRANSPOSED(8, int16_t, vld4q_s16);
IMPL_LOAD4_TRANSPOSED(16, int8_t, vld4q_s8);
IMPL_LOAD4_TRANSPOSED(4, float, vld4q_f32);
#undef IMPL_LOAD4_TRANSPOSED
#elif defined(__SSE__)
template<>
SK_ALWAYS_INLINE void strided_load4(const float* v, float4& a, float4& b, float4& c, float4& d) {
    using skvx::bit_pun;
    __m128 a_ = _mm_loadu_ps(v);
    __m128 b_ = _mm_loadu_ps(v+4);
    __m128 c_ = _mm_loadu_ps(v+8);
    __m128 d_ = _mm_loadu_ps(v+12);
    _MM_TRANSPOSE4_PS(a_, b_, c_, d_);
    a = bit_pun<float4>(a_);
    b = bit_pun<float4>(b_);
    c = bit_pun<float4>(c_);
    d = bit_pun<float4>(d_);
}
#endif
#endif

// De-interleaving load of 2 vectors.
//
// WARNING: These are really only supported well on NEON. Consider restructuring your data before
// resorting to these methods.
template<typename T>
SK_ALWAYS_INLINE void strided_load2(const T* v, skvx::Vec<1,T>& a, skvx::Vec<1,T>& b) {
    a.val = v[0];
    b.val = v[1];
}
template<int N, typename T>
SK_ALWAYS_INLINE typename std::enable_if<N >= 2, void>::type
strided_load2(const T* v, skvx::Vec<N,T>& a, skvx::Vec<N,T>& b) {
    strided_load2(v, a.lo, b.lo);
    strided_load2(v + 2*(N/2), a.hi, b.hi);
}
#if !defined(SKNX_NO_SIMD)
#if defined(__ARM_NEON)
#define IMPL_LOAD2_TRANSPOSED(N, T, VLD) \
template<> \
SK_ALWAYS_INLINE void strided_load2(const T* v, skvx::Vec<N,T>& a, skvx::Vec<N,T>& b) { \
    auto mat = VLD(v); \
    a = skvx::bit_pun<skvx::Vec<N,T>>(mat.val[0]); \
    b = skvx::bit_pun<skvx::Vec<N,T>>(mat.val[1]); \
}
IMPL_LOAD2_TRANSPOSED(2, uint32_t, vld2_u32);
IMPL_LOAD2_TRANSPOSED(4, uint16_t, vld2_u16);
IMPL_LOAD2_TRANSPOSED(8, uint8_t, vld2_u8);
IMPL_LOAD2_TRANSPOSED(2, int32_t, vld2_s32);
IMPL_LOAD2_TRANSPOSED(4, int16_t, vld2_s16);
IMPL_LOAD2_TRANSPOSED(8, int8_t, vld2_s8);
IMPL_LOAD2_TRANSPOSED(2, float, vld2_f32);
IMPL_LOAD2_TRANSPOSED(4, uint32_t, vld2q_u32);
IMPL_LOAD2_TRANSPOSED(8, uint16_t, vld2q_u16);
IMPL_LOAD2_TRANSPOSED(16, uint8_t, vld2q_u8);
IMPL_LOAD2_TRANSPOSED(4, int32_t, vld2q_s32);
IMPL_LOAD2_TRANSPOSED(8, int16_t, vld2q_s16);
IMPL_LOAD2_TRANSPOSED(16, int8_t, vld2q_s8);
IMPL_LOAD2_TRANSPOSED(4, float, vld2q_f32);
#undef IMPL_LOAD2_TRANSPOSED
#endif
#endif

#if defined(__clang__)
    #pragma STDC FP_CONTRACT DEFAULT
#endif

};  // namespace grvx

#endif
