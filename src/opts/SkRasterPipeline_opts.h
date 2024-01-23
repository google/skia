/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkUtils.h"  // unaligned_{load,store}
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineContextUtils.h"
#include "src/sksl/tracing/SkSLTraceHook.h"

#include <cstdint>
#include <type_traits>

// Every function in this file should be marked static and inline using SI.
#if defined(__clang__) || defined(__GNUC__)
    #define SI __attribute__((always_inline)) static inline
#else
    #define SI static inline
#endif

#if defined(__clang__)
    #define SK_UNROLL _Pragma("unroll")
#else
    #define SK_UNROLL
#endif

#if defined(__clang__)
    template <int N, typename T> using Vec = T __attribute__((ext_vector_type(N)));
#elif defined(__GNUC__)
    // Unfortunately, GCC does not allow us to omit the struct. This will not compile:
    //   template <int N, typename T> using Vec = T __attribute__((vector_size(N*sizeof(T))));
    template <int N, typename T> struct VecHelper {
        typedef T __attribute__((vector_size(N * sizeof(T)))) V;
    };
    template <int N, typename T> using Vec = typename VecHelper<N, T>::V;
#endif

template <typename Dst, typename Src>
SI Dst widen_cast(const Src& src) {
    static_assert(sizeof(Dst) > sizeof(Src));
    static_assert(std::is_trivially_copyable<Dst>::value);
    static_assert(std::is_trivially_copyable<Src>::value);
    Dst dst;
    memcpy(&dst, &src, sizeof(Src));
    return dst;
}

struct Ctx {
    SkRasterPipelineStage* fStage;

    template <typename T>
    operator T*() {
        return (T*)fStage->ctx;
    }
};

using NoCtx = const void*;

#if defined(JUMPER_IS_SCALAR) || defined(JUMPER_IS_NEON) || defined(JUMPER_IS_HSW) || \
        defined(JUMPER_IS_SKX) || defined(JUMPER_IS_AVX) || defined(JUMPER_IS_SSE41) || \
        defined(JUMPER_IS_SSE2)
    // Honor the existing setting
#elif !defined(__clang__) && !defined(__GNUC__)
    #define JUMPER_IS_SCALAR
#elif defined(SK_ARM_HAS_NEON)
    #define JUMPER_IS_NEON
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SKX
    #define JUMPER_IS_SKX
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    #define JUMPER_IS_HSW
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
    #define JUMPER_IS_AVX
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    #define JUMPER_IS_SSE41
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #define JUMPER_IS_SSE2
#else
    #define JUMPER_IS_SCALAR
#endif

// Older Clangs seem to crash when generating non-optimized NEON code for ARMv7.
#if defined(__clang__) && !defined(__OPTIMIZE__) && defined(SK_CPU_ARM32)
    // Apple Clang 9 and vanilla Clang 5 are fine, and may even be conservative.
    #if defined(__apple_build_version__) && __clang_major__ < 9
        #define JUMPER_IS_SCALAR
    #elif __clang_major__ < 5
        #define JUMPER_IS_SCALAR
    #endif

    #if defined(JUMPER_IS_NEON) && defined(JUMPER_IS_SCALAR)
        #undef  JUMPER_IS_NEON
    #endif
#endif

#if defined(JUMPER_IS_SCALAR)
    #include <math.h>
#elif defined(JUMPER_IS_NEON)
    #include <arm_neon.h>
#else
    #include <immintrin.h>
#endif

// Notes:
// * rcp_fast and rcp_precise both produce a reciprocal, but rcp_fast is an estimate with at least
//   12 bits of precision while rcp_precise should be accurate for float size. For ARM rcp_precise
//   requires 2 Newton-Raphson refinement steps because its estimate has 8 bit precision, and for
//   Intel this requires one additional step because its estimate has 12 bit precision.
//
// * Don't call rcp_approx or rsqrt_approx directly; only use rcp_fast and rsqrt.

namespace SK_OPTS_NS {
#if defined(JUMPER_IS_SCALAR)
    // This path should lead to portable scalar code.
    using F   = float   ;
    using I32 =  int32_t;
    using U64 = uint64_t;
    using U32 = uint32_t;
    using U16 = uint16_t;
    using U8  = uint8_t ;

    SI F   min(F a, F b)     { return fminf(a,b); }
    SI I32 min(I32 a, I32 b) { return a < b ? a : b; }
    SI U32 min(U32 a, U32 b) { return a < b ? a : b; }
    SI F   max(F a, F b)     { return fmaxf(a,b); }
    SI I32 max(I32 a, I32 b) { return a > b ? a : b; }
    SI U32 max(U32 a, U32 b) { return a > b ? a : b; }

    SI F   mad(F f, F m, F a)   { return f*m+a; }
    SI F   abs_  (F v)          { return fabsf(v); }
    SI I32 abs_  (I32 v)        { return v < 0 ? -v : v; }
    SI F   floor_(F v)          { return floorf(v); }
    SI F    ceil_(F v)          { return ceilf(v); }
    SI F   rcp_approx(F v)      { return 1.0f / v; }  // use rcp_fast instead
    SI F   rsqrt_approx(F v)    { return 1.0f / sqrtf(v); }
    SI F   sqrt_ (F v)          { return sqrtf(v); }
    SI F   rcp_precise (F v)    { return 1.0f / v; }

    SI U32 round(F v)           { return (uint32_t)(v + 0.5f); }
    SI U32 round(F v, F scale)  { return (uint32_t)(v*scale + 0.5f); }
    SI U16 pack(U32 v)          { return (U16)v; }
    SI U8  pack(U16 v)          { return  (U8)v; }

    SI F   if_then_else(I32 c, F   t, F   e) { return c ? t : e; }
    SI I32 if_then_else(I32 c, I32 t, I32 e) { return c ? t : e; }

    SI bool any(I32 c)                 { return c != 0; }
    SI bool all(I32 c)                 { return c != 0; }

    template <typename T>
    SI T gather(const T* p, U32 ix) { return p[ix]; }

    SI void scatter_masked(I32 src, int* dst, U32 ix, I32 mask) {
        dst[ix] = mask ? src : dst[ix];
    }

    SI void load2(const uint16_t* ptr, U16* r, U16* g) {
        *r = ptr[0];
        *g = ptr[1];
    }
    SI void store2(uint16_t* ptr, U16 r, U16 g) {
        ptr[0] = r;
        ptr[1] = g;
    }
    SI void load4(const uint16_t* ptr, U16* r, U16* g, U16* b, U16* a) {
        *r = ptr[0];
        *g = ptr[1];
        *b = ptr[2];
        *a = ptr[3];
    }
    SI void store4(uint16_t* ptr, U16 r, U16 g, U16 b, U16 a) {
        ptr[0] = r;
        ptr[1] = g;
        ptr[2] = b;
        ptr[3] = a;
    }

    SI void load4(const float* ptr, F* r, F* g, F* b, F* a) {
        *r = ptr[0];
        *g = ptr[1];
        *b = ptr[2];
        *a = ptr[3];
    }
    SI void store4(float* ptr, F r, F g, F b, F a) {
        ptr[0] = r;
        ptr[1] = g;
        ptr[2] = b;
        ptr[3] = a;
    }

#elif defined(JUMPER_IS_NEON)
    template <typename T> using V = Vec<4, T>;
    using F   = V<float   >;
    using I32 = V< int32_t>;
    using U64 = V<uint64_t>;
    using U32 = V<uint32_t>;
    using U16 = V<uint16_t>;
    using U8  = V<uint8_t >;

    // We polyfill a few routines that Clang doesn't build into ext_vector_types.
    SI F   min(F a, F b)     { return vminq_f32(a,b); }
    SI I32 min(I32 a, I32 b) { return vminq_s32(a,b); }
    SI U32 min(U32 a, U32 b) { return vminq_u32(a,b); }
    SI F   max(F a, F b)     { return vmaxq_f32(a,b); }
    SI I32 max(I32 a, I32 b) { return vmaxq_s32(a,b); }
    SI U32 max(U32 a, U32 b) { return vmaxq_u32(a,b); }

    SI F   abs_  (F v)       { return vabsq_f32(v); }
    SI I32 abs_  (I32 v)     { return vabsq_s32(v); }
    SI F   rcp_approx(F v)   { auto e = vrecpeq_f32(v);  return vrecpsq_f32 (v,e  ) * e; }
    SI F   rcp_precise(F v)  { auto e = rcp_approx(v);   return vrecpsq_f32 (v,e  ) * e; }
    SI F   rsqrt_approx(F v) { auto e = vrsqrteq_f32(v); return vrsqrtsq_f32(v,e*e) * e; }

    SI U16 pack(U32 v)       { return __builtin_convertvector(v, U16); }
    SI U8  pack(U16 v)       { return __builtin_convertvector(v,  U8); }

    SI F   if_then_else(I32 c, F   t, F   e) { return vbslq_f32((U32)c,t,e); }
    SI I32 if_then_else(I32 c, I32 t, I32 e) { return vbslq_s32((U32)c,t,e); }

    #if defined(SK_CPU_ARM64)
        SI bool any(I32 c) { return vmaxvq_u32((U32)c) != 0; }
        SI bool all(I32 c) { return vminvq_u32((U32)c) != 0; }

        SI F     mad(F f, F m, F a) { return vfmaq_f32(a,f,m); }
        SI F  floor_(F v) { return vrndmq_f32(v); }
        SI F   ceil_(F v) { return vrndpq_f32(v); }
        SI F   sqrt_(F v) { return vsqrtq_f32(v); }
        SI U32 round(F v) { return vcvtnq_u32_f32(v); }
        SI U32 round(F v, F scale) { return vcvtnq_u32_f32(v*scale); }
    #else
        SI bool any(I32 c) { return c[0] | c[1] | c[2] | c[3]; }
        SI bool all(I32 c) { return c[0] & c[1] & c[2] & c[3]; }

        SI F mad(F f, F m, F a) { return vmlaq_f32(a,f,m); }
        SI F floor_(F v) {
            F roundtrip = vcvtq_f32_s32(vcvtq_s32_f32(v));
            return roundtrip - if_then_else(roundtrip > v, F(1), F(0));
        }

        SI F ceil_(F v) {
            F roundtrip = vcvtq_f32_s32(vcvtq_s32_f32(v));
            return roundtrip + if_then_else(roundtrip < v, F(1), F(0));
        }

        SI F sqrt_(F v) {
            auto e = vrsqrteq_f32(v);  // Estimate and two refinement steps for e = rsqrt(v).
            e *= vrsqrtsq_f32(v,e*e);
            e *= vrsqrtsq_f32(v,e*e);
            return v*e;                // sqrt(v) == v*rsqrt(v).
        }

        SI U32 round(F v) {
            return vcvtq_u32_f32(v + 0.5f);
        }

        SI U32 round(F v, F scale) {
            return vcvtq_u32_f32(mad(v,scale,0.5f));
        }
    #endif

    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return V<T>{p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]};
    }
    SI void scatter_masked(I32 src, int* dst, U32 ix, I32 mask) {
        I32 before = gather(dst, ix);
        I32 after = if_then_else(mask, src, before);
        dst[ix[0]] = after[0];
        dst[ix[1]] = after[1];
        dst[ix[2]] = after[2];
        dst[ix[3]] = after[3];
    }
    SI void load2(const uint16_t* ptr, U16* r, U16* g) {
        uint16x4x2_t rg = vld2_u16(ptr);
        *r = rg.val[0];
        *g = rg.val[1];
    }
    SI void store2(uint16_t* ptr, U16 r, U16 g) {
        vst2_u16(ptr, (uint16x4x2_t{{r,g}}));
    }
    SI void load4(const uint16_t* ptr, U16* r, U16* g, U16* b, U16* a) {
        uint16x4x4_t rgba = vld4_u16(ptr);
        *r = rgba.val[0];
        *g = rgba.val[1];
        *b = rgba.val[2];
        *a = rgba.val[3];
    }

    SI void store4(uint16_t* ptr, U16 r, U16 g, U16 b, U16 a) {
        vst4_u16(ptr, (uint16x4x4_t{{r,g,b,a}}));
    }
    SI void load4(const float* ptr, F* r, F* g, F* b, F* a) {
        float32x4x4_t rgba = vld4q_f32(ptr);
        *r = rgba.val[0];
        *g = rgba.val[1];
        *b = rgba.val[2];
        *a = rgba.val[3];
    }
    SI void store4(float* ptr, F r, F g, F b, F a) {
        vst4q_f32(ptr, (float32x4x4_t{{r,g,b,a}}));
    }

#elif defined(JUMPER_IS_SKX)
    template <typename T> using V = Vec<16, T>;
    using F   = V<float   >;
    using I32 = V< int32_t>;
    using U64 = V<uint64_t>;
    using U32 = V<uint32_t>;
    using U16 = V<uint16_t>;
    using U8  = V<uint8_t >;

    SI F   mad(F f, F m, F a) { return _mm512_fmadd_ps(f, m, a); }
    SI F   min(F a, F b)     { return _mm512_min_ps(a,b);    }
    SI I32 min(I32 a, I32 b) { return (I32)_mm512_min_epi32((__m512i)a,(__m512i)b); }
    SI U32 min(U32 a, U32 b) { return (U32)_mm512_min_epu32((__m512i)a,(__m512i)b); }
    SI F   max(F a, F b)     { return _mm512_max_ps(a,b);    }
    SI I32 max(I32 a, I32 b) { return (I32)_mm512_max_epi32((__m512i)a,(__m512i)b); }
    SI U32 max(U32 a, U32 b) { return (U32)_mm512_max_epu32((__m512i)a,(__m512i)b); }
    SI F   abs_  (F v)   { return _mm512_and_ps(v, _mm512_sub_ps(_mm512_setzero(), v)); }
    SI I32 abs_  (I32 v) { return (I32)_mm512_abs_epi32((__m512i)v);   }
    SI F   floor_(F v)   { return _mm512_floor_ps(v);    }
    SI F   ceil_(F v)    { return _mm512_ceil_ps(v);     }
    SI F   rcp_approx(F v) { return _mm512_rcp14_ps  (v);  }
    SI F   rsqrt_approx (F v)   { return _mm512_rsqrt14_ps(v);  }
    SI F   sqrt_ (F v)   { return _mm512_sqrt_ps (v);    }
    SI F rcp_precise (F v) {
        F e = rcp_approx(v);
        return _mm512_fnmadd_ps(v, e, _mm512_set1_ps(2.0f)) * e;
    }
    SI U32 round(F v)          { return (U32)_mm512_cvtps_epi32(v); }
    SI U32 round(F v, F scale) { return (U32)_mm512_cvtps_epi32(v*scale); }
    SI U16 pack(U32 v) {
        __m256i rst = _mm256_packus_epi32(_mm512_castsi512_si256((__m512i)v),
                                _mm512_extracti64x4_epi64((__m512i)v, 1));
        return (U16)_mm256_permutex_epi64(rst, 216);
    }
    SI U8 pack(U16 v) {
        __m256i rst = _mm256_packus_epi16((__m256i)v, (__m256i)v);
        return (U8)_mm256_castsi256_si128(_mm256_permute4x64_epi64(rst, 8));
    }
    SI F if_then_else(I32 c, F t, F e) {
        __m512i mask = _mm512_set1_epi32(0x80000000);
        __m512i aa = _mm512_and_si512((__m512i)c, mask);
        return _mm512_mask_blend_ps(_mm512_test_epi32_mask(aa, aa),e,t);
    }
    SI I32 if_then_else(I32 c, I32 t, I32 e) {
        __m512i mask = _mm512_set1_epi32(0x80000000);
        __m512i aa = _mm512_and_si512((__m512i)c, mask);
        return (I32)_mm512_mask_blend_epi32(_mm512_test_epi32_mask(aa, aa),(__m512i)e,(__m512i)t);
    }
    SI bool any(I32 c) {
        __mmask16 mask32 = _mm512_test_epi32_mask((__m512i)c, (__m512i)c);
        return mask32 != 0;
    }
    SI bool all(I32 c) {
        __mmask16 mask32 = _mm512_test_epi32_mask((__m512i)c, (__m512i)c);
        return mask32 == 0xffff;
    }
    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return V<T>{ p[ix[ 0]], p[ix[ 1]], p[ix[ 2]], p[ix[ 3]],
                     p[ix[ 4]], p[ix[ 5]], p[ix[ 6]], p[ix[ 7]],
                     p[ix[ 8]], p[ix[ 9]], p[ix[10]], p[ix[11]],
                     p[ix[12]], p[ix[13]], p[ix[14]], p[ix[15]] };
    }
    SI F   gather(const float* p, U32 ix) { return _mm512_i32gather_ps((__m512i)ix, p, 4); }
    SI U32 gather(const uint32_t* p, U32 ix) {
        return (U32)_mm512_i32gather_epi32((__m512i)ix, p, 4); }
    SI U64 gather(const uint64_t* p, U32 ix) {
        __m512i parts[] = {
            _mm512_i32gather_epi64(_mm512_castsi512_si256((__m512i)ix), p, 8),
            _mm512_i32gather_epi64(_mm512_extracti32x8_epi32((__m512i)ix, 1), p, 8),
        };
        return sk_bit_cast<U64>(parts);
    }
    template <typename V, typename S>
    SI void scatter_masked(V src, S* dst, U32 ix, I32 mask) {
        V before = gather(dst, ix);
        V after = if_then_else(mask, src, before);
        dst[ix[0]] = after[0];
        dst[ix[1]] = after[1];
        dst[ix[2]] = after[2];
        dst[ix[3]] = after[3];
        dst[ix[4]] = after[4];
        dst[ix[5]] = after[5];
        dst[ix[6]] = after[6];
        dst[ix[7]] = after[7];
        dst[ix[8]] = after[8];
        dst[ix[9]] = after[9];
        dst[ix[10]] = after[10];
        dst[ix[11]] = after[11];
        dst[ix[12]] = after[12];
        dst[ix[13]] = after[13];
        dst[ix[14]] = after[14];
        dst[ix[15]] = after[15];
    }

    SI void load2(const uint16_t* ptr, U16* r, U16* g) {
        __m256i _01234567 = _mm256_loadu_si256(((const __m256i*)ptr) + 0);
        __m256i _89abcdef = _mm256_loadu_si256(((const __m256i*)ptr) + 1);

        *r = (U16)_mm256_permute4x64_epi64(_mm256_packs_epi32(_mm256_srai_epi32(_mm256_slli_epi32
            (_01234567, 16), 16), _mm256_srai_epi32(_mm256_slli_epi32(_89abcdef, 16), 16)), 216);
        *g = (U16)_mm256_permute4x64_epi64(_mm256_packs_epi32(_mm256_srai_epi32(_01234567, 16),
                             _mm256_srai_epi32(_89abcdef, 16)), 216);
    }
    SI void store2(uint16_t* ptr, U16 r, U16 g) {
        __m256i _01234567 = _mm256_unpacklo_epi16((__m256i)r, (__m256i)g);
        __m256i _89abcdef = _mm256_unpackhi_epi16((__m256i)r, (__m256i)g);
        __m512i combinedVector = _mm512_inserti64x4(_mm512_castsi256_si512(_01234567),
                    _89abcdef, 1);
        __m512i aa = _mm512_permutexvar_epi64(_mm512_setr_epi64(0,1,4,5,2,3,6,7), combinedVector);
        _01234567 = _mm512_castsi512_si256(aa);
        _89abcdef = _mm512_extracti64x4_epi64(aa, 1);

        _mm256_storeu_si256((__m256i*)ptr + 0, _01234567);
        _mm256_storeu_si256((__m256i*)ptr + 1, _89abcdef);
    }

    SI void load4(const uint16_t* ptr, U16* r, U16* g, U16* b, U16* a) {
        __m256i _0123 = _mm256_loadu_si256((const __m256i*)ptr),
                _4567 = _mm256_loadu_si256(((const __m256i*)ptr) + 1),
                _89ab = _mm256_loadu_si256(((const __m256i*)ptr) + 2),
                _cdef = _mm256_loadu_si256(((const __m256i*)ptr) + 3);

        auto a0 = _mm256_unpacklo_epi16(_0123, _4567),
             a1 = _mm256_unpackhi_epi16(_0123, _4567),
             b0 = _mm256_unpacklo_epi16(a0, a1),
             b1 = _mm256_unpackhi_epi16(a0, a1),
             a2 = _mm256_unpacklo_epi16(_89ab, _cdef),
             a3 = _mm256_unpackhi_epi16(_89ab, _cdef),
             b2 = _mm256_unpacklo_epi16(a2, a3),
             b3 = _mm256_unpackhi_epi16(a2, a3),
             rr = _mm256_unpacklo_epi64(b0, b2),
             gg = _mm256_unpackhi_epi64(b0, b2),
             bb = _mm256_unpacklo_epi64(b1, b3),
             aa = _mm256_unpackhi_epi64(b1, b3);

        *r = (U16)_mm256_permutexvar_epi32(_mm256_setr_epi32(0,4,1,5,2,6,3,7), rr);
        *g = (U16)_mm256_permutexvar_epi32(_mm256_setr_epi32(0,4,1,5,2,6,3,7), gg);
        *b = (U16)_mm256_permutexvar_epi32(_mm256_setr_epi32(0,4,1,5,2,6,3,7), bb);
        *a = (U16)_mm256_permutexvar_epi32(_mm256_setr_epi32(0,4,1,5,2,6,3,7), aa);
    }
    SI void store4(uint16_t* ptr, U16 r, U16 g, U16 b, U16 a) {
        auto rg012389ab = _mm256_unpacklo_epi16((__m256i)r, (__m256i)g),
             rg4567cdef = _mm256_unpackhi_epi16((__m256i)r, (__m256i)g),
             ba012389ab = _mm256_unpacklo_epi16((__m256i)b, (__m256i)a),
             ba4567cdef = _mm256_unpackhi_epi16((__m256i)b, (__m256i)a);

        auto _0189 = _mm256_unpacklo_epi32(rg012389ab, ba012389ab),
             _23ab = _mm256_unpackhi_epi32(rg012389ab, ba012389ab),
             _45cd = _mm256_unpacklo_epi32(rg4567cdef, ba4567cdef),
             _67ef = _mm256_unpackhi_epi32(rg4567cdef, ba4567cdef);

        auto _ab23 = _mm256_permutex_epi64(_23ab, 78),
             _0123 = _mm256_blend_epi32(_0189, _ab23, 0xf0),
             _89ab = _mm256_permutex_epi64(_mm256_blend_epi32(_0189, _ab23, 0x0f), 78),
             _ef67 = _mm256_permutex_epi64(_67ef, 78),
             _4567 = _mm256_blend_epi32(_45cd, _ef67, 0xf0),
             _cdef = _mm256_permutex_epi64(_mm256_blend_epi32(_45cd, _ef67, 0x0f), 78);

        _mm256_storeu_si256((__m256i*)ptr, _0123);
        _mm256_storeu_si256((__m256i*)ptr + 1, _4567);
        _mm256_storeu_si256((__m256i*)ptr + 2, _89ab);
        _mm256_storeu_si256((__m256i*)ptr + 3, _cdef);
    }

    SI void load4(const float* ptr, F* r, F* g, F* b, F* a) {
        F _048c, _159d, _26ae, _37bf;

        _048c = _mm512_castps128_ps512(_mm_loadu_ps(ptr)         );
        _048c = _mm512_insertf32x4(_048c, _mm_loadu_ps(ptr+16), 1);
        _048c = _mm512_insertf32x4(_048c, _mm_loadu_ps(ptr+32), 2);
        _048c = _mm512_insertf32x4(_048c, _mm_loadu_ps(ptr+48), 3);
        _159d = _mm512_castps128_ps512(_mm_loadu_ps(ptr+4)       );
        _159d = _mm512_insertf32x4(_159d, _mm_loadu_ps(ptr+20), 1);
        _159d = _mm512_insertf32x4(_159d, _mm_loadu_ps(ptr+36), 2);
        _159d = _mm512_insertf32x4(_159d, _mm_loadu_ps(ptr+52), 3);
        _26ae = _mm512_castps128_ps512(_mm_loadu_ps(ptr+8)       );
        _26ae = _mm512_insertf32x4(_26ae, _mm_loadu_ps(ptr+24), 1);
        _26ae = _mm512_insertf32x4(_26ae, _mm_loadu_ps(ptr+40), 2);
        _26ae = _mm512_insertf32x4(_26ae, _mm_loadu_ps(ptr+56), 3);
        _37bf = _mm512_castps128_ps512(_mm_loadu_ps(ptr+12)      );
        _37bf = _mm512_insertf32x4(_37bf, _mm_loadu_ps(ptr+28), 1);
        _37bf = _mm512_insertf32x4(_37bf, _mm_loadu_ps(ptr+44), 2);
        _37bf = _mm512_insertf32x4(_37bf, _mm_loadu_ps(ptr+60), 3);

        F rg02468acf = _mm512_unpacklo_ps(_048c, _26ae),
          ba02468acf = _mm512_unpackhi_ps(_048c, _26ae),
          rg13579bde = _mm512_unpacklo_ps(_159d, _37bf),
          ba13579bde = _mm512_unpackhi_ps(_159d, _37bf);

        *r = (F)_mm512_unpacklo_ps(rg02468acf, rg13579bde);
        *g = (F)_mm512_unpackhi_ps(rg02468acf, rg13579bde);
        *b = (F)_mm512_unpacklo_ps(ba02468acf, ba13579bde);
        *a = (F)_mm512_unpackhi_ps(ba02468acf, ba13579bde);
    }

    SI void store4(float* ptr, F r, F g, F b, F a) {
        F rg014589cd = _mm512_unpacklo_ps(r, g),
          rg2367abef = _mm512_unpackhi_ps(r, g),
          ba014589cd = _mm512_unpacklo_ps(b, a),
          ba2367abef = _mm512_unpackhi_ps(b, a);

        F _048c = (F)_mm512_unpacklo_pd((__m512d)rg014589cd, (__m512d)ba014589cd),
          _26ae = (F)_mm512_unpacklo_pd((__m512d)rg2367abef, (__m512d)ba2367abef),
          _159d = (F)_mm512_unpackhi_pd((__m512d)rg014589cd, (__m512d)ba014589cd),
          _37bf = (F)_mm512_unpackhi_pd((__m512d)rg2367abef, (__m512d)ba2367abef);

        F _ae26 = (F)_mm512_permutexvar_pd(_mm512_setr_epi64(4,5,6,7,0,1,2,3), (__m512d)_26ae),
          _bf37 = (F)_mm512_permutexvar_pd(_mm512_setr_epi64(4,5,6,7,0,1,2,3), (__m512d)_37bf),
          _8c04 = (F)_mm512_permutexvar_pd(_mm512_setr_epi64(4,5,6,7,0,1,2,3), (__m512d)_048c),
          _9d15 = (F)_mm512_permutexvar_pd(_mm512_setr_epi64(4,5,6,7,0,1,2,3), (__m512d)_159d);

        __m512i index = _mm512_setr_epi32(4,5,6,7,0,1,2,3,12,13,14,15,8,9,10,11);
        F _0426 = (F)_mm512_permutex2var_pd((__m512d)_048c, _mm512_setr_epi64(0,1,2,3,12,13,14,15),
                    (__m512d)_ae26),
          _1537 = (F)_mm512_permutex2var_pd((__m512d)_159d, _mm512_setr_epi64(0,1,2,3,12,13,14,15),
                    (__m512d)_bf37),
          _5173 = _mm512_permutexvar_ps(index, _1537),
          _0123 = (F)_mm512_permutex2var_pd((__m512d)_0426, _mm512_setr_epi64(0,1,10,11,4,5,14,15),
                    (__m512d)_5173);

        F _5476 = (F)_mm512_permutex2var_pd((__m512d)_5173, _mm512_setr_epi64(0,1,10,11,4,5,14,15),
                    (__m512d)_0426),
          _4567 = _mm512_permutexvar_ps(index, _5476),
          _8cae = (F)_mm512_permutex2var_pd((__m512d)_8c04, _mm512_setr_epi64(0,1,2,3,12,13,14,15),
                    (__m512d)_26ae),
          _9dbf = (F)_mm512_permutex2var_pd((__m512d)_9d15, _mm512_setr_epi64(0,1,2,3,12,13,14,15),
                    (__m512d)_37bf),
          _d9fb = _mm512_permutexvar_ps(index, _9dbf),
          _89ab = (F)_mm512_permutex2var_pd((__m512d)_8cae, _mm512_setr_epi64(0,1,10,11,4,5,14,15),
                    (__m512d)_d9fb),
          _dcfe = (F)_mm512_permutex2var_pd((__m512d)_d9fb, _mm512_setr_epi64(0,1,10,11,4,5,14,15),
                    (__m512d)_8cae),
          _cdef = _mm512_permutexvar_ps(index, _dcfe);

        _mm512_storeu_ps(ptr+0, _0123);
        _mm512_storeu_ps(ptr+16, _4567);
        _mm512_storeu_ps(ptr+32, _89ab);
        _mm512_storeu_ps(ptr+48, _cdef);
    }

#elif defined(JUMPER_IS_HSW)
    // These are __m256 and __m256i, but friendlier and strongly-typed.
    template <typename T> using V = Vec<8, T>;
    using F   = V<float   >;
    using I32 = V< int32_t>;
    using U64 = V<uint64_t>;
    using U32 = V<uint32_t>;
    using U16 = V<uint16_t>;
    using U8  = V<uint8_t >;

    SI F   mad(F f, F m, F a) { return _mm256_fmadd_ps(f, m, a); }

    SI F   min(F a, F b)     { return _mm256_min_ps(a,b);    }
    SI I32 min(I32 a, I32 b) { return (I32)_mm256_min_epi32((__m256i)a,(__m256i)b); }
    SI U32 min(U32 a, U32 b) { return (U32)_mm256_min_epu32((__m256i)a,(__m256i)b); }
    SI F   max(F a, F b)     { return _mm256_max_ps(a,b);    }
    SI I32 max(I32 a, I32 b) { return (I32)_mm256_max_epi32((__m256i)a,(__m256i)b); }
    SI U32 max(U32 a, U32 b) { return (U32)_mm256_max_epu32((__m256i)a,(__m256i)b); }

    SI F   abs_  (F v)       { return _mm256_and_ps(v, 0-v); }
    SI I32 abs_  (I32 v)     { return (I32)_mm256_abs_epi32((__m256i)v); }
    SI F   floor_(F v)       { return _mm256_floor_ps(v);    }
    SI F   ceil_(F v)        { return _mm256_ceil_ps(v);     }
    SI F   rcp_approx(F v)   { return _mm256_rcp_ps  (v);    }  // use rcp_fast instead
    SI F   rsqrt_approx(F v) { return _mm256_rsqrt_ps(v);    }
    SI F   sqrt_ (F v)       { return _mm256_sqrt_ps (v);    }
    SI F   rcp_precise (F v) {
        F e = rcp_approx(v);
        return _mm256_fnmadd_ps(v, e, _mm256_set1_ps(2.0f)) * e;
    }

    SI U32 round(F v)          { return (U32)_mm256_cvtps_epi32(v); }
    SI U32 round(F v, F scale) { return (U32)_mm256_cvtps_epi32(v*scale); }
    SI U16 pack(U32 v) {
        return (U16)_mm_packus_epi32(_mm256_extractf128_si256((__m256i)v, 0),
                                     _mm256_extractf128_si256((__m256i)v, 1));
    }
    SI U8 pack(U16 v) {
        auto r = _mm_packus_epi16((__m128i)v,(__m128i)v);
        return sk_unaligned_load<U8>(&r);
    }

    SI F   if_then_else(I32 c, F   t, F   e) { return _mm256_blendv_ps(e, t, (__m256)c); }
    SI I32 if_then_else(I32 c, I32 t, I32 e) {
        return (I32)_mm256_blendv_ps((__m256)e, (__m256)t, (__m256)c);
    }

    // NOTE: This version of 'all' only works with mask values (true == all bits set)
    SI bool any(I32 c) { return !_mm256_testz_si256((__m256i)c, _mm256_set1_epi32(-1)); }
    SI bool all(I32 c) { return  _mm256_testc_si256((__m256i)c, _mm256_set1_epi32(-1)); }

    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return V<T>{ p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]],
                     p[ix[4]], p[ix[5]], p[ix[6]], p[ix[7]], };
    }
    SI F   gather(const float*    p, U32 ix) { return _mm256_i32gather_ps(p, (__m256i)ix, 4); }
    SI U32 gather(const uint32_t* p, U32 ix) {
        return (U32)_mm256_i32gather_epi32((const int*)p, (__m256i)ix, 4);
    }
    SI U64 gather(const uint64_t* p, U32 ix) {
        __m256i parts[] = {
                _mm256_i32gather_epi64(
                        (const long long int*)p, _mm256_extracti128_si256((__m256i)ix, 0), 8),
                _mm256_i32gather_epi64(
                        (const long long int*)p, _mm256_extracti128_si256((__m256i)ix, 1), 8),
        };
        return sk_bit_cast<U64>(parts);
    }
    SI void scatter_masked(I32 src, int* dst, U32 ix, I32 mask) {
        I32 before = gather(dst, ix);
        I32 after = if_then_else(mask, src, before);
        dst[ix[0]] = after[0];
        dst[ix[1]] = after[1];
        dst[ix[2]] = after[2];
        dst[ix[3]] = after[3];
        dst[ix[4]] = after[4];
        dst[ix[5]] = after[5];
        dst[ix[6]] = after[6];
        dst[ix[7]] = after[7];
    }

    SI void load2(const uint16_t* ptr, U16* r, U16* g) {
        __m128i _0123 = _mm_loadu_si128(((const __m128i*)ptr) + 0),
                _4567 = _mm_loadu_si128(((const __m128i*)ptr) + 1);
        *r = (U16)_mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32(_0123, 16), 16),
                                  _mm_srai_epi32(_mm_slli_epi32(_4567, 16), 16));
        *g = (U16)_mm_packs_epi32(_mm_srai_epi32(_0123, 16),
                                  _mm_srai_epi32(_4567, 16));
    }
    SI void store2(uint16_t* ptr, U16 r, U16 g) {
        auto _0123 = _mm_unpacklo_epi16((__m128i)r, (__m128i)g),
             _4567 = _mm_unpackhi_epi16((__m128i)r, (__m128i)g);
        _mm_storeu_si128((__m128i*)ptr + 0, _0123);
        _mm_storeu_si128((__m128i*)ptr + 1, _4567);
    }

    SI void load4(const uint16_t* ptr, U16* r, U16* g, U16* b, U16* a) {
        __m128i _01 = _mm_loadu_si128(((const __m128i*)ptr) + 0),
                _23 = _mm_loadu_si128(((const __m128i*)ptr) + 1),
                _45 = _mm_loadu_si128(((const __m128i*)ptr) + 2),
                _67 = _mm_loadu_si128(((const __m128i*)ptr) + 3);

        auto _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
             _13 = _mm_unpackhi_epi16(_01, _23),  // r1 r3 g1 g3 b1 b3 a1 a3
             _46 = _mm_unpacklo_epi16(_45, _67),
             _57 = _mm_unpackhi_epi16(_45, _67);

        auto rg0123 = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
             ba0123 = _mm_unpackhi_epi16(_02, _13),  // b0 b1 b2 b3 a0 a1 a2 a3
             rg4567 = _mm_unpacklo_epi16(_46, _57),
             ba4567 = _mm_unpackhi_epi16(_46, _57);

        *r = (U16)_mm_unpacklo_epi64(rg0123, rg4567);
        *g = (U16)_mm_unpackhi_epi64(rg0123, rg4567);
        *b = (U16)_mm_unpacklo_epi64(ba0123, ba4567);
        *a = (U16)_mm_unpackhi_epi64(ba0123, ba4567);
    }
    SI void store4(uint16_t* ptr, U16 r, U16 g, U16 b, U16 a) {
        auto rg0123 = _mm_unpacklo_epi16((__m128i)r, (__m128i)g),  // r0 g0 r1 g1 r2 g2 r3 g3
             rg4567 = _mm_unpackhi_epi16((__m128i)r, (__m128i)g),  // r4 g4 r5 g5 r6 g6 r7 g7
             ba0123 = _mm_unpacklo_epi16((__m128i)b, (__m128i)a),
             ba4567 = _mm_unpackhi_epi16((__m128i)b, (__m128i)a);

        auto _01 = _mm_unpacklo_epi32(rg0123, ba0123),
             _23 = _mm_unpackhi_epi32(rg0123, ba0123),
             _45 = _mm_unpacklo_epi32(rg4567, ba4567),
             _67 = _mm_unpackhi_epi32(rg4567, ba4567);

        _mm_storeu_si128((__m128i*)ptr + 0, _01);
        _mm_storeu_si128((__m128i*)ptr + 1, _23);
        _mm_storeu_si128((__m128i*)ptr + 2, _45);
        _mm_storeu_si128((__m128i*)ptr + 3, _67);
    }

    SI void load4(const float* ptr, F* r, F* g, F* b, F* a) {
        F _04 = _mm256_castps128_ps256(_mm_loadu_ps(ptr+ 0)),
          _15 = _mm256_castps128_ps256(_mm_loadu_ps(ptr+ 4)),
          _26 = _mm256_castps128_ps256(_mm_loadu_ps(ptr+ 8)),
          _37 = _mm256_castps128_ps256(_mm_loadu_ps(ptr+12));
        _04 = _mm256_insertf128_ps(_04, _mm_loadu_ps(ptr+16), 1);
        _15 = _mm256_insertf128_ps(_15, _mm_loadu_ps(ptr+20), 1);
        _26 = _mm256_insertf128_ps(_26, _mm_loadu_ps(ptr+24), 1);
        _37 = _mm256_insertf128_ps(_37, _mm_loadu_ps(ptr+28), 1);

        F rg0145 = _mm256_unpacklo_ps(_04,_15),  // r0 r1 g0 g1 | r4 r5 g4 g5
          ba0145 = _mm256_unpackhi_ps(_04,_15),
          rg2367 = _mm256_unpacklo_ps(_26,_37),
          ba2367 = _mm256_unpackhi_ps(_26,_37);

        *r = (F)_mm256_unpacklo_pd((__m256d)rg0145, (__m256d)rg2367);
        *g = (F)_mm256_unpackhi_pd((__m256d)rg0145, (__m256d)rg2367);
        *b = (F)_mm256_unpacklo_pd((__m256d)ba0145, (__m256d)ba2367);
        *a = (F)_mm256_unpackhi_pd((__m256d)ba0145, (__m256d)ba2367);
    }
    SI void store4(float* ptr, F r, F g, F b, F a) {
        F rg0145 = _mm256_unpacklo_ps(r, g),  // r0 g0 r1 g1 | r4 g4 r5 g5
          rg2367 = _mm256_unpackhi_ps(r, g),  // r2 ...      | r6 ...
          ba0145 = _mm256_unpacklo_ps(b, a),  // b0 a0 b1 a1 | b4 a4 b5 a5
          ba2367 = _mm256_unpackhi_ps(b, a);  // b2 ...      | b6 ...

        F _04 = (F)_mm256_unpacklo_pd((__m256d)rg0145, (__m256d)ba0145),// r0 g0 b0 a0 | r4 g4 b4 a4
          _15 = (F)_mm256_unpackhi_pd((__m256d)rg0145, (__m256d)ba0145),// r1 ...      | r5 ...
          _26 = (F)_mm256_unpacklo_pd((__m256d)rg2367, (__m256d)ba2367),// r2 ...      | r6 ...
          _37 = (F)_mm256_unpackhi_pd((__m256d)rg2367, (__m256d)ba2367);// r3 ...      | r7 ...

        F _01 = _mm256_permute2f128_ps(_04, _15, 32),  // 32 == 0010 0000 == lo, lo
          _23 = _mm256_permute2f128_ps(_26, _37, 32),
          _45 = _mm256_permute2f128_ps(_04, _15, 49),  // 49 == 0011 0001 == hi, hi
          _67 = _mm256_permute2f128_ps(_26, _37, 49);
        _mm256_storeu_ps(ptr+ 0, _01);
        _mm256_storeu_ps(ptr+ 8, _23);
        _mm256_storeu_ps(ptr+16, _45);
        _mm256_storeu_ps(ptr+24, _67);
    }

#elif defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    template <typename T> using V = Vec<4, T>;
    using F   = V<float   >;
    using I32 = V< int32_t>;
    using U64 = V<uint64_t>;
    using U32 = V<uint32_t>;
    using U16 = V<uint16_t>;
    using U8  = V<uint8_t >;

    SI F if_then_else(I32 c, F t, F e) {
        return _mm_or_ps(_mm_and_ps((__m128)c, t), _mm_andnot_ps((__m128)c, e));
    }
    SI I32 if_then_else(I32 c, I32 t, I32 e) {
        return (I32)_mm_or_ps(_mm_and_ps((__m128)c, (__m128)t),
                              _mm_andnot_ps((__m128)c, (__m128)e));
    }

    SI F   min(F a, F b)     { return _mm_min_ps(a,b); }
    SI F   max(F a, F b)     { return _mm_max_ps(a,b); }
#if defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    SI I32 min(I32 a, I32 b) { return (I32)_mm_min_epi32((__m128i)a,(__m128i)b); }
    SI U32 min(U32 a, U32 b) { return (U32)_mm_min_epu32((__m128i)a,(__m128i)b); }
    SI I32 max(I32 a, I32 b) { return (I32)_mm_max_epi32((__m128i)a,(__m128i)b); }
    SI U32 max(U32 a, U32 b) { return (U32)_mm_max_epu32((__m128i)a,(__m128i)b); }
#else
    SI I32 min(I32 a, I32 b) { return if_then_else(a < b, a, b); }
    SI I32 max(I32 a, I32 b) { return if_then_else(a > b, a, b); }
    SI U32 min(U32 a, U32 b) {
        return sk_bit_cast<U32>(if_then_else(a < b, sk_bit_cast<I32>(a), sk_bit_cast<I32>(b)));
    }
    SI U32 max(U32 a, U32 b) {
        return sk_bit_cast<U32>(if_then_else(a > b, sk_bit_cast<I32>(a), sk_bit_cast<I32>(b)));
    }
#endif

    SI F   mad(F f, F m, F a)  { return f*m+a;              }
    SI F   abs_(F v)           { return _mm_and_ps(v, 0-v); }
#if defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    SI I32 abs_(I32 v)         { return (I32)_mm_abs_epi32((__m128i)v); }
#else
    SI I32 abs_(I32 v)         { return max(v, -v); }
#endif
    SI F   rcp_approx(F v)     { return _mm_rcp_ps  (v);    }  // use rcp_fast instead
    SI F   rcp_precise (F v)   { F e = rcp_approx(v); return e * (2.0f - v * e); }
    SI F   rsqrt_approx(F v)   { return _mm_rsqrt_ps(v);    }
    SI F    sqrt_(F v)         { return _mm_sqrt_ps (v);    }

    SI U32 round(F v)          { return (U32)_mm_cvtps_epi32(v); }
    SI U32 round(F v, F scale) { return (U32)_mm_cvtps_epi32(v*scale); }

    SI U16 pack(U32 v) {
    #if defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
        auto p = _mm_packus_epi32((__m128i)v,(__m128i)v);
    #else
        // Sign extend so that _mm_packs_epi32() does the pack we want.
        auto p = _mm_srai_epi32(_mm_slli_epi32((__m128i)v, 16), 16);
        p = _mm_packs_epi32(p,p);
    #endif
        return sk_unaligned_load<U16>(&p);  // We have two copies.  Return (the lower) one.
    }
    SI U8 pack(U16 v) {
        auto r = widen_cast<__m128i>(v);
        r = _mm_packus_epi16(r,r);
        return sk_unaligned_load<U8>(&r);
    }

    // NOTE: This only checks the top bit of each lane, and is incorrect with non-mask values.
    SI bool any(I32 c) { return _mm_movemask_ps(sk_bit_cast<F>(c)) != 0b0000; }
    SI bool all(I32 c) { return _mm_movemask_ps(sk_bit_cast<F>(c)) == 0b1111; }

    SI F floor_(F v) {
    #if defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
        return _mm_floor_ps(v);
    #else
        F roundtrip = _mm_cvtepi32_ps(_mm_cvttps_epi32(v));
        return roundtrip - if_then_else(roundtrip > v, F() + 1, F() + 0);
    #endif
    }

    SI F ceil_(F v) {
    #if defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
        return _mm_ceil_ps(v);
    #else
        F roundtrip = _mm_cvtepi32_ps(_mm_cvttps_epi32(v));
        return roundtrip + if_then_else(roundtrip < v, F() + 1, F() + 0);
    #endif
    }

    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return V<T>{p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]};
    }
    SI void scatter_masked(I32 src, int* dst, U32 ix, I32 mask) {
        I32 before = gather(dst, ix);
        I32 after = if_then_else(mask, src, before);
        dst[ix[0]] = after[0];
        dst[ix[1]] = after[1];
        dst[ix[2]] = after[2];
        dst[ix[3]] = after[3];
    }
    SI void load2(const uint16_t* ptr, U16* r, U16* g) {
        __m128i _01 = _mm_loadu_si128(((const __m128i*)ptr) + 0); // r0 g0 r1 g1 r2 g2 r3 g3
        auto rg01_23 = _mm_shufflelo_epi16(_01, 0xD8);            // r0 r1 g0 g1 r2 g2 r3 g3
        auto rg      = _mm_shufflehi_epi16(rg01_23, 0xD8);        // r0 r1 g0 g1 r2 r3 g2 g3

        auto R = _mm_shuffle_epi32(rg, 0x88);  // r0 r1 r2 r3 r0 r1 r2 r3
        auto G = _mm_shuffle_epi32(rg, 0xDD);  // g0 g1 g2 g3 g0 g1 g2 g3
        *r = sk_unaligned_load<U16>(&R);
        *g = sk_unaligned_load<U16>(&G);
    }
    SI void store2(uint16_t* ptr, U16 r, U16 g) {
        __m128i rg = _mm_unpacklo_epi16(widen_cast<__m128i>(r), widen_cast<__m128i>(g));
        _mm_storeu_si128((__m128i*)ptr + 0, rg);
    }

    SI void load4(const uint16_t* ptr, U16* r, U16* g, U16* b, U16* a) {
        __m128i _01 = _mm_loadu_si128(((const __m128i*)ptr) + 0), // r0 g0 b0 a0 r1 g1 b1 a1
                _23 = _mm_loadu_si128(((const __m128i*)ptr) + 1); // r2 g2 b2 a2 r3 g3 b3 a3

        auto _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
             _13 = _mm_unpackhi_epi16(_01, _23);  // r1 r3 g1 g3 b1 b3 a1 a3

        auto rg = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
             ba = _mm_unpackhi_epi16(_02, _13);  // b0 b1 b2 b3 a0 a1 a2 a3

        *r = sk_unaligned_load<U16>((uint16_t*)&rg + 0);
        *g = sk_unaligned_load<U16>((uint16_t*)&rg + 4);
        *b = sk_unaligned_load<U16>((uint16_t*)&ba + 0);
        *a = sk_unaligned_load<U16>((uint16_t*)&ba + 4);
    }

    SI void store4(uint16_t* ptr, U16 r, U16 g, U16 b, U16 a) {
        auto rg = _mm_unpacklo_epi16(widen_cast<__m128i>(r), widen_cast<__m128i>(g)),
             ba = _mm_unpacklo_epi16(widen_cast<__m128i>(b), widen_cast<__m128i>(a));

        _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi32(rg, ba));
        _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi32(rg, ba));
    }

    SI void load4(const float* ptr, F* r, F* g, F* b, F* a) {
        F _0 = _mm_loadu_ps(ptr + 0),
          _1 = _mm_loadu_ps(ptr + 4),
          _2 = _mm_loadu_ps(ptr + 8),
          _3 = _mm_loadu_ps(ptr +12);
        _MM_TRANSPOSE4_PS(_0,_1,_2,_3);
        *r = _0;
        *g = _1;
        *b = _2;
        *a = _3;
    }

    SI void store4(float* ptr, F r, F g, F b, F a) {
        _MM_TRANSPOSE4_PS(r,g,b,a);
        _mm_storeu_ps(ptr + 0, r);
        _mm_storeu_ps(ptr + 4, g);
        _mm_storeu_ps(ptr + 8, b);
        _mm_storeu_ps(ptr +12, a);
    }
#endif

// Helpers to do scalar -> vector promotion on GCC (clang does this automatically)
// We need to subtract (not add) zero to keep float conversion zero-cost. See:
// https://stackoverflow.com/q/48255293
//
// The GCC implementation should be usable everywhere, but Mac clang (only) complains that the
// expressions make these functions not constexpr.
//
// Further: We can't use the subtract-zero version in scalar mode. There, the subtraction will
// really happen (at least at low optimization levels), which can alter the bit pattern of NaNs.
// Because F_() is used when copying uniforms (even integer uniforms), this can corrupt values.
// The vector subtraction of zero doesn't appear to ever alter NaN bit patterns.
#if defined(__clang__) || defined(JUMPER_IS_SCALAR)
SI constexpr F F_(float x) { return x; }
SI constexpr I32 I32_(int32_t x) { return x; }
SI constexpr U32 U32_(uint32_t x) { return x; }
#else
SI constexpr F F_(float x) { return x - F(); }
SI constexpr I32 I32_(int32_t x) { return x + I32(); }
SI constexpr U32 U32_(uint32_t x) { return x + U32(); }
#endif

// Extremely helpful literals:
static constexpr F F0 = F_(0.0f),
                   F1 = F_(1.0f);

#if !defined(JUMPER_IS_SCALAR)
    SI F min(F a, float b) { return min(a, F_(b)); }
    SI F min(float a, F b) { return min(F_(a), b); }
    SI F max(F a, float b) { return max(a, F_(b)); }
    SI F max(float a, F b) { return max(F_(a), b); }

    SI F mad(F f, F m, float a) { return mad(f, m, F_(a)); }
    SI F mad(F f, float m, F a) { return mad(f, F_(m), a); }
    SI F mad(F f, float m, float a) { return mad(f, F_(m), F_(a)); }
    SI F mad(float f, F m, F a) { return mad(F_(f), m, a); }
    SI F mad(float f, F m, float a) { return mad(F_(f), m, F_(a)); }
    SI F mad(float f, float m, F a) { return mad(F_(f), F_(m), a); }
#endif

// We need to be a careful with casts.
// (F)x means cast x to float in the portable path, but bit_cast x to float in the others.
// These named casts and bit_cast() are always what they seem to be.
#if defined(JUMPER_IS_SCALAR)
    SI F   cast  (U32 v) { return   (F)v; }
    SI F   cast64(U64 v) { return   (F)v; }
    SI U32 trunc_(F   v) { return (U32)v; }
    SI U32 expand(U16 v) { return (U32)v; }
    SI U32 expand(U8  v) { return (U32)v; }
#else
    SI F   cast  (U32 v) { return      __builtin_convertvector((I32)v,   F); }
    SI F   cast64(U64 v) { return      __builtin_convertvector(     v,   F); }
    SI U32 trunc_(F   v) { return (U32)__builtin_convertvector(     v, I32); }
    SI U32 expand(U16 v) { return      __builtin_convertvector(     v, U32); }
    SI U32 expand(U8  v) { return      __builtin_convertvector(     v, U32); }
#endif

#if !defined(JUMPER_IS_SCALAR)
SI F if_then_else(I32 c, F     t, float e) { return if_then_else(c,    t , F_(e)); }
SI F if_then_else(I32 c, float t, F     e) { return if_then_else(c, F_(t),    e ); }
SI F if_then_else(I32 c, float t, float e) { return if_then_else(c, F_(t), F_(e)); }
#endif

SI F fract(F v) { return v - floor_(v); }

// See http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html
SI F approx_log2(F x) {
    // e - 127 is a fair approximation of log2(x) in its own right...
    F e = cast(sk_bit_cast<U32>(x)) * (1.0f / (1<<23));

    // ... but using the mantissa to refine its error is _much_ better.
    F m = sk_bit_cast<F>((sk_bit_cast<U32>(x) & 0x007fffff) | 0x3f000000);
    return e
         - 124.225514990f
         -   1.498030302f * m
         -   1.725879990f / (0.3520887068f + m);
}

SI F approx_log(F x) {
    const float ln2 = 0.69314718f;
    return ln2 * approx_log2(x);
}

SI F approx_pow2(F x) {
    constexpr float kInfinityBits = 0x7f800000;

    F f = fract(x);
    F approx = x + 121.274057500f;
      approx -= f * 1.490129070f;
      approx += 27.728023300f / (4.84252568f - f);
      approx *= 1.0f * (1<<23);
      approx  = min(max(approx, F0), F_(kInfinityBits));  // guard against underflow/overflow

    return sk_bit_cast<F>(round(approx));
}

SI F approx_exp(F x) {
    const float log2_e = 1.4426950408889634074f;
    return approx_pow2(log2_e * x);
}

SI F approx_powf(F x, F y) {
    return if_then_else((x == 0)|(x == 1), x
                                         , approx_pow2(approx_log2(x) * y));
}
#if !defined(JUMPER_IS_SCALAR)
SI F approx_powf(F x, float y) { return approx_powf(x, F_(y)); }
#endif

SI F from_half(U16 h) {
#if defined(JUMPER_IS_NEON) && defined(SK_CPU_ARM64)
    return vcvt_f32_f16((float16x4_t)h);

#elif defined(JUMPER_IS_SKX)
    return _mm512_cvtph_ps((__m256i)h);

#elif defined(JUMPER_IS_HSW)
    return _mm256_cvtph_ps((__m128i)h);

#else
    // Remember, a half is 1-5-10 (sign-exponent-mantissa) with 15 exponent bias.
    U32 sem = expand(h),
        s   = sem & 0x8000,
         em = sem ^ s;

    // Convert to 1-8-23 float with 127 bias, flushing denorm halfs (including zero) to zero.
    auto denorm = (I32)em < 0x0400;      // I32 comparison is often quicker, and always safe here.
    return if_then_else(denorm, F0
                              , sk_bit_cast<F>( (s<<16) + (em<<13) + ((127-15)<<23) ));
#endif
}

SI U16 to_half(F f) {
#if defined(JUMPER_IS_NEON) && defined(SK_CPU_ARM64)
    return (U16)vcvt_f16_f32(f);

#elif defined(JUMPER_IS_SKX)
    return (U16)_mm512_cvtps_ph(f, _MM_FROUND_CUR_DIRECTION);

#elif defined(JUMPER_IS_HSW)
    return (U16)_mm256_cvtps_ph(f, _MM_FROUND_CUR_DIRECTION);

#else
    // Remember, a float is 1-8-23 (sign-exponent-mantissa) with 127 exponent bias.
    U32 sem = sk_bit_cast<U32>(f),
        s   = sem & 0x80000000,
         em = sem ^ s;

    // Convert to 1-5-10 half with 15 bias, flushing denorm halfs (including zero) to zero.
    auto denorm = (I32)em < 0x38800000;  // I32 comparison is often quicker, and always safe here.
    return pack((U32)if_then_else(denorm, I32_(0)
                                        , (I32)((s>>16) + (em>>13) - ((127-15)<<10))));
#endif
}

static void patch_memory_contexts(SkSpan<SkRasterPipeline_MemoryCtxPatch> memoryCtxPatches,
                                  size_t dx, size_t dy, size_t tail) {
    for (SkRasterPipeline_MemoryCtxPatch& patch : memoryCtxPatches) {
        SkRasterPipeline_MemoryCtx* ctx = patch.info.context;

        const ptrdiff_t offset = patch.info.bytesPerPixel * (dy * ctx->stride + dx);
        if (patch.info.load) {
            void* ctxData = SkTAddOffset<void>(ctx->pixels, offset);
            memcpy(patch.scratch, ctxData, patch.info.bytesPerPixel * tail);
        }

        SkASSERT(patch.backup == nullptr);
        void* scratchFakeBase = SkTAddOffset<void>(patch.scratch, -offset);
        patch.backup = ctx->pixels;
        ctx->pixels = scratchFakeBase;
    }
}

static void restore_memory_contexts(SkSpan<SkRasterPipeline_MemoryCtxPatch> memoryCtxPatches,
                                    size_t dx, size_t dy, size_t tail) {
    for (SkRasterPipeline_MemoryCtxPatch& patch : memoryCtxPatches) {
        SkRasterPipeline_MemoryCtx* ctx = patch.info.context;

        SkASSERT(patch.backup != nullptr);
        ctx->pixels = patch.backup;
        patch.backup = nullptr;

        const ptrdiff_t offset = patch.info.bytesPerPixel * (dy * ctx->stride + dx);
        if (patch.info.store) {
            void* ctxData = SkTAddOffset<void>(ctx->pixels, offset);
            memcpy(ctxData, patch.scratch, patch.info.bytesPerPixel * tail);
        }
    }
}

#if defined(JUMPER_IS_SCALAR) || defined(JUMPER_IS_SSE2)
    // In scalar and SSE2 mode, we always use precise math so we can have more predictable results.
    // Chrome will use the SSE2 implementation when --disable-skia-runtime-opts is set. (b/40042946)
    SI F rcp_fast(F v) { return rcp_precise(v); }
    SI F rsqrt(F v)    { return rcp_precise(sqrt_(v)); }
#else
    SI F rcp_fast(F v) { return rcp_approx(v); }
    SI F rsqrt(F v)    { return rsqrt_approx(v); }
#endif

// Our fundamental vector depth is our pixel stride.
static constexpr size_t N = sizeof(F) / sizeof(float);

// We're finally going to get to what a Stage function looks like!

// Any custom ABI to use for all (non-externally-facing) stage functions?
// Also decide here whether to use narrow (compromise) or wide (ideal) stages.
#if defined(SK_CPU_ARM32) && defined(JUMPER_IS_NEON)
    // This lets us pass vectors more efficiently on 32-bit ARM.
    // We can still only pass 16 floats, so best as 4x {r,g,b,a}.
    #define ABI __attribute__((pcs("aapcs-vfp")))
    #define JUMPER_NARROW_STAGES 1
#elif defined(_MSC_VER)
    // Even if not vectorized, this lets us pass {r,g,b,a} as registers,
    // instead of {b,a} on the stack.  Narrow stages work best for __vectorcall.
    #define ABI __vectorcall
    #define JUMPER_NARROW_STAGES 1
#elif defined(__x86_64__) || defined(SK_CPU_ARM64)
    // These platforms are ideal for wider stages, and their default ABI is ideal.
    #define ABI
    #define JUMPER_NARROW_STAGES 0
#else
    // 32-bit or unknown... shunt them down the narrow path.
    // Odds are these have few registers and are better off there.
    #define ABI
    #define JUMPER_NARROW_STAGES 1
#endif

#if JUMPER_NARROW_STAGES
    struct Params {
        size_t dx, dy;
        std::byte* base;
        F dr,dg,db,da;
    };
    using Stage = void(ABI*)(Params*, SkRasterPipelineStage* program, F r, F g, F b, F a);
#else
    using Stage = void(ABI*)(SkRasterPipelineStage* program, size_t dx, size_t dy,
                             std::byte* base, F,F,F,F, F,F,F,F);
#endif

static void start_pipeline(size_t dx, size_t dy,
                           size_t xlimit, size_t ylimit,
                           SkRasterPipelineStage* program,
                           SkSpan<SkRasterPipeline_MemoryCtxPatch> memoryCtxPatches,
                           uint8_t* tailPointer) {
    uint8_t unreferencedTail;
    if (!tailPointer) {
        tailPointer = &unreferencedTail;
    }
    auto start = (Stage)program->fn;
    const size_t x0 = dx;
    std::byte* const base = nullptr;
    for (; dy < ylimit; dy++) {
    #if JUMPER_NARROW_STAGES
        Params params = { x0,dy,base, F0,F0,F0,F0 };
        while (params.dx + N <= xlimit) {
            start(&params,program, F0,F0,F0,F0);
            params.dx += N;
        }
        if (size_t tail = xlimit - params.dx) {
            *tailPointer = tail;
            patch_memory_contexts(memoryCtxPatches, params.dx, dy, tail);
            start(&params,program, F0,F0,F0,F0);
            restore_memory_contexts(memoryCtxPatches, params.dx, dy, tail);
            *tailPointer = 0xFF;
        }
    #else
        dx = x0;
        while (dx + N <= xlimit) {
            start(program,dx,dy,base, F0,F0,F0,F0, F0,F0,F0,F0);
            dx += N;
        }
        if (size_t tail = xlimit - dx) {
            *tailPointer = tail;
            patch_memory_contexts(memoryCtxPatches, dx, dy, tail);
            start(program,dx,dy,base, F0,F0,F0,F0, F0,F0,F0,F0);
            restore_memory_contexts(memoryCtxPatches, dx, dy, tail);
            *tailPointer = 0xFF;
        }
    #endif
    }
}

#if SK_HAS_MUSTTAIL
    #define JUMPER_MUSTTAIL [[clang::musttail]]
#else
    #define JUMPER_MUSTTAIL
#endif

#if JUMPER_NARROW_STAGES
    #define DECLARE_STAGE(name, ARG, STAGE_RET, INC, OFFSET, MUSTTAIL)                     \
        SI STAGE_RET name##_k(ARG, size_t dx, size_t dy, std::byte*& base,                 \
                              F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);         \
        static void ABI name(Params* params, SkRasterPipelineStage* program,               \
                             F r, F g, F b, F a) {                                         \
            OFFSET name##_k(Ctx{program}, params->dx,params->dy,params->base,              \
                            r,g,b,a, params->dr, params->dg, params->db, params->da);      \
            INC;                                                                           \
            auto fn = (Stage)program->fn;                                                  \
            MUSTTAIL return fn(params, program, r,g,b,a);                                  \
        }                                                                                  \
        SI STAGE_RET name##_k(ARG, size_t dx, size_t dy, std::byte*& base,                 \
                              F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#else
    #define DECLARE_STAGE(name, ARG, STAGE_RET, INC, OFFSET, MUSTTAIL)                           \
        SI STAGE_RET name##_k(ARG, size_t dx, size_t dy, std::byte*& base,                       \
                              F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);               \
        static void ABI name(SkRasterPipelineStage* program, size_t dx, size_t dy,               \
                             std::byte* base, F r, F g, F b, F a, F dr, F dg, F db, F da) {      \
            OFFSET name##_k(Ctx{program}, dx,dy,base, r,g,b,a, dr,dg,db,da);                     \
            INC;                                                                                 \
            auto fn = (Stage)program->fn;                                                        \
            MUSTTAIL return fn(program, dx,dy,base, r,g,b,a, dr,dg,db,da);                       \
        }                                                                                        \
        SI STAGE_RET name##_k(ARG, size_t dx, size_t dy, std::byte*& base,                       \
                              F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#endif

// A typical stage returns void, always increments the program counter by 1, and lets the optimizer
// decide whether or not tail-calling is appropriate.
#define STAGE(name, arg) \
    DECLARE_STAGE(name, arg, void, ++program, /*no offset*/, /*no musttail*/)

// A tail stage returns void, always increments the program counter by 1, and uses tail-calling.
// Tail-calling is necessary in SkSL-generated programs, which can be thousands of ops long, and
// could overflow the stack (particularly in debug).
#define STAGE_TAIL(name, arg) \
    DECLARE_STAGE(name, arg, void, ++program, /*no offset*/, JUMPER_MUSTTAIL)

// A branch stage returns an integer, which is added directly to the program counter, and tailcalls.
#define STAGE_BRANCH(name, arg) \
    DECLARE_STAGE(name, arg, int, /*no increment*/, program +=, JUMPER_MUSTTAIL)

// just_return() is a simple no-op stage that only exists to end the chain,
// returning back up to start_pipeline(), and from there to the caller.
#if JUMPER_NARROW_STAGES
    static void ABI just_return(Params*, SkRasterPipelineStage*, F,F,F,F) {}
#else
    static void ABI just_return(SkRasterPipelineStage*, size_t,size_t, std::byte*,
                                F,F,F,F, F,F,F,F) {}
#endif

// Note that in release builds, most stages consume no stack (thanks to tail call optimization).
// However: certain builds (especially with non-clang compilers) may fail to optimize tail
// calls, resulting in actual stack frames being generated.
//
// stack_checkpoint() and stack_rewind() are special stages that can be used to manage stack growth.
// If a pipeline contains a stack_checkpoint, followed by any number of stack_rewind (at any point),
// the C++ stack will be reset to the state it was at when the stack_checkpoint was initially hit.
//
// All instances of stack_rewind (as well as the one instance of stack_checkpoint near the start of
// a pipeline) share a single context (of type SkRasterPipeline_RewindCtx). That context holds the
// full state of the mutable registers that are normally passed to the next stage in the program.
//
// stack_rewind is the only stage other than just_return that actually returns (rather than jumping
// to the next stage in the program). Before it does so, it stashes all of the registers in the
// context. This includes the updated `program` pointer. Unlike stages that tail call exactly once,
// stack_checkpoint calls the next stage in the program repeatedly, as long as the `program` in the
// context is overwritten (i.e., as long as a stack_rewind was the reason the pipeline returned,
// rather than a just_return).
//
// Normally, just_return is the only stage that returns, and no other stage does anything after a
// subsequent (called) stage returns, so the stack just unwinds all the way to start_pipeline.
// With stack_checkpoint on the stack, any stack_rewind stages will return all the way up to the
// stack_checkpoint. That grabs the values that would have been passed to the next stage (from the
// context), and continues the linear execution of stages, but has reclaimed all of the stack frames
// pushed before the stack_rewind before doing so.
#if JUMPER_NARROW_STAGES
    static void ABI stack_checkpoint(Params* params, SkRasterPipelineStage* program,
                                     F r, F g, F b, F a) {
        SkRasterPipeline_RewindCtx* ctx = Ctx{program};
        while (program) {
            auto next = (Stage)(++program)->fn;

            ctx->stage = nullptr;
            next(params, program, r, g, b, a);
            program = ctx->stage;

            if (program) {
                r            = sk_unaligned_load<F>(ctx->r );
                g            = sk_unaligned_load<F>(ctx->g );
                b            = sk_unaligned_load<F>(ctx->b );
                a            = sk_unaligned_load<F>(ctx->a );
                params->dr   = sk_unaligned_load<F>(ctx->dr);
                params->dg   = sk_unaligned_load<F>(ctx->dg);
                params->db   = sk_unaligned_load<F>(ctx->db);
                params->da   = sk_unaligned_load<F>(ctx->da);
                params->base = ctx->base;
            }
        }
    }
    static void ABI stack_rewind(Params* params, SkRasterPipelineStage* program,
                                 F r, F g, F b, F a) {
        SkRasterPipeline_RewindCtx* ctx = Ctx{program};
        sk_unaligned_store(ctx->r , r );
        sk_unaligned_store(ctx->g , g );
        sk_unaligned_store(ctx->b , b );
        sk_unaligned_store(ctx->a , a );
        sk_unaligned_store(ctx->dr, params->dr);
        sk_unaligned_store(ctx->dg, params->dg);
        sk_unaligned_store(ctx->db, params->db);
        sk_unaligned_store(ctx->da, params->da);
        ctx->base  = params->base;
        ctx->stage = program;
    }
#else
    static void ABI stack_checkpoint(SkRasterPipelineStage* program,
                                     size_t dx, size_t dy, std::byte* base,
                                     F r, F g, F b, F a, F dr, F dg, F db, F da) {
        SkRasterPipeline_RewindCtx* ctx = Ctx{program};
        while (program) {
            auto next = (Stage)(++program)->fn;

            ctx->stage = nullptr;
            next(program, dx, dy, base, r, g, b, a, dr, dg, db, da);
            program = ctx->stage;

            if (program) {
                r    = sk_unaligned_load<F>(ctx->r );
                g    = sk_unaligned_load<F>(ctx->g );
                b    = sk_unaligned_load<F>(ctx->b );
                a    = sk_unaligned_load<F>(ctx->a );
                dr   = sk_unaligned_load<F>(ctx->dr);
                dg   = sk_unaligned_load<F>(ctx->dg);
                db   = sk_unaligned_load<F>(ctx->db);
                da   = sk_unaligned_load<F>(ctx->da);
                base = ctx->base;
            }
        }
    }
    static void ABI stack_rewind(SkRasterPipelineStage* program,
                                 size_t dx, size_t dy, std::byte* base,
                                 F r, F g, F b, F a, F dr, F dg, F db, F da) {
        SkRasterPipeline_RewindCtx* ctx = Ctx{program};
        sk_unaligned_store(ctx->r , r );
        sk_unaligned_store(ctx->g , g );
        sk_unaligned_store(ctx->b , b );
        sk_unaligned_store(ctx->a , a );
        sk_unaligned_store(ctx->dr, dr);
        sk_unaligned_store(ctx->dg, dg);
        sk_unaligned_store(ctx->db, db);
        sk_unaligned_store(ctx->da, da);
        ctx->base  = base;
        ctx->stage = program;
    }
#endif


// We could start defining normal Stages now.  But first, some helper functions.

template <typename V, typename T>
SI V load(const T* src) {
    return sk_unaligned_load<V>(src);
}

template <typename V, typename T>
SI void store(T* dst, V v) {
    sk_unaligned_store(dst, v);
}

SI F from_byte(U8 b) {
    return cast(expand(b)) * (1/255.0f);
}
SI F from_short(U16 s) {
    return cast(expand(s)) * (1/65535.0f);
}
SI void from_565(U16 _565, F* r, F* g, F* b) {
    U32 wide = expand(_565);
    *r = cast(wide & (31<<11)) * (1.0f / (31<<11));
    *g = cast(wide & (63<< 5)) * (1.0f / (63<< 5));
    *b = cast(wide & (31<< 0)) * (1.0f / (31<< 0));
}
SI void from_4444(U16 _4444, F* r, F* g, F* b, F* a) {
    U32 wide = expand(_4444);
    *r = cast(wide & (15<<12)) * (1.0f / (15<<12));
    *g = cast(wide & (15<< 8)) * (1.0f / (15<< 8));
    *b = cast(wide & (15<< 4)) * (1.0f / (15<< 4));
    *a = cast(wide & (15<< 0)) * (1.0f / (15<< 0));
}
SI void from_8888(U32 _8888, F* r, F* g, F* b, F* a) {
    *r = cast((_8888      ) & 0xff) * (1/255.0f);
    *g = cast((_8888 >>  8) & 0xff) * (1/255.0f);
    *b = cast((_8888 >> 16) & 0xff) * (1/255.0f);
    *a = cast((_8888 >> 24)       ) * (1/255.0f);
}
SI void from_88(U16 _88, F* r, F* g) {
    U32 wide = expand(_88);
    *r = cast((wide      ) & 0xff) * (1/255.0f);
    *g = cast((wide >>  8) & 0xff) * (1/255.0f);
}
SI void from_1010102(U32 rgba, F* r, F* g, F* b, F* a) {
    *r = cast((rgba      ) & 0x3ff) * (1/1023.0f);
    *g = cast((rgba >> 10) & 0x3ff) * (1/1023.0f);
    *b = cast((rgba >> 20) & 0x3ff) * (1/1023.0f);
    *a = cast((rgba >> 30)        ) * (1/   3.0f);
}
SI void from_1010102_xr(U32 rgba, F* r, F* g, F* b, F* a) {
    static constexpr float min = -0.752941f;
    static constexpr float max = 1.25098f;
    static constexpr float range = max - min;
    *r = cast((rgba      ) & 0x3ff) * (1/1023.0f) * range + min;
    *g = cast((rgba >> 10) & 0x3ff) * (1/1023.0f) * range + min;
    *b = cast((rgba >> 20) & 0x3ff) * (1/1023.0f) * range + min;
    *a = cast((rgba >> 30)        ) * (1/   3.0f);
}
SI void from_10x6(U64 _10x6, F* r, F* g, F* b, F* a) {
    *r = cast64((_10x6 >>  6) & 0x3ff) * (1/1023.0f);
    *g = cast64((_10x6 >> 22) & 0x3ff) * (1/1023.0f);
    *b = cast64((_10x6 >> 38) & 0x3ff) * (1/1023.0f);
    *a = cast64((_10x6 >> 54) & 0x3ff) * (1/1023.0f);
}
SI void from_1616(U32 _1616, F* r, F* g) {
    *r = cast((_1616      ) & 0xffff) * (1/65535.0f);
    *g = cast((_1616 >> 16) & 0xffff) * (1/65535.0f);
}
SI void from_16161616(U64 _16161616, F* r, F* g, F* b, F* a) {
    *r = cast64((_16161616      ) & 0xffff) * (1/65535.0f);
    *g = cast64((_16161616 >> 16) & 0xffff) * (1/65535.0f);
    *b = cast64((_16161616 >> 32) & 0xffff) * (1/65535.0f);
    *a = cast64((_16161616 >> 48) & 0xffff) * (1/65535.0f);
}

// Used by load_ and store_ stages to get to the right (dx,dy) starting point of contiguous memory.
template <typename T>
SI T* ptr_at_xy(const SkRasterPipeline_MemoryCtx* ctx, size_t dx, size_t dy) {
    return (T*)ctx->pixels + dy*ctx->stride + dx;
}

// clamp v to [0,limit).
SI F clamp(F v, F limit) {
    F inclusive = sk_bit_cast<F>( sk_bit_cast<U32>(limit) - 1 );  // Exclusive -> inclusive.
    return min(max(0.0f, v), inclusive);
}

// clamp to (0,limit).
SI F clamp_ex(F v, float limit) {
    const F inclusiveZ = F_(std::numeric_limits<float>::min()),
            inclusiveL = sk_bit_cast<F>( sk_bit_cast<U32>(F_(limit)) - 1 );
    return min(max(inclusiveZ, v), inclusiveL);
}

// Polynomial approximation of degree 5 for sin(x * 2 * pi) in the range [-1/4, 1/4]
// Adapted from https://github.com/google/swiftshader/blob/master/docs/Sin-Cos-Optimization.pdf
SI F sin5q_(F x) {
    // A * x + B * x^3 + C * x^5
    // Exact at x = 0, 1/12, 1/6, 1/4, and their negatives,
    // which correspond to x * 2 * pi = 0, pi/6, pi/3, pi/2
    constexpr float A = 6.28230858f;
    constexpr float B = -41.1693687f;
    constexpr float C = 74.4388885f;
    F x2 = x * x;
    return x * mad(mad(x2, C, B), x2, A);
}

SI F sin_(F x) {
    constexpr float one_over_pi2 = 1 / (2 * SK_FloatPI);
    x = mad(x, -one_over_pi2, 0.25f);
    x = 0.25f - abs_(x - floor_(x + 0.5f));
    return sin5q_(x);
}

SI F cos_(F x) {
    constexpr float one_over_pi2 = 1 / (2 * SK_FloatPI);
    x *= one_over_pi2;
    x = 0.25f - abs_(x - floor_(x + 0.5f));
    return sin5q_(x);
}

/*  "GENERATING ACCURATE VALUES FOR THE TANGENT FUNCTION"
     https://mae.ufl.edu/~uhk/ACCURATE-TANGENT.pdf

    approx = x + (1/3)x^3 + (2/15)x^5 + (17/315)x^7 + (62/2835)x^9

    Some simplifications:
    1. tan(x) is periodic, -PI/2 < x < PI/2
    2. tan(x) is odd, so tan(-x) = -tan(x)
    3. Our polynomial approximation is best near zero, so we use the following identity
                    tan(x) + tan(y)
       tan(x + y) = -----------------
                   1 - tan(x)*tan(y)
       tan(PI/4) = 1

       So for x > PI/8, we do the following refactor:
       x' = x - PI/4

                1 + tan(x')
       tan(x) = ------------
                1 - tan(x')
 */
SI F tan_(F x) {
    constexpr float Pi = SK_FloatPI;
    // periodic between -pi/2 ... pi/2
    // shift to 0...Pi, scale 1/Pi to get into 0...1, then fract, scale-up, shift-back
    x = fract((1/Pi)*x + 0.5f) * Pi - (Pi/2);

    I32 neg = (x < 0.0f);
    x = if_then_else(neg, -x, x);

    // minimize total error by shifting if x > pi/8
    I32 use_quotient = (x > (Pi/8));
    x = if_then_else(use_quotient, x - (Pi/4), x);

    // 9th order poly = 4th order(x^2) * x
    const float c4 = 62 / 2835.0f;
    const float c3 = 17 / 315.0f;
    const float c2 = 2 / 15.0f;
    const float c1 = 1 / 3.0f;
    const float c0 = 1.0f;
    F x2 = x * x;
    x *= mad(x2, mad(x2, mad(x2, mad(x2, c4, c3), c2), c1), c0);
    x = if_then_else(use_quotient, (1+x)/(1-x), x);
    x = if_then_else(neg, -x, x);
    return x;
}

/*  Use 4th order polynomial approximation from https://arachnoid.com/polysolve/
        with 129 values of x,atan(x) for x:[0...1]
    This only works for 0 <= x <= 1
 */
SI F approx_atan_unit(F x) {
    // y =   0.14130025741326729 x⁴
    //     - 0.34312835980675116 x³
    //     - 0.016172900528248768 x²
    //     + 1.00376969762003850 x
    //     - 0.00014758242182738969
    const float c4 =  0.14130025741326729f;
    const float c3 = -0.34312835980675116f;
    const float c2 = -0.016172900528248768f;
    const float c1 =  1.0037696976200385f;
    const float c0 = -0.00014758242182738969f;
    return mad(x, mad(x, mad(x, mad(x, c4, c3), c2), c1), c0);
}

// Use identity atan(x) = pi/2 - atan(1/x) for x > 1
SI F atan_(F x) {
    I32 neg = (x < 0.0f);
    x = if_then_else(neg, -x, x);
    I32 flip = (x > 1.0f);
    x = if_then_else(flip, 1/x, x);
    x = approx_atan_unit(x);
    x = if_then_else(flip, SK_FloatPI/2 - x, x);
    x = if_then_else(neg, -x, x);
    return x;
}

// Handbook of Mathematical Functions, by Milton Abramowitz and Irene Stegun:
// https://books.google.com/books/content?id=ZboM5tOFWtsC&pg=PA81&img=1&zoom=3&hl=en&bul=1&sig=ACfU3U2M75tG_iGVOS92eQspr14LTq02Nw&ci=0%2C15%2C999%2C1279&edge=0
// http://screen/8YGJxUGFQ49bVX6
SI F asin_(F x) {
    I32 neg = (x < 0.0f);
    x = if_then_else(neg, -x, x);
    const float c3 = -0.0187293f;
    const float c2 = 0.0742610f;
    const float c1 = -0.2121144f;
    const float c0 = 1.5707288f;
    F poly = mad(x, mad(x, mad(x, c3, c2), c1), c0);
    x = SK_FloatPI/2 - sqrt_(1 - x) * poly;
    x = if_then_else(neg, -x, x);
    return x;
}

SI F acos_(F x) {
    return SK_FloatPI/2 - asin_(x);
}

/*  Use identity atan(x) = pi/2 - atan(1/x) for x > 1
    By swapping y,x to ensure the ratio is <= 1, we can safely call atan_unit()
    which avoids a 2nd divide instruction if we had instead called atan().
 */
SI F atan2_(F y0, F x0) {
    I32 flip = (abs_(y0) > abs_(x0));
    F   y = if_then_else(flip, x0, y0);
    F   x = if_then_else(flip, y0, x0);
    F   arg = y/x;

    I32 neg = (arg < 0.0f);
    arg = if_then_else(neg, -arg, arg);

    F r = approx_atan_unit(arg);
    r = if_then_else(flip, SK_FloatPI/2 - r, r);
    r = if_then_else(neg, -r, r);

    // handle quadrant distinctions
    r = if_then_else((y0 >= 0) & (x0  < 0), r + SK_FloatPI, r);
    r = if_then_else((y0  < 0) & (x0 <= 0), r - SK_FloatPI, r);
    // Note: we don't try to handle 0,0 or infinities
    return r;
}

// Used by gather_ stages to calculate the base pointer and a vector of indices to load.
template <typename T>
SI U32 ix_and_ptr(T** ptr, const SkRasterPipeline_GatherCtx* ctx, F x, F y) {
    // We use exclusive clamp so that our min value is > 0 because ULP subtraction using U32 would
    // produce a NaN if applied to +0.f.
    x = clamp_ex(x, ctx->width );
    y = clamp_ex(y, ctx->height);
    x = sk_bit_cast<F>(sk_bit_cast<U32>(x) - (uint32_t)ctx->roundDownAtInteger);
    y = sk_bit_cast<F>(sk_bit_cast<U32>(y) - (uint32_t)ctx->roundDownAtInteger);
    *ptr = (const T*)ctx->pixels;
    return trunc_(y)*ctx->stride + trunc_(x);
}

// We often have a nominally [0,1] float value we need to scale and convert to an integer,
// whether for a table lookup or to pack back down into bytes for storage.
//
// In practice, especially when dealing with interesting color spaces, that notionally
// [0,1] float may be out of [0,1] range.  Unorms cannot represent that, so we must clamp.
//
// You can adjust the expected input to [0,bias] by tweaking that parameter.
SI U32 to_unorm(F v, float scale, float bias = 1.0f) {
    // Any time we use round() we probably want to use to_unorm().
    return round(min(max(0.0f, v), bias), F_(scale));
}

SI I32 cond_to_mask(I32 cond) {
#if defined(JUMPER_IS_SCALAR)
    // In scalar mode, conditions are bools (0 or 1), but we want to store and operate on masks
    // (eg, using bitwise operations to select values).
    return if_then_else(cond, I32(~0), I32(0));
#else
    // In SIMD mode, our various instruction sets already represent conditions as masks.
    return cond;
#endif
}

#if defined(JUMPER_IS_SCALAR)
// In scalar mode, `data` only contains a single lane.
SI uint32_t select_lane(uint32_t data, int /*lane*/) { return data; }
SI  int32_t select_lane( int32_t data, int /*lane*/) { return data; }
#else
// In SIMD mode, `data` contains a vector of lanes.
SI uint32_t select_lane(U32 data, int lane) { return data[lane]; }
SI  int32_t select_lane(I32 data, int lane) { return data[lane]; }
#endif

// Now finally, normal Stages!

STAGE(seed_shader, NoCtx) {
    static constexpr float iota[] = {
        0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f,
        8.5f, 9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f,
    };
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride_highp);

    // It's important for speed to explicitly cast(dx) and cast(dy),
    // which has the effect of splatting them to vectors before converting to floats.
    // On Intel this breaks a data dependency on previous loop iterations' registers.
    r = cast(U32_(dx)) + sk_unaligned_load<F>(iota);
    g = cast(U32_(dy)) + 0.5f;
    b = F1;  // This is w=1 for matrix multiplies by the device coords.
    a = F0;
}

STAGE(dither, const float* rate) {
    // Get [(dx,dy), (dx+1,dy), (dx+2,dy), ...] loaded up in integer vectors.
    uint32_t iota[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride_highp);

    U32 X = U32_(dx) + sk_unaligned_load<U32>(iota),
        Y = U32_(dy);

    // We're doing 8x8 ordered dithering, see https://en.wikipedia.org/wiki/Ordered_dithering.
    // In this case n=8 and we're using the matrix that looks like 1/64 x [ 0 48 12 60 ... ].

    // We only need X and X^Y from here on, so it's easier to just think of that as "Y".
    Y ^= X;

    // We'll mix the bottom 3 bits of each of X and Y to make 6 bits,
    // for 2^6 == 64 == 8x8 matrix values.  If X=abc and Y=def, we make fcebda.
    U32 M = (Y & 1) << 5 | (X & 1) << 4
          | (Y & 2) << 2 | (X & 2) << 1
          | (Y & 4) >> 1 | (X & 4) >> 2;

    // Scale that dither to [0,1), then (-0.5,+0.5), here using 63/128 = 0.4921875 as 0.5-epsilon.
    // We want to make sure our dither is less than 0.5 in either direction to keep exact values
    // like 0 and 1 unchanged after rounding.
    F dither = cast(M) * (2/128.0f) - (63/128.0f);

    r += *rate*dither;
    g += *rate*dither;
    b += *rate*dither;

    r = max(0.0f, min(r, a));
    g = max(0.0f, min(g, a));
    b = max(0.0f, min(b, a));
}

// load 4 floats from memory, and splat them into r,g,b,a
STAGE(uniform_color, const SkRasterPipeline_UniformColorCtx* c) {
    r = F_(c->r);
    g = F_(c->g);
    b = F_(c->b);
    a = F_(c->a);
}
STAGE(unbounded_uniform_color, const SkRasterPipeline_UniformColorCtx* c) {
    r = F_(c->r);
    g = F_(c->g);
    b = F_(c->b);
    a = F_(c->a);
}
// load 4 floats from memory, and splat them into dr,dg,db,da
STAGE(uniform_color_dst, const SkRasterPipeline_UniformColorCtx* c) {
    dr = F_(c->r);
    dg = F_(c->g);
    db = F_(c->b);
    da = F_(c->a);
}

// splats opaque-black into r,g,b,a
STAGE(black_color, NoCtx) {
    r = g = b = F0;
    a = F1;
}

STAGE(white_color, NoCtx) {
    r = g = b = a = F1;
}

// load registers r,g,b,a from context (mirrors store_src)
STAGE(load_src, const float* ptr) {
    r = sk_unaligned_load<F>(ptr + 0*N);
    g = sk_unaligned_load<F>(ptr + 1*N);
    b = sk_unaligned_load<F>(ptr + 2*N);
    a = sk_unaligned_load<F>(ptr + 3*N);
}

// store registers r,g,b,a into context (mirrors load_src)
STAGE(store_src, float* ptr) {
    sk_unaligned_store(ptr + 0*N, r);
    sk_unaligned_store(ptr + 1*N, g);
    sk_unaligned_store(ptr + 2*N, b);
    sk_unaligned_store(ptr + 3*N, a);
}
// store registers r,g into context
STAGE(store_src_rg, float* ptr) {
    sk_unaligned_store(ptr + 0*N, r);
    sk_unaligned_store(ptr + 1*N, g);
}
// load registers r,g from context
STAGE(load_src_rg, float* ptr) {
    r = sk_unaligned_load<F>(ptr + 0*N);
    g = sk_unaligned_load<F>(ptr + 1*N);
}
// store register a into context
STAGE(store_src_a, float* ptr) {
    sk_unaligned_store(ptr, a);
}

// load registers dr,dg,db,da from context (mirrors store_dst)
STAGE(load_dst, const float* ptr) {
    dr = sk_unaligned_load<F>(ptr + 0*N);
    dg = sk_unaligned_load<F>(ptr + 1*N);
    db = sk_unaligned_load<F>(ptr + 2*N);
    da = sk_unaligned_load<F>(ptr + 3*N);
}

// store registers dr,dg,db,da into context (mirrors load_dst)
STAGE(store_dst, float* ptr) {
    sk_unaligned_store(ptr + 0*N, dr);
    sk_unaligned_store(ptr + 1*N, dg);
    sk_unaligned_store(ptr + 2*N, db);
    sk_unaligned_store(ptr + 3*N, da);
}

// Most blend modes apply the same logic to each channel.
#define BLEND_MODE(name)                       \
    SI F name##_channel(F s, F d, F sa, F da); \
    STAGE(name, NoCtx) {                   \
        r = name##_channel(r,dr,a,da);         \
        g = name##_channel(g,dg,a,da);         \
        b = name##_channel(b,db,a,da);         \
        a = name##_channel(a,da,a,da);         \
    }                                          \
    SI F name##_channel(F s, F d, F sa, F da)

SI F inv(F x) { return 1.0f - x; }
SI F two(F x) { return x + x; }


BLEND_MODE(clear)    { return F0; }
BLEND_MODE(srcatop)  { return s*da + d*inv(sa); }
BLEND_MODE(dstatop)  { return d*sa + s*inv(da); }
BLEND_MODE(srcin)    { return s * da; }
BLEND_MODE(dstin)    { return d * sa; }
BLEND_MODE(srcout)   { return s * inv(da); }
BLEND_MODE(dstout)   { return d * inv(sa); }
BLEND_MODE(srcover)  { return mad(d, inv(sa), s); }
BLEND_MODE(dstover)  { return mad(s, inv(da), d); }

BLEND_MODE(modulate) { return s*d; }
BLEND_MODE(multiply) { return s*inv(da) + d*inv(sa) + s*d; }
BLEND_MODE(plus_)    { return min(s + d, 1.0f); }  // We can clamp to either 1 or sa.
BLEND_MODE(screen)   { return s + d - s*d; }
BLEND_MODE(xor_)     { return s*inv(da) + d*inv(sa); }
#undef BLEND_MODE

// Most other blend modes apply the same logic to colors, and srcover to alpha.
#define BLEND_MODE(name)                       \
    SI F name##_channel(F s, F d, F sa, F da); \
    STAGE(name, NoCtx) {                   \
        r = name##_channel(r,dr,a,da);         \
        g = name##_channel(g,dg,a,da);         \
        b = name##_channel(b,db,a,da);         \
        a = mad(da, inv(a), a);                \
    }                                          \
    SI F name##_channel(F s, F d, F sa, F da)

BLEND_MODE(darken)     { return s + d -     max(s*da, d*sa) ; }
BLEND_MODE(lighten)    { return s + d -     min(s*da, d*sa) ; }
BLEND_MODE(difference) { return s + d - two(min(s*da, d*sa)); }
BLEND_MODE(exclusion)  { return s + d - two(s*d); }

BLEND_MODE(colorburn) {
    return if_then_else(d == da,    d +    s*inv(da),
           if_then_else(s ==  0, /* s + */ d*inv(sa),
                                sa*(da - min(da, (da-d)*sa*rcp_fast(s))) + s*inv(da) + d*inv(sa)));
}
BLEND_MODE(colordodge) {
    return if_then_else(d ==  0, /* d + */ s*inv(da),
           if_then_else(s == sa,    s +    d*inv(sa),
                                 sa*min(da, (d*sa)*rcp_fast(sa - s)) + s*inv(da) + d*inv(sa)));
}
BLEND_MODE(hardlight) {
    return s*inv(da) + d*inv(sa)
         + if_then_else(two(s) <= sa, two(s*d), sa*da - two((da-d)*(sa-s)));
}
BLEND_MODE(overlay) {
    return s*inv(da) + d*inv(sa)
         + if_then_else(two(d) <= da, two(s*d), sa*da - two((da-d)*(sa-s)));
}

BLEND_MODE(softlight) {
    F m  = if_then_else(da > 0, d / da, 0.0f),
      s2 = two(s),
      m4 = two(two(m));

    // The logic forks three ways:
    //    1. dark src?
    //    2. light src, dark dst?
    //    3. light src, light dst?
    F darkSrc = d*(sa + (s2 - sa)*(1.0f - m)),     // Used in case 1.
      darkDst = (m4*m4 + m4)*(m - 1.0f) + 7.0f*m,  // Used in case 2.
      liteDst = sqrt_(m) - m,
      liteSrc = d*sa + da*(s2 - sa) * if_then_else(two(two(d)) <= da, darkDst, liteDst); // 2 or 3?
    return s*inv(da) + d*inv(sa) + if_then_else(s2 <= sa, darkSrc, liteSrc);      // 1 or (2 or 3)?
}
#undef BLEND_MODE

// We're basing our implemenation of non-separable blend modes on
//   https://www.w3.org/TR/compositing-1/#blendingnonseparable.
// and
//   https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
// They're equivalent, but ES' math has been better simplified.
//
// Anything extra we add beyond that is to make the math work with premul inputs.

SI F sat(F r, F g, F b) { return max(r, max(g,b)) - min(r, min(g,b)); }
SI F lum(F r, F g, F b) { return mad(r, 0.30f, mad(g, 0.59f, b*0.11f)); }

SI void set_sat(F* r, F* g, F* b, F s) {
    F mn  = min(*r, min(*g,*b)),
      mx  = max(*r, max(*g,*b)),
      sat = mx - mn;

    // Map min channel to 0, max channel to s, and scale the middle proportionally.
    auto scale = [=](F c) {
        return if_then_else(sat == 0, 0.0f, (c - mn) * s / sat);
    };
    *r = scale(*r);
    *g = scale(*g);
    *b = scale(*b);
}
SI void set_lum(F* r, F* g, F* b, F l) {
    F diff = l - lum(*r, *g, *b);
    *r += diff;
    *g += diff;
    *b += diff;
}
SI void clip_color(F* r, F* g, F* b, F a) {
    F mn = min(*r, min(*g, *b)),
      mx = max(*r, max(*g, *b)),
      l  = lum(*r, *g, *b);

    auto clip = [=](F c) {
        c = if_then_else(mn < 0 && l != mn, l + (c - l) * (    l) / (l - mn), c);
        c = if_then_else(mx > a && l != mx, l + (c - l) * (a - l) / (mx - l), c);
        c = max(c, 0.0f);  // Sometimes without this we may dip just a little negative.
        return c;
    };
    *r = clip(*r);
    *g = clip(*g);
    *b = clip(*b);
}

STAGE(hue, NoCtx) {
    F R = r*a,
      G = g*a,
      B = b*a;

    set_sat(&R, &G, &B, sat(dr,dg,db)*a);
    set_lum(&R, &G, &B, lum(dr,dg,db)*a);
    clip_color(&R,&G,&B, a*da);

    r = r*inv(da) + dr*inv(a) + R;
    g = g*inv(da) + dg*inv(a) + G;
    b = b*inv(da) + db*inv(a) + B;
    a = a + da - a*da;
}
STAGE(saturation, NoCtx) {
    F R = dr*a,
      G = dg*a,
      B = db*a;

    set_sat(&R, &G, &B, sat( r, g, b)*da);
    set_lum(&R, &G, &B, lum(dr,dg,db)* a);  // (This is not redundant.)
    clip_color(&R,&G,&B, a*da);

    r = r*inv(da) + dr*inv(a) + R;
    g = g*inv(da) + dg*inv(a) + G;
    b = b*inv(da) + db*inv(a) + B;
    a = a + da - a*da;
}
STAGE(color, NoCtx) {
    F R = r*da,
      G = g*da,
      B = b*da;

    set_lum(&R, &G, &B, lum(dr,dg,db)*a);
    clip_color(&R,&G,&B, a*da);

    r = r*inv(da) + dr*inv(a) + R;
    g = g*inv(da) + dg*inv(a) + G;
    b = b*inv(da) + db*inv(a) + B;
    a = a + da - a*da;
}
STAGE(luminosity, NoCtx) {
    F R = dr*a,
      G = dg*a,
      B = db*a;

    set_lum(&R, &G, &B, lum(r,g,b)*da);
    clip_color(&R,&G,&B, a*da);

    r = r*inv(da) + dr*inv(a) + R;
    g = g*inv(da) + dg*inv(a) + G;
    b = b*inv(da) + db*inv(a) + B;
    a = a + da - a*da;
}

STAGE(srcover_rgba_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 dst = load<U32>(ptr);
    dr = cast((dst      ) & 0xff);
    dg = cast((dst >>  8) & 0xff);
    db = cast((dst >> 16) & 0xff);
    da = cast((dst >> 24)       );
    // {dr,dg,db,da} are in [0,255]
    // { r, g, b, a} are in [0,  1] (but may be out of gamut)

    r = mad(dr, inv(a), r*255.0f);
    g = mad(dg, inv(a), g*255.0f);
    b = mad(db, inv(a), b*255.0f);
    a = mad(da, inv(a), a*255.0f);
    // { r, g, b, a} are now in [0,255]  (but may be out of gamut)

    // to_unorm() clamps back to gamut.  Scaling by 1 since we're already 255-biased.
    dst = to_unorm(r, 1, 255)
        | to_unorm(g, 1, 255) <<  8
        | to_unorm(b, 1, 255) << 16
        | to_unorm(a, 1, 255) << 24;
    store(ptr, dst);
}

SI F clamp_01_(F v) { return min(max(0.0f, v), 1.0f); }

STAGE(clamp_01, NoCtx) {
    r = clamp_01_(r);
    g = clamp_01_(g);
    b = clamp_01_(b);
    a = clamp_01_(a);
}

STAGE(clamp_gamut, NoCtx) {
    a = min(max(a, 0.0f), 1.0f);
    r = min(max(r, 0.0f), a);
    g = min(max(g, 0.0f), a);
    b = min(max(b, 0.0f), a);
}

STAGE(set_rgb, const float* rgb) {
    r = F_(rgb[0]);
    g = F_(rgb[1]);
    b = F_(rgb[2]);
}

STAGE(unbounded_set_rgb, const float* rgb) {
    r = F_(rgb[0]);
    g = F_(rgb[1]);
    b = F_(rgb[2]);
}

STAGE(swap_rb, NoCtx) {
    auto tmp = r;
    r = b;
    b = tmp;
}
STAGE(swap_rb_dst, NoCtx) {
    auto tmp = dr;
    dr = db;
    db = tmp;
}

STAGE(move_src_dst, NoCtx) {
    dr = r;
    dg = g;
    db = b;
    da = a;
}
STAGE(move_dst_src, NoCtx) {
    r = dr;
    g = dg;
    b = db;
    a = da;
}
STAGE(swap_src_dst, NoCtx) {
    std::swap(r, dr);
    std::swap(g, dg);
    std::swap(b, db);
    std::swap(a, da);
}

STAGE(premul, NoCtx) {
    r = r * a;
    g = g * a;
    b = b * a;
}
STAGE(premul_dst, NoCtx) {
    dr = dr * da;
    dg = dg * da;
    db = db * da;
}
STAGE(unpremul, NoCtx) {
    float inf = sk_bit_cast<float>(0x7f800000);
    auto scale = if_then_else(1.0f/a < inf, 1.0f/a, 0.0f);
    r *= scale;
    g *= scale;
    b *= scale;
}
STAGE(unpremul_polar, NoCtx) {
    float inf = sk_bit_cast<float>(0x7f800000);
    auto scale = if_then_else(1.0f/a < inf, 1.0f/a, 0.0f);
    g *= scale;
    b *= scale;
}

STAGE(force_opaque    , NoCtx) {  a = F1; }
STAGE(force_opaque_dst, NoCtx) { da = F1; }

STAGE(rgb_to_hsl, NoCtx) {
    F mx = max(r, max(g,b)),
      mn = min(r, min(g,b)),
      d = mx - mn,
      d_rcp = 1.0f / d;

    F h = (1/6.0f) *
          if_then_else(mx == mn, 0.0f,
          if_then_else(mx ==  r, (g-b)*d_rcp + if_then_else(g < b, 6.0f, 0.0f),
          if_then_else(mx ==  g, (b-r)*d_rcp + 2.0f,
                                 (r-g)*d_rcp + 4.0f)));

    F l = (mx + mn) * 0.5f;
    F s = if_then_else(mx == mn, 0.0f,
                       d / if_then_else(l > 0.5f, 2.0f-mx-mn, mx+mn));

    r = h;
    g = s;
    b = l;
}
STAGE(hsl_to_rgb, NoCtx) {
    // See GrRGBToHSLFilterEffect.fp

    F h = r,
      s = g,
      l = b,
      c = (1.0f - abs_(2.0f * l - 1)) * s;

    auto hue_to_rgb = [&](F hue) {
        F q = clamp_01_(abs_(fract(hue) * 6.0f - 3.0f) - 1.0f);
        return (q - 0.5f) * c + l;
    };

    r = hue_to_rgb(h + 0.0f/3.0f);
    g = hue_to_rgb(h + 2.0f/3.0f);
    b = hue_to_rgb(h + 1.0f/3.0f);
}

// Color conversion functions used in gradient interpolation, based on
// https://www.w3.org/TR/css-color-4/#color-conversion-code
STAGE(css_lab_to_xyz, NoCtx) {
    constexpr float k = 24389 / 27.0f;
    constexpr float e = 216 / 24389.0f;

    F f[3];
    f[1] = (r + 16) * (1 / 116.0f);
    f[0] = (g * (1 / 500.0f)) + f[1];
    f[2] = f[1] - (b * (1 / 200.0f));

    F f_cubed[3] = { f[0]*f[0]*f[0], f[1]*f[1]*f[1], f[2]*f[2]*f[2] };

    F xyz[3] = {
        if_then_else(f_cubed[0] > e, f_cubed[0], (116 * f[0] - 16) * (1 / k)),
        if_then_else(r > k * e,      f_cubed[1], r * (1 / k)),
        if_then_else(f_cubed[2] > e, f_cubed[2], (116 * f[2] - 16) * (1 / k))
    };

    constexpr float D50[3] = { 0.3457f / 0.3585f, 1.0f, (1.0f - 0.3457f - 0.3585f) / 0.3585f };
    r = xyz[0]*D50[0];
    g = xyz[1]*D50[1];
    b = xyz[2]*D50[2];
}

STAGE(css_oklab_to_linear_srgb, NoCtx) {
    F l_ = r + 0.3963377774f * g + 0.2158037573f * b,
      m_ = r - 0.1055613458f * g - 0.0638541728f * b,
      s_ = r - 0.0894841775f * g - 1.2914855480f * b;

    F l = l_*l_*l_,
      m = m_*m_*m_,
      s = s_*s_*s_;

    r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
    g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
    b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;
}

STAGE(css_oklab_gamut_map_to_linear_srgb, NoCtx) {
    // TODO(https://crbug.com/1508329): Add support for gamut mapping.
    // Return a greyscale value, so that accidental use is obvious.
    F l_ = r,
      m_ = r,
      s_ = r;

    F l = l_*l_*l_,
      m = m_*m_*m_,
      s = s_*s_*s_;

    r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
    g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
    b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;
}

// Skia stores all polar colors with hue in the first component, so this "LCH -> Lab" transform
// actually takes "HCL". This is also used to do the same polar transform for OkHCL to OkLAB.
// See similar comments & logic in SkGradientBaseShader.cpp.
STAGE(css_hcl_to_lab, NoCtx) {
    F H = r,
      C = g,
      L = b;

    F hueRadians = H * (SK_FloatPI / 180);

    r = L;
    g = C * cos_(hueRadians);
    b = C * sin_(hueRadians);
}

SI F mod_(F x, float y) {
    return x - y * floor_(x * (1 / y));
}

struct RGB { F r, g, b; };

SI RGB css_hsl_to_srgb_(F h, F s, F l) {
    h = mod_(h, 360);

    s *= 0.01f;
    l *= 0.01f;

    F k[3] = {
        mod_(0 + h * (1 / 30.0f), 12),
        mod_(8 + h * (1 / 30.0f), 12),
        mod_(4 + h * (1 / 30.0f), 12)
    };
    F a  = s * min(l, 1 - l);
    return {
        l - a * max(-1.0f, min(min(k[0] - 3.0f, 9.0f - k[0]), 1.0f)),
        l - a * max(-1.0f, min(min(k[1] - 3.0f, 9.0f - k[1]), 1.0f)),
        l - a * max(-1.0f, min(min(k[2] - 3.0f, 9.0f - k[2]), 1.0f))
    };
}

STAGE(css_hsl_to_srgb, NoCtx) {
    RGB rgb = css_hsl_to_srgb_(r, g, b);
    r = rgb.r;
    g = rgb.g;
    b = rgb.b;
}

STAGE(css_hwb_to_srgb, NoCtx) {
    g *= 0.01f;
    b *= 0.01f;

    F gray = g / (g + b);

    RGB rgb = css_hsl_to_srgb_(r, F_(100.0f), F_(50.0f));
    rgb.r = rgb.r * (1 - g - b) + g;
    rgb.g = rgb.g * (1 - g - b) + g;
    rgb.b = rgb.b * (1 - g - b) + g;

    auto isGray = (g + b) >= 1;

    r = if_then_else(isGray, gray, rgb.r);
    g = if_then_else(isGray, gray, rgb.g);
    b = if_then_else(isGray, gray, rgb.b);
}

// Derive alpha's coverage from rgb coverage and the values of src and dst alpha.
SI F alpha_coverage_from_rgb_coverage(F a, F da, F cr, F cg, F cb) {
    return if_then_else(a < da, min(cr, min(cg,cb))
                              , max(cr, max(cg,cb)));
}

STAGE(scale_1_float, const float* c) {
    r = r * *c;
    g = g * *c;
    b = b * *c;
    a = a * *c;
}
STAGE(scale_u8, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    auto scales = load<U8>(ptr);
    auto c = from_byte(scales);

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(scale_565, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    F cr,cg,cb;
    from_565(load<U16>(ptr), &cr, &cg, &cb);

    F ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = r * cr;
    g = g * cg;
    b = b * cb;
    a = a * ca;
}

SI F lerp(F from, F to, F t) {
    return mad(to-from, t, from);
}

STAGE(lerp_1_float, const float* c) {
    r = lerp(dr, r, F_(*c));
    g = lerp(dg, g, F_(*c));
    b = lerp(db, b, F_(*c));
    a = lerp(da, a, F_(*c));
}
STAGE(scale_native, const float scales[]) {
    auto c = sk_unaligned_load<F>(scales);
    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(lerp_native, const float scales[]) {
    auto c = sk_unaligned_load<F>(scales);
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_u8, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    auto scales = load<U8>(ptr);
    auto c = from_byte(scales);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_565, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    F cr,cg,cb;
    from_565(load<U16>(ptr), &cr, &cg, &cb);

    F ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = lerp(da, a, ca);
}

STAGE(emboss, const SkRasterPipeline_EmbossCtx* ctx) {
    auto mptr = ptr_at_xy<const uint8_t>(&ctx->mul, dx,dy),
         aptr = ptr_at_xy<const uint8_t>(&ctx->add, dx,dy);

    F mul = from_byte(load<U8>(mptr)),
      add = from_byte(load<U8>(aptr));

    r = mad(r, mul, add);
    g = mad(g, mul, add);
    b = mad(b, mul, add);
}

STAGE(byte_tables, const SkRasterPipeline_TablesCtx* tables) {
    r = from_byte(gather(tables->r, to_unorm(r, 255)));
    g = from_byte(gather(tables->g, to_unorm(g, 255)));
    b = from_byte(gather(tables->b, to_unorm(b, 255)));
    a = from_byte(gather(tables->a, to_unorm(a, 255)));
}

SI F strip_sign(F x, U32* sign) {
    U32 bits = sk_bit_cast<U32>(x);
    *sign = bits & 0x80000000;
    return sk_bit_cast<F>(bits ^ *sign);
}

SI F apply_sign(F x, U32 sign) {
    return sk_bit_cast<F>(sign | sk_bit_cast<U32>(x));
}

STAGE(parametric, const skcms_TransferFunction* ctx) {
    auto fn = [&](F v) {
        U32 sign;
        v = strip_sign(v, &sign);

        F r = if_then_else(v <= ctx->d, mad(ctx->c, v, ctx->f)
                                      , approx_powf(mad(ctx->a, v, ctx->b), ctx->g) + ctx->e);
        return apply_sign(r, sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(gamma_, const float* G) {
    auto fn = [&](F v) {
        U32 sign;
        v = strip_sign(v, &sign);
        return apply_sign(approx_powf(v, *G), sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(PQish, const skcms_TransferFunction* ctx) {
    auto fn = [&](F v) {
        U32 sign;
        v = strip_sign(v, &sign);

        F r = approx_powf(max(mad(ctx->b, approx_powf(v, ctx->c), ctx->a), 0.0f)
                           / (mad(ctx->e, approx_powf(v, ctx->c), ctx->d)),
                        ctx->f);

        return apply_sign(r, sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(HLGish, const skcms_TransferFunction* ctx) {
    auto fn = [&](F v) {
        U32 sign;
        v = strip_sign(v, &sign);

        const float R = ctx->a, G = ctx->b,
                    a = ctx->c, b = ctx->d, c = ctx->e,
                    K = ctx->f + 1.0f;

        F r = if_then_else(v*R <= 1, approx_powf(v*R, G)
                                   , approx_exp((v-c)*a) + b);

        return K * apply_sign(r, sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(HLGinvish, const skcms_TransferFunction* ctx) {
    auto fn = [&](F v) {
        U32 sign;
        v = strip_sign(v, &sign);

        const float R = ctx->a, G = ctx->b,
                    a = ctx->c, b = ctx->d, c = ctx->e,
                    K = ctx->f + 1.0f;

        v /= K;
        F r = if_then_else(v <= 1, R * approx_powf(v, G)
                                 , a * approx_log(v - b) + c);

        return apply_sign(r, sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(load_a8, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    r = g = b = F0;
    a = from_byte(load<U8>(ptr));
}
STAGE(load_a8_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    dr = dg = db = F0;
    da = from_byte(load<U8>(ptr));
}
STAGE(gather_a8, const SkRasterPipeline_GatherCtx* ctx) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = g = b = F0;
    a = from_byte(gather(ptr, ix));
}
STAGE(store_a8, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint8_t>(ctx, dx,dy);

    U8 packed = pack(pack(to_unorm(a, 255)));
    store(ptr, packed);
}
STAGE(store_r8, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint8_t>(ctx, dx,dy);

    U8 packed = pack(pack(to_unorm(r, 255)));
    store(ptr, packed);
}

STAGE(load_565, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    from_565(load<U16>(ptr), &r,&g,&b);
    a = F1;
}
STAGE(load_565_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    from_565(load<U16>(ptr), &dr,&dg,&db);
    da = F1;
}
STAGE(gather_565, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_565(gather(ptr, ix), &r,&g,&b);
    a = F1;
}
STAGE(store_565, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);

    U16 px = pack( to_unorm(r, 31) << 11
                 | to_unorm(g, 63) <<  5
                 | to_unorm(b, 31)      );
    store(ptr, px);
}

STAGE(load_4444, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    from_4444(load<U16>(ptr), &r,&g,&b,&a);
}
STAGE(load_4444_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    from_4444(load<U16>(ptr), &dr,&dg,&db,&da);
}
STAGE(gather_4444, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_4444(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_4444, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);
    U16 px = pack( to_unorm(r, 15) << 12
                 | to_unorm(g, 15) <<  8
                 | to_unorm(b, 15) <<  4
                 | to_unorm(a, 15)      );
    store(ptr, px);
}

STAGE(load_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_8888(load<U32>(ptr), &r,&g,&b,&a);
}
STAGE(load_8888_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_8888(load<U32>(ptr), &dr,&dg,&db,&da);
}
STAGE(gather_8888, const SkRasterPipeline_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_8888(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 px = to_unorm(r, 255)
           | to_unorm(g, 255) <<  8
           | to_unorm(b, 255) << 16
           | to_unorm(a, 255) << 24;
    store(ptr, px);
}

STAGE(load_rg88, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx, dy);
    from_88(load<U16>(ptr), &r, &g);
    b = F0;
    a = F1;
}
STAGE(load_rg88_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx, dy);
    from_88(load<U16>(ptr), &dr, &dg);
    db = F0;
    da = F1;
}
STAGE(gather_rg88, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r, g);
    from_88(gather(ptr, ix), &r, &g);
    b = F0;
    a = F1;
}
STAGE(store_rg88, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx, dy);
    U16 px = pack( to_unorm(r, 255) | to_unorm(g, 255) <<  8 );
    store(ptr, px);
}

STAGE(load_a16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    r = g = b = F0;
    a = from_short(load<U16>(ptr));
}
STAGE(load_a16_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx, dy);
    dr = dg = db = F0;
    da = from_short(load<U16>(ptr));
}
STAGE(gather_a16, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r, g);
    r = g = b = F0;
    a = from_short(gather(ptr, ix));
}
STAGE(store_a16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);

    U16 px = pack(to_unorm(a, 65535));
    store(ptr, px);
}

STAGE(load_rg1616, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx, dy);
    b = F0;
    a = F1;
    from_1616(load<U32>(ptr), &r,&g);
}
STAGE(load_rg1616_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx, dy);
    from_1616(load<U32>(ptr), &dr, &dg);
    db = F0;
    da = F1;
}
STAGE(gather_rg1616, const SkRasterPipeline_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r, g);
    from_1616(gather(ptr, ix), &r, &g);
    b = F0;
    a = F1;
}
STAGE(store_rg1616, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 px = to_unorm(r, 65535)
           | to_unorm(g, 65535) <<  16;
    store(ptr, px);
}

STAGE(load_16161616, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx, dy);
    from_16161616(load<U64>(ptr), &r,&g, &b, &a);
}
STAGE(load_16161616_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx, dy);
    from_16161616(load<U64>(ptr), &dr, &dg, &db, &da);
}
STAGE(gather_16161616, const SkRasterPipeline_GatherCtx* ctx) {
    const uint64_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r, g);
    from_16161616(gather(ptr, ix), &r, &g, &b, &a);
}
STAGE(store_16161616, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, 4*dx,4*dy);

    U16 R = pack(to_unorm(r, 65535)),
        G = pack(to_unorm(g, 65535)),
        B = pack(to_unorm(b, 65535)),
        A = pack(to_unorm(a, 65535));

    store4(ptr, R,G,B,A);
}

STAGE(load_10x6, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx, dy);
    from_10x6(load<U64>(ptr), &r,&g, &b, &a);
}
STAGE(load_10x6_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx, dy);
    from_10x6(load<U64>(ptr), &dr, &dg, &db, &da);
}
STAGE(gather_10x6, const SkRasterPipeline_GatherCtx* ctx) {
    const uint64_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r, g);
    from_10x6(gather(ptr, ix), &r, &g, &b, &a);
}
STAGE(store_10x6, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, 4*dx,4*dy);

    U16 R = pack(to_unorm(r, 1023)) << 6,
        G = pack(to_unorm(g, 1023)) << 6,
        B = pack(to_unorm(b, 1023)) << 6,
        A = pack(to_unorm(a, 1023)) << 6;

    store4(ptr, R,G,B,A);
}


STAGE(load_1010102, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_1010102(load<U32>(ptr), &r,&g,&b,&a);
}
STAGE(load_1010102_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_1010102(load<U32>(ptr), &dr,&dg,&db,&da);
}
STAGE(load_1010102_xr, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_1010102_xr(load<U32>(ptr), &r,&g,&b,&a);
}
STAGE(load_1010102_xr_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_1010102_xr(load<U32>(ptr), &dr,&dg,&db,&da);
}
STAGE(gather_1010102, const SkRasterPipeline_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_1010102(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(gather_1010102_xr, const SkRasterPipeline_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r, g);
    from_1010102_xr(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_1010102, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 px = to_unorm(r, 1023)
           | to_unorm(g, 1023) << 10
           | to_unorm(b, 1023) << 20
           | to_unorm(a,    3) << 30;
    store(ptr, px);
}
STAGE(store_1010102_xr, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);
    static constexpr float min = -0.752941f;
    static constexpr float max = 1.25098f;
    static constexpr float range = max - min;
    U32 px = to_unorm((r - min) / range, 1023)
           | to_unorm((g - min) / range, 1023) << 10
           | to_unorm((b - min) / range, 1023) << 20
           | to_unorm(a,    3) << 30;
    store(ptr, px);
}

STAGE(load_f16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx,dy);

    U16 R,G,B,A;
    load4((const uint16_t*)ptr, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}
STAGE(load_f16_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx,dy);

    U16 R,G,B,A;
    load4((const uint16_t*)ptr, &R,&G,&B,&A);
    dr = from_half(R);
    dg = from_half(G);
    db = from_half(B);
    da = from_half(A);
}
STAGE(gather_f16, const SkRasterPipeline_GatherCtx* ctx) {
    const uint64_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    auto px = gather(ptr, ix);

    U16 R,G,B,A;
    load4((const uint16_t*)&px, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}
STAGE(store_f16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint64_t>(ctx, dx,dy);
    store4((uint16_t*)ptr, to_half(r)
                         , to_half(g)
                         , to_half(b)
                         , to_half(a));
}

STAGE(load_af16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    U16 A = load<U16>((const uint16_t*)ptr);
    r = F0;
    g = F0;
    b = F0;
    a = from_half(A);
}
STAGE(load_af16_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx, dy);

    U16 A = load<U16>((const uint16_t*)ptr);
    dr = dg = db = F0;
    da = from_half(A);
}
STAGE(gather_af16, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r, g);
    r = g = b = F0;
    a = from_half(gather(ptr, ix));
}
STAGE(store_af16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);
    store(ptr, to_half(a));
}

STAGE(load_rgf16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx, dy);

    U16 R,G;
    load2((const uint16_t*)ptr, &R, &G);
    r = from_half(R);
    g = from_half(G);
    b = F0;
    a = F1;
}
STAGE(load_rgf16_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx, dy);

    U16 R,G;
    load2((const uint16_t*)ptr, &R, &G);
    dr = from_half(R);
    dg = from_half(G);
    db = F0;
    da = F1;
}
STAGE(gather_rgf16, const SkRasterPipeline_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r, g);
    auto px = gather(ptr, ix);

    U16 R,G;
    load2((const uint16_t*)&px, &R, &G);
    r = from_half(R);
    g = from_half(G);
    b = F0;
    a = F1;
}
STAGE(store_rgf16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx, dy);
    store2((uint16_t*)ptr, to_half(r)
                         , to_half(g));
}

STAGE(load_f32, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const float>(ctx, 4*dx,4*dy);
    load4(ptr, &r,&g,&b,&a);
}
STAGE(load_f32_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const float>(ctx, 4*dx,4*dy);
    load4(ptr, &dr,&dg,&db,&da);
}
STAGE(gather_f32, const SkRasterPipeline_GatherCtx* ctx) {
    const float* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = gather(ptr, 4*ix + 0);
    g = gather(ptr, 4*ix + 1);
    b = gather(ptr, 4*ix + 2);
    a = gather(ptr, 4*ix + 3);
}
STAGE(store_f32, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<float>(ctx, 4*dx,4*dy);
    store4(ptr, r,g,b,a);
}

SI F exclusive_repeat(F v, const SkRasterPipeline_TileCtx* ctx) {
    return v - floor_(v*ctx->invScale)*ctx->scale;
}
SI F exclusive_mirror(F v, const SkRasterPipeline_TileCtx* ctx) {
    auto limit = ctx->scale;
    auto invLimit = ctx->invScale;

    // This is "repeat" over the range 0..2*limit
    auto u = v - floor_(v*invLimit*0.5f)*2*limit;
    // s will be 0 when moving forward (e.g. [0, limit)) and 1 when moving backward (e.g.
    // [limit, 2*limit)).
    auto s = floor_(u*invLimit);
    // This is the mirror result.
    auto m = u - 2*s*(u - limit);
    // Apply a bias to m if moving backwards so that we snap consistently at exact integer coords in
    // the logical infinite image. This is tested by mirror_tile GM. Note that all values
    // that have a non-zero bias applied are > 0.
    auto biasInUlps = trunc_(s);
    return sk_bit_cast<F>(sk_bit_cast<U32>(m) + ctx->mirrorBiasDir*biasInUlps);
}
// Tile x or y to [0,limit) == [0,limit - 1 ulp] (think, sampling from images).
// The gather stages will hard clamp the output of these stages to [0,limit)...
// we just need to do the basic repeat or mirroring.
STAGE(repeat_x, const SkRasterPipeline_TileCtx* ctx) { r = exclusive_repeat(r, ctx); }
STAGE(repeat_y, const SkRasterPipeline_TileCtx* ctx) { g = exclusive_repeat(g, ctx); }
STAGE(mirror_x, const SkRasterPipeline_TileCtx* ctx) { r = exclusive_mirror(r, ctx); }
STAGE(mirror_y, const SkRasterPipeline_TileCtx* ctx) { g = exclusive_mirror(g, ctx); }

STAGE( clamp_x_1, NoCtx) { r = clamp_01_(r); }
STAGE(repeat_x_1, NoCtx) { r = clamp_01_(r - floor_(r)); }
STAGE(mirror_x_1, NoCtx) { r = clamp_01_(abs_( (r-1.0f) - two(floor_((r-1.0f)*0.5f)) - 1.0f )); }

STAGE(clamp_x_and_y, const SkRasterPipeline_CoordClampCtx* ctx) {
    r = min(ctx->max_x, max(ctx->min_x, r));
    g = min(ctx->max_y, max(ctx->min_y, g));
}

// Decal stores a 32bit mask after checking the coordinate (x and/or y) against its domain:
//      mask == 0x00000000 if the coordinate(s) are out of bounds
//      mask == 0xFFFFFFFF if the coordinate(s) are in bounds
// After the gather stage, the r,g,b,a values are AND'd with this mask, setting them to 0
// if either of the coordinates were out of bounds.

STAGE(decal_x, SkRasterPipeline_DecalTileCtx* ctx) {
    auto w = ctx->limit_x;
    auto e = ctx->inclusiveEdge_x;
    auto cond = ((0 < r) & (r < w)) | (r == e);
    sk_unaligned_store(ctx->mask, cond_to_mask(cond));
}
STAGE(decal_y, SkRasterPipeline_DecalTileCtx* ctx) {
    auto h = ctx->limit_y;
    auto e = ctx->inclusiveEdge_y;
    auto cond = ((0 < g) & (g < h)) | (g == e);
    sk_unaligned_store(ctx->mask, cond_to_mask(cond));
}
STAGE(decal_x_and_y, SkRasterPipeline_DecalTileCtx* ctx) {
    auto w = ctx->limit_x;
    auto h = ctx->limit_y;
    auto ex = ctx->inclusiveEdge_x;
    auto ey = ctx->inclusiveEdge_y;
    auto cond = (((0 < r) & (r < w)) | (r == ex))
              & (((0 < g) & (g < h)) | (g == ey));
    sk_unaligned_store(ctx->mask, cond_to_mask(cond));
}
STAGE(check_decal_mask, SkRasterPipeline_DecalTileCtx* ctx) {
    auto mask = sk_unaligned_load<U32>(ctx->mask);
    r = sk_bit_cast<F>(sk_bit_cast<U32>(r) & mask);
    g = sk_bit_cast<F>(sk_bit_cast<U32>(g) & mask);
    b = sk_bit_cast<F>(sk_bit_cast<U32>(b) & mask);
    a = sk_bit_cast<F>(sk_bit_cast<U32>(a) & mask);
}

STAGE(alpha_to_gray, NoCtx) {
    r = g = b = a;
    a = F1;
}
STAGE(alpha_to_gray_dst, NoCtx) {
    dr = dg = db = da;
    da = F1;
}
STAGE(alpha_to_red, NoCtx) {
    r = a;
    a = F1;
}
STAGE(alpha_to_red_dst, NoCtx) {
    dr = da;
    da = F1;
}

STAGE(bt709_luminance_or_luma_to_alpha, NoCtx) {
    a = r*0.2126f + g*0.7152f + b*0.0722f;
    r = g = b = F0;
}
STAGE(bt709_luminance_or_luma_to_rgb, NoCtx) {
    r = g = b = r*0.2126f + g*0.7152f + b*0.0722f;
}

STAGE(matrix_translate, const float* m) {
    r += m[0];
    g += m[1];
}
STAGE(matrix_scale_translate, const float* m) {
    r = mad(r,m[0], m[2]);
    g = mad(g,m[1], m[3]);
}
STAGE(matrix_2x3, const float* m) {
    auto R = mad(r,m[0], mad(g,m[1], m[2])),
         G = mad(r,m[3], mad(g,m[4], m[5]));
    r = R;
    g = G;
}
STAGE(matrix_3x3, const float* m) {
    auto R = mad(r,m[0], mad(g,m[3], b*m[6])),
         G = mad(r,m[1], mad(g,m[4], b*m[7])),
         B = mad(r,m[2], mad(g,m[5], b*m[8]));
    r = R;
    g = G;
    b = B;
}
STAGE(matrix_3x4, const float* m) {
    auto R = mad(r,m[0], mad(g,m[3], mad(b,m[6], m[ 9]))),
         G = mad(r,m[1], mad(g,m[4], mad(b,m[7], m[10]))),
         B = mad(r,m[2], mad(g,m[5], mad(b,m[8], m[11])));
    r = R;
    g = G;
    b = B;
}
STAGE(matrix_4x5, const float* m) {
    auto R = mad(r,m[ 0], mad(g,m[ 1], mad(b,m[ 2], mad(a,m[ 3], m[ 4])))),
         G = mad(r,m[ 5], mad(g,m[ 6], mad(b,m[ 7], mad(a,m[ 8], m[ 9])))),
         B = mad(r,m[10], mad(g,m[11], mad(b,m[12], mad(a,m[13], m[14])))),
         A = mad(r,m[15], mad(g,m[16], mad(b,m[17], mad(a,m[18], m[19]))));
    r = R;
    g = G;
    b = B;
    a = A;
}
STAGE(matrix_4x3, const float* m) {
    auto X = r,
         Y = g;

    r = mad(X, m[0], mad(Y, m[4], m[ 8]));
    g = mad(X, m[1], mad(Y, m[5], m[ 9]));
    b = mad(X, m[2], mad(Y, m[6], m[10]));
    a = mad(X, m[3], mad(Y, m[7], m[11]));
}
STAGE(matrix_perspective, const float* m) {
    // N.B. Unlike the other matrix_ stages, this matrix is row-major.
    auto R = mad(r,m[0], mad(g,m[1], m[2])),
         G = mad(r,m[3], mad(g,m[4], m[5])),
         Z = mad(r,m[6], mad(g,m[7], m[8]));
    r = R * rcp_precise(Z);
    g = G * rcp_precise(Z);
}

SI void gradient_lookup(const SkRasterPipeline_GradientCtx* c, U32 idx, F t,
                        F* r, F* g, F* b, F* a) {
    F fr, br, fg, bg, fb, bb, fa, ba;
#if defined(JUMPER_IS_HSW)
    if (c->stopCount <=8) {
        fr = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[0]), (__m256i)idx);
        br = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[0]), (__m256i)idx);
        fg = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[1]), (__m256i)idx);
        bg = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[1]), (__m256i)idx);
        fb = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[2]), (__m256i)idx);
        bb = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[2]), (__m256i)idx);
        fa = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[3]), (__m256i)idx);
        ba = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[3]), (__m256i)idx);
    } else
#endif
    {
        fr = gather(c->fs[0], idx);
        br = gather(c->bs[0], idx);
        fg = gather(c->fs[1], idx);
        bg = gather(c->bs[1], idx);
        fb = gather(c->fs[2], idx);
        bb = gather(c->bs[2], idx);
        fa = gather(c->fs[3], idx);
        ba = gather(c->bs[3], idx);
    }

    *r = mad(t, fr, br);
    *g = mad(t, fg, bg);
    *b = mad(t, fb, bb);
    *a = mad(t, fa, ba);
}

STAGE(evenly_spaced_gradient, const SkRasterPipeline_GradientCtx* c) {
    auto t = r;
    auto idx = trunc_(t * static_cast<float>(c->stopCount-1));
    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE(gradient, const SkRasterPipeline_GradientCtx* c) {
    auto t = r;
    U32 idx = U32_(0);

    // N.B. The loop starts at 1 because idx 0 is the color to use before the first stop.
    for (size_t i = 1; i < c->stopCount; i++) {
        idx += (U32)if_then_else(t >= c->ts[i], I32_(1), I32_(0));
    }

    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE(evenly_spaced_2_stop_gradient, const SkRasterPipeline_EvenlySpaced2StopGradientCtx* c) {
    auto t = r;
    r = mad(t, c->f[0], c->b[0]);
    g = mad(t, c->f[1], c->b[1]);
    b = mad(t, c->f[2], c->b[2]);
    a = mad(t, c->f[3], c->b[3]);
}

STAGE(xy_to_unit_angle, NoCtx) {
    F X = r,
      Y = g;
    F xabs = abs_(X),
      yabs = abs_(Y);

    F slope = min(xabs, yabs)/max(xabs, yabs);
    F s = slope * slope;

    // Use a 7th degree polynomial to approximate atan.
    // This was generated using sollya.gforge.inria.fr.
    // A float optimized polynomial was generated using the following command.
    // P1 = fpminimax((1/(2*Pi))*atan(x),[|1,3,5,7|],[|24...|],[2^(-40),1],relative);
    F phi = slope
             * (0.15912117063999176025390625f     + s
             * (-5.185396969318389892578125e-2f   + s
             * (2.476101927459239959716796875e-2f + s
             * (-7.0547382347285747528076171875e-3f))));

    phi = if_then_else(xabs < yabs, 1.0f/4.0f - phi, phi);
    phi = if_then_else(X < 0.0f   , 1.0f/2.0f - phi, phi);
    phi = if_then_else(Y < 0.0f   , 1.0f - phi     , phi);
    phi = if_then_else(phi != phi , 0.0f           , phi);  // Check for NaN.
    r = phi;
}

STAGE(xy_to_radius, NoCtx) {
    F X2 = r * r,
      Y2 = g * g;
    r = sqrt_(X2 + Y2);
}

// Please see https://skia.org/dev/design/conical for how our 2pt conical shader works.

STAGE(negate_x, NoCtx) { r = -r; }

STAGE(xy_to_2pt_conical_strip, const SkRasterPipeline_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = x + sqrt_(ctx->fP0 - y*y); // ctx->fP0 = r0 * r0
}

STAGE(xy_to_2pt_conical_focal_on_circle, NoCtx) {
    F x = r, y = g, &t = r;
    t = x + y*y / x; // (x^2 + y^2) / x
}

STAGE(xy_to_2pt_conical_well_behaved, const SkRasterPipeline_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = sqrt_(x*x + y*y) - x * ctx->fP0; // ctx->fP0 = 1/r1
}

STAGE(xy_to_2pt_conical_greater, const SkRasterPipeline_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = sqrt_(x*x - y*y) - x * ctx->fP0; // ctx->fP0 = 1/r1
}

STAGE(xy_to_2pt_conical_smaller, const SkRasterPipeline_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = -sqrt_(x*x - y*y) - x * ctx->fP0; // ctx->fP0 = 1/r1
}

STAGE(alter_2pt_conical_compensate_focal, const SkRasterPipeline_2PtConicalCtx* ctx) {
    F& t = r;
    t = t + ctx->fP1; // ctx->fP1 = f
}

STAGE(alter_2pt_conical_unswap, NoCtx) {
    F& t = r;
    t = 1 - t;
}

STAGE(mask_2pt_conical_nan, SkRasterPipeline_2PtConicalCtx* c) {
    F& t = r;
    auto is_degenerate = (t != t); // NaN
    t = if_then_else(is_degenerate, F0, t);
    sk_unaligned_store(&c->fMask, cond_to_mask(!is_degenerate));
}

STAGE(mask_2pt_conical_degenerates, SkRasterPipeline_2PtConicalCtx* c) {
    F& t = r;
    auto is_degenerate = (t <= 0) | (t != t);
    t = if_then_else(is_degenerate, F0, t);
    sk_unaligned_store(&c->fMask, cond_to_mask(!is_degenerate));
}

STAGE(apply_vector_mask, const uint32_t* ctx) {
    const U32 mask = sk_unaligned_load<U32>(ctx);
    r = sk_bit_cast<F>(sk_bit_cast<U32>(r) & mask);
    g = sk_bit_cast<F>(sk_bit_cast<U32>(g) & mask);
    b = sk_bit_cast<F>(sk_bit_cast<U32>(b) & mask);
    a = sk_bit_cast<F>(sk_bit_cast<U32>(a) & mask);
}

SI void save_xy(F* r, F* g, SkRasterPipeline_SamplerCtx* c) {
    // Whether bilinear or bicubic, all sample points are at the same fractional offset (fx,fy).
    // They're either the 4 corners of a logical 1x1 pixel or the 16 corners of a 3x3 grid
    // surrounding (x,y) at (0.5,0.5) off-center.
    F fx = fract(*r + 0.5f),
      fy = fract(*g + 0.5f);

    // Samplers will need to load x and fx, or y and fy.
    sk_unaligned_store(c->x,  *r);
    sk_unaligned_store(c->y,  *g);
    sk_unaligned_store(c->fx, fx);
    sk_unaligned_store(c->fy, fy);
}

STAGE(accumulate, const SkRasterPipeline_SamplerCtx* c) {
    // Bilinear and bicubic filters are both separable, so we produce independent contributions
    // from x and y, multiplying them together here to get each pixel's total scale factor.
    auto scale = sk_unaligned_load<F>(c->scalex)
               * sk_unaligned_load<F>(c->scaley);
    dr = mad(scale, r, dr);
    dg = mad(scale, g, dg);
    db = mad(scale, b, db);
    da = mad(scale, a, da);
}

// In bilinear interpolation, the 4 pixels at +/- 0.5 offsets from the sample pixel center
// are combined in direct proportion to their area overlapping that logical query pixel.
// At positive offsets, the x-axis contribution to that rectangle is fx, or (1-fx) at negative x.
// The y-axis is symmetric.

template <int kScale>
SI void bilinear_x(SkRasterPipeline_SamplerCtx* ctx, F* x) {
    *x = sk_unaligned_load<F>(ctx->x) + (kScale * 0.5f);
    F fx = sk_unaligned_load<F>(ctx->fx);

    F scalex;
    if (kScale == -1) { scalex = 1.0f - fx; }
    if (kScale == +1) { scalex =        fx; }
    sk_unaligned_store(ctx->scalex, scalex);
}
template <int kScale>
SI void bilinear_y(SkRasterPipeline_SamplerCtx* ctx, F* y) {
    *y = sk_unaligned_load<F>(ctx->y) + (kScale * 0.5f);
    F fy = sk_unaligned_load<F>(ctx->fy);

    F scaley;
    if (kScale == -1) { scaley = 1.0f - fy; }
    if (kScale == +1) { scaley =        fy; }
    sk_unaligned_store(ctx->scaley, scaley);
}

STAGE(bilinear_setup, SkRasterPipeline_SamplerCtx* ctx) {
    save_xy(&r, &g, ctx);
    // Init for accumulate
    dr = dg = db = da = F0;
}

STAGE(bilinear_nx, SkRasterPipeline_SamplerCtx* ctx) { bilinear_x<-1>(ctx, &r); }
STAGE(bilinear_px, SkRasterPipeline_SamplerCtx* ctx) { bilinear_x<+1>(ctx, &r); }
STAGE(bilinear_ny, SkRasterPipeline_SamplerCtx* ctx) { bilinear_y<-1>(ctx, &g); }
STAGE(bilinear_py, SkRasterPipeline_SamplerCtx* ctx) { bilinear_y<+1>(ctx, &g); }


// In bicubic interpolation, the 16 pixels and +/- 0.5 and +/- 1.5 offsets from the sample
// pixel center are combined with a non-uniform cubic filter, with higher values near the center.
//
// This helper computes the total weight along one axis (our bicubic filter is separable), given one
// column of the sampling matrix, and a fractional pixel offset. See SkCubicResampler for details.

SI F bicubic_wts(F t, float A, float B, float C, float D) {
    return mad(t, mad(t, mad(t, D, C), B), A);
}

template <int kScale>
SI void bicubic_x(SkRasterPipeline_SamplerCtx* ctx, F* x) {
    *x = sk_unaligned_load<F>(ctx->x) + (kScale * 0.5f);

    F scalex;
    if (kScale == -3) { scalex = sk_unaligned_load<F>(ctx->wx[0]); }
    if (kScale == -1) { scalex = sk_unaligned_load<F>(ctx->wx[1]); }
    if (kScale == +1) { scalex = sk_unaligned_load<F>(ctx->wx[2]); }
    if (kScale == +3) { scalex = sk_unaligned_load<F>(ctx->wx[3]); }
    sk_unaligned_store(ctx->scalex, scalex);
}
template <int kScale>
SI void bicubic_y(SkRasterPipeline_SamplerCtx* ctx, F* y) {
    *y = sk_unaligned_load<F>(ctx->y) + (kScale * 0.5f);

    F scaley;
    if (kScale == -3) { scaley = sk_unaligned_load<F>(ctx->wy[0]); }
    if (kScale == -1) { scaley = sk_unaligned_load<F>(ctx->wy[1]); }
    if (kScale == +1) { scaley = sk_unaligned_load<F>(ctx->wy[2]); }
    if (kScale == +3) { scaley = sk_unaligned_load<F>(ctx->wy[3]); }
    sk_unaligned_store(ctx->scaley, scaley);
}

STAGE(bicubic_setup, SkRasterPipeline_SamplerCtx* ctx) {
    save_xy(&r, &g, ctx);

    const float* w = ctx->weights;

    F fx = sk_unaligned_load<F>(ctx->fx);
    sk_unaligned_store(ctx->wx[0], bicubic_wts(fx, w[0], w[4], w[ 8], w[12]));
    sk_unaligned_store(ctx->wx[1], bicubic_wts(fx, w[1], w[5], w[ 9], w[13]));
    sk_unaligned_store(ctx->wx[2], bicubic_wts(fx, w[2], w[6], w[10], w[14]));
    sk_unaligned_store(ctx->wx[3], bicubic_wts(fx, w[3], w[7], w[11], w[15]));

    F fy = sk_unaligned_load<F>(ctx->fy);
    sk_unaligned_store(ctx->wy[0], bicubic_wts(fy, w[0], w[4], w[ 8], w[12]));
    sk_unaligned_store(ctx->wy[1], bicubic_wts(fy, w[1], w[5], w[ 9], w[13]));
    sk_unaligned_store(ctx->wy[2], bicubic_wts(fy, w[2], w[6], w[10], w[14]));
    sk_unaligned_store(ctx->wy[3], bicubic_wts(fy, w[3], w[7], w[11], w[15]));

    // Init for accumulate
    dr = dg = db = da = F0;
}

STAGE(bicubic_n3x, SkRasterPipeline_SamplerCtx* ctx) { bicubic_x<-3>(ctx, &r); }
STAGE(bicubic_n1x, SkRasterPipeline_SamplerCtx* ctx) { bicubic_x<-1>(ctx, &r); }
STAGE(bicubic_p1x, SkRasterPipeline_SamplerCtx* ctx) { bicubic_x<+1>(ctx, &r); }
STAGE(bicubic_p3x, SkRasterPipeline_SamplerCtx* ctx) { bicubic_x<+3>(ctx, &r); }

STAGE(bicubic_n3y, SkRasterPipeline_SamplerCtx* ctx) { bicubic_y<-3>(ctx, &g); }
STAGE(bicubic_n1y, SkRasterPipeline_SamplerCtx* ctx) { bicubic_y<-1>(ctx, &g); }
STAGE(bicubic_p1y, SkRasterPipeline_SamplerCtx* ctx) { bicubic_y<+1>(ctx, &g); }
STAGE(bicubic_p3y, SkRasterPipeline_SamplerCtx* ctx) { bicubic_y<+3>(ctx, &g); }

STAGE(mipmap_linear_init, SkRasterPipeline_MipmapCtx* ctx) {
    sk_unaligned_store(ctx->x, r);
    sk_unaligned_store(ctx->y, g);
}

STAGE(mipmap_linear_update, SkRasterPipeline_MipmapCtx* ctx) {
    sk_unaligned_store(ctx->r, r);
    sk_unaligned_store(ctx->g, g);
    sk_unaligned_store(ctx->b, b);
    sk_unaligned_store(ctx->a, a);

    r = sk_unaligned_load<F>(ctx->x) * ctx->scaleX;
    g = sk_unaligned_load<F>(ctx->y) * ctx->scaleY;
}

STAGE(mipmap_linear_finish, SkRasterPipeline_MipmapCtx* ctx) {
    r = lerp(sk_unaligned_load<F>(ctx->r), r, F_(ctx->lowerWeight));
    g = lerp(sk_unaligned_load<F>(ctx->g), g, F_(ctx->lowerWeight));
    b = lerp(sk_unaligned_load<F>(ctx->b), b, F_(ctx->lowerWeight));
    a = lerp(sk_unaligned_load<F>(ctx->a), a, F_(ctx->lowerWeight));
}

STAGE(callback, SkRasterPipeline_CallbackCtx* c) {
    store4(c->rgba, r,g,b,a);
    c->fn(c, N);
    load4(c->read_from, &r,&g,&b,&a);
}

STAGE_TAIL(set_base_pointer, std::byte* p) {
    base = p;
}

// All control flow stages used by SkSL maintain some state in the common registers:
//   r: condition mask
//   g: loop mask
//   b: return mask
//   a: execution mask (intersection of all three masks)
// After updating r/g/b, you must invoke update_execution_mask().
#define execution_mask()        sk_bit_cast<I32>(a)
#define update_execution_mask() a = sk_bit_cast<F>(sk_bit_cast<I32>(r) & \
                                                   sk_bit_cast<I32>(g) & \
                                                   sk_bit_cast<I32>(b))

STAGE_TAIL(init_lane_masks, SkRasterPipeline_InitLaneMasksCtx* ctx) {
    uint32_t iota[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride_highp);

    I32 mask = cond_to_mask(sk_unaligned_load<U32>(iota) < *ctx->tail);
    r = g = b = a = sk_bit_cast<F>(mask);
}

STAGE_TAIL(store_device_xy01, F* dst) {
    // This is very similar to `seed_shader + store_src`, but b/a are backwards.
    // (sk_FragCoord actually puts w=1 in the w slot.)
    static constexpr float iota[] = {
        0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f,
        8.5f, 9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f,
    };
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride_highp);

    dst[0] = cast(U32_(dx)) + sk_unaligned_load<F>(iota);
    dst[1] = cast(U32_(dy)) + 0.5f;
    dst[2] = F0;
    dst[3] = F1;
}

STAGE_TAIL(exchange_src, F* rgba) {
    // Swaps r,g,b,a registers with the values at `rgba`.
    F temp[4] = {r, g, b, a};
    r = rgba[0];
    rgba[0] = temp[0];
    g = rgba[1];
    rgba[1] = temp[1];
    b = rgba[2];
    rgba[2] = temp[2];
    a = rgba[3];
    rgba[3] = temp[3];
}

STAGE_TAIL(load_condition_mask, F* ctx) {
    r = sk_unaligned_load<F>(ctx);
    update_execution_mask();
}

STAGE_TAIL(store_condition_mask, F* ctx) {
    sk_unaligned_store(ctx, r);
}

STAGE_TAIL(merge_condition_mask, I32* ptr) {
    // Set the condition-mask to the intersection of two adjacent masks at the pointer.
    r = sk_bit_cast<F>(ptr[0] & ptr[1]);
    update_execution_mask();
}

STAGE_TAIL(merge_inv_condition_mask, I32* ptr) {
    // Set the condition-mask to the intersection of the first mask and the inverse of the second.
    r = sk_bit_cast<F>(ptr[0] & ~ptr[1]);
    update_execution_mask();
}

STAGE_TAIL(load_loop_mask, F* ctx) {
    g = sk_unaligned_load<F>(ctx);
    update_execution_mask();
}

STAGE_TAIL(store_loop_mask, F* ctx) {
    sk_unaligned_store(ctx, g);
}

STAGE_TAIL(mask_off_loop_mask, NoCtx) {
    // We encountered a break statement. If a lane was active, it should be masked off now, and stay
    // masked-off until the termination of the loop.
    g = sk_bit_cast<F>(sk_bit_cast<I32>(g) & ~execution_mask());
    update_execution_mask();
}

STAGE_TAIL(reenable_loop_mask, I32* ptr) {
    // Set the loop-mask to the union of the current loop-mask with the mask at the pointer.
    g = sk_bit_cast<F>(sk_bit_cast<I32>(g) | ptr[0]);
    update_execution_mask();
}

STAGE_TAIL(merge_loop_mask, I32* ptr) {
    // Set the loop-mask to the intersection of the current loop-mask with the mask at the pointer.
    // (Note: this behavior subtly differs from merge_condition_mask!)
    g = sk_bit_cast<F>(sk_bit_cast<I32>(g) & ptr[0]);
    update_execution_mask();
}

STAGE_TAIL(continue_op, I32* continueMask) {
    // Set any currently-executing lanes in the continue-mask to true.
    *continueMask |= execution_mask();

    // Disable any currently-executing lanes from the loop mask. (Just like `mask_off_loop_mask`.)
    g = sk_bit_cast<F>(sk_bit_cast<I32>(g) & ~execution_mask());
    update_execution_mask();
}

STAGE_TAIL(case_op, SkRasterPipeline_CaseOpCtx* packed) {
    auto ctx = SkRPCtxUtils::Unpack(packed);

    // Check each lane to see if the case value matches the expectation.
    I32* actualValue = (I32*)(base + ctx.offset);
    I32 caseMatches = cond_to_mask(*actualValue == ctx.expectedValue);

    // In lanes where we found a match, enable the loop mask...
    g = sk_bit_cast<F>(sk_bit_cast<I32>(g) | caseMatches);
    update_execution_mask();

    // ... and clear the default-case mask.
    I32* defaultMask = actualValue + 1;
    *defaultMask &= ~caseMatches;
}

STAGE_TAIL(load_return_mask, F* ctx) {
    b = sk_unaligned_load<F>(ctx);
    update_execution_mask();
}

STAGE_TAIL(store_return_mask, F* ctx) {
    sk_unaligned_store(ctx, b);
}

STAGE_TAIL(mask_off_return_mask, NoCtx) {
    // We encountered a return statement. If a lane was active, it should be masked off now, and
    // stay masked-off until the end of the function.
    b = sk_bit_cast<F>(sk_bit_cast<I32>(b) & ~execution_mask());
    update_execution_mask();
}

STAGE_BRANCH(branch_if_all_lanes_active, SkRasterPipeline_BranchIfAllLanesActiveCtx* ctx) {
    uint32_t iota[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride_highp);

    I32 tailLanes = cond_to_mask(*ctx->tail <= sk_unaligned_load<U32>(iota));
    return all(execution_mask() | tailLanes) ? ctx->offset : 1;
}

STAGE_BRANCH(branch_if_any_lanes_active, SkRasterPipeline_BranchCtx* ctx) {
    return any(execution_mask()) ? ctx->offset : 1;
}

STAGE_BRANCH(branch_if_no_lanes_active, SkRasterPipeline_BranchCtx* ctx) {
    return any(execution_mask()) ? 1 : ctx->offset;
}

STAGE_BRANCH(jump, SkRasterPipeline_BranchCtx* ctx) {
    return ctx->offset;
}

STAGE_BRANCH(branch_if_no_active_lanes_eq, SkRasterPipeline_BranchIfEqualCtx* ctx) {
    // Compare each lane against the expected value...
    I32 match = cond_to_mask(*(const I32*)ctx->ptr == ctx->value);
    // ... but mask off lanes that aren't executing.
    match &= execution_mask();
    // If any lanes matched, don't take the branch.
    return any(match) ? 1 : ctx->offset;
}

STAGE_TAIL(trace_line, SkRasterPipeline_TraceLineCtx* ctx) {
    const I32* traceMask = (const I32*)ctx->traceMask;
    if (any(execution_mask() & *traceMask)) {
        ctx->traceHook->line(ctx->lineNumber);
    }
}

STAGE_TAIL(trace_enter, SkRasterPipeline_TraceFuncCtx* ctx) {
    const I32* traceMask = (const I32*)ctx->traceMask;
    if (any(execution_mask() & *traceMask)) {
        ctx->traceHook->enter(ctx->funcIdx);
    }
}

STAGE_TAIL(trace_exit, SkRasterPipeline_TraceFuncCtx* ctx) {
    const I32* traceMask = (const I32*)ctx->traceMask;
    if (any(execution_mask() & *traceMask)) {
        ctx->traceHook->exit(ctx->funcIdx);
    }
}

STAGE_TAIL(trace_scope, SkRasterPipeline_TraceScopeCtx* ctx) {
    // Note that trace_scope intentionally does not incorporate the execution mask. Otherwise, the
    // scopes would become unbalanced if the execution mask changed in the middle of a block. The
    // caller is responsible for providing a combined trace- and execution-mask.
    const I32* traceMask = (const I32*)ctx->traceMask;
    if (any(*traceMask)) {
        ctx->traceHook->scope(ctx->delta);
    }
}

STAGE_TAIL(trace_var, SkRasterPipeline_TraceVarCtx* ctx) {
    const I32* traceMask = (const I32*)ctx->traceMask;
    I32 mask = execution_mask() & *traceMask;
    if (any(mask)) {
        for (size_t lane = 0; lane < N; ++lane) {
            if (select_lane(mask, lane)) {
                const I32* data = (const I32*)ctx->data;
                int slotIdx = ctx->slotIdx, numSlots = ctx->numSlots;
                if (ctx->indirectOffset) {
                    // If this was an indirect store, apply the indirect-offset to the data pointer.
                    uint32_t indirectOffset = select_lane(*(const U32*)ctx->indirectOffset, lane);
                    indirectOffset = std::min<uint32_t>(indirectOffset, ctx->indirectLimit);
                    data += indirectOffset;
                    slotIdx += indirectOffset;
                }
                while (numSlots--) {
                    ctx->traceHook->var(slotIdx, select_lane(*data, lane));
                    ++slotIdx;
                    ++data;
                }
                break;
            }
        }
    }
}

STAGE_TAIL(copy_uniform, SkRasterPipeline_UniformCtx* ctx) {
    const int* src = ctx->src;
    I32* dst = (I32*)ctx->dst;
    dst[0] = I32_(src[0]);
}
STAGE_TAIL(copy_2_uniforms, SkRasterPipeline_UniformCtx* ctx) {
    const int* src = ctx->src;
    I32* dst = (I32*)ctx->dst;
    dst[0] = I32_(src[0]);
    dst[1] = I32_(src[1]);
}
STAGE_TAIL(copy_3_uniforms, SkRasterPipeline_UniformCtx* ctx) {
    const int* src = ctx->src;
    I32* dst = (I32*)ctx->dst;
    dst[0] = I32_(src[0]);
    dst[1] = I32_(src[1]);
    dst[2] = I32_(src[2]);
}
STAGE_TAIL(copy_4_uniforms, SkRasterPipeline_UniformCtx* ctx) {
    const int* src = ctx->src;
    I32* dst = (I32*)ctx->dst;
    dst[0] = I32_(src[0]);
    dst[1] = I32_(src[1]);
    dst[2] = I32_(src[2]);
    dst[3] = I32_(src[3]);
}

STAGE_TAIL(copy_constant, SkRasterPipeline_ConstantCtx* packed) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    I32* dst = (I32*)(base + ctx.dst);
    I32 value = I32_(ctx.value);
    dst[0] = value;
}
STAGE_TAIL(splat_2_constants, SkRasterPipeline_ConstantCtx* packed) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    I32* dst = (I32*)(base + ctx.dst);
    I32 value = I32_(ctx.value);
    dst[0] = dst[1] = value;
}
STAGE_TAIL(splat_3_constants, SkRasterPipeline_ConstantCtx* packed) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    I32* dst = (I32*)(base + ctx.dst);
    I32 value = I32_(ctx.value);
    dst[0] = dst[1] = dst[2] = value;
}
STAGE_TAIL(splat_4_constants, SkRasterPipeline_ConstantCtx* packed) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    I32* dst = (I32*)(base + ctx.dst);
    I32 value = I32_(ctx.value);
    dst[0] = dst[1] = dst[2] = dst[3] = value;
}

template <int NumSlots>
SI void copy_n_slots_unmasked_fn(SkRasterPipeline_BinaryOpCtx* packed, std::byte* base) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    F* dst = (F*)(base + ctx.dst);
    F* src = (F*)(base + ctx.src);
    memcpy(dst, src, sizeof(F) * NumSlots);
}

STAGE_TAIL(copy_slot_unmasked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_slots_unmasked_fn<1>(packed, base);
}
STAGE_TAIL(copy_2_slots_unmasked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_slots_unmasked_fn<2>(packed, base);
}
STAGE_TAIL(copy_3_slots_unmasked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_slots_unmasked_fn<3>(packed, base);
}
STAGE_TAIL(copy_4_slots_unmasked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_slots_unmasked_fn<4>(packed, base);
}

template <int NumSlots>
SI void copy_n_immutable_unmasked_fn(SkRasterPipeline_BinaryOpCtx* packed, std::byte* base) {
    auto ctx = SkRPCtxUtils::Unpack(packed);

    // Load the scalar values.
    float* src = (float*)(base + ctx.src);
    float values[NumSlots];
    SK_UNROLL for (int index = 0; index < NumSlots; ++index) {
        values[index] = src[index];
    }
    // Broadcast the scalars into the destination.
    F* dst = (F*)(base + ctx.dst);
    SK_UNROLL for (int index = 0; index < NumSlots; ++index) {
        dst[index] = F_(values[index]);
    }
}

STAGE_TAIL(copy_immutable_unmasked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_immutable_unmasked_fn<1>(packed, base);
}
STAGE_TAIL(copy_2_immutables_unmasked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_immutable_unmasked_fn<2>(packed, base);
}
STAGE_TAIL(copy_3_immutables_unmasked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_immutable_unmasked_fn<3>(packed, base);
}
STAGE_TAIL(copy_4_immutables_unmasked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_immutable_unmasked_fn<4>(packed, base);
}

template <int NumSlots>
SI void copy_n_slots_masked_fn(SkRasterPipeline_BinaryOpCtx* packed, std::byte* base, I32 mask) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    I32* dst = (I32*)(base + ctx.dst);
    I32* src = (I32*)(base + ctx.src);
    SK_UNROLL for (int count = 0; count < NumSlots; ++count) {
        *dst = if_then_else(mask, *src, *dst);
        dst += 1;
        src += 1;
    }
}

STAGE_TAIL(copy_slot_masked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_slots_masked_fn<1>(packed, base, execution_mask());
}
STAGE_TAIL(copy_2_slots_masked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_slots_masked_fn<2>(packed, base, execution_mask());
}
STAGE_TAIL(copy_3_slots_masked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_slots_masked_fn<3>(packed, base, execution_mask());
}
STAGE_TAIL(copy_4_slots_masked, SkRasterPipeline_BinaryOpCtx* packed) {
    copy_n_slots_masked_fn<4>(packed, base, execution_mask());
}

template <int LoopCount, typename OffsetType>
SI void shuffle_fn(std::byte* ptr, OffsetType* offsets, int numSlots) {
    F scratch[16];
    SK_UNROLL for (int count = 0; count < LoopCount; ++count) {
        scratch[count] = *(F*)(ptr + offsets[count]);
    }
    // Surprisingly, this switch generates significantly better code than a memcpy (on x86-64) when
    // the number of slots is unknown at compile time, and generates roughly identical code when the
    // number of slots is hardcoded. Using a switch allows `scratch` to live in ymm0-ymm15 instead
    // of being written out to the stack and then read back in. Also, the intrinsic memcpy assumes
    // that `numSlots` could be arbitrarily large, and so it emits more code than we need.
    F* dst = (F*)ptr;
    switch (numSlots) {
        case 16: dst[15] = scratch[15]; [[fallthrough]];
        case 15: dst[14] = scratch[14]; [[fallthrough]];
        case 14: dst[13] = scratch[13]; [[fallthrough]];
        case 13: dst[12] = scratch[12]; [[fallthrough]];
        case 12: dst[11] = scratch[11]; [[fallthrough]];
        case 11: dst[10] = scratch[10]; [[fallthrough]];
        case 10: dst[ 9] = scratch[ 9]; [[fallthrough]];
        case  9: dst[ 8] = scratch[ 8]; [[fallthrough]];
        case  8: dst[ 7] = scratch[ 7]; [[fallthrough]];
        case  7: dst[ 6] = scratch[ 6]; [[fallthrough]];
        case  6: dst[ 5] = scratch[ 5]; [[fallthrough]];
        case  5: dst[ 4] = scratch[ 4]; [[fallthrough]];
        case  4: dst[ 3] = scratch[ 3]; [[fallthrough]];
        case  3: dst[ 2] = scratch[ 2]; [[fallthrough]];
        case  2: dst[ 1] = scratch[ 1]; [[fallthrough]];
        case  1: dst[ 0] = scratch[ 0];
    }
}

template <int N>
SI void small_swizzle_fn(SkRasterPipeline_SwizzleCtx* packed, std::byte* base) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    shuffle_fn<N>(base + ctx.dst, ctx.offsets, N);
}

STAGE_TAIL(swizzle_1, SkRasterPipeline_SwizzleCtx* packed) {
    small_swizzle_fn<1>(packed, base);
}
STAGE_TAIL(swizzle_2, SkRasterPipeline_SwizzleCtx* packed) {
    small_swizzle_fn<2>(packed, base);
}
STAGE_TAIL(swizzle_3, SkRasterPipeline_SwizzleCtx* packed) {
    small_swizzle_fn<3>(packed, base);
}
STAGE_TAIL(swizzle_4, SkRasterPipeline_SwizzleCtx* packed) {
    small_swizzle_fn<4>(packed, base);
}
STAGE_TAIL(shuffle, SkRasterPipeline_ShuffleCtx* ctx) {
    shuffle_fn<16>((std::byte*)ctx->ptr, ctx->offsets, ctx->count);
}

template <int NumSlots>
SI void swizzle_copy_masked_fn(I32* dst, const I32* src, uint16_t* offsets, I32 mask) {
    std::byte* dstB = (std::byte*)dst;
    SK_UNROLL for (int count = 0; count < NumSlots; ++count) {
        I32* dstS = (I32*)(dstB + *offsets);
        *dstS = if_then_else(mask, *src, *dstS);
        offsets += 1;
        src     += 1;
    }
}

STAGE_TAIL(swizzle_copy_slot_masked, SkRasterPipeline_SwizzleCopyCtx* ctx) {
    swizzle_copy_masked_fn<1>((I32*)ctx->dst, (const I32*)ctx->src, ctx->offsets, execution_mask());
}
STAGE_TAIL(swizzle_copy_2_slots_masked, SkRasterPipeline_SwizzleCopyCtx* ctx) {
    swizzle_copy_masked_fn<2>((I32*)ctx->dst, (const I32*)ctx->src, ctx->offsets, execution_mask());
}
STAGE_TAIL(swizzle_copy_3_slots_masked, SkRasterPipeline_SwizzleCopyCtx* ctx) {
    swizzle_copy_masked_fn<3>((I32*)ctx->dst, (const I32*)ctx->src, ctx->offsets, execution_mask());
}
STAGE_TAIL(swizzle_copy_4_slots_masked, SkRasterPipeline_SwizzleCopyCtx* ctx) {
    swizzle_copy_masked_fn<4>((I32*)ctx->dst, (const I32*)ctx->src, ctx->offsets, execution_mask());
}

STAGE_TAIL(copy_from_indirect_unmasked, SkRasterPipeline_CopyIndirectCtx* ctx) {
    // Clamp the indirect offsets to stay within the limit.
    U32 offsets = *(const U32*)ctx->indirectOffset;
    offsets = min(offsets, U32_(ctx->indirectLimit));

    // Scale up the offsets to account for the N lanes per value.
    offsets *= N;

    // Adjust the offsets forward so that they fetch from the correct lane.
    static constexpr uint32_t iota[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride_highp);
    offsets += sk_unaligned_load<U32>(iota);

    // Use gather to perform indirect lookups; write the results into `dst`.
    const int* src = ctx->src;
    I32*       dst = (I32*)ctx->dst;
    I32*       end = dst + ctx->slots;
    do {
        *dst = gather(src, offsets);
        dst += 1;
        src += N;
    } while (dst != end);
}

STAGE_TAIL(copy_from_indirect_uniform_unmasked, SkRasterPipeline_CopyIndirectCtx* ctx) {
    // Clamp the indirect offsets to stay within the limit.
    U32 offsets = *(const U32*)ctx->indirectOffset;
    offsets = min(offsets, U32_(ctx->indirectLimit));

    // Use gather to perform indirect lookups; write the results into `dst`.
    const int* src = ctx->src;
    I32*       dst = (I32*)ctx->dst;
    I32*       end = dst + ctx->slots;
    do {
        *dst = gather(src, offsets);
        dst += 1;
        src += 1;
    } while (dst != end);
}

STAGE_TAIL(copy_to_indirect_masked, SkRasterPipeline_CopyIndirectCtx* ctx) {
    // Clamp the indirect offsets to stay within the limit.
    U32 offsets = *(const U32*)ctx->indirectOffset;
    offsets = min(offsets, U32_(ctx->indirectLimit));

    // Scale up the offsets to account for the N lanes per value.
    offsets *= N;

    // Adjust the offsets forward so that they store into the correct lane.
    static constexpr uint32_t iota[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride_highp);
    offsets += sk_unaligned_load<U32>(iota);

    // Perform indirect, masked writes into `dst`.
    const I32* src = (const I32*)ctx->src;
    const I32* end = src + ctx->slots;
    int*       dst = ctx->dst;
    I32        mask = execution_mask();
    do {
        scatter_masked(*src, dst, offsets, mask);
        dst += N;
        src += 1;
    } while (src != end);
}

STAGE_TAIL(swizzle_copy_to_indirect_masked, SkRasterPipeline_SwizzleCopyIndirectCtx* ctx) {
    // Clamp the indirect offsets to stay within the limit.
    U32 offsets = *(const U32*)ctx->indirectOffset;
    offsets = min(offsets, U32_(ctx->indirectLimit));

    // Scale up the offsets to account for the N lanes per value.
    offsets *= N;

    // Adjust the offsets forward so that they store into the correct lane.
    static constexpr uint32_t iota[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride_highp);
    offsets += sk_unaligned_load<U32>(iota);

    // Perform indirect, masked, swizzled writes into `dst`.
    const I32*      src   = (const I32*)ctx->src;
    const I32*      end   = src + ctx->slots;
    std::byte*      dstB    = (std::byte*)ctx->dst;
    const uint16_t* swizzle = ctx->offsets;
    I32             mask    = execution_mask();
    do {
        int* dst = (int*)(dstB + *swizzle);
        scatter_masked(*src, dst, offsets, mask);
        swizzle += 1;
        src     += 1;
    } while (src != end);
}

// Unary operations take a single input, and overwrite it with their output.
// Unlike binary or ternary operations, we provide variations of 1-4 slots, but don't provide
// an arbitrary-width "n-slot" variation; the Builder can chain together longer sequences manually.
template <typename T, void (*ApplyFn)(T*)>
SI void apply_adjacent_unary(T* dst, T* end) {
    do {
        ApplyFn(dst);
        dst += 1;
    } while (dst != end);
}

#if defined(JUMPER_IS_SCALAR)
template <typename T>
SI void cast_to_float_from_fn(T* dst) {
    *dst = sk_bit_cast<T>((F)*dst);
}
SI void cast_to_int_from_fn(F* dst) {
    *dst = sk_bit_cast<F>((I32)*dst);
}
SI void cast_to_uint_from_fn(F* dst) {
    *dst = sk_bit_cast<F>((U32)*dst);
}
#else
template <typename T>
SI void cast_to_float_from_fn(T* dst) {
    *dst = sk_bit_cast<T>(__builtin_convertvector(*dst, F));
}
SI void cast_to_int_from_fn(F* dst) {
    *dst = sk_bit_cast<F>(__builtin_convertvector(*dst, I32));
}
SI void cast_to_uint_from_fn(F* dst) {
    *dst = sk_bit_cast<F>(__builtin_convertvector(*dst, U32));
}
#endif

SI void abs_fn(I32* dst) {
    *dst = abs_(*dst);
}

SI void floor_fn(F* dst) {
    *dst = floor_(*dst);
}

SI void ceil_fn(F* dst) {
    *dst = ceil_(*dst);
}

SI void invsqrt_fn(F* dst) {
    *dst = rsqrt(*dst);
}

#define DECLARE_UNARY_FLOAT(name)                                                              \
    STAGE_TAIL(name##_float, F* dst) { apply_adjacent_unary<F, &name##_fn>(dst, dst + 1); }    \
    STAGE_TAIL(name##_2_floats, F* dst) { apply_adjacent_unary<F, &name##_fn>(dst, dst + 2); } \
    STAGE_TAIL(name##_3_floats, F* dst) { apply_adjacent_unary<F, &name##_fn>(dst, dst + 3); } \
    STAGE_TAIL(name##_4_floats, F* dst) { apply_adjacent_unary<F, &name##_fn>(dst, dst + 4); }

#define DECLARE_UNARY_INT(name)                                                                  \
    STAGE_TAIL(name##_int, I32* dst) { apply_adjacent_unary<I32, &name##_fn>(dst, dst + 1); }    \
    STAGE_TAIL(name##_2_ints, I32* dst) { apply_adjacent_unary<I32, &name##_fn>(dst, dst + 2); } \
    STAGE_TAIL(name##_3_ints, I32* dst) { apply_adjacent_unary<I32, &name##_fn>(dst, dst + 3); } \
    STAGE_TAIL(name##_4_ints, I32* dst) { apply_adjacent_unary<I32, &name##_fn>(dst, dst + 4); }

#define DECLARE_UNARY_UINT(name)                                                                  \
    STAGE_TAIL(name##_uint, U32* dst) { apply_adjacent_unary<U32, &name##_fn>(dst, dst + 1); }    \
    STAGE_TAIL(name##_2_uints, U32* dst) { apply_adjacent_unary<U32, &name##_fn>(dst, dst + 2); } \
    STAGE_TAIL(name##_3_uints, U32* dst) { apply_adjacent_unary<U32, &name##_fn>(dst, dst + 3); } \
    STAGE_TAIL(name##_4_uints, U32* dst) { apply_adjacent_unary<U32, &name##_fn>(dst, dst + 4); }

DECLARE_UNARY_INT(cast_to_float_from) DECLARE_UNARY_UINT(cast_to_float_from)
DECLARE_UNARY_FLOAT(cast_to_int_from)
DECLARE_UNARY_FLOAT(cast_to_uint_from)
DECLARE_UNARY_FLOAT(floor)
DECLARE_UNARY_FLOAT(ceil)
DECLARE_UNARY_FLOAT(invsqrt)
DECLARE_UNARY_INT(abs)

#undef DECLARE_UNARY_FLOAT
#undef DECLARE_UNARY_INT
#undef DECLARE_UNARY_UINT

// For complex unary ops, we only provide a 1-slot version to reduce code bloat.
STAGE_TAIL(sin_float, F* dst)  { *dst = sin_(*dst); }
STAGE_TAIL(cos_float, F* dst)  { *dst = cos_(*dst); }
STAGE_TAIL(tan_float, F* dst)  { *dst = tan_(*dst); }
STAGE_TAIL(asin_float, F* dst) { *dst = asin_(*dst); }
STAGE_TAIL(acos_float, F* dst) { *dst = acos_(*dst); }
STAGE_TAIL(atan_float, F* dst) { *dst = atan_(*dst); }
STAGE_TAIL(sqrt_float, F* dst) { *dst = sqrt_(*dst); }
STAGE_TAIL(exp_float, F* dst)  { *dst = approx_exp(*dst); }
STAGE_TAIL(exp2_float, F* dst) { *dst = approx_pow2(*dst); }
STAGE_TAIL(log_float, F* dst)  { *dst = approx_log(*dst); }
STAGE_TAIL(log2_float, F* dst) { *dst = approx_log2(*dst); }

STAGE_TAIL(inverse_mat2, F* dst) {
    F a00 = dst[0], a01 = dst[1],
      a10 = dst[2], a11 = dst[3];
    F det = mad(a00, a11, -a01 * a10),
      invdet = rcp_precise(det);
    dst[0] =  invdet * a11;
    dst[1] = -invdet * a01;
    dst[2] = -invdet * a10;
    dst[3] =  invdet * a00;
}

STAGE_TAIL(inverse_mat3, F* dst) {
    F a00 = dst[0], a01 = dst[1], a02 = dst[2],
      a10 = dst[3], a11 = dst[4], a12 = dst[5],
      a20 = dst[6], a21 = dst[7], a22 = dst[8];
    F b01 = mad(a22, a11, -a12 * a21),
      b11 = mad(a12, a20, -a22 * a10),
      b21 = mad(a21, a10, -a11 * a20);
    F det = mad(a00, b01, mad(a01, b11, a02 * b21)),
      invdet = rcp_precise(det);
    dst[0] = invdet * b01;
    dst[1] = invdet * mad(a02, a21, -a22 * a01);
    dst[2] = invdet * mad(a12, a01, -a02 * a11);
    dst[3] = invdet * b11;
    dst[4] = invdet * mad(a22, a00, -a02 * a20);
    dst[5] = invdet * mad(a02, a10, -a12 * a00);
    dst[6] = invdet * b21;
    dst[7] = invdet * mad(a01, a20, -a21 * a00);
    dst[8] = invdet * mad(a11, a00, -a01 * a10);
}

STAGE_TAIL(inverse_mat4, F* dst) {
    F a00 = dst[0],  a01 = dst[1],  a02 = dst[2],  a03 = dst[3],
      a10 = dst[4],  a11 = dst[5],  a12 = dst[6],  a13 = dst[7],
      a20 = dst[8],  a21 = dst[9],  a22 = dst[10], a23 = dst[11],
      a30 = dst[12], a31 = dst[13], a32 = dst[14], a33 = dst[15];
    F b00 = mad(a00, a11, -a01 * a10),
      b01 = mad(a00, a12, -a02 * a10),
      b02 = mad(a00, a13, -a03 * a10),
      b03 = mad(a01, a12, -a02 * a11),
      b04 = mad(a01, a13, -a03 * a11),
      b05 = mad(a02, a13, -a03 * a12),
      b06 = mad(a20, a31, -a21 * a30),
      b07 = mad(a20, a32, -a22 * a30),
      b08 = mad(a20, a33, -a23 * a30),
      b09 = mad(a21, a32, -a22 * a31),
      b10 = mad(a21, a33, -a23 * a31),
      b11 = mad(a22, a33, -a23 * a32),
      det = mad(b00, b11, b05 * b06) + mad(b02, b09, b03 * b08) - mad(b01, b10, b04 * b07),
      invdet = rcp_precise(det);
    b00 *= invdet;
    b01 *= invdet;
    b02 *= invdet;
    b03 *= invdet;
    b04 *= invdet;
    b05 *= invdet;
    b06 *= invdet;
    b07 *= invdet;
    b08 *= invdet;
    b09 *= invdet;
    b10 *= invdet;
    b11 *= invdet;
    dst[0]  = mad(a11, b11, a13*b09) - a12*b10;
    dst[1]  = a02*b10 - mad(a01, b11, a03*b09);
    dst[2]  = mad(a31, b05, a33*b03) - a32*b04;
    dst[3]  = a22*b04 - mad(a21, b05, a23*b03);
    dst[4]  = a12*b08 - mad(a10, b11, a13*b07);
    dst[5]  = mad(a00, b11, a03*b07) - a02*b08;
    dst[6]  = a32*b02 - mad(a30, b05, a33*b01);
    dst[7]  = mad(a20, b05, a23*b01) - a22*b02;
    dst[8]  = mad(a10, b10, a13*b06) - a11*b08;
    dst[9]  = a01*b08 - mad(a00, b10, a03*b06);
    dst[10] = mad(a30, b04, a33*b00) - a31*b02;
    dst[11] = a21*b02 - mad(a20, b04, a23*b00);
    dst[12] = a11*b07 - mad(a10, b09, a12*b06);
    dst[13] = mad(a00, b09, a02*b06) - a01*b07;
    dst[14] = a31*b01 - mad(a30, b03, a32*b00);
    dst[15] = mad(a20, b03, a22*b00) - a21*b01;
}

// Binary operations take two adjacent inputs, and write their output in the first position.
template <typename T, void (*ApplyFn)(T*, T*)>
SI void apply_adjacent_binary(T* dst, T* src) {
    T* end = src;
    do {
        ApplyFn(dst, src);
        dst += 1;
        src += 1;
    } while (dst != end);
}

template <typename T, void (*ApplyFn)(T*, T*)>
SI void apply_adjacent_binary_packed(SkRasterPipeline_BinaryOpCtx* packed, std::byte* base) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    std::byte* dst = base + ctx.dst;
    std::byte* src = base + ctx.src;
    apply_adjacent_binary<T, ApplyFn>((T*)dst, (T*)src);
}

template <int N, typename V, typename S, void (*ApplyFn)(V*, V*)>
SI void apply_binary_immediate(SkRasterPipeline_ConstantCtx* packed, std::byte* base) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    V* dst = (V*)(base + ctx.dst);         // get a pointer to the destination
    S scalar = sk_bit_cast<S>(ctx.value);  // bit-pun the constant value as desired
    V src = scalar - V();                  // broadcast the constant value into a vector
    SK_UNROLL for (int index = 0; index < N; ++index) {
        ApplyFn(dst, &src);                // perform the operation
        dst += 1;
    }
}

template <typename T>
SI void add_fn(T* dst, T* src) {
    *dst += *src;
}

template <typename T>
SI void sub_fn(T* dst, T* src) {
    *dst -= *src;
}

template <typename T>
SI void mul_fn(T* dst, T* src) {
    *dst *= *src;
}

template <typename T>
SI void div_fn(T* dst, T* src) {
    T divisor = *src;
    if constexpr (!std::is_same_v<T, F>) {
        // We will crash if we integer-divide against zero. Convert 0 to ~0 to avoid this.
        divisor |= (T)cond_to_mask(divisor == 0);
    }
    *dst /= divisor;
}

SI void bitwise_and_fn(I32* dst, I32* src) {
    *dst &= *src;
}

SI void bitwise_or_fn(I32* dst, I32* src) {
    *dst |= *src;
}

SI void bitwise_xor_fn(I32* dst, I32* src) {
    *dst ^= *src;
}

template <typename T>
SI void max_fn(T* dst, T* src) {
    *dst = max(*dst, *src);
}

template <typename T>
SI void min_fn(T* dst, T* src) {
    *dst = min(*dst, *src);
}

template <typename T>
SI void cmplt_fn(T* dst, T* src) {
    static_assert(sizeof(T) == sizeof(I32));
    I32 result = cond_to_mask(*dst < *src);
    memcpy(dst, &result, sizeof(I32));
}

template <typename T>
SI void cmple_fn(T* dst, T* src) {
    static_assert(sizeof(T) == sizeof(I32));
    I32 result = cond_to_mask(*dst <= *src);
    memcpy(dst, &result, sizeof(I32));
}

template <typename T>
SI void cmpeq_fn(T* dst, T* src) {
    static_assert(sizeof(T) == sizeof(I32));
    I32 result = cond_to_mask(*dst == *src);
    memcpy(dst, &result, sizeof(I32));
}

template <typename T>
SI void cmpne_fn(T* dst, T* src) {
    static_assert(sizeof(T) == sizeof(I32));
    I32 result = cond_to_mask(*dst != *src);
    memcpy(dst, &result, sizeof(I32));
}

SI void atan2_fn(F* dst, F* src) {
    *dst = atan2_(*dst, *src);
}

SI void pow_fn(F* dst, F* src) {
    *dst = approx_powf(*dst, *src);
}

SI void mod_fn(F* dst, F* src) {
    *dst = *dst - *src * floor_(*dst / *src);
}

#define DECLARE_N_WAY_BINARY_FLOAT(name)                                \
    STAGE_TAIL(name##_n_floats, SkRasterPipeline_BinaryOpCtx* packed) { \
        apply_adjacent_binary_packed<F, &name##_fn>(packed, base);      \
    }

#define DECLARE_BINARY_FLOAT(name)                                                              \
    STAGE_TAIL(name##_float, F* dst) { apply_adjacent_binary<F, &name##_fn>(dst, dst + 1); }    \
    STAGE_TAIL(name##_2_floats, F* dst) { apply_adjacent_binary<F, &name##_fn>(dst, dst + 2); } \
    STAGE_TAIL(name##_3_floats, F* dst) { apply_adjacent_binary<F, &name##_fn>(dst, dst + 3); } \
    STAGE_TAIL(name##_4_floats, F* dst) { apply_adjacent_binary<F, &name##_fn>(dst, dst + 4); } \
    DECLARE_N_WAY_BINARY_FLOAT(name)

#define DECLARE_N_WAY_BINARY_INT(name)                                \
    STAGE_TAIL(name##_n_ints, SkRasterPipeline_BinaryOpCtx* packed) { \
        apply_adjacent_binary_packed<I32, &name##_fn>(packed, base);  \
    }

#define DECLARE_BINARY_INT(name)                                                                  \
    STAGE_TAIL(name##_int, I32* dst) { apply_adjacent_binary<I32, &name##_fn>(dst, dst + 1); }    \
    STAGE_TAIL(name##_2_ints, I32* dst) { apply_adjacent_binary<I32, &name##_fn>(dst, dst + 2); } \
    STAGE_TAIL(name##_3_ints, I32* dst) { apply_adjacent_binary<I32, &name##_fn>(dst, dst + 3); } \
    STAGE_TAIL(name##_4_ints, I32* dst) { apply_adjacent_binary<I32, &name##_fn>(dst, dst + 4); } \
    DECLARE_N_WAY_BINARY_INT(name)

#define DECLARE_N_WAY_BINARY_UINT(name)                                \
    STAGE_TAIL(name##_n_uints, SkRasterPipeline_BinaryOpCtx* packed) { \
        apply_adjacent_binary_packed<U32, &name##_fn>(packed, base);   \
    }

#define DECLARE_BINARY_UINT(name)                                                                  \
    STAGE_TAIL(name##_uint, U32* dst) { apply_adjacent_binary<U32, &name##_fn>(dst, dst + 1); }    \
    STAGE_TAIL(name##_2_uints, U32* dst) { apply_adjacent_binary<U32, &name##_fn>(dst, dst + 2); } \
    STAGE_TAIL(name##_3_uints, U32* dst) { apply_adjacent_binary<U32, &name##_fn>(dst, dst + 3); } \
    STAGE_TAIL(name##_4_uints, U32* dst) { apply_adjacent_binary<U32, &name##_fn>(dst, dst + 4); } \
    DECLARE_N_WAY_BINARY_UINT(name)

// Many ops reuse the int stages when performing uint arithmetic, since they're equivalent on a
// two's-complement machine. (Even multiplication is equivalent in the lower 32 bits.)
DECLARE_BINARY_FLOAT(add)    DECLARE_BINARY_INT(add)
DECLARE_BINARY_FLOAT(sub)    DECLARE_BINARY_INT(sub)
DECLARE_BINARY_FLOAT(mul)    DECLARE_BINARY_INT(mul)
DECLARE_BINARY_FLOAT(div)    DECLARE_BINARY_INT(div)    DECLARE_BINARY_UINT(div)
                             DECLARE_BINARY_INT(bitwise_and)
                             DECLARE_BINARY_INT(bitwise_or)
                             DECLARE_BINARY_INT(bitwise_xor)
DECLARE_BINARY_FLOAT(mod)
DECLARE_BINARY_FLOAT(min)    DECLARE_BINARY_INT(min)    DECLARE_BINARY_UINT(min)
DECLARE_BINARY_FLOAT(max)    DECLARE_BINARY_INT(max)    DECLARE_BINARY_UINT(max)
DECLARE_BINARY_FLOAT(cmplt)  DECLARE_BINARY_INT(cmplt)  DECLARE_BINARY_UINT(cmplt)
DECLARE_BINARY_FLOAT(cmple)  DECLARE_BINARY_INT(cmple)  DECLARE_BINARY_UINT(cmple)
DECLARE_BINARY_FLOAT(cmpeq)  DECLARE_BINARY_INT(cmpeq)
DECLARE_BINARY_FLOAT(cmpne)  DECLARE_BINARY_INT(cmpne)

// Sufficiently complex ops only provide an N-way version, to avoid code bloat from the dedicated
// 1-4 slot versions.
DECLARE_N_WAY_BINARY_FLOAT(atan2)
DECLARE_N_WAY_BINARY_FLOAT(pow)

// Some ops have an optimized version when the right-side is an immediate value.
#define DECLARE_IMM_BINARY_FLOAT(name)                                   \
    STAGE_TAIL(name##_imm_float, SkRasterPipeline_ConstantCtx* packed) { \
        apply_binary_immediate<1, F, float, &name##_fn>(packed, base);   \
    }
#define DECLARE_IMM_BINARY_INT(name)                                       \
    STAGE_TAIL(name##_imm_int, SkRasterPipeline_ConstantCtx* packed) {     \
        apply_binary_immediate<1, I32, int32_t, &name##_fn>(packed, base); \
    }
#define DECLARE_MULTI_IMM_BINARY_INT(name)                                 \
    STAGE_TAIL(name##_imm_int, SkRasterPipeline_ConstantCtx* packed) {     \
        apply_binary_immediate<1, I32, int32_t, &name##_fn>(packed, base); \
    }                                                                      \
    STAGE_TAIL(name##_imm_2_ints, SkRasterPipeline_ConstantCtx* packed) {  \
        apply_binary_immediate<2, I32, int32_t, &name##_fn>(packed, base); \
    }                                                                      \
    STAGE_TAIL(name##_imm_3_ints, SkRasterPipeline_ConstantCtx* packed) {  \
        apply_binary_immediate<3, I32, int32_t, &name##_fn>(packed, base); \
    }                                                                      \
    STAGE_TAIL(name##_imm_4_ints, SkRasterPipeline_ConstantCtx* packed) {  \
        apply_binary_immediate<4, I32, int32_t, &name##_fn>(packed, base); \
    }
#define DECLARE_IMM_BINARY_UINT(name)                                       \
    STAGE_TAIL(name##_imm_uint, SkRasterPipeline_ConstantCtx* packed) {     \
        apply_binary_immediate<1, U32, uint32_t, &name##_fn>(packed, base); \
    }

DECLARE_IMM_BINARY_FLOAT(add)   DECLARE_IMM_BINARY_INT(add)
DECLARE_IMM_BINARY_FLOAT(mul)   DECLARE_IMM_BINARY_INT(mul)
                                DECLARE_MULTI_IMM_BINARY_INT(bitwise_and)
                                DECLARE_IMM_BINARY_FLOAT(max)
                                DECLARE_IMM_BINARY_FLOAT(min)
                                DECLARE_IMM_BINARY_INT(bitwise_xor)
DECLARE_IMM_BINARY_FLOAT(cmplt) DECLARE_IMM_BINARY_INT(cmplt) DECLARE_IMM_BINARY_UINT(cmplt)
DECLARE_IMM_BINARY_FLOAT(cmple) DECLARE_IMM_BINARY_INT(cmple) DECLARE_IMM_BINARY_UINT(cmple)
DECLARE_IMM_BINARY_FLOAT(cmpeq) DECLARE_IMM_BINARY_INT(cmpeq)
DECLARE_IMM_BINARY_FLOAT(cmpne) DECLARE_IMM_BINARY_INT(cmpne)

#undef DECLARE_MULTI_IMM_BINARY_INT
#undef DECLARE_IMM_BINARY_FLOAT
#undef DECLARE_IMM_BINARY_INT
#undef DECLARE_IMM_BINARY_UINT
#undef DECLARE_BINARY_FLOAT
#undef DECLARE_BINARY_INT
#undef DECLARE_BINARY_UINT
#undef DECLARE_N_WAY_BINARY_FLOAT
#undef DECLARE_N_WAY_BINARY_INT
#undef DECLARE_N_WAY_BINARY_UINT

// Dots can be represented with multiply and add ops, but they are so foundational that it's worth
// having dedicated ops.
STAGE_TAIL(dot_2_floats, F* dst) {
    dst[0] = mad(dst[0],  dst[2],
                 dst[1] * dst[3]);
}

STAGE_TAIL(dot_3_floats, F* dst) {
    dst[0] = mad(dst[0],  dst[3],
             mad(dst[1],  dst[4],
                 dst[2] * dst[5]));
}

STAGE_TAIL(dot_4_floats, F* dst) {
    dst[0] = mad(dst[0],  dst[4],
             mad(dst[1],  dst[5],
             mad(dst[2],  dst[6],
                 dst[3] * dst[7])));
}

// MxM, VxM and MxV multiplication all use matrix_multiply. Vectors are treated like a matrix with a
// single column or row.
template <int N>
SI void matrix_multiply(SkRasterPipeline_MatrixMultiplyCtx* packed, std::byte* base) {
    auto ctx = SkRPCtxUtils::Unpack(packed);

    int outColumns   = ctx.rightColumns,
        outRows      = ctx.leftRows;

    SkASSERT(outColumns >= 1);
    SkASSERT(outRows    >= 1);
    SkASSERT(outColumns <= 4);
    SkASSERT(outRows    <= 4);

    SkASSERT(ctx.leftColumns == ctx.rightRows);
    SkASSERT(N == ctx.leftColumns);  // N should match the result width

#if !defined(JUMPER_IS_SCALAR)
    // This prevents Clang from generating early-out checks for zero-sized matrices.
    SK_ASSUME(outColumns >= 1);
    SK_ASSUME(outRows    >= 1);
    SK_ASSUME(outColumns <= 4);
    SK_ASSUME(outRows    <= 4);
#endif

    // Get pointers to the adjacent left- and right-matrices.
    F* resultMtx  = (F*)(base + ctx.dst);
    F* leftMtx    = &resultMtx[ctx.rightColumns * ctx.leftRows];
    F* rightMtx   = &leftMtx[N * ctx.leftRows];

    // Emit each matrix element.
    for (int c = 0; c < outColumns; ++c) {
        for (int r = 0; r < outRows; ++r) {
            // Dot a vector from leftMtx[*][r] with rightMtx[c][*].
            F* leftRow     = &leftMtx [r];
            F* rightColumn = &rightMtx[c * N];

            F element = *leftRow * *rightColumn;
            for (int idx = 1; idx < N; ++idx) {
                leftRow     += outRows;
                rightColumn += 1;
                element = mad(*leftRow, *rightColumn, element);
            }

            *resultMtx++ = element;
        }
    }
}

STAGE_TAIL(matrix_multiply_2, SkRasterPipeline_MatrixMultiplyCtx* packed) {
    matrix_multiply<2>(packed, base);
}

STAGE_TAIL(matrix_multiply_3, SkRasterPipeline_MatrixMultiplyCtx* packed) {
    matrix_multiply<3>(packed, base);
}

STAGE_TAIL(matrix_multiply_4, SkRasterPipeline_MatrixMultiplyCtx* packed) {
    matrix_multiply<4>(packed, base);
}

// Refract always operates on 4-wide incident and normal vectors; for narrower inputs, the code
// generator fills in the input columns with zero, and discards the extra output columns.
STAGE_TAIL(refract_4_floats, F* dst) {
    // Algorithm adapted from https://registry.khronos.org/OpenGL-Refpages/gl4/html/refract.xhtml
    F *incident = dst + 0;
    F *normal = dst + 4;
    F eta = dst[8];

    F dotNI = mad(normal[0],  incident[0],
              mad(normal[1],  incident[1],
              mad(normal[2],  incident[2],
                  normal[3] * incident[3])));

    F k = 1.0 - eta * eta * (1.0 - dotNI * dotNI);
    F sqrt_k = sqrt_(k);

    for (int idx = 0; idx < 4; ++idx) {
        dst[idx] = if_then_else(k >= 0,
                                eta * incident[idx] - (eta * dotNI + sqrt_k) * normal[idx],
                                0.0);
    }
}

// Ternary operations work like binary ops (see immediately above) but take two source inputs.
template <typename T, void (*ApplyFn)(T*, T*, T*)>
SI void apply_adjacent_ternary(T* dst, T* src0, T* src1) {
    int count = src0 - dst;
#if !defined(JUMPER_IS_SCALAR)
    SK_ASSUME(count >= 1);
#endif

    for (int index = 0; index < count; ++index) {
        ApplyFn(dst, src0, src1);
        dst += 1;
        src0 += 1;
        src1 += 1;
    }
}

template <typename T, void (*ApplyFn)(T*, T*, T*)>
SI void apply_adjacent_ternary_packed(SkRasterPipeline_TernaryOpCtx* packed, std::byte* base) {
    auto ctx = SkRPCtxUtils::Unpack(packed);
    std::byte* dst  = base + ctx.dst;
    std::byte* src0 = dst  + ctx.delta;
    std::byte* src1 = src0 + ctx.delta;
    apply_adjacent_ternary<T, ApplyFn>((T*)dst, (T*)src0, (T*)src1);
}

SI void mix_fn(F* a, F* x, F* y) {
    // We reorder the arguments here to match lerp's GLSL-style order (interpolation point last).
    *a = lerp(*x, *y, *a);
}

SI void mix_fn(I32* a, I32* x, I32* y) {
    // We reorder the arguments here to match if_then_else's expected order (y before x).
    *a = if_then_else(*a, *y, *x);
}

SI void smoothstep_fn(F* edge0, F* edge1, F* x) {
    F t = clamp_01_((*x - *edge0) / (*edge1 - *edge0));
    *edge0 = t * t * (3.0 - 2.0 * t);
}

#define DECLARE_N_WAY_TERNARY_FLOAT(name)                                \
    STAGE_TAIL(name##_n_floats, SkRasterPipeline_TernaryOpCtx* packed) { \
        apply_adjacent_ternary_packed<F, &name##_fn>(packed, base);      \
    }

#define DECLARE_TERNARY_FLOAT(name)                                                           \
    STAGE_TAIL(name##_float, F* p) { apply_adjacent_ternary<F, &name##_fn>(p, p+1, p+2); }    \
    STAGE_TAIL(name##_2_floats, F* p) { apply_adjacent_ternary<F, &name##_fn>(p, p+2, p+4); } \
    STAGE_TAIL(name##_3_floats, F* p) { apply_adjacent_ternary<F, &name##_fn>(p, p+3, p+6); } \
    STAGE_TAIL(name##_4_floats, F* p) { apply_adjacent_ternary<F, &name##_fn>(p, p+4, p+8); } \
    DECLARE_N_WAY_TERNARY_FLOAT(name)

#define DECLARE_TERNARY_INT(name)                                                               \
    STAGE_TAIL(name##_int, I32* p) { apply_adjacent_ternary<I32, &name##_fn>(p, p+1, p+2); }    \
    STAGE_TAIL(name##_2_ints, I32* p) { apply_adjacent_ternary<I32, &name##_fn>(p, p+2, p+4); } \
    STAGE_TAIL(name##_3_ints, I32* p) { apply_adjacent_ternary<I32, &name##_fn>(p, p+3, p+6); } \
    STAGE_TAIL(name##_4_ints, I32* p) { apply_adjacent_ternary<I32, &name##_fn>(p, p+4, p+8); } \
    STAGE_TAIL(name##_n_ints, SkRasterPipeline_TernaryOpCtx* packed) {                          \
        apply_adjacent_ternary_packed<I32, &name##_fn>(packed, base);                           \
    }

DECLARE_N_WAY_TERNARY_FLOAT(smoothstep)
DECLARE_TERNARY_FLOAT(mix)
DECLARE_TERNARY_INT(mix)

#undef DECLARE_N_WAY_TERNARY_FLOAT
#undef DECLARE_TERNARY_FLOAT
#undef DECLARE_TERNARY_INT

STAGE(gauss_a_to_rgba, NoCtx) {
    // x = 1 - x;
    // exp(-x * x * 4) - 0.018f;
    // ... now approximate with quartic
    //
    const float c4 = -2.26661229133605957031f;
    const float c3 = 2.89795351028442382812f;
    const float c2 = 0.21345567703247070312f;
    const float c1 = 0.15489584207534790039f;
    const float c0 = 0.00030726194381713867f;
    a = mad(a, mad(a, mad(a, mad(a, c4, c3), c2), c1), c0);
    r = a;
    g = a;
    b = a;
}

// A specialized fused image shader for clamp-x, clamp-y, non-sRGB sampling.
STAGE(bilerp_clamp_8888, const SkRasterPipeline_GatherCtx* ctx) {
    // (cx,cy) are the center of our sample.
    F cx = r,
      cy = g;

    // All sample points are at the same fractional offset (fx,fy).
    // They're the 4 corners of a logical 1x1 pixel surrounding (x,y) at (0.5,0.5) offsets.
    F fx = fract(cx + 0.5f),
      fy = fract(cy + 0.5f);

    // We'll accumulate the color of all four samples into {r,g,b,a} directly.
    r = g = b = a = F0;

    for (float py = -0.5f; py <= +0.5f; py += 1.0f)
    for (float px = -0.5f; px <= +0.5f; px += 1.0f) {
        // (x,y) are the coordinates of this sample point.
        F x = cx + px,
          y = cy + py;

        // ix_and_ptr() will clamp to the image's bounds for us.
        const uint32_t* ptr;
        U32 ix = ix_and_ptr(&ptr, ctx, x,y);

        F sr,sg,sb,sa;
        from_8888(gather(ptr, ix), &sr,&sg,&sb,&sa);

        // In bilinear interpolation, the 4 pixels at +/- 0.5 offsets from the sample pixel center
        // are combined in direct proportion to their area overlapping that logical query pixel.
        // At positive offsets, the x-axis contribution to that rectangle is fx,
        // or (1-fx) at negative x.  Same deal for y.
        F sx = (px > 0) ? fx : 1.0f - fx,
          sy = (py > 0) ? fy : 1.0f - fy,
          area = sx * sy;

        r += sr * area;
        g += sg * area;
        b += sb * area;
        a += sa * area;
    }
}

// A specialized fused image shader for clamp-x, clamp-y, non-sRGB sampling.
STAGE(bicubic_clamp_8888, const SkRasterPipeline_GatherCtx* ctx) {
    // (cx,cy) are the center of our sample.
    F cx = r,
      cy = g;

    // All sample points are at the same fractional offset (fx,fy).
    // They're the 4 corners of a logical 1x1 pixel surrounding (x,y) at (0.5,0.5) offsets.
    F fx = fract(cx + 0.5f),
      fy = fract(cy + 0.5f);

    // We'll accumulate the color of all four samples into {r,g,b,a} directly.
    r = g = b = a = F0;

    const float* w = ctx->weights;
    const F scaley[4] = {bicubic_wts(fy, w[0], w[4], w[ 8], w[12]),
                         bicubic_wts(fy, w[1], w[5], w[ 9], w[13]),
                         bicubic_wts(fy, w[2], w[6], w[10], w[14]),
                         bicubic_wts(fy, w[3], w[7], w[11], w[15])};
    const F scalex[4] = {bicubic_wts(fx, w[0], w[4], w[ 8], w[12]),
                         bicubic_wts(fx, w[1], w[5], w[ 9], w[13]),
                         bicubic_wts(fx, w[2], w[6], w[10], w[14]),
                         bicubic_wts(fx, w[3], w[7], w[11], w[15])};

    F sample_y = cy - 1.5f;
    for (int yy = 0; yy <= 3; ++yy) {
        F sample_x = cx - 1.5f;
        for (int xx = 0; xx <= 3; ++xx) {
            F scale = scalex[xx] * scaley[yy];

            // ix_and_ptr() will clamp to the image's bounds for us.
            const uint32_t* ptr;
            U32 ix = ix_and_ptr(&ptr, ctx, sample_x, sample_y);

            F sr,sg,sb,sa;
            from_8888(gather(ptr, ix), &sr,&sg,&sb,&sa);

            r = mad(scale, sr, r);
            g = mad(scale, sg, g);
            b = mad(scale, sb, b);
            a = mad(scale, sa, a);

            sample_x += 1;
        }
        sample_y += 1;
    }
}

// ~~~~~~ skgpu::Swizzle stage ~~~~~~ //

STAGE(swizzle, void* ctx) {
    auto ir = r, ig = g, ib = b, ia = a;
    F* o[] = {&r, &g, &b, &a};
    char swiz[4];
    memcpy(swiz, &ctx, sizeof(swiz));

    for (int i = 0; i < 4; ++i) {
        switch (swiz[i]) {
            case 'r': *o[i] = ir; break;
            case 'g': *o[i] = ig; break;
            case 'b': *o[i] = ib; break;
            case 'a': *o[i] = ia; break;
            case '0': *o[i] = F0; break;
            case '1': *o[i] = F1; break;
            default:              break;
        }
    }
}

namespace lowp {
#if defined(JUMPER_IS_SCALAR) || defined(SK_DISABLE_LOWP_RASTER_PIPELINE)
    // If we're not compiled by Clang, or otherwise switched into scalar mode (old Clang, manually),
    // we don't generate lowp stages.  All these nullptrs will tell SkJumper.cpp to always use the
    // highp float pipeline.
    #define M(st) static void (*st)(void) = nullptr;
        SK_RASTER_PIPELINE_OPS_LOWP(M)
    #undef M
    static void (*just_return)(void) = nullptr;

    static void start_pipeline(size_t,size_t,size_t,size_t, SkRasterPipelineStage*,
                               SkSpan<SkRasterPipeline_MemoryCtxPatch>,
                               uint8_t* tailPointer) {}

#else  // We are compiling vector code with Clang... let's make some lowp stages!

#if defined(JUMPER_IS_SKX) || defined(JUMPER_IS_HSW)
    template <typename T> using V = Vec<16, T>;
#else
    template <typename T> using V = Vec<8, T>;
#endif

using U8  = V<uint8_t >;
using U16 = V<uint16_t>;
using I16 = V< int16_t>;
using I32 = V< int32_t>;
using U32 = V<uint32_t>;
using I64 = V< int64_t>;
using U64 = V<uint64_t>;
using F   = V<float   >;

static constexpr size_t N = sizeof(U16) / sizeof(uint16_t);

// Promotion helpers (for GCC)
#if defined(__clang__)
SI constexpr U16 U16_(uint16_t x) { return x; }
SI constexpr I32 I32_( int32_t x) { return x; }
SI constexpr U32 U32_(uint32_t x) { return x; }
SI constexpr F   F_  (float    x) { return x; }
#else
SI constexpr U16 U16_(uint16_t x) { return x + U16(); }
SI constexpr I32 I32_( int32_t x) { return x + I32(); }
SI constexpr U32 U32_(uint32_t x) { return x + U32(); }
SI constexpr F   F_  (float    x) { return x - F  (); }
#endif

static constexpr U16 U16_0   = U16_(0),
                     U16_255 = U16_(255);

// Once again, some platforms benefit from a restricted Stage calling convention,
// but others can pass tons and tons of registers and we're happy to exploit that.
// It's exactly the same decision and implementation strategy as the F stages above.
#if JUMPER_NARROW_STAGES
    struct Params {
        size_t dx, dy;
        U16 dr,dg,db,da;
    };
    using Stage = void (ABI*)(Params*, SkRasterPipelineStage* program, U16 r, U16 g, U16 b, U16 a);
#else
    using Stage = void (ABI*)(SkRasterPipelineStage* program,
                              size_t dx, size_t dy,
                              U16  r, U16  g, U16  b, U16  a,
                              U16 dr, U16 dg, U16 db, U16 da);
#endif

static void start_pipeline(size_t x0,     size_t y0,
                           size_t xlimit, size_t ylimit,
                           SkRasterPipelineStage* program,
                           SkSpan<SkRasterPipeline_MemoryCtxPatch> memoryCtxPatches,
                           uint8_t* tailPointer) {
    uint8_t unreferencedTail;
    if (!tailPointer) {
        tailPointer = &unreferencedTail;
    }
    auto start = (Stage)program->fn;
    for (size_t dy = y0; dy < ylimit; dy++) {
    #if JUMPER_NARROW_STAGES
        Params params = { x0,dy, U16_0,U16_0,U16_0,U16_0 };
        for (; params.dx + N <= xlimit; params.dx += N) {
            start(&params, program, U16_0,U16_0,U16_0,U16_0);
        }
        if (size_t tail = xlimit - params.dx) {
            *tailPointer = tail;
            patch_memory_contexts(memoryCtxPatches, params.dx, dy, tail);
            start(&params, program, U16_0,U16_0,U16_0,U16_0);
            restore_memory_contexts(memoryCtxPatches, params.dx, dy, tail);
            *tailPointer = 0xFF;
        }
    #else
        size_t dx = x0;
        for (; dx + N <= xlimit; dx += N) {
            start(program, dx,dy, U16_0,U16_0,U16_0,U16_0, U16_0,U16_0,U16_0,U16_0);
        }
        if (size_t tail = xlimit - dx) {
            *tailPointer = tail;
            patch_memory_contexts(memoryCtxPatches, dx, dy, tail);
            start(program, dx,dy, U16_0,U16_0,U16_0,U16_0, U16_0,U16_0,U16_0,U16_0);
            restore_memory_contexts(memoryCtxPatches, dx, dy, tail);
            *tailPointer = 0xFF;
        }
    #endif
    }
}

#if JUMPER_NARROW_STAGES
    static void ABI just_return(Params*, SkRasterPipelineStage*, U16,U16,U16,U16) {}
#else
    static void ABI just_return(SkRasterPipelineStage*, size_t,size_t,
                                U16,U16,U16,U16, U16,U16,U16,U16) {}
#endif

// All stages use the same function call ABI to chain into each other, but there are three types:
//   GG: geometry in, geometry out  -- think, a matrix
//   GP: geometry in, pixels out.   -- think, a memory gather
//   PP: pixels in, pixels out.     -- think, a blend mode
//
// (Some stages ignore their inputs or produce no logical output.  That's perfectly fine.)
//
// These three STAGE_ macros let you define each type of stage,
// and will have (x,y) geometry and/or (r,g,b,a, dr,dg,db,da) pixel arguments as appropriate.

#if JUMPER_NARROW_STAGES
    #define STAGE_GG(name, ARG)                                                                \
        SI void name##_k(ARG, size_t dx, size_t dy, F& x, F& y);                               \
        static void ABI name(Params* params, SkRasterPipelineStage* program,                   \
                             U16 r, U16 g, U16 b, U16 a) {                                     \
            auto x = join<F>(r,g),                                                             \
                 y = join<F>(b,a);                                                             \
            name##_k(Ctx{program}, params->dx,params->dy, x,y);                                \
            split(x, &r,&g);                                                                   \
            split(y, &b,&a);                                                                   \
            auto fn = (Stage)(++program)->fn;                                                  \
            fn(params, program, r,g,b,a);                                                      \
        }                                                                                      \
        SI void name##_k(ARG, size_t dx, size_t dy, F& x, F& y)

    #define STAGE_GP(name, ARG)                                                            \
        SI void name##_k(ARG, size_t dx, size_t dy, F x, F y,                              \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da);                              \
        static void ABI name(Params* params, SkRasterPipelineStage* program,               \
                             U16 r, U16 g, U16 b, U16 a) {                                 \
            auto x = join<F>(r,g),                                                         \
                 y = join<F>(b,a);                                                         \
            name##_k(Ctx{program}, params->dx,params->dy, x,y, r,g,b,a,                    \
                     params->dr,params->dg,params->db,params->da);                         \
            auto fn = (Stage)(++program)->fn;                                              \
            fn(params, program, r,g,b,a);                                                  \
        }                                                                                  \
        SI void name##_k(ARG, size_t dx, size_t dy, F x, F y,                              \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da)

    #define STAGE_PP(name, ARG)                                                            \
        SI void name##_k(ARG, size_t dx, size_t dy,                                        \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da);                              \
        static void ABI name(Params* params, SkRasterPipelineStage* program,               \
                             U16 r, U16 g, U16 b, U16 a) {                                 \
            name##_k(Ctx{program}, params->dx,params->dy, r,g,b,a,                         \
                     params->dr,params->dg,params->db,params->da);                         \
            auto fn = (Stage)(++program)->fn;                                              \
            fn(params, program, r,g,b,a);                                                  \
        }                                                                                  \
        SI void name##_k(ARG, size_t dx, size_t dy,                                        \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da)
#else
    #define STAGE_GG(name, ARG)                                                            \
        SI void name##_k(ARG, size_t dx, size_t dy, F& x, F& y);                           \
        static void ABI name(SkRasterPipelineStage* program,                               \
                             size_t dx, size_t dy,                                         \
                             U16  r, U16  g, U16  b, U16  a,                               \
                             U16 dr, U16 dg, U16 db, U16 da) {                             \
            auto x = join<F>(r,g),                                                         \
                 y = join<F>(b,a);                                                         \
            name##_k(Ctx{program}, dx,dy, x,y);                                            \
            split(x, &r,&g);                                                               \
            split(y, &b,&a);                                                               \
            auto fn = (Stage)(++program)->fn;                                              \
            fn(program, dx,dy, r,g,b,a, dr,dg,db,da);                                      \
        }                                                                                  \
        SI void name##_k(ARG, size_t dx, size_t dy, F& x, F& y)

    #define STAGE_GP(name, ARG)                                                            \
        SI void name##_k(ARG, size_t dx, size_t dy, F x, F y,                              \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da);                              \
        static void ABI name(SkRasterPipelineStage* program,                               \
                             size_t dx, size_t dy,                                         \
                             U16  r, U16  g, U16  b, U16  a,                               \
                             U16 dr, U16 dg, U16 db, U16 da) {                             \
            auto x = join<F>(r,g),                                                         \
                 y = join<F>(b,a);                                                         \
            name##_k(Ctx{program}, dx,dy, x,y, r,g,b,a, dr,dg,db,da);                      \
            auto fn = (Stage)(++program)->fn;                                              \
            fn(program, dx,dy, r,g,b,a, dr,dg,db,da);                                      \
        }                                                                                  \
        SI void name##_k(ARG, size_t dx, size_t dy, F x, F y,                              \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da)

    #define STAGE_PP(name, ARG)                                                            \
        SI void name##_k(ARG, size_t dx, size_t dy,                                        \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da);                              \
        static void ABI name(SkRasterPipelineStage* program,                               \
                             size_t dx, size_t dy,                                         \
                             U16  r, U16  g, U16  b, U16  a,                               \
                             U16 dr, U16 dg, U16 db, U16 da) {                             \
            name##_k(Ctx{program}, dx,dy, r,g,b,a, dr,dg,db,da);                           \
            auto fn = (Stage)(++program)->fn;                                              \
            fn(program, dx,dy, r,g,b,a, dr,dg,db,da);                                      \
        }                                                                                  \
        SI void name##_k(ARG, size_t dx, size_t dy,                                        \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da)
#endif

// ~~~~~~ Commonly used helper functions ~~~~~~ //

/**
 * Helpers to to properly rounded division (by 255). The ideal answer we want to compute is slow,
 * thanks to a division by a non-power of two:
 *   [1]  (v + 127) / 255
 *
 * There is a two-step process that computes the correct answer for all inputs:
 *   [2]  (v + 128 + ((v + 128) >> 8)) >> 8
 *
 * There is also a single iteration approximation, but it's wrong (+-1) ~25% of the time:
 *   [3]  (v + 255) >> 8;
 *
 * We offer two different implementations here, depending on the requirements of the calling stage.
 */

/**
 * div255 favors speed over accuracy. It uses formula [2] on NEON (where we can compute it as fast
 * as [3]), and uses [3] elsewhere.
 */
SI U16 div255(U16 v) {
#if defined(JUMPER_IS_NEON)
    // With NEON we can compute [2] just as fast as [3], so let's be correct.
    // First we compute v + ((v+128)>>8), then one more round of (...+128)>>8 to finish up:
    return vrshrq_n_u16(vrsraq_n_u16(v, v, 8), 8);
#else
    // Otherwise, use [3], which is never wrong by more than 1:
    return (v+255)/256;
#endif
}

/**
 * div255_accurate guarantees the right answer on all platforms, at the expense of performance.
 */
SI U16 div255_accurate(U16 v) {
#if defined(JUMPER_IS_NEON)
    // Our NEON implementation of div255 is already correct for all inputs:
    return div255(v);
#else
    // This is [2] (the same formulation as NEON), but written without the benefit of intrinsics:
    v += 128;
    return (v+(v/256))/256;
#endif
}

SI U16 inv(U16 v) { return 255-v; }

SI U16 if_then_else(I16 c, U16 t, U16 e) {
    return (t & sk_bit_cast<U16>(c)) | (e & sk_bit_cast<U16>(~c));
}
SI U32 if_then_else(I32 c, U32 t, U32 e) {
    return (t & sk_bit_cast<U32>(c)) | (e & sk_bit_cast<U32>(~c));
}

SI U16 max(U16 x, U16 y) { return if_then_else(x < y, y, x); }
SI U16 min(U16 x, U16 y) { return if_then_else(x < y, x, y); }

SI U16 max(U16      a, uint16_t b) { return max(     a , U16_(b)); }
SI U16 max(uint16_t a, U16      b) { return max(U16_(a),      b ); }
SI U16 min(U16      a, uint16_t b) { return min(     a , U16_(b)); }
SI U16 min(uint16_t a, U16      b) { return min(U16_(a),      b ); }

SI U16 from_float(float f) { return U16_(f * 255.0f + 0.5f); }

SI U16 lerp(U16 from, U16 to, U16 t) { return div255( from*inv(t) + to*t ); }

template <typename D, typename S>
SI D cast(S src) {
    return __builtin_convertvector(src, D);
}

template <typename D, typename S>
SI void split(S v, D* lo, D* hi) {
    static_assert(2*sizeof(D) == sizeof(S), "");
    memcpy(lo, (const char*)&v + 0*sizeof(D), sizeof(D));
    memcpy(hi, (const char*)&v + 1*sizeof(D), sizeof(D));
}
template <typename D, typename S>
SI D join(S lo, S hi) {
    static_assert(sizeof(D) == 2*sizeof(S), "");
    D v;
    memcpy((char*)&v + 0*sizeof(S), &lo, sizeof(S));
    memcpy((char*)&v + 1*sizeof(S), &hi, sizeof(S));
    return v;
}

SI F if_then_else(I32 c, F t, F e) {
    return sk_bit_cast<F>( (sk_bit_cast<I32>(t) & c) | (sk_bit_cast<I32>(e) & ~c) );
}
SI F if_then_else(I32 c, F     t, float e) { return if_then_else(c,    t , F_(e)); }
SI F if_then_else(I32 c, float t, F     e) { return if_then_else(c, F_(t),    e ); }

SI F max(F x, F y) { return if_then_else(x < y, y, x); }
SI F min(F x, F y) { return if_then_else(x < y, x, y); }

SI F max(F     a, float b) { return max(   a , F_(b)); }
SI F max(float a, F     b) { return max(F_(a),    b ); }
SI F min(F     a, float b) { return min(   a , F_(b)); }
SI F min(float a, F     b) { return min(F_(a),    b ); }

SI I32 if_then_else(I32 c, I32 t, I32 e) {
    return (t & c) | (e & ~c);
}
SI I32 max(I32 x, I32 y) { return if_then_else(x < y, y, x); }
SI I32 min(I32 x, I32 y) { return if_then_else(x < y, x, y); }

SI I32 max(I32     a, int32_t b) { return max(     a , I32_(b)); }
SI I32 max(int32_t a, I32     b) { return max(I32_(a),      b ); }
SI I32 min(I32     a, int32_t b) { return min(     a , I32_(b)); }
SI I32 min(int32_t a, I32     b) { return min(I32_(a),      b ); }

SI F mad(F     f, F     m, F     a) { return f*m+a; }
SI F mad(F     f, F     m, float a) { return mad(   f ,    m , F_(a)); }
SI F mad(F     f, float m, F     a) { return mad(   f , F_(m),    a ); }
SI F mad(F     f, float m, float a) { return mad(   f , F_(m), F_(a)); }
SI F mad(float f, F     m, F     a) { return mad(F_(f),    m ,    a ); }
SI F mad(float f, F     m, float a) { return mad(F_(f),    m , F_(a)); }
SI F mad(float f, float m, F     a) { return mad(F_(f), F_(m),    a ); }

SI U32 trunc_(F x) { return (U32)cast<I32>(x); }

// Use approximate instructions and one Newton-Raphson step to calculate 1/x.
SI F rcp_precise(F x) {
#if defined(JUMPER_IS_SKX)
    F e = _mm512_rcp14_ps(x);
    return _mm512_fnmadd_ps(x, e, _mm512_set1_ps(2.0f)) * e;
#elif defined(JUMPER_IS_HSW)
    __m256 lo,hi;
    split(x, &lo,&hi);
    return join<F>(SK_OPTS_NS::rcp_precise(lo), SK_OPTS_NS::rcp_precise(hi));
#elif defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    __m128 lo,hi;
    split(x, &lo,&hi);
    return join<F>(SK_OPTS_NS::rcp_precise(lo), SK_OPTS_NS::rcp_precise(hi));
#elif defined(JUMPER_IS_NEON)
    float32x4_t lo,hi;
    split(x, &lo,&hi);
    return join<F>(SK_OPTS_NS::rcp_precise(lo), SK_OPTS_NS::rcp_precise(hi));
#else
    return 1.0f / x;
#endif
}
SI F sqrt_(F x) {
#if defined(JUMPER_IS_SKX)
    return _mm512_sqrt_ps(x);
#elif defined(JUMPER_IS_HSW)
    __m256 lo,hi;
    split(x, &lo,&hi);
    return join<F>(_mm256_sqrt_ps(lo), _mm256_sqrt_ps(hi));
#elif defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    __m128 lo,hi;
    split(x, &lo,&hi);
    return join<F>(_mm_sqrt_ps(lo), _mm_sqrt_ps(hi));
#elif defined(SK_CPU_ARM64)
    float32x4_t lo,hi;
    split(x, &lo,&hi);
    return join<F>(vsqrtq_f32(lo), vsqrtq_f32(hi));
#elif defined(JUMPER_IS_NEON)
    auto sqrt = [](float32x4_t v) {
        auto est = vrsqrteq_f32(v);  // Estimate and two refinement steps for est = rsqrt(v).
        est *= vrsqrtsq_f32(v,est*est);
        est *= vrsqrtsq_f32(v,est*est);
        return v*est;                // sqrt(v) == v*rsqrt(v).
    };
    float32x4_t lo,hi;
    split(x, &lo,&hi);
    return join<F>(sqrt(lo), sqrt(hi));
#else
    return F{
        sqrtf(x[0]), sqrtf(x[1]), sqrtf(x[2]), sqrtf(x[3]),
        sqrtf(x[4]), sqrtf(x[5]), sqrtf(x[6]), sqrtf(x[7]),
    };
#endif
}

SI F floor_(F x) {
#if defined(SK_CPU_ARM64)
    float32x4_t lo,hi;
    split(x, &lo,&hi);
    return join<F>(vrndmq_f32(lo), vrndmq_f32(hi));
#elif defined(JUMPER_IS_SKX)
    return _mm512_floor_ps(x);
#elif defined(JUMPER_IS_HSW)
    __m256 lo,hi;
    split(x, &lo,&hi);
    return join<F>(_mm256_floor_ps(lo), _mm256_floor_ps(hi));
#elif defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    __m128 lo,hi;
    split(x, &lo,&hi);
    return join<F>(_mm_floor_ps(lo), _mm_floor_ps(hi));
#else
    F roundtrip = cast<F>(cast<I32>(x));
    return roundtrip - if_then_else(roundtrip > x, F_(1), F_(0));
#endif
}

// scaled_mult interprets a and b as number on [-1, 1) which are numbers in Q15 format. Functionally
// this multiply is:
//     (2 * a * b + (1 << 15)) >> 16
// The result is a number on [-1, 1).
// Note: on neon this is a saturating multiply while the others are not.
SI I16 scaled_mult(I16 a, I16 b) {
#if defined(JUMPER_IS_SKX)
    return (I16)_mm256_mulhrs_epi16((__m256i)a, (__m256i)b);
#elif defined(JUMPER_IS_HSW)
    return (I16)_mm256_mulhrs_epi16((__m256i)a, (__m256i)b);
#elif defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    return (I16)_mm_mulhrs_epi16((__m128i)a, (__m128i)b);
#elif defined(SK_CPU_ARM64)
    return vqrdmulhq_s16(a, b);
#elif defined(JUMPER_IS_NEON)
    return vqrdmulhq_s16(a, b);
#else
    const I32 roundingTerm = I32_(1 << 14);
    return cast<I16>((cast<I32>(a) * cast<I32>(b) + roundingTerm) >> 15);
#endif
}

// This sum is to support lerp where the result will always be a positive number. In general,
// a sum like this would require an additional bit, but because we know the range of the result
// we know that the extra bit will always be zero.
SI U16 constrained_add(I16 a, U16 b) {
    #if defined(SK_DEBUG)
        for (size_t i = 0; i < N; i++) {
            // Ensure that a + b is on the interval [0, UINT16_MAX]
            int ia = a[i],
                ib = b[i];
            // Use 65535 here because fuchsia's compiler evaluates UINT16_MAX - ib, which is
            // 65536U - ib, as an uint32_t instead of an int32_t. This was forcing ia to be
            // interpreted as an uint32_t.
            SkASSERT(-ib <= ia && ia <= 65535 - ib);
        }
    #endif
    return b + sk_bit_cast<U16>(a);
}

SI F fract(F x) { return x - floor_(x); }
SI F abs_(F x) { return sk_bit_cast<F>( sk_bit_cast<I32>(x) & 0x7fffffff ); }

// ~~~~~~ Basic / misc. stages ~~~~~~ //

STAGE_GG(seed_shader, NoCtx) {
    static constexpr float iota[] = {
        0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f,
        8.5f, 9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f,
    };
    static_assert(std::size(iota) >= SkRasterPipeline_kMaxStride);

    x = cast<F>(I32_(dx)) + sk_unaligned_load<F>(iota);
    y = cast<F>(I32_(dy)) + 0.5f;
}

STAGE_GG(matrix_translate, const float* m) {
    x += m[0];
    y += m[1];
}
STAGE_GG(matrix_scale_translate, const float* m) {
    x = mad(x,m[0], m[2]);
    y = mad(y,m[1], m[3]);
}
STAGE_GG(matrix_2x3, const float* m) {
    auto X = mad(x,m[0], mad(y,m[1], m[2])),
         Y = mad(x,m[3], mad(y,m[4], m[5]));
    x = X;
    y = Y;
}
STAGE_GG(matrix_perspective, const float* m) {
    // N.B. Unlike the other matrix_ stages, this matrix is row-major.
    auto X = mad(x,m[0], mad(y,m[1], m[2])),
         Y = mad(x,m[3], mad(y,m[4], m[5])),
         Z = mad(x,m[6], mad(y,m[7], m[8]));
    x = X * rcp_precise(Z);
    y = Y * rcp_precise(Z);
}

STAGE_PP(uniform_color, const SkRasterPipeline_UniformColorCtx* c) {
    r = U16_(c->rgba[0]);
    g = U16_(c->rgba[1]);
    b = U16_(c->rgba[2]);
    a = U16_(c->rgba[3]);
}
STAGE_PP(uniform_color_dst, const SkRasterPipeline_UniformColorCtx* c) {
    dr = U16_(c->rgba[0]);
    dg = U16_(c->rgba[1]);
    db = U16_(c->rgba[2]);
    da = U16_(c->rgba[3]);
}
STAGE_PP(black_color, NoCtx) { r = g = b =   U16_0; a = U16_255; }
STAGE_PP(white_color, NoCtx) { r = g = b = U16_255; a = U16_255; }

STAGE_PP(set_rgb, const float rgb[3]) {
    r = from_float(rgb[0]);
    g = from_float(rgb[1]);
    b = from_float(rgb[2]);
}

// No need to clamp against 0 here (values are unsigned)
STAGE_PP(clamp_01, NoCtx) {
    r = min(r, 255);
    g = min(g, 255);
    b = min(b, 255);
    a = min(a, 255);
}

STAGE_PP(clamp_gamut, NoCtx) {
    a = min(a, 255);
    r = min(r, a);
    g = min(g, a);
    b = min(b, a);
}

STAGE_PP(premul, NoCtx) {
    r = div255_accurate(r * a);
    g = div255_accurate(g * a);
    b = div255_accurate(b * a);
}
STAGE_PP(premul_dst, NoCtx) {
    dr = div255_accurate(dr * da);
    dg = div255_accurate(dg * da);
    db = div255_accurate(db * da);
}

STAGE_PP(force_opaque    , NoCtx) {  a = U16_255; }
STAGE_PP(force_opaque_dst, NoCtx) { da = U16_255; }

STAGE_PP(swap_rb, NoCtx) {
    auto tmp = r;
    r = b;
    b = tmp;
}
STAGE_PP(swap_rb_dst, NoCtx) {
    auto tmp = dr;
    dr = db;
    db = tmp;
}

STAGE_PP(move_src_dst, NoCtx) {
    dr = r;
    dg = g;
    db = b;
    da = a;
}

STAGE_PP(move_dst_src, NoCtx) {
    r = dr;
    g = dg;
    b = db;
    a = da;
}

STAGE_PP(swap_src_dst, NoCtx) {
    std::swap(r, dr);
    std::swap(g, dg);
    std::swap(b, db);
    std::swap(a, da);
}

// ~~~~~~ Blend modes ~~~~~~ //

// The same logic applied to all 4 channels.
#define BLEND_MODE(name)                                 \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da); \
    STAGE_PP(name, NoCtx) {                          \
        r = name##_channel(r,dr,a,da);                   \
        g = name##_channel(g,dg,a,da);                   \
        b = name##_channel(b,db,a,da);                   \
        a = name##_channel(a,da,a,da);                   \
    }                                                    \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da)

#if defined(SK_USE_INACCURATE_DIV255_IN_BLEND)
    BLEND_MODE(clear)    { return U16_0; }
    BLEND_MODE(srcatop)  { return div255( s*da + d*inv(sa) ); }
    BLEND_MODE(dstatop)  { return div255( d*sa + s*inv(da) ); }
    BLEND_MODE(srcin)    { return div255( s*da ); }
    BLEND_MODE(dstin)    { return div255( d*sa ); }
    BLEND_MODE(srcout)   { return div255( s*inv(da) ); }
    BLEND_MODE(dstout)   { return div255( d*inv(sa) ); }
    BLEND_MODE(srcover)  { return s + div255( d*inv(sa) ); }
    BLEND_MODE(dstover)  { return d + div255( s*inv(da) ); }
    BLEND_MODE(modulate) { return div255( s*d ); }
    BLEND_MODE(multiply) { return div255( s*inv(da) + d*inv(sa) + s*d ); }
    BLEND_MODE(plus_)    { return min(s+d, 255); }
    BLEND_MODE(screen)   { return s + d - div255( s*d ); }
    BLEND_MODE(xor_)     { return div255( s*inv(da) + d*inv(sa) ); }
#else
    BLEND_MODE(clear)    { return U16_0; }
    BLEND_MODE(srcatop)  { return div255( s*da + d*inv(sa) ); }
    BLEND_MODE(dstatop)  { return div255( d*sa + s*inv(da) ); }
    BLEND_MODE(srcin)    { return div255_accurate( s*da ); }
    BLEND_MODE(dstin)    { return div255_accurate( d*sa ); }
    BLEND_MODE(srcout)   { return div255_accurate( s*inv(da) ); }
    BLEND_MODE(dstout)   { return div255_accurate( d*inv(sa) ); }
    BLEND_MODE(srcover)  { return s + div255_accurate( d*inv(sa) ); }
    BLEND_MODE(dstover)  { return d + div255_accurate( s*inv(da) ); }
    BLEND_MODE(modulate) { return div255_accurate( s*d ); }
    BLEND_MODE(multiply) { return div255( s*inv(da) + d*inv(sa) + s*d ); }
    BLEND_MODE(plus_)    { return min(s+d, 255); }
    BLEND_MODE(screen)   { return s + d - div255_accurate( s*d ); }
    BLEND_MODE(xor_)     { return div255( s*inv(da) + d*inv(sa) ); }
#endif
#undef BLEND_MODE

// The same logic applied to color, and srcover for alpha.
#define BLEND_MODE(name)                                 \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da); \
    STAGE_PP(name, NoCtx) {                          \
        r = name##_channel(r,dr,a,da);                   \
        g = name##_channel(g,dg,a,da);                   \
        b = name##_channel(b,db,a,da);                   \
        a = a + div255( da*inv(a) );                     \
    }                                                    \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da)

    BLEND_MODE(darken)     { return s + d -   div255( max(s*da, d*sa) ); }
    BLEND_MODE(lighten)    { return s + d -   div255( min(s*da, d*sa) ); }
    BLEND_MODE(difference) { return s + d - 2*div255( min(s*da, d*sa) ); }
    BLEND_MODE(exclusion)  { return s + d - 2*div255( s*d ); }

    BLEND_MODE(hardlight) {
        return div255( s*inv(da) + d*inv(sa) +
                       if_then_else(2*s <= sa, 2*s*d, sa*da - 2*(sa-s)*(da-d)) );
    }
    BLEND_MODE(overlay) {
        return div255( s*inv(da) + d*inv(sa) +
                       if_then_else(2*d <= da, 2*s*d, sa*da - 2*(sa-s)*(da-d)) );
    }
#undef BLEND_MODE

// ~~~~~~ Helpers for interacting with memory ~~~~~~ //

template <typename T>
SI T* ptr_at_xy(const SkRasterPipeline_MemoryCtx* ctx, size_t dx, size_t dy) {
    return (T*)ctx->pixels + dy*ctx->stride + dx;
}

template <typename T>
SI U32 ix_and_ptr(T** ptr, const SkRasterPipeline_GatherCtx* ctx, F x, F y) {
    // Exclusive -> inclusive.
    const F w = F_(sk_bit_cast<float>( sk_bit_cast<uint32_t>(ctx->width ) - 1)),
            h = F_(sk_bit_cast<float>( sk_bit_cast<uint32_t>(ctx->height) - 1));

    const F z = F_(std::numeric_limits<float>::min());

    x = min(max(z, x), w);
    y = min(max(z, y), h);

    x = sk_bit_cast<F>(sk_bit_cast<U32>(x) - (uint32_t)ctx->roundDownAtInteger);
    y = sk_bit_cast<F>(sk_bit_cast<U32>(y) - (uint32_t)ctx->roundDownAtInteger);

    *ptr = (const T*)ctx->pixels;
    return trunc_(y)*ctx->stride + trunc_(x);
}

template <typename T>
SI U32 ix_and_ptr(T** ptr, const SkRasterPipeline_GatherCtx* ctx, I32 x, I32 y) {
    // This flag doesn't make sense when the coords are integers.
    SkASSERT(ctx->roundDownAtInteger == 0);
    // Exclusive -> inclusive.
    const I32 w = I32_( ctx->width - 1),
              h = I32_(ctx->height - 1);

    U32 ax = cast<U32>(min(max(0, x), w)),
        ay = cast<U32>(min(max(0, y), h));

    *ptr = (const T*)ctx->pixels;
    return ay * ctx->stride + ax;
}

template <typename V, typename T>
SI V load(const T* ptr) {
    V v;
    memcpy(&v, ptr, sizeof(v));
    return v;
}
template <typename V, typename T>
SI void store(T* ptr, V v) {
    memcpy(ptr, &v, sizeof(v));
}

#if defined(JUMPER_IS_SKX)
    template <typename V, typename T>
    SI V gather(const T* ptr, U32 ix) {
        return V{ ptr[ix[ 0]], ptr[ix[ 1]], ptr[ix[ 2]], ptr[ix[ 3]],
                  ptr[ix[ 4]], ptr[ix[ 5]], ptr[ix[ 6]], ptr[ix[ 7]],
                  ptr[ix[ 8]], ptr[ix[ 9]], ptr[ix[10]], ptr[ix[11]],
                  ptr[ix[12]], ptr[ix[13]], ptr[ix[14]], ptr[ix[15]], };
    }

    template<>
    F gather(const float* ptr, U32 ix) {
        return _mm512_i32gather_ps((__m512i)ix, ptr, 4);
    }

    template<>
    U32 gather(const uint32_t* ptr, U32 ix) {
        return (U32)_mm512_i32gather_epi32((__m512i)ix, ptr, 4);
    }

#elif defined(JUMPER_IS_HSW)
    template <typename V, typename T>
    SI V gather(const T* ptr, U32 ix) {
        return V{ ptr[ix[ 0]], ptr[ix[ 1]], ptr[ix[ 2]], ptr[ix[ 3]],
                  ptr[ix[ 4]], ptr[ix[ 5]], ptr[ix[ 6]], ptr[ix[ 7]],
                  ptr[ix[ 8]], ptr[ix[ 9]], ptr[ix[10]], ptr[ix[11]],
                  ptr[ix[12]], ptr[ix[13]], ptr[ix[14]], ptr[ix[15]], };
    }

    template<>
    F gather(const float* ptr, U32 ix) {
        __m256i lo, hi;
        split(ix, &lo, &hi);

        return join<F>(_mm256_i32gather_ps(ptr, lo, 4),
                       _mm256_i32gather_ps(ptr, hi, 4));
    }

    template<>
    U32 gather(const uint32_t* ptr, U32 ix) {
        __m256i lo, hi;
        split(ix, &lo, &hi);

        return join<U32>(_mm256_i32gather_epi32((const int*)ptr, lo, 4),
                         _mm256_i32gather_epi32((const int*)ptr, hi, 4));
    }
#else
    template <typename V, typename T>
    SI V gather(const T* ptr, U32 ix) {
        return V{ ptr[ix[ 0]], ptr[ix[ 1]], ptr[ix[ 2]], ptr[ix[ 3]],
                  ptr[ix[ 4]], ptr[ix[ 5]], ptr[ix[ 6]], ptr[ix[ 7]], };
    }
#endif


// ~~~~~~ 32-bit memory loads and stores ~~~~~~ //

SI void from_8888(U32 rgba, U16* r, U16* g, U16* b, U16* a) {
#if defined(JUMPER_IS_SKX)
    rgba = (U32)_mm512_permutexvar_epi64(_mm512_setr_epi64(0,1,4,5,2,3,6,7), (__m512i)rgba);
    auto cast_U16 = [](U32 v) -> U16 {
        return (U16)_mm256_packus_epi32(_mm512_castsi512_si256((__m512i)v),
                    _mm512_extracti64x4_epi64((__m512i)v, 1));
    };
#elif defined(JUMPER_IS_HSW)
    // Swap the middle 128-bit lanes to make _mm256_packus_epi32() in cast_U16() work out nicely.
    __m256i _01,_23;
    split(rgba, &_01, &_23);
    __m256i _02 = _mm256_permute2x128_si256(_01,_23, 0x20),
            _13 = _mm256_permute2x128_si256(_01,_23, 0x31);
    rgba = join<U32>(_02, _13);

    auto cast_U16 = [](U32 v) -> U16 {
        __m256i _02,_13;
        split(v, &_02,&_13);
        return (U16)_mm256_packus_epi32(_02,_13);
    };
#else
    auto cast_U16 = [](U32 v) -> U16 {
        return cast<U16>(v);
    };
#endif
    *r = cast_U16(rgba & 65535) & 255;
    *g = cast_U16(rgba & 65535) >>  8;
    *b = cast_U16(rgba >>   16) & 255;
    *a = cast_U16(rgba >>   16) >>  8;
}

SI void load_8888_(const uint32_t* ptr, U16* r, U16* g, U16* b, U16* a) {
#if 1 && defined(JUMPER_IS_NEON)
    uint8x8x4_t rgba = vld4_u8((const uint8_t*)(ptr));
    *r = cast<U16>(rgba.val[0]);
    *g = cast<U16>(rgba.val[1]);
    *b = cast<U16>(rgba.val[2]);
    *a = cast<U16>(rgba.val[3]);
#else
    from_8888(load<U32>(ptr), r,g,b,a);
#endif
}
SI void store_8888_(uint32_t* ptr, U16 r, U16 g, U16 b, U16 a) {
    r = min(r, 255);
    g = min(g, 255);
    b = min(b, 255);
    a = min(a, 255);

#if 1 && defined(JUMPER_IS_NEON)
    uint8x8x4_t rgba = {{
        cast<U8>(r),
        cast<U8>(g),
        cast<U8>(b),
        cast<U8>(a),
    }};
    vst4_u8((uint8_t*)(ptr), rgba);
#else
    store(ptr, cast<U32>(r | (g<<8)) <<  0
             | cast<U32>(b | (a<<8)) << 16);
#endif
}

STAGE_PP(load_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    load_8888_(ptr_at_xy<const uint32_t>(ctx, dx,dy), &r,&g,&b,&a);
}
STAGE_PP(load_8888_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    load_8888_(ptr_at_xy<const uint32_t>(ctx, dx,dy), &dr,&dg,&db,&da);
}
STAGE_PP(store_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    store_8888_(ptr_at_xy<uint32_t>(ctx, dx,dy), r,g,b,a);
}
STAGE_GP(gather_8888, const SkRasterPipeline_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    from_8888(gather<U32>(ptr, ix), &r, &g, &b, &a);
}

// ~~~~~~ 16-bit memory loads and stores ~~~~~~ //

SI void from_565(U16 rgb, U16* r, U16* g, U16* b) {
    // Format for 565 buffers: 15|rrrrr gggggg bbbbb|0
    U16 R = (rgb >> 11) & 31,
        G = (rgb >>  5) & 63,
        B = (rgb >>  0) & 31;

    // These bit replications are the same as multiplying by 255/31 or 255/63 to scale to 8-bit.
    *r = (R << 3) | (R >> 2);
    *g = (G << 2) | (G >> 4);
    *b = (B << 3) | (B >> 2);
}
SI void load_565_(const uint16_t* ptr, U16* r, U16* g, U16* b) {
    from_565(load<U16>(ptr), r,g,b);
}
SI void store_565_(uint16_t* ptr, U16 r, U16 g, U16 b) {
    r = min(r, 255);
    g = min(g, 255);
    b = min(b, 255);

    // Round from [0,255] to [0,31] or [0,63], as if x * (31/255.0f) + 0.5f.
    // (Don't feel like you need to find some fundamental truth in these...
    // they were brute-force searched.)
    U16 R = (r *  9 + 36) / 74,   //  9/74 ≈ 31/255, plus 36/74, about half.
        G = (g * 21 + 42) / 85,   // 21/85 = 63/255 exactly.
        B = (b *  9 + 36) / 74;
    // Pack them back into 15|rrrrr gggggg bbbbb|0.
    store(ptr, R << 11
             | G <<  5
             | B <<  0);
}

STAGE_PP(load_565, const SkRasterPipeline_MemoryCtx* ctx) {
    load_565_(ptr_at_xy<const uint16_t>(ctx, dx,dy), &r,&g,&b);
    a = U16_255;
}
STAGE_PP(load_565_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    load_565_(ptr_at_xy<const uint16_t>(ctx, dx,dy), &dr,&dg,&db);
    da = U16_255;
}
STAGE_PP(store_565, const SkRasterPipeline_MemoryCtx* ctx) {
    store_565_(ptr_at_xy<uint16_t>(ctx, dx,dy), r,g,b);
}
STAGE_GP(gather_565, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    from_565(gather<U16>(ptr, ix), &r, &g, &b);
    a = U16_255;
}

SI void from_4444(U16 rgba, U16* r, U16* g, U16* b, U16* a) {
    // Format for 4444 buffers: 15|rrrr gggg bbbb aaaa|0.
    U16 R = (rgba >> 12) & 15,
        G = (rgba >>  8) & 15,
        B = (rgba >>  4) & 15,
        A = (rgba >>  0) & 15;

    // Scale [0,15] to [0,255].
    *r = (R << 4) | R;
    *g = (G << 4) | G;
    *b = (B << 4) | B;
    *a = (A << 4) | A;
}
SI void load_4444_(const uint16_t* ptr, U16* r, U16* g, U16* b, U16* a) {
    from_4444(load<U16>(ptr), r,g,b,a);
}
SI void store_4444_(uint16_t* ptr, U16 r, U16 g, U16 b, U16 a) {
    r = min(r, 255);
    g = min(g, 255);
    b = min(b, 255);
    a = min(a, 255);

    // Round from [0,255] to [0,15], producing the same value as (x*(15/255.0f) + 0.5f).
    U16 R = (r + 8) / 17,
        G = (g + 8) / 17,
        B = (b + 8) / 17,
        A = (a + 8) / 17;
    // Pack them back into 15|rrrr gggg bbbb aaaa|0.
    store(ptr, R << 12
             | G <<  8
             | B <<  4
             | A <<  0);
}

STAGE_PP(load_4444, const SkRasterPipeline_MemoryCtx* ctx) {
    load_4444_(ptr_at_xy<const uint16_t>(ctx, dx,dy), &r,&g,&b,&a);
}
STAGE_PP(load_4444_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    load_4444_(ptr_at_xy<const uint16_t>(ctx, dx,dy), &dr,&dg,&db,&da);
}
STAGE_PP(store_4444, const SkRasterPipeline_MemoryCtx* ctx) {
    store_4444_(ptr_at_xy<uint16_t>(ctx, dx,dy), r,g,b,a);
}
STAGE_GP(gather_4444, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    from_4444(gather<U16>(ptr, ix), &r,&g,&b,&a);
}

SI void from_88(U16 rg, U16* r, U16* g) {
    *r = (rg & 0xFF);
    *g = (rg >> 8);
}

SI void load_88_(const uint16_t* ptr, U16* r, U16* g) {
#if 1 && defined(JUMPER_IS_NEON)
    uint8x8x2_t rg = vld2_u8((const uint8_t*)(ptr));
    *r = cast<U16>(rg.val[0]);
    *g = cast<U16>(rg.val[1]);
#else
    from_88(load<U16>(ptr), r,g);
#endif
}

SI void store_88_(uint16_t* ptr, U16 r, U16 g) {
    r = min(r, 255);
    g = min(g, 255);

#if 1 && defined(JUMPER_IS_NEON)
    uint8x8x2_t rg = {{
        cast<U8>(r),
        cast<U8>(g),
    }};
    vst2_u8((uint8_t*)(ptr), rg);
#else
    store(ptr, cast<U16>(r | (g<<8)) <<  0);
#endif
}

STAGE_PP(load_rg88, const SkRasterPipeline_MemoryCtx* ctx) {
    load_88_(ptr_at_xy<const uint16_t>(ctx, dx, dy), &r, &g);
    b = U16_0;
    a = U16_255;
}
STAGE_PP(load_rg88_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    load_88_(ptr_at_xy<const uint16_t>(ctx, dx, dy), &dr, &dg);
    db = U16_0;
    da = U16_255;
}
STAGE_PP(store_rg88, const SkRasterPipeline_MemoryCtx* ctx) {
    store_88_(ptr_at_xy<uint16_t>(ctx, dx, dy), r, g);
}
STAGE_GP(gather_rg88, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x, y);
    from_88(gather<U16>(ptr, ix), &r, &g);
    b = U16_0;
    a = U16_255;
}

// ~~~~~~ 8-bit memory loads and stores ~~~~~~ //

SI U16 load_8(const uint8_t* ptr) {
    return cast<U16>(load<U8>(ptr));
}
SI void store_8(uint8_t* ptr, U16 v) {
    v = min(v, 255);
    store(ptr, cast<U8>(v));
}

STAGE_PP(load_a8, const SkRasterPipeline_MemoryCtx* ctx) {
    r = g = b = U16_0;
    a = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy));
}
STAGE_PP(load_a8_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    dr = dg = db = U16_0;
    da = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy));
}
STAGE_PP(store_a8, const SkRasterPipeline_MemoryCtx* ctx) {
    store_8(ptr_at_xy<uint8_t>(ctx, dx,dy), a);
}
STAGE_GP(gather_a8, const SkRasterPipeline_GatherCtx* ctx) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    r = g = b = U16_0;
    a = cast<U16>(gather<U8>(ptr, ix));
}
STAGE_PP(store_r8, const SkRasterPipeline_MemoryCtx* ctx) {
    store_8(ptr_at_xy<uint8_t>(ctx, dx,dy), r);
}

STAGE_PP(alpha_to_gray, NoCtx) {
    r = g = b = a;
    a = U16_255;
}
STAGE_PP(alpha_to_gray_dst, NoCtx) {
    dr = dg = db = da;
    da = U16_255;
}
STAGE_PP(alpha_to_red, NoCtx) {
    r = a;
    a = U16_255;
}
STAGE_PP(alpha_to_red_dst, NoCtx) {
    dr = da;
    da = U16_255;
}

STAGE_PP(bt709_luminance_or_luma_to_alpha, NoCtx) {
    a = (r*54 + g*183 + b*19)/256;  // 0.2126, 0.7152, 0.0722 with 256 denominator.
    r = g = b = U16_0;
}
STAGE_PP(bt709_luminance_or_luma_to_rgb, NoCtx) {
    r = g = b =(r*54 + g*183 + b*19)/256;  // 0.2126, 0.7152, 0.0722 with 256 denominator.
}

// ~~~~~~ Coverage scales / lerps ~~~~~~ //

STAGE_PP(load_src, const uint16_t* ptr) {
    r = sk_unaligned_load<U16>(ptr + 0*N);
    g = sk_unaligned_load<U16>(ptr + 1*N);
    b = sk_unaligned_load<U16>(ptr + 2*N);
    a = sk_unaligned_load<U16>(ptr + 3*N);
}
STAGE_PP(store_src, uint16_t* ptr) {
    sk_unaligned_store(ptr + 0*N, r);
    sk_unaligned_store(ptr + 1*N, g);
    sk_unaligned_store(ptr + 2*N, b);
    sk_unaligned_store(ptr + 3*N, a);
}
STAGE_PP(store_src_a, uint16_t* ptr) {
    sk_unaligned_store(ptr, a);
}
STAGE_PP(load_dst, const uint16_t* ptr) {
    dr = sk_unaligned_load<U16>(ptr + 0*N);
    dg = sk_unaligned_load<U16>(ptr + 1*N);
    db = sk_unaligned_load<U16>(ptr + 2*N);
    da = sk_unaligned_load<U16>(ptr + 3*N);
}
STAGE_PP(store_dst, uint16_t* ptr) {
    sk_unaligned_store(ptr + 0*N, dr);
    sk_unaligned_store(ptr + 1*N, dg);
    sk_unaligned_store(ptr + 2*N, db);
    sk_unaligned_store(ptr + 3*N, da);
}

// ~~~~~~ Coverage scales / lerps ~~~~~~ //

STAGE_PP(scale_1_float, const float* f) {
    U16 c = from_float(*f);
    r = div255( r * c );
    g = div255( g * c );
    b = div255( b * c );
    a = div255( a * c );
}
STAGE_PP(lerp_1_float, const float* f) {
    U16 c = from_float(*f);
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE_PP(scale_native, const uint16_t scales[]) {
    auto c = sk_unaligned_load<U16>(scales);
    r = div255( r * c );
    g = div255( g * c );
    b = div255( b * c );
    a = div255( a * c );
}

STAGE_PP(lerp_native, const uint16_t scales[]) {
    auto c = sk_unaligned_load<U16>(scales);
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

STAGE_PP(scale_u8, const SkRasterPipeline_MemoryCtx* ctx) {
    U16 c = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy));
    r = div255( r * c );
    g = div255( g * c );
    b = div255( b * c );
    a = div255( a * c );
}
STAGE_PP(lerp_u8, const SkRasterPipeline_MemoryCtx* ctx) {
    U16 c = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy));
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

// Derive alpha's coverage from rgb coverage and the values of src and dst alpha.
SI U16 alpha_coverage_from_rgb_coverage(U16 a, U16 da, U16 cr, U16 cg, U16 cb) {
    return if_then_else(a < da, min(cr, min(cg,cb))
                              , max(cr, max(cg,cb)));
}
STAGE_PP(scale_565, const SkRasterPipeline_MemoryCtx* ctx) {
    U16 cr,cg,cb;
    load_565_(ptr_at_xy<const uint16_t>(ctx, dx,dy), &cr,&cg,&cb);
    U16 ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = div255( r * cr );
    g = div255( g * cg );
    b = div255( b * cb );
    a = div255( a * ca );
}
STAGE_PP(lerp_565, const SkRasterPipeline_MemoryCtx* ctx) {
    U16 cr,cg,cb;
    load_565_(ptr_at_xy<const uint16_t>(ctx, dx,dy), &cr,&cg,&cb);
    U16 ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = lerp(da, a, ca);
}

STAGE_PP(emboss, const SkRasterPipeline_EmbossCtx* ctx) {
    U16 mul = load_8(ptr_at_xy<const uint8_t>(&ctx->mul, dx,dy)),
        add = load_8(ptr_at_xy<const uint8_t>(&ctx->add, dx,dy));

    r = min(div255(r*mul) + add, a);
    g = min(div255(g*mul) + add, a);
    b = min(div255(b*mul) + add, a);
}


// ~~~~~~ Gradient stages ~~~~~~ //

// Clamp x to [0,1], both sides inclusive (think, gradients).
// Even repeat and mirror funnel through a clamp to handle bad inputs like +Inf, NaN.
SI F clamp_01_(F v) { return min(max(0, v), 1); }

STAGE_GG(clamp_x_1 , NoCtx) { x = clamp_01_(x); }
STAGE_GG(repeat_x_1, NoCtx) { x = clamp_01_(x - floor_(x)); }
STAGE_GG(mirror_x_1, NoCtx) {
    auto two = [](F x){ return x+x; };
    x = clamp_01_(abs_( (x-1.0f) - two(floor_((x-1.0f)*0.5f)) - 1.0f ));
}

SI I16 cond_to_mask_16(I32 cond) { return cast<I16>(cond); }

STAGE_GG(decal_x, SkRasterPipeline_DecalTileCtx* ctx) {
    auto w = ctx->limit_x;
    sk_unaligned_store(ctx->mask, cond_to_mask_16((0 <= x) & (x < w)));
}
STAGE_GG(decal_y, SkRasterPipeline_DecalTileCtx* ctx) {
    auto h = ctx->limit_y;
    sk_unaligned_store(ctx->mask, cond_to_mask_16((0 <= y) & (y < h)));
}
STAGE_GG(decal_x_and_y, SkRasterPipeline_DecalTileCtx* ctx) {
    auto w = ctx->limit_x;
    auto h = ctx->limit_y;
    sk_unaligned_store(ctx->mask, cond_to_mask_16((0 <= x) & (x < w) & (0 <= y) & (y < h)));
}
STAGE_GG(clamp_x_and_y, SkRasterPipeline_CoordClampCtx* ctx) {
    x = min(ctx->max_x, max(ctx->min_x, x));
    y = min(ctx->max_y, max(ctx->min_y, y));
}
STAGE_PP(check_decal_mask, SkRasterPipeline_DecalTileCtx* ctx) {
    auto mask = sk_unaligned_load<U16>(ctx->mask);
    r = r & mask;
    g = g & mask;
    b = b & mask;
    a = a & mask;
}

SI void round_F_to_U16(F R, F G, F B, F A, U16* r, U16* g, U16* b, U16* a) {
    auto round_color = [](F x) { return cast<U16>(x * 255.0f + 0.5f); };

    *r = round_color(min(max(0, R), 1));
    *g = round_color(min(max(0, G), 1));
    *b = round_color(min(max(0, B), 1));
    *a = round_color(A);  // we assume alpha is already in [0,1].
}

SI void gradient_lookup(const SkRasterPipeline_GradientCtx* c, U32 idx, F t,
                        U16* r, U16* g, U16* b, U16* a) {

    F fr, fg, fb, fa, br, bg, bb, ba;
#if defined(JUMPER_IS_HSW)
    if (c->stopCount <=8) {
        __m256i lo, hi;
        split(idx, &lo, &hi);

        fr = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[0]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[0]), hi));
        br = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[0]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[0]), hi));
        fg = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[1]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[1]), hi));
        bg = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[1]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[1]), hi));
        fb = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[2]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[2]), hi));
        bb = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[2]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[2]), hi));
        fa = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[3]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[3]), hi));
        ba = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[3]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[3]), hi));
    } else
#endif
    {
        fr = gather<F>(c->fs[0], idx);
        fg = gather<F>(c->fs[1], idx);
        fb = gather<F>(c->fs[2], idx);
        fa = gather<F>(c->fs[3], idx);
        br = gather<F>(c->bs[0], idx);
        bg = gather<F>(c->bs[1], idx);
        bb = gather<F>(c->bs[2], idx);
        ba = gather<F>(c->bs[3], idx);
    }
    round_F_to_U16(mad(t, fr, br),
                   mad(t, fg, bg),
                   mad(t, fb, bb),
                   mad(t, fa, ba),
                   r,g,b,a);
}

STAGE_GP(gradient, const SkRasterPipeline_GradientCtx* c) {
    auto t = x;
    U32 idx = U32_(0);

    // N.B. The loop starts at 1 because idx 0 is the color to use before the first stop.
    for (size_t i = 1; i < c->stopCount; i++) {
        idx += if_then_else(t >= c->ts[i], U32_(1), U32_(0));
    }

    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE_GP(evenly_spaced_gradient, const SkRasterPipeline_GradientCtx* c) {
    auto t = x;
    auto idx = trunc_(t * static_cast<float>(c->stopCount-1));
    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE_GP(evenly_spaced_2_stop_gradient, const SkRasterPipeline_EvenlySpaced2StopGradientCtx* c) {
    auto t = x;
    round_F_to_U16(mad(t, c->f[0], c->b[0]),
                   mad(t, c->f[1], c->b[1]),
                   mad(t, c->f[2], c->b[2]),
                   mad(t, c->f[3], c->b[3]),
                   &r,&g,&b,&a);
}

STAGE_GP(bilerp_clamp_8888, const SkRasterPipeline_GatherCtx* ctx) {
    // Quantize sample point and transform into lerp coordinates converting them to 16.16 fixed
    // point number.
    I32 qx = cast<I32>(floor_(65536.0f * x + 0.5f)) - 32768,
        qy = cast<I32>(floor_(65536.0f * y + 0.5f)) - 32768;

    // Calculate screen coordinates sx & sy by flooring qx and qy.
    I32 sx = qx >> 16,
        sy = qy >> 16;

    // We are going to perform a change of parameters for qx on [0, 1) to tx on [-1, 1).
    // This will put tx in Q15 format for use with q_mult.
    // Calculate tx and ty on the interval of [-1, 1). Give {qx} and {qy} are on the interval
    // [0, 1), where {v} is fract(v), we can transform to tx in the following manner ty follows
    // the same math:
    //     tx = 2 * {qx} - 1, so
    //     {qx} = (tx + 1) / 2.
    // Calculate {qx} - 1 and {qy} - 1 where the {} operation is handled by the cast, and the - 1
    // is handled by the ^ 0x8000, dividing by 2 is deferred and handled in lerpX and lerpY in
    // order to use the full 16-bit resolution.
    I16 tx = cast<I16>(qx ^ 0x8000),
        ty = cast<I16>(qy ^ 0x8000);

    // Substituting the {qx} by the equation for tx from above into the lerp equation where v is
    // the lerped value:
    //         v = {qx}*(R - L) + L,
    //         v = 1/2*(tx + 1)*(R - L) + L
    //     2 * v = (tx + 1)*(R - L) + 2*L
    //           = tx*R - tx*L + R - L + 2*L
    //           = tx*(R - L) + (R + L).
    // Since R and L are on [0, 255] we need them on the interval [0, 1/2] to get them into form
    // for Q15_mult. If L and R where in 16.16 format, this would be done by dividing by 2^9. In
    // code, we can multiply by 2^7 to get the value directly.
    //            2 * v = tx*(R - L) + (R + L)
    //     2^-9 * 2 * v = tx*(R - L)*2^-9 + (R + L)*2^-9
    //         2^-8 * v = 2^-9 * (tx*(R - L) + (R + L))
    //                v = 1/2 * (tx*(R - L) + (R + L))
    auto lerpX = [&](U16 left, U16 right) -> U16 {
        I16 width  = (I16)(right - left) << 7;
        U16 middle = (right + left) << 7;
        // The constrained_add is the most subtle part of lerp. The first term is on the interval
        // [-1, 1), and the second term is on the interval is on the interval [0, 1) because
        // both terms are too high by a factor of 2 which will be handled below. (Both R and L are
        // on [0, 1/2), but the sum R + L is on the interval [0, 1).) Generally, the sum below
        // should overflow, but because we know that sum produces an output on the
        // interval [0, 1) we know that the extra bit that would be needed will always be 0. So
        // we need to be careful to treat this sum as an unsigned positive number in the divide
        // by 2 below. Add +1 for rounding.
        U16 v2  = constrained_add(scaled_mult(tx, width), middle) + 1;
        // Divide by 2 to calculate v and at the same time bring the intermediate value onto the
        // interval [0, 1/2] to set up for the lerpY.
        return v2 >> 1;
    };

    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, sx, sy);
    U16 leftR, leftG, leftB, leftA;
    from_8888(gather<U32>(ptr, ix), &leftR,&leftG,&leftB,&leftA);

    ix = ix_and_ptr(&ptr, ctx, sx+1, sy);
    U16 rightR, rightG, rightB, rightA;
    from_8888(gather<U32>(ptr, ix), &rightR,&rightG,&rightB,&rightA);

    U16 topR = lerpX(leftR, rightR),
        topG = lerpX(leftG, rightG),
        topB = lerpX(leftB, rightB),
        topA = lerpX(leftA, rightA);

    ix = ix_and_ptr(&ptr, ctx, sx, sy+1);
    from_8888(gather<U32>(ptr, ix), &leftR,&leftG,&leftB,&leftA);

    ix = ix_and_ptr(&ptr, ctx, sx+1, sy+1);
    from_8888(gather<U32>(ptr, ix), &rightR,&rightG,&rightB,&rightA);

    U16 bottomR = lerpX(leftR, rightR),
        bottomG = lerpX(leftG, rightG),
        bottomB = lerpX(leftB, rightB),
        bottomA = lerpX(leftA, rightA);

    // lerpY plays the same mathematical tricks as lerpX, but the final divide is by 256 resulting
    // in a value on [0, 255].
    auto lerpY = [&](U16 top, U16 bottom) -> U16 {
        I16 width  = (I16)bottom - (I16)top;
        U16 middle = bottom + top;
        // Add + 0x80 for rounding.
        U16 blend  = constrained_add(scaled_mult(ty, width), middle) + 0x80;

        return blend >> 8;
    };

    r = lerpY(topR, bottomR);
    g = lerpY(topG, bottomG);
    b = lerpY(topB, bottomB);
    a = lerpY(topA, bottomA);
}

STAGE_GG(xy_to_unit_angle, NoCtx) {
    F xabs = abs_(x),
      yabs = abs_(y);

    F slope = min(xabs, yabs)/max(xabs, yabs);
    F s = slope * slope;

    // Use a 7th degree polynomial to approximate atan.
    // This was generated using sollya.gforge.inria.fr.
    // A float optimized polynomial was generated using the following command.
    // P1 = fpminimax((1/(2*Pi))*atan(x),[|1,3,5,7|],[|24...|],[2^(-40),1],relative);
    F phi = slope
             * (0.15912117063999176025390625f     + s
             * (-5.185396969318389892578125e-2f   + s
             * (2.476101927459239959716796875e-2f + s
             * (-7.0547382347285747528076171875e-3f))));

    phi = if_then_else(xabs < yabs, 1.0f/4.0f - phi, phi);
    phi = if_then_else(x < 0.0f   , 1.0f/2.0f - phi, phi);
    phi = if_then_else(y < 0.0f   , 1.0f - phi     , phi);
    phi = if_then_else(phi != phi , 0              , phi);  // Check for NaN.
    x = phi;
}
STAGE_GG(xy_to_radius, NoCtx) {
    x = sqrt_(x*x + y*y);
}

// ~~~~~~ Compound stages ~~~~~~ //

STAGE_PP(srcover_rgba_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    load_8888_(ptr, &dr,&dg,&db,&da);
    r = r + div255( dr*inv(a) );
    g = g + div255( dg*inv(a) );
    b = b + div255( db*inv(a) );
    a = a + div255( da*inv(a) );
    store_8888_(ptr, r,g,b,a);
}

// ~~~~~~ skgpu::Swizzle stage ~~~~~~ //

STAGE_PP(swizzle, void* ctx) {
    auto ir = r, ig = g, ib = b, ia = a;
    U16* o[] = {&r, &g, &b, &a};
    char swiz[4];
    memcpy(swiz, &ctx, sizeof(swiz));

    for (int i = 0; i < 4; ++i) {
        switch (swiz[i]) {
            case 'r': *o[i] = ir;       break;
            case 'g': *o[i] = ig;       break;
            case 'b': *o[i] = ib;       break;
            case 'a': *o[i] = ia;       break;
            case '0': *o[i] = U16_0;    break;
            case '1': *o[i] = U16_255;  break;
            default:                    break;
        }
    }
}

#endif//defined(JUMPER_IS_SCALAR) controlling whether we build lowp stages
}  // namespace lowp

/* This gives us SK_OPTS::lowp::N if lowp::N has been set, or SK_OPTS::N if it hasn't. */
namespace lowp { static constexpr size_t lowp_N = N; }

/** Allow outside code to access the Raster Pipeline pixel stride. */
constexpr size_t raster_pipeline_lowp_stride() { return lowp::lowp_N; }
constexpr size_t raster_pipeline_highp_stride() { return N; }

}  // namespace SK_OPTS_NS

#undef SI

#endif//SkRasterPipeline_opts_DEFINED
