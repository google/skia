/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

#include "include/core/SkTypes.h"
#include "src/core/SkUtils.h"  // unaligned_{load,store}
#include "src/sksl/SkSLByteCode.h"

// Every function in this file should be marked static and inline using SI.
#if defined(__clang__)
    #define SI __attribute__((always_inline)) static inline
#else
    #define SI static inline
#endif

template <typename Dst, typename Src>
SI Dst bit_cast(const Src& src) {
    static_assert(sizeof(Dst) == sizeof(Src), "");
    return sk_unaligned_load<Dst>(&src);
}

template <typename Dst, typename Src>
SI Dst widen_cast(const Src& src) {
    static_assert(sizeof(Dst) > sizeof(Src), "");
    Dst dst;
    memcpy(&dst, &src, sizeof(Src));
    return dst;
}

// Our program is an array of void*, either
//   - 1 void* per stage with no context pointer, the next stage;
//   - 2 void* per stage with a context pointer, first the context pointer, then the next stage.

// load_and_inc() steps the program forward by 1 void*, returning that pointer.
SI void* load_and_inc(void**& program) {
#if defined(__GNUC__) && defined(__x86_64__)
    // If program is in %rsi (we try to make this likely) then this is a single instruction.
    void* rax;
    asm("lodsq" : "=a"(rax), "+S"(program));  // Write-only %rax, read-write %rsi.
    return rax;
#else
    // On ARM *program++ compiles into pretty ideal code without any handholding.
    return *program++;
#endif
}

// Lazily resolved on first cast.  Does nothing if cast to Ctx::None.
struct Ctx {
    struct None {};

    void*   ptr;
    void**& program;

    explicit Ctx(void**& p) : ptr(nullptr), program(p) {}

    template <typename T>
    operator T*() {
        if (!ptr) { ptr = load_and_inc(program); }
        return (T*)ptr;
    }
    operator None() { return None{}; }
};


#if !defined(__clang__)
    #define JUMPER_IS_SCALAR
#elif defined(SK_ARM_HAS_NEON)
    #define JUMPER_IS_NEON
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX512
    #define JUMPER_IS_AVX512
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

namespace SK_OPTS_NS {

#if defined(JUMPER_IS_SCALAR)
    // This path should lead to portable scalar code.
    using F   = float   ;
    using I32 =  int32_t;
    using U64 = uint64_t;
    using U32 = uint32_t;
    using U16 = uint16_t;
    using U8  = uint8_t ;

    SI F   mad(F f, F m, F a)   { return f*m+a; }
    SI F   min(F a, F b)        { return fminf(a,b); }
    SI F   max(F a, F b)        { return fmaxf(a,b); }
    SI F   abs_  (F v)          { return fabsf(v); }
    SI F   floor_(F v)          { return floorf(v); }
    SI F   rcp   (F v)          { return 1.0f / v; }
    SI F   rsqrt (F v)          { return 1.0f / sqrtf(v); }
    SI F    sqrt_(F v)          { return sqrtf(v); }
    SI U32 round (F v, F scale) { return (uint32_t)(v*scale + 0.5f); }
    SI U16 pack(U32 v)          { return (U16)v; }
    SI U8  pack(U16 v)          { return  (U8)v; }

    SI F if_then_else(I32 c, F t, F e) { return c ? t : e; }

    template <typename T>
    SI T gather(const T* p, U32 ix) { return p[ix]; }

    SI void load2(const uint16_t* ptr, size_t tail, U16* r, U16* g) {
        *r = ptr[0];
        *g = ptr[1];
    }
    SI void store2(uint16_t* ptr, size_t tail, U16 r, U16 g) {
        ptr[0] = r;
        ptr[1] = g;
    }
    SI void load3(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
        *r = ptr[0];
        *g = ptr[1];
        *b = ptr[2];
    }
    SI void load4(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        *r = ptr[0];
        *g = ptr[1];
        *b = ptr[2];
        *a = ptr[3];
    }
    SI void store4(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        ptr[0] = r;
        ptr[1] = g;
        ptr[2] = b;
        ptr[3] = a;
    }

    SI void load2(const float* ptr, size_t tail, F* r, F* g) {
        *r = ptr[0];
        *g = ptr[1];
    }
    SI void store2(float* ptr, size_t tail, F r, F g) {
        ptr[0] = r;
        ptr[1] = g;
    }
    SI void load4(const float* ptr, size_t tail, F* r, F* g, F* b, F* a) {
        *r = ptr[0];
        *g = ptr[1];
        *b = ptr[2];
        *a = ptr[3];
    }
    SI void store4(float* ptr, size_t tail, F r, F g, F b, F a) {
        ptr[0] = r;
        ptr[1] = g;
        ptr[2] = b;
        ptr[3] = a;
    }

#elif defined(JUMPER_IS_NEON)
    // Since we know we're using Clang, we can use its vector extensions.
    template <typename T> using V = T __attribute__((ext_vector_type(4)));
    using F   = V<float   >;
    using I32 = V< int32_t>;
    using U64 = V<uint64_t>;
    using U32 = V<uint32_t>;
    using U16 = V<uint16_t>;
    using U8  = V<uint8_t >;

    // We polyfill a few routines that Clang doesn't build into ext_vector_types.
    SI F   min(F a, F b)                         { return vminq_f32(a,b);          }
    SI F   max(F a, F b)                         { return vmaxq_f32(a,b);          }
    SI F   abs_  (F v)                           { return vabsq_f32(v);            }
    SI F   rcp   (F v) { auto e = vrecpeq_f32 (v); return vrecpsq_f32 (v,e  ) * e; }
    SI F   rsqrt (F v) { auto e = vrsqrteq_f32(v); return vrsqrtsq_f32(v,e*e) * e; }
    SI U16 pack(U32 v)                           { return __builtin_convertvector(v, U16); }
    SI U8  pack(U16 v)                           { return __builtin_convertvector(v,  U8); }

    SI F if_then_else(I32 c, F t, F e) { return vbslq_f32((U32)c,t,e); }

    #if defined(SK_CPU_ARM64)
        SI F     mad(F f, F m, F a) { return vfmaq_f32(a,f,m); }
        SI F  floor_(F v) { return vrndmq_f32(v); }
        SI F   sqrt_(F v) { return vsqrtq_f32(v); }
        SI U32 round(F v, F scale) { return vcvtnq_u32_f32(v*scale); }
    #else
        SI F mad(F f, F m, F a) { return vmlaq_f32(a,f,m); }
        SI F floor_(F v) {
            F roundtrip = vcvtq_f32_s32(vcvtq_s32_f32(v));
            return roundtrip - if_then_else(roundtrip > v, 1, 0);
        }

        SI F sqrt_(F v) {
            auto e = vrsqrteq_f32(v);  // Estimate and two refinement steps for e = rsqrt(v).
            e *= vrsqrtsq_f32(v,e*e);
            e *= vrsqrtsq_f32(v,e*e);
            return v*e;                // sqrt(v) == v*rsqrt(v).
        }

        SI U32 round(F v, F scale) {
            return vcvtq_u32_f32(mad(v,scale,0.5f));
        }
    #endif


    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]};
    }
    SI void load2(const uint16_t* ptr, size_t tail, U16* r, U16* g) {
        uint16x4x2_t rg;
        if (__builtin_expect(tail,0)) {
            if (  true  ) { rg = vld2_lane_u16(ptr + 0, rg, 0); }
            if (tail > 1) { rg = vld2_lane_u16(ptr + 2, rg, 1); }
            if (tail > 2) { rg = vld2_lane_u16(ptr + 4, rg, 2); }
        } else {
            rg = vld2_u16(ptr);
        }
        *r = rg.val[0];
        *g = rg.val[1];
    }
    SI void store2(uint16_t* ptr, size_t tail, U16 r, U16 g) {
        if (__builtin_expect(tail,0)) {
            if (  true  ) { vst2_lane_u16(ptr + 0, (uint16x4x2_t{{r,g}}), 0); }
            if (tail > 1) { vst2_lane_u16(ptr + 2, (uint16x4x2_t{{r,g}}), 1); }
            if (tail > 2) { vst2_lane_u16(ptr + 4, (uint16x4x2_t{{r,g}}), 2); }
        } else {
            vst2_u16(ptr, (uint16x4x2_t{{r,g}}));
        }
    }
    SI void load3(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
        uint16x4x3_t rgb;
        if (__builtin_expect(tail,0)) {
            if (  true  ) { rgb = vld3_lane_u16(ptr + 0, rgb, 0); }
            if (tail > 1) { rgb = vld3_lane_u16(ptr + 3, rgb, 1); }
            if (tail > 2) { rgb = vld3_lane_u16(ptr + 6, rgb, 2); }
        } else {
            rgb = vld3_u16(ptr);
        }
        *r = rgb.val[0];
        *g = rgb.val[1];
        *b = rgb.val[2];
    }
    SI void load4(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        uint16x4x4_t rgba;
        if (__builtin_expect(tail,0)) {
            if (  true  ) { rgba = vld4_lane_u16(ptr + 0, rgba, 0); }
            if (tail > 1) { rgba = vld4_lane_u16(ptr + 4, rgba, 1); }
            if (tail > 2) { rgba = vld4_lane_u16(ptr + 8, rgba, 2); }
        } else {
            rgba = vld4_u16(ptr);
        }
        *r = rgba.val[0];
        *g = rgba.val[1];
        *b = rgba.val[2];
        *a = rgba.val[3];
    }

    SI void store4(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        if (__builtin_expect(tail,0)) {
            if (  true  ) { vst4_lane_u16(ptr + 0, (uint16x4x4_t{{r,g,b,a}}), 0); }
            if (tail > 1) { vst4_lane_u16(ptr + 4, (uint16x4x4_t{{r,g,b,a}}), 1); }
            if (tail > 2) { vst4_lane_u16(ptr + 8, (uint16x4x4_t{{r,g,b,a}}), 2); }
        } else {
            vst4_u16(ptr, (uint16x4x4_t{{r,g,b,a}}));
        }
    }
    SI void load2(const float* ptr, size_t tail, F* r, F* g) {
        float32x4x2_t rg;
        if (__builtin_expect(tail,0)) {
            if (  true  ) { rg = vld2q_lane_f32(ptr + 0, rg, 0); }
            if (tail > 1) { rg = vld2q_lane_f32(ptr + 2, rg, 1); }
            if (tail > 2) { rg = vld2q_lane_f32(ptr + 4, rg, 2); }
        } else {
            rg = vld2q_f32(ptr);
        }
        *r = rg.val[0];
        *g = rg.val[1];
    }
    SI void store2(float* ptr, size_t tail, F r, F g) {
        if (__builtin_expect(tail,0)) {
            if (  true  ) { vst2q_lane_f32(ptr + 0, (float32x4x2_t{{r,g}}), 0); }
            if (tail > 1) { vst2q_lane_f32(ptr + 2, (float32x4x2_t{{r,g}}), 1); }
            if (tail > 2) { vst2q_lane_f32(ptr + 4, (float32x4x2_t{{r,g}}), 2); }
        } else {
            vst2q_f32(ptr, (float32x4x2_t{{r,g}}));
        }
    }
    SI void load4(const float* ptr, size_t tail, F* r, F* g, F* b, F* a) {
        float32x4x4_t rgba;
        if (__builtin_expect(tail,0)) {
            if (  true  ) { rgba = vld4q_lane_f32(ptr + 0, rgba, 0); }
            if (tail > 1) { rgba = vld4q_lane_f32(ptr + 4, rgba, 1); }
            if (tail > 2) { rgba = vld4q_lane_f32(ptr + 8, rgba, 2); }
        } else {
            rgba = vld4q_f32(ptr);
        }
        *r = rgba.val[0];
        *g = rgba.val[1];
        *b = rgba.val[2];
        *a = rgba.val[3];
    }
    SI void store4(float* ptr, size_t tail, F r, F g, F b, F a) {
        if (__builtin_expect(tail,0)) {
            if (  true  ) { vst4q_lane_f32(ptr + 0, (float32x4x4_t{{r,g,b,a}}), 0); }
            if (tail > 1) { vst4q_lane_f32(ptr + 4, (float32x4x4_t{{r,g,b,a}}), 1); }
            if (tail > 2) { vst4q_lane_f32(ptr + 8, (float32x4x4_t{{r,g,b,a}}), 2); }
        } else {
            vst4q_f32(ptr, (float32x4x4_t{{r,g,b,a}}));
        }
    }

#elif defined(JUMPER_IS_AVX) || defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
    // These are __m256 and __m256i, but friendlier and strongly-typed.
    template <typename T> using V = T __attribute__((ext_vector_type(8)));
    using F   = V<float   >;
    using I32 = V< int32_t>;
    using U64 = V<uint64_t>;
    using U32 = V<uint32_t>;
    using U16 = V<uint16_t>;
    using U8  = V<uint8_t >;

    SI F mad(F f, F m, F a)  {
    #if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
        return _mm256_fmadd_ps(f,m,a);
    #else
        return f*m+a;
    #endif
    }

    SI F   min(F a, F b)        { return _mm256_min_ps(a,b);    }
    SI F   max(F a, F b)        { return _mm256_max_ps(a,b);    }
    SI F   abs_  (F v)          { return _mm256_and_ps(v, 0-v); }
    SI F   floor_(F v)          { return _mm256_floor_ps(v);    }
    SI F   rcp   (F v)          { return _mm256_rcp_ps  (v);    }
    SI F   rsqrt (F v)          { return _mm256_rsqrt_ps(v);    }
    SI F    sqrt_(F v)          { return _mm256_sqrt_ps (v);    }
    SI U32 round (F v, F scale) { return _mm256_cvtps_epi32(v*scale); }

    SI U16 pack(U32 v) {
        return _mm_packus_epi32(_mm256_extractf128_si256(v, 0),
                                _mm256_extractf128_si256(v, 1));
    }
    SI U8 pack(U16 v) {
        auto r = _mm_packus_epi16(v,v);
        return sk_unaligned_load<U8>(&r);
    }

    SI F if_then_else(I32 c, F t, F e) { return _mm256_blendv_ps(e,t,c); }

    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return { p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]],
                 p[ix[4]], p[ix[5]], p[ix[6]], p[ix[7]], };
    }
    #if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
        SI F   gather(const float*    p, U32 ix) { return _mm256_i32gather_ps   (p, ix, 4); }
        SI U32 gather(const uint32_t* p, U32 ix) { return _mm256_i32gather_epi32(p, ix, 4); }
        SI U64 gather(const uint64_t* p, U32 ix) {
            __m256i parts[] = {
                _mm256_i32gather_epi64(p, _mm256_extracti128_si256(ix,0), 8),
                _mm256_i32gather_epi64(p, _mm256_extracti128_si256(ix,1), 8),
            };
            return bit_cast<U64>(parts);
        }
    #endif

    SI void load2(const uint16_t* ptr, size_t tail, U16* r, U16* g) {
        U16 _0123, _4567;
        if (__builtin_expect(tail,0)) {
            _0123 = _4567 = _mm_setzero_si128();
            auto* d = &_0123;
            if (tail > 3) {
                *d = _mm_loadu_si128(((__m128i*)ptr) + 0);
                tail -= 4;
                ptr += 8;
                d = &_4567;
            }
            bool high = false;
            if (tail > 1) {
                *d = _mm_loadu_si64(ptr);
                tail -= 2;
                ptr += 4;
                high = true;
            }
            if (tail > 0) {
                (*d)[high ? 4 : 0] = *(ptr + 0);
                (*d)[high ? 5 : 1] = *(ptr + 1);
            }
        } else {
            _0123 = _mm_loadu_si128(((__m128i*)ptr) + 0);
            _4567 = _mm_loadu_si128(((__m128i*)ptr) + 1);
        }
        *r = _mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32(_0123, 16), 16),
                             _mm_srai_epi32(_mm_slli_epi32(_4567, 16), 16));
        *g = _mm_packs_epi32(_mm_srai_epi32(_0123, 16),
                             _mm_srai_epi32(_4567, 16));
    }
    SI void store2(uint16_t* ptr, size_t tail, U16 r, U16 g) {
        auto _0123 = _mm_unpacklo_epi16(r, g),
             _4567 = _mm_unpackhi_epi16(r, g);
        if (__builtin_expect(tail,0)) {
            const auto* s = &_0123;
            if (tail > 3) {
                _mm_storeu_si128((__m128i*)ptr, *s);
                s = &_4567;
                tail -= 4;
                ptr += 8;
            }
            bool high = false;
            if (tail > 1) {
                _mm_storel_epi64((__m128i*)ptr, *s);
                ptr += 4;
                tail -= 2;
                high = true;
            }
            if (tail > 0) {
                if (high) {
                    *(int32_t*)ptr = _mm_extract_epi32(*s, 2);
                } else {
                    *(int32_t*)ptr = _mm_cvtsi128_si32(*s);
                }
            }
        } else {
            _mm_storeu_si128((__m128i*)ptr + 0, _0123);
            _mm_storeu_si128((__m128i*)ptr + 1, _4567);
        }
    }

    SI void load3(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
        __m128i _0,_1,_2,_3,_4,_5,_6,_7;
        if (__builtin_expect(tail,0)) {
            auto load_rgb = [](const uint16_t* src) {
                auto v = _mm_cvtsi32_si128(*(const uint32_t*)src);
                return _mm_insert_epi16(v, src[2], 2);
            };
            _1 = _2 = _3 = _4 = _5 = _6 = _7 = _mm_setzero_si128();
            if (  true  ) { _0 = load_rgb(ptr +  0); }
            if (tail > 1) { _1 = load_rgb(ptr +  3); }
            if (tail > 2) { _2 = load_rgb(ptr +  6); }
            if (tail > 3) { _3 = load_rgb(ptr +  9); }
            if (tail > 4) { _4 = load_rgb(ptr + 12); }
            if (tail > 5) { _5 = load_rgb(ptr + 15); }
            if (tail > 6) { _6 = load_rgb(ptr + 18); }
        } else {
            // Load 0+1, 2+3, 4+5 normally, and 6+7 backed up 4 bytes so we don't run over.
            auto _01 =                _mm_loadu_si128((const __m128i*)(ptr +  0))    ;
            auto _23 =                _mm_loadu_si128((const __m128i*)(ptr +  6))    ;
            auto _45 =                _mm_loadu_si128((const __m128i*)(ptr + 12))    ;
            auto _67 = _mm_srli_si128(_mm_loadu_si128((const __m128i*)(ptr + 16)), 4);
            _0 = _01; _1 = _mm_srli_si128(_01, 6);
            _2 = _23; _3 = _mm_srli_si128(_23, 6);
            _4 = _45; _5 = _mm_srli_si128(_45, 6);
            _6 = _67; _7 = _mm_srli_si128(_67, 6);
        }

        auto _02 = _mm_unpacklo_epi16(_0, _2),  // r0 r2 g0 g2 b0 b2 xx xx
             _13 = _mm_unpacklo_epi16(_1, _3),
             _46 = _mm_unpacklo_epi16(_4, _6),
             _57 = _mm_unpacklo_epi16(_5, _7);

        auto rg0123 = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
             bx0123 = _mm_unpackhi_epi16(_02, _13),  // b0 b1 b2 b3 xx xx xx xx
             rg4567 = _mm_unpacklo_epi16(_46, _57),
             bx4567 = _mm_unpackhi_epi16(_46, _57);

        *r = _mm_unpacklo_epi64(rg0123, rg4567);
        *g = _mm_unpackhi_epi64(rg0123, rg4567);
        *b = _mm_unpacklo_epi64(bx0123, bx4567);
    }
    SI void load4(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        __m128i _01, _23, _45, _67;
        if (__builtin_expect(tail,0)) {
            auto src = (const double*)ptr;
            _01 = _23 = _45 = _67 = _mm_setzero_si128();
            if (tail > 0) { _01 = _mm_loadl_pd(_01, src+0); }
            if (tail > 1) { _01 = _mm_loadh_pd(_01, src+1); }
            if (tail > 2) { _23 = _mm_loadl_pd(_23, src+2); }
            if (tail > 3) { _23 = _mm_loadh_pd(_23, src+3); }
            if (tail > 4) { _45 = _mm_loadl_pd(_45, src+4); }
            if (tail > 5) { _45 = _mm_loadh_pd(_45, src+5); }
            if (tail > 6) { _67 = _mm_loadl_pd(_67, src+6); }
        } else {
            _01 = _mm_loadu_si128(((__m128i*)ptr) + 0);
            _23 = _mm_loadu_si128(((__m128i*)ptr) + 1);
            _45 = _mm_loadu_si128(((__m128i*)ptr) + 2);
            _67 = _mm_loadu_si128(((__m128i*)ptr) + 3);
        }

        auto _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
             _13 = _mm_unpackhi_epi16(_01, _23),  // r1 r3 g1 g3 b1 b3 a1 a3
             _46 = _mm_unpacklo_epi16(_45, _67),
             _57 = _mm_unpackhi_epi16(_45, _67);

        auto rg0123 = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
             ba0123 = _mm_unpackhi_epi16(_02, _13),  // b0 b1 b2 b3 a0 a1 a2 a3
             rg4567 = _mm_unpacklo_epi16(_46, _57),
             ba4567 = _mm_unpackhi_epi16(_46, _57);

        *r = _mm_unpacklo_epi64(rg0123, rg4567);
        *g = _mm_unpackhi_epi64(rg0123, rg4567);
        *b = _mm_unpacklo_epi64(ba0123, ba4567);
        *a = _mm_unpackhi_epi64(ba0123, ba4567);
    }
    SI void store4(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        auto rg0123 = _mm_unpacklo_epi16(r, g),  // r0 g0 r1 g1 r2 g2 r3 g3
             rg4567 = _mm_unpackhi_epi16(r, g),  // r4 g4 r5 g5 r6 g6 r7 g7
             ba0123 = _mm_unpacklo_epi16(b, a),
             ba4567 = _mm_unpackhi_epi16(b, a);

        auto _01 = _mm_unpacklo_epi32(rg0123, ba0123),
             _23 = _mm_unpackhi_epi32(rg0123, ba0123),
             _45 = _mm_unpacklo_epi32(rg4567, ba4567),
             _67 = _mm_unpackhi_epi32(rg4567, ba4567);

        if (__builtin_expect(tail,0)) {
            auto dst = (double*)ptr;
            if (tail > 0) { _mm_storel_pd(dst+0, _01); }
            if (tail > 1) { _mm_storeh_pd(dst+1, _01); }
            if (tail > 2) { _mm_storel_pd(dst+2, _23); }
            if (tail > 3) { _mm_storeh_pd(dst+3, _23); }
            if (tail > 4) { _mm_storel_pd(dst+4, _45); }
            if (tail > 5) { _mm_storeh_pd(dst+5, _45); }
            if (tail > 6) { _mm_storel_pd(dst+6, _67); }
        } else {
            _mm_storeu_si128((__m128i*)ptr + 0, _01);
            _mm_storeu_si128((__m128i*)ptr + 1, _23);
            _mm_storeu_si128((__m128i*)ptr + 2, _45);
            _mm_storeu_si128((__m128i*)ptr + 3, _67);
        }
    }

    SI void load2(const float* ptr, size_t tail, F* r, F* g) {
        F _0123, _4567;
        if (__builtin_expect(tail, 0)) {
            _0123 = _4567 = _mm256_setzero_ps();
            F* d = &_0123;
            if (tail > 3) {
                *d = _mm256_loadu_ps(ptr);
                ptr += 8;
                tail -= 4;
                d = &_4567;
            }
            bool high = false;
            if (tail > 1) {
                *d = _mm256_castps128_ps256(_mm_loadu_ps(ptr));
                ptr += 4;
                tail -= 2;
                high = true;
            }
            if (tail > 0) {
                *d = high ? _mm256_insertf128_ps(*d, _mm_loadu_si64(ptr), 1)
                          : _mm256_insertf128_ps(*d, _mm_loadu_si64(ptr), 0);
            }
        } else {
            _0123 = _mm256_loadu_ps(ptr + 0);
            _4567 = _mm256_loadu_ps(ptr + 8);
        }

        F _0145 = _mm256_permute2f128_pd(_0123, _4567, 0x20),
          _2367 = _mm256_permute2f128_pd(_0123, _4567, 0x31);

        *r = _mm256_shuffle_ps(_0145, _2367, 0x88);
        *g = _mm256_shuffle_ps(_0145, _2367, 0xDD);
    }
    SI void store2(float* ptr, size_t tail, F r, F g) {
        F _0145 = _mm256_unpacklo_ps(r, g),
          _2367 = _mm256_unpackhi_ps(r, g);
        F _0123 = _mm256_permute2f128_pd(_0145, _2367, 0x20),
          _4567 = _mm256_permute2f128_pd(_0145, _2367, 0x31);

        if (__builtin_expect(tail, 0)) {
            const __m256* s = &_0123;
            if (tail > 3) {
                _mm256_storeu_ps(ptr, *s);
                s = &_4567;
                tail -= 4;
                ptr += 8;
            }
            bool high = false;
            if (tail > 1) {
                _mm_storeu_ps(ptr, _mm256_extractf128_ps(*s, 0));
                ptr += 4;
                tail -= 2;
                high = true;
            }
            if (tail > 0) {
                *(ptr + 0) = (*s)[ high ? 4 : 0];
                *(ptr + 1) = (*s)[ high ? 5 : 1];
            }
        } else {
            _mm256_storeu_ps(ptr + 0, _0123);
            _mm256_storeu_ps(ptr + 8, _4567);
        }
    }

    SI void load4(const float* ptr, size_t tail, F* r, F* g, F* b, F* a) {
        F _04, _15, _26, _37;
        _04 = _15 = _26 = _37 = 0;
        switch (tail) {
            case 0: _37 = _mm256_insertf128_ps(_37, _mm_loadu_ps(ptr+28), 1);
            case 7: _26 = _mm256_insertf128_ps(_26, _mm_loadu_ps(ptr+24), 1);
            case 6: _15 = _mm256_insertf128_ps(_15, _mm_loadu_ps(ptr+20), 1);
            case 5: _04 = _mm256_insertf128_ps(_04, _mm_loadu_ps(ptr+16), 1);
            case 4: _37 = _mm256_insertf128_ps(_37, _mm_loadu_ps(ptr+12), 0);
            case 3: _26 = _mm256_insertf128_ps(_26, _mm_loadu_ps(ptr+ 8), 0);
            case 2: _15 = _mm256_insertf128_ps(_15, _mm_loadu_ps(ptr+ 4), 0);
            case 1: _04 = _mm256_insertf128_ps(_04, _mm_loadu_ps(ptr+ 0), 0);
        }

        F rg0145 = _mm256_unpacklo_ps(_04,_15),  // r0 r1 g0 g1 | r4 r5 g4 g5
          ba0145 = _mm256_unpackhi_ps(_04,_15),
          rg2367 = _mm256_unpacklo_ps(_26,_37),
          ba2367 = _mm256_unpackhi_ps(_26,_37);

        *r = _mm256_unpacklo_pd(rg0145, rg2367);
        *g = _mm256_unpackhi_pd(rg0145, rg2367);
        *b = _mm256_unpacklo_pd(ba0145, ba2367);
        *a = _mm256_unpackhi_pd(ba0145, ba2367);
    }
    SI void store4(float* ptr, size_t tail, F r, F g, F b, F a) {
        F rg0145 = _mm256_unpacklo_ps(r, g),  // r0 g0 r1 g1 | r4 g4 r5 g5
          rg2367 = _mm256_unpackhi_ps(r, g),  // r2 ...      | r6 ...
          ba0145 = _mm256_unpacklo_ps(b, a),  // b0 a0 b1 a1 | b4 a4 b5 a5
          ba2367 = _mm256_unpackhi_ps(b, a);  // b2 ...      | b6 ...

        F _04 = _mm256_unpacklo_pd(rg0145, ba0145),  // r0 g0 b0 a0 | r4 g4 b4 a4
          _15 = _mm256_unpackhi_pd(rg0145, ba0145),  // r1 ...      | r5 ...
          _26 = _mm256_unpacklo_pd(rg2367, ba2367),  // r2 ...      | r6 ...
          _37 = _mm256_unpackhi_pd(rg2367, ba2367);  // r3 ...      | r7 ...

        if (__builtin_expect(tail, 0)) {
            if (tail > 0) { _mm_storeu_ps(ptr+ 0, _mm256_extractf128_ps(_04, 0)); }
            if (tail > 1) { _mm_storeu_ps(ptr+ 4, _mm256_extractf128_ps(_15, 0)); }
            if (tail > 2) { _mm_storeu_ps(ptr+ 8, _mm256_extractf128_ps(_26, 0)); }
            if (tail > 3) { _mm_storeu_ps(ptr+12, _mm256_extractf128_ps(_37, 0)); }
            if (tail > 4) { _mm_storeu_ps(ptr+16, _mm256_extractf128_ps(_04, 1)); }
            if (tail > 5) { _mm_storeu_ps(ptr+20, _mm256_extractf128_ps(_15, 1)); }
            if (tail > 6) { _mm_storeu_ps(ptr+24, _mm256_extractf128_ps(_26, 1)); }
        } else {
            F _01 = _mm256_permute2f128_ps(_04, _15, 32),  // 32 == 0010 0000 == lo, lo
              _23 = _mm256_permute2f128_ps(_26, _37, 32),
              _45 = _mm256_permute2f128_ps(_04, _15, 49),  // 49 == 0011 0001 == hi, hi
              _67 = _mm256_permute2f128_ps(_26, _37, 49);
            _mm256_storeu_ps(ptr+ 0, _01);
            _mm256_storeu_ps(ptr+ 8, _23);
            _mm256_storeu_ps(ptr+16, _45);
            _mm256_storeu_ps(ptr+24, _67);
        }
    }

#elif defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41)
    template <typename T> using V = T __attribute__((ext_vector_type(4)));
    using F   = V<float   >;
    using I32 = V< int32_t>;
    using U64 = V<uint64_t>;
    using U32 = V<uint32_t>;
    using U16 = V<uint16_t>;
    using U8  = V<uint8_t >;

    SI F   mad(F f, F m, F a)  { return f*m+a;              }
    SI F   min(F a, F b)       { return _mm_min_ps(a,b);    }
    SI F   max(F a, F b)       { return _mm_max_ps(a,b);    }
    SI F   abs_(F v)           { return _mm_and_ps(v, 0-v); }
    SI F   rcp   (F v)         { return _mm_rcp_ps  (v);    }
    SI F   rsqrt (F v)         { return _mm_rsqrt_ps(v);    }
    SI F    sqrt_(F v)         { return _mm_sqrt_ps (v);    }
    SI U32 round(F v, F scale) { return _mm_cvtps_epi32(v*scale); }

    SI U16 pack(U32 v) {
    #if defined(JUMPER_IS_SSE41)
        auto p = _mm_packus_epi32(v,v);
    #else
        // Sign extend so that _mm_packs_epi32() does the pack we want.
        auto p = _mm_srai_epi32(_mm_slli_epi32(v, 16), 16);
        p = _mm_packs_epi32(p,p);
    #endif
        return sk_unaligned_load<U16>(&p);  // We have two copies.  Return (the lower) one.
    }
    SI U8 pack(U16 v) {
        auto r = widen_cast<__m128i>(v);
        r = _mm_packus_epi16(r,r);
        return sk_unaligned_load<U8>(&r);
    }

    SI F if_then_else(I32 c, F t, F e) {
        return _mm_or_ps(_mm_and_ps(c, t), _mm_andnot_ps(c, e));
    }

    SI F floor_(F v) {
    #if defined(JUMPER_IS_SSE41)
        return _mm_floor_ps(v);
    #else
        F roundtrip = _mm_cvtepi32_ps(_mm_cvttps_epi32(v));
        return roundtrip - if_then_else(roundtrip > v, 1, 0);
    #endif
    }

    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]};
    }

    SI void load2(const uint16_t* ptr, size_t tail, U16* r, U16* g) {
        __m128i _01;
        if (__builtin_expect(tail,0)) {
            _01 = _mm_setzero_si128();
            if (tail > 1) {
                _01 = _mm_loadl_pd(_01, (const double*)ptr);            // r0 g0 r1 g1 00 00 00 00
                if (tail > 2) {
                    _01 = _mm_loadh_pi(_01, (__m64 const* )(ptr + 4));  // r0 g0 r1 g1 r2 g2 00 00
                }
            } else {
                _01 = _mm_loadl_pi(_01, (__m64 const*)ptr + 0);         // r0 g0 00 00 00 00 00 00
            }
        } else {
            _01 = _mm_loadu_si128(((__m128i*)ptr) + 0);  // r0 g0 r1 g1 r2 g2 r3 g3
        }
        auto rg01_23 = _mm_shufflelo_epi16(_01, 0xD8);      // r0 r1 g0 g1 r2 g2 r3 g3
        auto rg      = _mm_shufflehi_epi16(rg01_23, 0xD8);  // r0 r1 g0 g1 r2 r3 g2 g3

        auto R = _mm_shuffle_epi32(rg, 0x88);  // r0 r1 r2 r3 r0 r1 r2 r3
        auto G = _mm_shuffle_epi32(rg, 0xDD);  // g0 g1 g2 g3 g0 g1 g2 g3
        *r = sk_unaligned_load<U16>(&R);
        *g = sk_unaligned_load<U16>(&G);
    }
    SI void store2(uint16_t* ptr, size_t tail, U16 r, U16 g) {
        U32 rg = _mm_unpacklo_epi16(widen_cast<__m128i>(r), widen_cast<__m128i>(g));
        if (__builtin_expect(tail, 0)) {
            if (tail > 1) {
                _mm_storel_epi64((__m128i*)ptr, rg);
                if (tail > 2) {
                    int32_t rgpair = rg[2];
                    memcpy(ptr + 4, &rgpair, sizeof(rgpair));
                }
            } else {
                int32_t rgpair = rg[0];
                memcpy(ptr, &rgpair, sizeof(rgpair));
            }
        } else {
            _mm_storeu_si128((__m128i*)ptr + 0, rg);
        }
    }

    SI void load3(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
        __m128i _0, _1, _2, _3;
        if (__builtin_expect(tail,0)) {
            _1 = _2 = _3 = _mm_setzero_si128();
            auto load_rgb = [](const uint16_t* src) {
                auto v = _mm_cvtsi32_si128(*(const uint32_t*)src);
                return _mm_insert_epi16(v, src[2], 2);
            };
            if (  true  ) { _0 = load_rgb(ptr + 0); }
            if (tail > 1) { _1 = load_rgb(ptr + 3); }
            if (tail > 2) { _2 = load_rgb(ptr + 6); }
        } else {
            // Load slightly weirdly to make sure we don't load past the end of 4x48 bits.
            auto _01 =                _mm_loadu_si128((const __m128i*)(ptr + 0))    ,
                 _23 = _mm_srli_si128(_mm_loadu_si128((const __m128i*)(ptr + 4)), 4);

            // Each _N holds R,G,B for pixel N in its lower 3 lanes (upper 5 are ignored).
            _0 = _01;
            _1 = _mm_srli_si128(_01, 6);
            _2 = _23;
            _3 = _mm_srli_si128(_23, 6);
        }

        // De-interlace to R,G,B.
        auto _02 = _mm_unpacklo_epi16(_0, _2),  // r0 r2 g0 g2 b0 b2 xx xx
             _13 = _mm_unpacklo_epi16(_1, _3);  // r1 r3 g1 g3 b1 b3 xx xx

        auto R = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
             G = _mm_srli_si128(R, 8),
             B = _mm_unpackhi_epi16(_02, _13);  // b0 b1 b2 b3 xx xx xx xx

        *r = sk_unaligned_load<U16>(&R);
        *g = sk_unaligned_load<U16>(&G);
        *b = sk_unaligned_load<U16>(&B);
    }

    SI void load4(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        __m128i _01, _23;
        if (__builtin_expect(tail,0)) {
            _01 = _23 = _mm_setzero_si128();
            auto src = (const double*)ptr;
            if (  true  ) { _01 = _mm_loadl_pd(_01, src + 0); } // r0 g0 b0 a0 00 00 00 00
            if (tail > 1) { _01 = _mm_loadh_pd(_01, src + 1); } // r0 g0 b0 a0 r1 g1 b1 a1
            if (tail > 2) { _23 = _mm_loadl_pd(_23, src + 2); } // r2 g2 b2 a2 00 00 00 00
        } else {
            _01 = _mm_loadu_si128(((__m128i*)ptr) + 0); // r0 g0 b0 a0 r1 g1 b1 a1
            _23 = _mm_loadu_si128(((__m128i*)ptr) + 1); // r2 g2 b2 a2 r3 g3 b3 a3
        }

        auto _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
             _13 = _mm_unpackhi_epi16(_01, _23);  // r1 r3 g1 g3 b1 b3 a1 a3

        auto rg = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
             ba = _mm_unpackhi_epi16(_02, _13);  // b0 b1 b2 b3 a0 a1 a2 a3

        *r = sk_unaligned_load<U16>((uint16_t*)&rg + 0);
        *g = sk_unaligned_load<U16>((uint16_t*)&rg + 4);
        *b = sk_unaligned_load<U16>((uint16_t*)&ba + 0);
        *a = sk_unaligned_load<U16>((uint16_t*)&ba + 4);
    }

    SI void store4(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        auto rg = _mm_unpacklo_epi16(widen_cast<__m128i>(r), widen_cast<__m128i>(g)),
             ba = _mm_unpacklo_epi16(widen_cast<__m128i>(b), widen_cast<__m128i>(a));

        if (__builtin_expect(tail, 0)) {
            auto dst = (double*)ptr;
            if (  true  ) { _mm_storel_pd(dst + 0, _mm_unpacklo_epi32(rg, ba)); }
            if (tail > 1) { _mm_storeh_pd(dst + 1, _mm_unpacklo_epi32(rg, ba)); }
            if (tail > 2) { _mm_storel_pd(dst + 2, _mm_unpackhi_epi32(rg, ba)); }
        } else {
            _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi32(rg, ba));
            _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi32(rg, ba));
        }
    }

    SI void load2(const float* ptr, size_t tail, F* r, F* g) {
        F _01, _23;
        if (__builtin_expect(tail, 0)) {
            _01 = _23 = _mm_setzero_si128();
            if (  true  ) { _01 = _mm_loadl_pi(_01, (__m64 const*)(ptr + 0)); }
            if (tail > 1) { _01 = _mm_loadh_pi(_01, (__m64 const*)(ptr + 2)); }
            if (tail > 2) { _23 = _mm_loadl_pi(_23, (__m64 const*)(ptr + 4)); }
        } else {
            _01 = _mm_loadu_ps(ptr + 0);
            _23 = _mm_loadu_ps(ptr + 4);
        }
        *r = _mm_shuffle_ps(_01, _23, 0x88);
        *g = _mm_shuffle_ps(_01, _23, 0xDD);
    }
    SI void store2(float* ptr, size_t tail, F r, F g) {
        F _01 = _mm_unpacklo_ps(r, g),
          _23 = _mm_unpackhi_ps(r, g);
        if (__builtin_expect(tail, 0)) {
            if (  true  ) { _mm_storel_pi((__m64*)(ptr + 0), _01); }
            if (tail > 1) { _mm_storeh_pi((__m64*)(ptr + 2), _01); }
            if (tail > 2) { _mm_storel_pi((__m64*)(ptr + 4), _23); }
        } else {
            _mm_storeu_ps(ptr + 0, _01);
            _mm_storeu_ps(ptr + 4, _23);
        }
    }

    SI void load4(const float* ptr, size_t tail, F* r, F* g, F* b, F* a) {
        F _0, _1, _2, _3;
        if (__builtin_expect(tail, 0)) {
            _1 = _2 = _3 = _mm_setzero_si128();
            if (  true  ) { _0 = _mm_loadu_ps(ptr + 0); }
            if (tail > 1) { _1 = _mm_loadu_ps(ptr + 4); }
            if (tail > 2) { _2 = _mm_loadu_ps(ptr + 8); }
        } else {
            _0 = _mm_loadu_ps(ptr + 0);
            _1 = _mm_loadu_ps(ptr + 4);
            _2 = _mm_loadu_ps(ptr + 8);
            _3 = _mm_loadu_ps(ptr +12);
        }
        _MM_TRANSPOSE4_PS(_0,_1,_2,_3);
        *r = _0;
        *g = _1;
        *b = _2;
        *a = _3;
    }

    SI void store4(float* ptr, size_t tail, F r, F g, F b, F a) {
        _MM_TRANSPOSE4_PS(r,g,b,a);
        if (__builtin_expect(tail, 0)) {
            if (  true  ) { _mm_storeu_ps(ptr + 0, r); }
            if (tail > 1) { _mm_storeu_ps(ptr + 4, g); }
            if (tail > 2) { _mm_storeu_ps(ptr + 8, b); }
        } else {
            _mm_storeu_ps(ptr + 0, r);
            _mm_storeu_ps(ptr + 4, g);
            _mm_storeu_ps(ptr + 8, b);
            _mm_storeu_ps(ptr +12, a);
        }
    }
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

template <typename V>
SI V if_then_else(I32 c, V t, V e) {
    return bit_cast<V>(if_then_else(c, bit_cast<F>(t), bit_cast<F>(e)));
}

SI U16 bswap(U16 x) {
#if defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41)
    // Somewhat inexplicably Clang decides to do (x<<8) | (x>>8) in 32-bit lanes
    // when generating code for SSE2 and SSE4.1.  We'll do it manually...
    auto v = widen_cast<__m128i>(x);
    v = _mm_slli_epi16(v,8) | _mm_srli_epi16(v,8);
    return sk_unaligned_load<U16>(&v);
#else
    return (x<<8) | (x>>8);
#endif
}

SI F fract(F v) { return v - floor_(v); }

// See http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html.
SI F approx_log2(F x) {
    // e - 127 is a fair approximation of log2(x) in its own right...
    F e = cast(bit_cast<U32>(x)) * (1.0f / (1<<23));

    // ... but using the mantissa to refine its error is _much_ better.
    F m = bit_cast<F>((bit_cast<U32>(x) & 0x007fffff) | 0x3f000000);
    return e
         - 124.225514990f
         -   1.498030302f * m
         -   1.725879990f / (0.3520887068f + m);
}
SI F approx_pow2(F x) {
    F f = fract(x);
    return bit_cast<F>(round(1.0f * (1<<23),
                             x + 121.274057500f
                               -   1.490129070f * f
                               +  27.728023300f / (4.84252568f - f)));
}

SI F approx_powf(F x, F y) {
#if defined(SK_LEGACY_APPROX_POWF_SPECIALCASE)
    return if_then_else((x == 0)         , 0
#else
    return if_then_else((x == 0)|(x == 1), x
#endif
                                         , approx_pow2(approx_log2(x) * y));
}

SI F from_half(U16 h) {
#if defined(SK_CPU_ARM64) && !defined(SK_BUILD_FOR_GOOGLE3)  // Temporary workaround for some Google3 builds.
    return vcvt_f32_f16(h);

#elif defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
    return _mm256_cvtph_ps(h);

#else
    // Remember, a half is 1-5-10 (sign-exponent-mantissa) with 15 exponent bias.
    U32 sem = expand(h),
        s   = sem & 0x8000,
         em = sem ^ s;

    // Convert to 1-8-23 float with 127 bias, flushing denorm halfs (including zero) to zero.
    auto denorm = (I32)em < 0x0400;      // I32 comparison is often quicker, and always safe here.
    return if_then_else(denorm, F(0)
                              , bit_cast<F>( (s<<16) + (em<<13) + ((127-15)<<23) ));
#endif
}

SI U16 to_half(F f) {
#if defined(SK_CPU_ARM64) && !defined(SK_BUILD_FOR_GOOGLE3)  // Temporary workaround for some Google3 builds.
    return vcvt_f16_f32(f);

#elif defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
    return _mm256_cvtps_ph(f, _MM_FROUND_CUR_DIRECTION);

#else
    // Remember, a float is 1-8-23 (sign-exponent-mantissa) with 127 exponent bias.
    U32 sem = bit_cast<U32>(f),
        s   = sem & 0x80000000,
         em = sem ^ s;

    // Convert to 1-5-10 half with 15 bias, flushing denorm halfs (including zero) to zero.
    auto denorm = (I32)em < 0x38800000;  // I32 comparison is often quicker, and always safe here.
    return pack(if_then_else(denorm, U32(0)
                                   , (s>>16) + (em>>13) - ((127-15)<<10)));
#endif
}

// Our fundamental vector depth is our pixel stride.
static const size_t N = sizeof(F) / sizeof(float);

// We're finally going to get to what a Stage function looks like!
//    tail == 0 ~~> work on a full N pixels
//    tail != 0 ~~> work on only the first tail pixels
// tail is always < N.

// Any custom ABI to use for all (non-externally-facing) stage functions?
// Also decide here whether to use narrow (compromise) or wide (ideal) stages.
#if defined(SK_CPU_ARM32) && defined(JUMPER_IS_NEON)
    // This lets us pass vectors more efficiently on 32-bit ARM.
    // We can still only pass 16 floats, so best as 4x {r,g,b,a}.
    #define ABI __attribute__((pcs("aapcs-vfp")))
    #define JUMPER_NARROW_STAGES 1
#elif 0 && defined(_MSC_VER) && defined(__clang__) && defined(__x86_64__)
    // SysV ABI makes it very sensible to use wide stages with clang-cl.
    // TODO: crashes during compilation  :(
    #define ABI __attribute__((sysv_abi))
    #define JUMPER_NARROW_STAGES 0
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
        size_t dx, dy, tail;
        F dr,dg,db,da;
    };
    using Stage = void(ABI*)(Params*, void** program, F r, F g, F b, F a);
#else
    // We keep program the second argument, so that it's passed in rsi for load_and_inc().
    using Stage = void(ABI*)(size_t tail, void** program, size_t dx, size_t dy, F,F,F,F, F,F,F,F);
#endif


static void start_pipeline(size_t dx, size_t dy, size_t xlimit, size_t ylimit, void** program) {
    auto start = (Stage)load_and_inc(program);
    const size_t x0 = dx;
    for (; dy < ylimit; dy++) {
    #if JUMPER_NARROW_STAGES
        Params params = { x0,dy,0, 0,0,0,0 };
        while (params.dx + N <= xlimit) {
            start(&params,program, 0,0,0,0);
            params.dx += N;
        }
        if (size_t tail = xlimit - params.dx) {
            params.tail = tail;
            start(&params,program, 0,0,0,0);
        }
    #else
        dx = x0;
        while (dx + N <= xlimit) {
            start(0,program,dx,dy,    0,0,0,0, 0,0,0,0);
            dx += N;
        }
        if (size_t tail = xlimit - dx) {
            start(tail,program,dx,dy, 0,0,0,0, 0,0,0,0);
        }
    #endif
    }
}

#if JUMPER_NARROW_STAGES
    #define STAGE(name, ...)                                                    \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,        \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);   \
        static void ABI name(Params* params, void** program,                    \
                             F r, F g, F b, F a) {                              \
            name##_k(Ctx{program},params->dx,params->dy,params->tail, r,g,b,a,  \
                     params->dr, params->dg, params->db, params->da);           \
            auto next = (Stage)load_and_inc(program);                           \
            next(params,program, r,g,b,a);                                      \
        }                                                                       \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,        \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#else
    #define STAGE(name, ...)                                                         \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,             \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);        \
        static void ABI name(size_t tail, void** program, size_t dx, size_t dy,      \
                             F r, F g, F b, F a, F dr, F dg, F db, F da) {           \
            name##_k(Ctx{program},dx,dy,tail, r,g,b,a, dr,dg,db,da);                 \
            auto next = (Stage)load_and_inc(program);                                \
            next(tail,program,dx,dy, r,g,b,a, dr,dg,db,da);                          \
        }                                                                            \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,             \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#endif


// just_return() is a simple no-op stage that only exists to end the chain,
// returning back up to start_pipeline(), and from there to the caller.
#if JUMPER_NARROW_STAGES
    static void ABI just_return(Params*, void**, F,F,F,F) {}
#else
    static void ABI just_return(size_t, void**, size_t,size_t, F,F,F,F, F,F,F,F) {}
#endif


// We could start defining normal Stages now.  But first, some helper functions.

// These load() and store() methods are tail-aware,
// but focus mainly on keeping the at-stride tail==0 case fast.

template <typename V, typename T>
SI V load(const T* src, size_t tail) {
#if !defined(JUMPER_IS_SCALAR)
    __builtin_assume(tail < N);
    if (__builtin_expect(tail, 0)) {
        V v{};  // Any inactive lanes are zeroed.
        switch (tail) {
            case 7: v[6] = src[6];
            case 6: v[5] = src[5];
            case 5: v[4] = src[4];
            case 4: memcpy(&v, src, 4*sizeof(T)); break;
            case 3: v[2] = src[2];
            case 2: memcpy(&v, src, 2*sizeof(T)); break;
            case 1: memcpy(&v, src, 1*sizeof(T)); break;
        }
        return v;
    }
#endif
    return sk_unaligned_load<V>(src);
}

template <typename V, typename T>
SI void store(T* dst, V v, size_t tail) {
#if !defined(JUMPER_IS_SCALAR)
    __builtin_assume(tail < N);
    if (__builtin_expect(tail, 0)) {
        switch (tail) {
            case 7: dst[6] = v[6];
            case 6: dst[5] = v[5];
            case 5: dst[4] = v[4];
            case 4: memcpy(dst, &v, 4*sizeof(T)); break;
            case 3: dst[2] = v[2];
            case 2: memcpy(dst, &v, 2*sizeof(T)); break;
            case 1: memcpy(dst, &v, 1*sizeof(T)); break;
        }
        return;
    }
#endif
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
    F inclusive = bit_cast<F>( bit_cast<U32>(limit) - 1 );  // Exclusive -> inclusive.
    return min(max(0, v), inclusive);
}

// Used by gather_ stages to calculate the base pointer and a vector of indices to load.
template <typename T>
SI U32 ix_and_ptr(T** ptr, const SkRasterPipeline_GatherCtx* ctx, F x, F y) {
    x = clamp(x, ctx->width);
    y = clamp(y, ctx->height);

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
SI U32 to_unorm(F v, F scale, F bias = 1.0f) {
    // TODO: platform-specific implementations to to_unorm(), removing round() entirely?
    // Any time we use round() we probably want to use to_unorm().
    return round(min(max(0, v), bias), scale);
}

SI I32 cond_to_mask(I32 cond) { return if_then_else(cond, I32(~0), I32(0)); }

// Now finally, normal Stages!

STAGE(seed_shader, Ctx::None) {
    static const float iota[] = {
        0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f,
        8.5f, 9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f,
    };
    // It's important for speed to explicitly cast(dx) and cast(dy),
    // which has the effect of splatting them to vectors before converting to floats.
    // On Intel this breaks a data dependency on previous loop iterations' registers.
    r = cast(dx) + sk_unaligned_load<F>(iota);
    g = cast(dy) + 0.5f;
    b = 1.0f;
    a = 0;
    dr = dg = db = da = 0;
}

STAGE(dither, const float* rate) {
    // Get [(dx,dy), (dx+1,dy), (dx+2,dy), ...] loaded up in integer vectors.
    uint32_t iota[] = {0,1,2,3,4,5,6,7};
    U32 X = dx + sk_unaligned_load<U32>(iota),
        Y = dy;

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

    r = max(0, min(r, a));
    g = max(0, min(g, a));
    b = max(0, min(b, a));
}

// load 4 floats from memory, and splat them into r,g,b,a
STAGE(uniform_color, const SkRasterPipeline_UniformColorCtx* c) {
    r = c->r;
    g = c->g;
    b = c->b;
    a = c->a;
}
STAGE(unbounded_uniform_color, const SkRasterPipeline_UniformColorCtx* c) {
    r = c->r;
    g = c->g;
    b = c->b;
    a = c->a;
}

// splats opaque-black into r,g,b,a
STAGE(black_color, Ctx::None) {
    r = g = b = 0.0f;
    a = 1.0f;
}

STAGE(white_color, Ctx::None) {
    r = g = b = a = 1.0f;
}

// load registers r,g,b,a from context (mirrors store_rgba)
STAGE(load_src, const float* ptr) {
    r = sk_unaligned_load<F>(ptr + 0*N);
    g = sk_unaligned_load<F>(ptr + 1*N);
    b = sk_unaligned_load<F>(ptr + 2*N);
    a = sk_unaligned_load<F>(ptr + 3*N);
}

// store registers r,g,b,a into context (mirrors load_rgba)
STAGE(store_src, float* ptr) {
    sk_unaligned_store(ptr + 0*N, r);
    sk_unaligned_store(ptr + 1*N, g);
    sk_unaligned_store(ptr + 2*N, b);
    sk_unaligned_store(ptr + 3*N, a);
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
    STAGE(name, Ctx::None) {                   \
        r = name##_channel(r,dr,a,da);         \
        g = name##_channel(g,dg,a,da);         \
        b = name##_channel(b,db,a,da);         \
        a = name##_channel(a,da,a,da);         \
    }                                          \
    SI F name##_channel(F s, F d, F sa, F da)

SI F inv(F x) { return 1.0f - x; }
SI F two(F x) { return x + x; }


BLEND_MODE(clear)    { return 0; }
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
    STAGE(name, Ctx::None) {                   \
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
                                 sa*(da - min(da, (da-d)*sa*rcp(s))) + s*inv(da) + d*inv(sa)));
}
BLEND_MODE(colordodge) {
    return if_then_else(d ==  0, /* d + */ s*inv(da),
           if_then_else(s == sa,    s +    d*inv(sa),
                                 sa*min(da, (d*sa)*rcp(sa - s)) + s*inv(da) + d*inv(sa)));
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
    F m  = if_then_else(da > 0, d / da, 0),
      s2 = two(s),
      m4 = two(two(m));

    // The logic forks three ways:
    //    1. dark src?
    //    2. light src, dark dst?
    //    3. light src, light dst?
    F darkSrc = d*(sa + (s2 - sa)*(1.0f - m)),     // Used in case 1.
      darkDst = (m4*m4 + m4)*(m - 1.0f) + 7.0f*m,  // Used in case 2.
      liteDst = rcp(rsqrt(m)) - m,                 // Used in case 3.
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

SI F max(F r, F g, F b) { return max(r, max(g, b)); }
SI F min(F r, F g, F b) { return min(r, min(g, b)); }

SI F sat(F r, F g, F b) { return max(r,g,b) - min(r,g,b); }
SI F lum(F r, F g, F b) { return r*0.30f + g*0.59f + b*0.11f; }

SI void set_sat(F* r, F* g, F* b, F s) {
    F mn  = min(*r,*g,*b),
      mx  = max(*r,*g,*b),
      sat = mx - mn;

    // Map min channel to 0, max channel to s, and scale the middle proportionally.
    auto scale = [=](F c) {
        return if_then_else(sat == 0, 0, (c - mn) * s / sat);
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
    F mn = min(*r, *g, *b),
      mx = max(*r, *g, *b),
      l  = lum(*r, *g, *b);

    auto clip = [=](F c) {
        c = if_then_else(mn >= 0, c, l + (c - l) * (    l) / (l - mn)   );
        c = if_then_else(mx >  a,    l + (c - l) * (a - l) / (mx - l), c);
        c = max(c, 0);  // Sometimes without this we may dip just a little negative.
        return c;
    };
    *r = clip(*r);
    *g = clip(*g);
    *b = clip(*b);
}

STAGE(hue, Ctx::None) {
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
STAGE(saturation, Ctx::None) {
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
STAGE(color, Ctx::None) {
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
STAGE(luminosity, Ctx::None) {
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

    U32 dst = load<U32>(ptr, tail);
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
    store(ptr, dst, tail);
}

STAGE(clamp_0, Ctx::None) {
    r = max(r, 0);
    g = max(g, 0);
    b = max(b, 0);
    a = max(a, 0);
}

STAGE(clamp_1, Ctx::None) {
    r = min(r, 1.0f);
    g = min(g, 1.0f);
    b = min(b, 1.0f);
    a = min(a, 1.0f);
}

STAGE(clamp_a, Ctx::None) {
    a = min(a, 1.0f);
    r = min(r, a);
    g = min(g, a);
    b = min(b, a);
}

STAGE(clamp_gamut, Ctx::None) {
    // If you're using this stage, a should already be in [0,1].
    r = min(max(r, 0), a);
    g = min(max(g, 0), a);
    b = min(max(b, 0), a);
}

STAGE(set_rgb, const float* rgb) {
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];
}
STAGE(unbounded_set_rgb, const float* rgb) {
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];
}

STAGE(swap_rb, Ctx::None) {
    auto tmp = r;
    r = b;
    b = tmp;
}
STAGE(swap_rb_dst, Ctx::None) {
    auto tmp = dr;
    dr = db;
    db = tmp;
}

STAGE(move_src_dst, Ctx::None) {
    dr = r;
    dg = g;
    db = b;
    da = a;
}
STAGE(move_dst_src, Ctx::None) {
    r = dr;
    g = dg;
    b = db;
    a = da;
}

STAGE(premul, Ctx::None) {
    r = r * a;
    g = g * a;
    b = b * a;
}
STAGE(premul_dst, Ctx::None) {
    dr = dr * da;
    dg = dg * da;
    db = db * da;
}
STAGE(unpremul, Ctx::None) {
    float inf = bit_cast<float>(0x7f800000);
    auto scale = if_then_else(1.0f/a < inf, 1.0f/a, 0);
    r *= scale;
    g *= scale;
    b *= scale;
}

STAGE(force_opaque    , Ctx::None) {  a = 1; }
STAGE(force_opaque_dst, Ctx::None) { da = 1; }

STAGE(rgb_to_hsl, Ctx::None) {
    F mx = max(r,g,b),
      mn = min(r,g,b),
      d = mx - mn,
      d_rcp = 1.0f / d;

    F h = (1/6.0f) *
          if_then_else(mx == mn, 0,
          if_then_else(mx ==  r, (g-b)*d_rcp + if_then_else(g < b, 6.0f, 0),
          if_then_else(mx ==  g, (b-r)*d_rcp + 2.0f,
                                 (r-g)*d_rcp + 4.0f)));

    F l = (mx + mn) * 0.5f;
    F s = if_then_else(mx == mn, 0,
                       d / if_then_else(l > 0.5f, 2.0f-mx-mn, mx+mn));

    r = h;
    g = s;
    b = l;
}
STAGE(hsl_to_rgb, Ctx::None) {
    F h = r,
      s = g,
      l = b;

    F q = l + if_then_else(l >= 0.5f, s - l*s, l*s),
      p = 2.0f*l - q;

    auto hue_to_rgb = [&](F t) {
        t = fract(t);

        F r = p;
        r = if_then_else(t >= 4/6.0f, r, p + (q-p)*(4.0f - 6.0f*t));
        r = if_then_else(t >= 3/6.0f, r, q);
        r = if_then_else(t >= 1/6.0f, r, p + (q-p)*(       6.0f*t));
        return r;
    };

    r = if_then_else(s == 0, l, hue_to_rgb(h + (1/3.0f)));
    g = if_then_else(s == 0, l, hue_to_rgb(h           ));
    b = if_then_else(s == 0, l, hue_to_rgb(h - (1/3.0f)));
}

// Derive alpha's coverage from rgb coverage and the values of src and dst alpha.
SI F alpha_coverage_from_rgb_coverage(F a, F da, F cr, F cg, F cb) {
    return if_then_else(a < da, min(cr,cg,cb)
                              , max(cr,cg,cb));
}

STAGE(scale_1_float, const float* c) {
    r = r * *c;
    g = g * *c;
    b = b * *c;
    a = a * *c;
}
STAGE(scale_u8, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    auto scales = load<U8>(ptr, tail);
    auto c = from_byte(scales);

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(scale_565, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    F cr,cg,cb;
    from_565(load<U16>(ptr, tail), &cr, &cg, &cb);

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
    r = lerp(dr, r, *c);
    g = lerp(dg, g, *c);
    b = lerp(db, b, *c);
    a = lerp(da, a, *c);
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

    auto scales = load<U8>(ptr, tail);
    auto c = from_byte(scales);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_565, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    F cr,cg,cb;
    from_565(load<U16>(ptr, tail), &cr, &cg, &cb);

    F ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = lerp(da, a, ca);
}

STAGE(emboss, const SkRasterPipeline_EmbossCtx* ctx) {
    auto mptr = ptr_at_xy<const uint8_t>(&ctx->mul, dx,dy),
         aptr = ptr_at_xy<const uint8_t>(&ctx->add, dx,dy);

    F mul = from_byte(load<U8>(mptr, tail)),
      add = from_byte(load<U8>(aptr, tail));

    r = mad(r, mul, add);
    g = mad(g, mul, add);
    b = mad(b, mul, add);
}

STAGE(byte_tables, const void* ctx) {  // TODO: rename Tables SkRasterPipeline_ByteTablesCtx
    struct Tables { const uint8_t *r, *g, *b, *a; };
    auto tables = (const Tables*)ctx;

    r = from_byte(gather(tables->r, to_unorm(r, 255)));
    g = from_byte(gather(tables->g, to_unorm(g, 255)));
    b = from_byte(gather(tables->b, to_unorm(b, 255)));
    a = from_byte(gather(tables->a, to_unorm(a, 255)));
}

SI F strip_sign(F x, U32* sign) {
    U32 bits = bit_cast<U32>(x);
    *sign = bits & 0x80000000;
    return bit_cast<F>(bits ^ *sign);
}

SI F apply_sign(F x, U32 sign) {
    return bit_cast<F>(sign | bit_cast<U32>(x));
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

STAGE(from_srgb, Ctx::None) {
    auto fn = [](F s) {
        U32 sign;
        s = strip_sign(s, &sign);
        auto lo = s * (1/12.92f);
        auto hi = mad(s*s, mad(s, 0.3000f, 0.6975f), 0.0025f);
        return apply_sign(if_then_else(s < 0.055f, lo, hi), sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}
STAGE(to_srgb, Ctx::None) {
    auto fn = [](F l) {
        U32 sign;
        l = strip_sign(l, &sign);
        // We tweak c and d for each instruction set to make sure fn(1) is exactly 1.
    #if defined(JUMPER_IS_AVX512)
        const float c = 1.130026340485f,
                    d = 0.141387879848f;
    #elif defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41) || \
          defined(JUMPER_IS_AVX ) || defined(JUMPER_IS_HSW )
        const float c = 1.130048394203f,
                    d = 0.141357362270f;
    #elif defined(JUMPER_IS_NEON)
        const float c = 1.129999995232f,
                    d = 0.141381442547f;
    #else
        const float c = 1.129999995232f,
                    d = 0.141377761960f;
    #endif
        F t = rsqrt(l);
        auto lo = l * 12.92f;
        auto hi = mad(t, mad(t, -0.0024542345f, 0.013832027f), c)
                * rcp(d + t);
        return apply_sign(if_then_else(l < 0.00465985f, lo, hi), sign);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(load_a8, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    r = g = b = 0.0f;
    a = from_byte(load<U8>(ptr, tail));
}
STAGE(load_a8_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    dr = dg = db = 0.0f;
    da = from_byte(load<U8>(ptr, tail));
}
STAGE(gather_a8, const SkRasterPipeline_GatherCtx* ctx) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = g = b = 0.0f;
    a = from_byte(gather(ptr, ix));
}
STAGE(store_a8, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint8_t>(ctx, dx,dy);

    U8 packed = pack(pack(to_unorm(a, 255)));
    store(ptr, packed, tail);
}

STAGE(load_565, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    from_565(load<U16>(ptr, tail), &r,&g,&b);
    a = 1.0f;
}
STAGE(load_565_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    from_565(load<U16>(ptr, tail), &dr,&dg,&db);
    da = 1.0f;
}
STAGE(gather_565, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_565(gather(ptr, ix), &r,&g,&b);
    a = 1.0f;
}
STAGE(store_565, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);

    U16 px = pack( to_unorm(r, 31) << 11
                 | to_unorm(g, 63) <<  5
                 | to_unorm(b, 31)      );
    store(ptr, px, tail);
}

STAGE(load_4444, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    from_4444(load<U16>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_4444_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    from_4444(load<U16>(ptr, tail), &dr,&dg,&db,&da);
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
    store(ptr, px, tail);
}

STAGE(load_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_8888(load<U32>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_8888_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_8888(load<U32>(ptr, tail), &dr,&dg,&db,&da);
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
    store(ptr, px, tail);
}

STAGE(load_rg88, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    b = 0;
    a = 1;
    from_88(load<U16>(ptr, tail), &r,&g);
}
STAGE(store_rg88, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);

    U16 px = pack( to_unorm(r, 255)
                 | to_unorm(g, 255) <<  8);
    store(ptr, px, tail);
}

STAGE(load_a16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    r = g = b = 0;
    a = from_short(load<U16>(ptr, tail));
}
STAGE(store_a16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);

    U16 px = pack(to_unorm(a, 65535));
    store(ptr, px, tail);
}
STAGE(load_rg1616, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    b = 0; a = 1;
    from_1616(load<U32>(ptr, tail), &r,&g);
}
STAGE(store_rg1616, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 px = to_unorm(r, 65535)
           | to_unorm(g, 65535) <<  16;
    store(ptr, px, tail);
}
STAGE(load_16161616, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx,dy);
    from_16161616(load<U64>(ptr, tail), &r,&g, &b, &a);
}
STAGE(store_16161616, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, 4*dx,4*dy);

    U16 R = pack(to_unorm(r, 65535)),
        G = pack(to_unorm(g, 65535)),
        B = pack(to_unorm(b, 65535)),
        A = pack(to_unorm(a, 65535));

    store4(ptr,tail, R,G,B,A);
}


STAGE(load_1010102, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_1010102(load<U32>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_1010102_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_1010102(load<U32>(ptr, tail), &dr,&dg,&db,&da);
}
STAGE(gather_1010102, const SkRasterPipeline_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_1010102(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_1010102, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 px = to_unorm(r, 1023)
           | to_unorm(g, 1023) << 10
           | to_unorm(b, 1023) << 20
           | to_unorm(a,    3) << 30;
    store(ptr, px, tail);
}

STAGE(load_f16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx,dy);

    U16 R,G,B,A;
    load4((const uint16_t*)ptr,tail, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}
STAGE(load_f16_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx,dy);

    U16 R,G,B,A;
    load4((const uint16_t*)ptr,tail, &R,&G,&B,&A);
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
    load4((const uint16_t*)&px,0, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}
STAGE(store_f16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint64_t>(ctx, dx,dy);
    store4((uint16_t*)ptr,tail, to_half(r)
                              , to_half(g)
                              , to_half(b)
                              , to_half(a));
}

STAGE(store_u16_be, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, 4*dx,dy);

    U16 R = bswap(pack(to_unorm(r, 65535))),
        G = bswap(pack(to_unorm(g, 65535))),
        B = bswap(pack(to_unorm(b, 65535))),
        A = bswap(pack(to_unorm(a, 65535)));

    store4(ptr,tail, R,G,B,A);
}

STAGE(load_af16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    U16 A = load<U16>((const uint16_t*)ptr, tail);
    r = 0;
    g = 0;
    b = 0;
    a = from_half(A);
}
STAGE(store_af16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);
    store(ptr, to_half(a), tail);
}

STAGE(load_rgf16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);

    U16 R,G;
    load2((const uint16_t*)ptr,tail, &R,&G);
    r = from_half(R);
    g = from_half(G);
    b = 0;
    a = from_half(0x3C00); // one
}
STAGE(store_rgf16, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);
    store2((uint16_t*)ptr, tail, to_half(r)
                               , to_half(g));
}

STAGE(load_f32, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const float>(ctx, 4*dx,4*dy);
    load4(ptr,tail, &r,&g,&b,&a);
}
STAGE(load_f32_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const float>(ctx, 4*dx,4*dy);
    load4(ptr,tail, &dr,&dg,&db,&da);
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
    store4(ptr,tail, r,g,b,a);
}

STAGE(load_rgf32, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const float>(ctx, 2*dx,2*dy);
    load2(ptr, tail, &r, &g);
    b = 0;
    a = 1;
}
STAGE(store_rgf32, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<float>(ctx, 2*dx,2*dy);
    store2(ptr, tail, r, g);
}

SI F exclusive_repeat(F v, const SkRasterPipeline_TileCtx* ctx) {
    return v - floor_(v*ctx->invScale)*ctx->scale;
}
SI F exclusive_mirror(F v, const SkRasterPipeline_TileCtx* ctx) {
    auto limit = ctx->scale;
    auto invLimit = ctx->invScale;
    return abs_( (v-limit) - (limit+limit)*floor_((v-limit)*(invLimit*0.5f)) - limit );
}
// Tile x or y to [0,limit) == [0,limit - 1 ulp] (think, sampling from images).
// The gather stages will hard clamp the output of these stages to [0,limit)...
// we just need to do the basic repeat or mirroring.
STAGE(repeat_x, const SkRasterPipeline_TileCtx* ctx) { r = exclusive_repeat(r, ctx); }
STAGE(repeat_y, const SkRasterPipeline_TileCtx* ctx) { g = exclusive_repeat(g, ctx); }
STAGE(mirror_x, const SkRasterPipeline_TileCtx* ctx) { r = exclusive_mirror(r, ctx); }
STAGE(mirror_y, const SkRasterPipeline_TileCtx* ctx) { g = exclusive_mirror(g, ctx); }

// Clamp x to [0,1], both sides inclusive (think, gradients).
// Even repeat and mirror funnel through a clamp to handle bad inputs like +Inf, NaN.
SI F clamp_01(F v) { return min(max(0, v), 1); }

STAGE( clamp_x_1, Ctx::None) { r = clamp_01(r); }
STAGE(repeat_x_1, Ctx::None) { r = clamp_01(r - floor_(r)); }
STAGE(mirror_x_1, Ctx::None) { r = clamp_01(abs_( (r-1.0f) - two(floor_((r-1.0f)*0.5f)) - 1.0f )); }

// Decal stores a 32bit mask after checking the coordinate (x and/or y) against its domain:
//      mask == 0x00000000 if the coordinate(s) are out of bounds
//      mask == 0xFFFFFFFF if the coordinate(s) are in bounds
// After the gather stage, the r,g,b,a values are AND'd with this mask, setting them to 0
// if either of the coordinates were out of bounds.

STAGE(decal_x, SkRasterPipeline_DecalTileCtx* ctx) {
    auto w = ctx->limit_x;
    sk_unaligned_store(ctx->mask, cond_to_mask((0 <= r) & (r < w)));
}
STAGE(decal_y, SkRasterPipeline_DecalTileCtx* ctx) {
    auto h = ctx->limit_y;
    sk_unaligned_store(ctx->mask, cond_to_mask((0 <= g) & (g < h)));
}
STAGE(decal_x_and_y, SkRasterPipeline_DecalTileCtx* ctx) {
    auto w = ctx->limit_x;
    auto h = ctx->limit_y;
    sk_unaligned_store(ctx->mask,
                    cond_to_mask((0 <= r) & (r < w) & (0 <= g) & (g < h)));
}
STAGE(check_decal_mask, SkRasterPipeline_DecalTileCtx* ctx) {
    auto mask = sk_unaligned_load<U32>(ctx->mask);
    r = bit_cast<F>( bit_cast<U32>(r) & mask );
    g = bit_cast<F>( bit_cast<U32>(g) & mask );
    b = bit_cast<F>( bit_cast<U32>(b) & mask );
    a = bit_cast<F>( bit_cast<U32>(a) & mask );
}

STAGE(alpha_to_gray, Ctx::None) {
    r = g = b = a;
    a = 1;
}
STAGE(alpha_to_gray_dst, Ctx::None) {
    dr = dg = db = da;
    da = 1;
}
STAGE(luminance_to_alpha, Ctx::None) {
    a = r*0.2126f + g*0.7152f + b*0.0722f;
    r = g = b = 0;
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
    auto R = mad(r,m[0], mad(g,m[2], m[4])),
         G = mad(r,m[1], mad(g,m[3], m[5]));
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
    r = R * rcp(Z);
    g = G * rcp(Z);
}

SI void gradient_lookup(const SkRasterPipeline_GradientCtx* c, U32 idx, F t,
                        F* r, F* g, F* b, F* a) {
    F fr, br, fg, bg, fb, bb, fa, ba;
#if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
    if (c->stopCount <=8) {
        fr = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[0]), idx);
        br = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[0]), idx);
        fg = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[1]), idx);
        bg = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[1]), idx);
        fb = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[2]), idx);
        bb = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[2]), idx);
        fa = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[3]), idx);
        ba = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[3]), idx);
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
    auto idx = trunc_(t * (c->stopCount-1));
    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE(gradient, const SkRasterPipeline_GradientCtx* c) {
    auto t = r;
    U32 idx = 0;

    // N.B. The loop starts at 1 because idx 0 is the color to use before the first stop.
    for (size_t i = 1; i < c->stopCount; i++) {
        idx += if_then_else(t >= c->ts[i], U32(1), U32(0));
    }

    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE(evenly_spaced_2_stop_gradient, const void* ctx) {
    // TODO: Rename Ctx SkRasterPipeline_EvenlySpaced2StopGradientCtx.
    struct Ctx { float f[4], b[4]; };
    auto c = (const Ctx*)ctx;

    auto t = r;
    r = mad(t, c->f[0], c->b[0]);
    g = mad(t, c->f[1], c->b[1]);
    b = mad(t, c->f[2], c->b[2]);
    a = mad(t, c->f[3], c->b[3]);
}

STAGE(xy_to_unit_angle, Ctx::None) {
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
    phi = if_then_else(phi != phi , 0              , phi);  // Check for NaN.
    r = phi;
}

STAGE(xy_to_radius, Ctx::None) {
    F X2 = r * r,
      Y2 = g * g;
    r = sqrt_(X2 + Y2);
}

// Please see https://skia.org/dev/design/conical for how our 2pt conical shader works.

STAGE(negate_x, Ctx::None) { r = -r; }

STAGE(xy_to_2pt_conical_strip, const SkRasterPipeline_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = x + sqrt_(ctx->fP0 - y*y); // ctx->fP0 = r0 * r0
}

STAGE(xy_to_2pt_conical_focal_on_circle, Ctx::None) {
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

STAGE(alter_2pt_conical_unswap, Ctx::None) {
    F& t = r;
    t = 1 - t;
}

STAGE(mask_2pt_conical_nan, SkRasterPipeline_2PtConicalCtx* c) {
    F& t = r;
    auto is_degenerate = (t != t); // NaN
    t = if_then_else(is_degenerate, F(0), t);
    sk_unaligned_store(&c->fMask, cond_to_mask(!is_degenerate));
}

STAGE(mask_2pt_conical_degenerates, SkRasterPipeline_2PtConicalCtx* c) {
    F& t = r;
    auto is_degenerate = (t <= 0) | (t != t);
    t = if_then_else(is_degenerate, F(0), t);
    sk_unaligned_store(&c->fMask, cond_to_mask(!is_degenerate));
}

STAGE(apply_vector_mask, const uint32_t* ctx) {
    const U32 mask = sk_unaligned_load<U32>(ctx);
    r = bit_cast<F>(bit_cast<U32>(r) & mask);
    g = bit_cast<F>(bit_cast<U32>(g) & mask);
    b = bit_cast<F>(bit_cast<U32>(b) & mask);
    a = bit_cast<F>(bit_cast<U32>(a) & mask);
}

STAGE(save_xy, SkRasterPipeline_SamplerCtx* c) {
    // Whether bilinear or bicubic, all sample points are at the same fractional offset (fx,fy).
    // They're either the 4 corners of a logical 1x1 pixel or the 16 corners of a 3x3 grid
    // surrounding (x,y) at (0.5,0.5) off-center.
    F fx = fract(r + 0.5f),
      fy = fract(g + 0.5f);

    // Samplers will need to load x and fx, or y and fy.
    sk_unaligned_store(c->x,  r);
    sk_unaligned_store(c->y,  g);
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

STAGE(bilinear_nx, SkRasterPipeline_SamplerCtx* ctx) { bilinear_x<-1>(ctx, &r); }
STAGE(bilinear_px, SkRasterPipeline_SamplerCtx* ctx) { bilinear_x<+1>(ctx, &r); }
STAGE(bilinear_ny, SkRasterPipeline_SamplerCtx* ctx) { bilinear_y<-1>(ctx, &g); }
STAGE(bilinear_py, SkRasterPipeline_SamplerCtx* ctx) { bilinear_y<+1>(ctx, &g); }


// In bicubic interpolation, the 16 pixels and +/- 0.5 and +/- 1.5 offsets from the sample
// pixel center are combined with a non-uniform cubic filter, with higher values near the center.
//
// We break this function into two parts, one for near 0.5 offsets and one for far 1.5 offsets.
// See GrCubicEffect for details of this particular filter.

SI F bicubic_near(F t) {
    // 1/18 + 9/18t + 27/18t^2 - 21/18t^3 == t ( t ( -21/18t + 27/18) + 9/18) + 1/18
    return mad(t, mad(t, mad((-21/18.0f), t, (27/18.0f)), (9/18.0f)), (1/18.0f));
}
SI F bicubic_far(F t) {
    // 0/18 + 0/18*t - 6/18t^2 + 7/18t^3 == t^2 (7/18t - 6/18)
    return (t*t)*mad((7/18.0f), t, (-6/18.0f));
}

template <int kScale>
SI void bicubic_x(SkRasterPipeline_SamplerCtx* ctx, F* x) {
    *x = sk_unaligned_load<F>(ctx->x) + (kScale * 0.5f);
    F fx = sk_unaligned_load<F>(ctx->fx);

    F scalex;
    if (kScale == -3) { scalex = bicubic_far (1.0f - fx); }
    if (kScale == -1) { scalex = bicubic_near(1.0f - fx); }
    if (kScale == +1) { scalex = bicubic_near(       fx); }
    if (kScale == +3) { scalex = bicubic_far (       fx); }
    sk_unaligned_store(ctx->scalex, scalex);
}
template <int kScale>
SI void bicubic_y(SkRasterPipeline_SamplerCtx* ctx, F* y) {
    *y = sk_unaligned_load<F>(ctx->y) + (kScale * 0.5f);
    F fy = sk_unaligned_load<F>(ctx->fy);

    F scaley;
    if (kScale == -3) { scaley = bicubic_far (1.0f - fy); }
    if (kScale == -1) { scaley = bicubic_near(1.0f - fy); }
    if (kScale == +1) { scaley = bicubic_near(       fy); }
    if (kScale == +3) { scaley = bicubic_far (       fy); }
    sk_unaligned_store(ctx->scaley, scaley);
}

STAGE(bicubic_n3x, SkRasterPipeline_SamplerCtx* ctx) { bicubic_x<-3>(ctx, &r); }
STAGE(bicubic_n1x, SkRasterPipeline_SamplerCtx* ctx) { bicubic_x<-1>(ctx, &r); }
STAGE(bicubic_p1x, SkRasterPipeline_SamplerCtx* ctx) { bicubic_x<+1>(ctx, &r); }
STAGE(bicubic_p3x, SkRasterPipeline_SamplerCtx* ctx) { bicubic_x<+3>(ctx, &r); }

STAGE(bicubic_n3y, SkRasterPipeline_SamplerCtx* ctx) { bicubic_y<-3>(ctx, &g); }
STAGE(bicubic_n1y, SkRasterPipeline_SamplerCtx* ctx) { bicubic_y<-1>(ctx, &g); }
STAGE(bicubic_p1y, SkRasterPipeline_SamplerCtx* ctx) { bicubic_y<+1>(ctx, &g); }
STAGE(bicubic_p3y, SkRasterPipeline_SamplerCtx* ctx) { bicubic_y<+3>(ctx, &g); }

STAGE(callback, SkRasterPipeline_CallbackCtx* c) {
    store4(c->rgba,0, r,g,b,a);
    c->fn(c, tail ? tail : N);
    load4(c->read_from,0, &r,&g,&b,&a);
}

STAGE(interpreter, SkRasterPipeline_InterpreterCtx* c) {
    float rr[N];
    float gg[N];
    float bb[N];
    float aa[N];

    sk_unaligned_store(rr, r);
    sk_unaligned_store(gg, g);
    sk_unaligned_store(bb, b);
    sk_unaligned_store(aa, a);

    float* args[] = { rr, gg, bb, aa };
    c->byteCode->runStriped(c->fn, args, SK_ARRAY_COUNT(args), tail ? tail : N,
                            (const float*)c->inputs, c->ninputs);

    r = sk_unaligned_load<F>(rr);
    g = sk_unaligned_load<F>(gg);
    b = sk_unaligned_load<F>(bb);
    a = sk_unaligned_load<F>(aa);
}

STAGE(gauss_a_to_rgba, Ctx::None) {
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
    r = g = b = a = 0;

    for (float dy = -0.5f; dy <= +0.5f; dy += 1.0f)
    for (float dx = -0.5f; dx <= +0.5f; dx += 1.0f) {
        // (x,y) are the coordinates of this sample point.
        F x = cx + dx,
          y = cy + dy;

        // ix_and_ptr() will clamp to the image's bounds for us.
        const uint32_t* ptr;
        U32 ix = ix_and_ptr(&ptr, ctx, x,y);

        F sr,sg,sb,sa;
        from_8888(gather(ptr, ix), &sr,&sg,&sb,&sa);

        // In bilinear interpolation, the 4 pixels at +/- 0.5 offsets from the sample pixel center
        // are combined in direct proportion to their area overlapping that logical query pixel.
        // At positive offsets, the x-axis contribution to that rectangle is fx,
        // or (1-fx) at negative x.  Same deal for y.
        F sx = (dx > 0) ? fx : 1.0f - fx,
          sy = (dy > 0) ? fy : 1.0f - fy,
          area = sx * sy;

        r += sr * area;
        g += sg * area;
        b += sb * area;
        a += sa * area;
    }
}

// ~~~~~~ GrSwizzle stage ~~~~~~ //

STAGE(swizzle, void* ctx) {
    auto ir = r, ig = g, ib = b, ia = a;
    F* o[] = {&r, &g, &b, &a};
    char swiz[4];
    memcpy(swiz, &ctx, sizeof(swiz));

    for (int i = 0; i < 4; ++i) {
        switch (swiz[i]) {
            case 'r': *o[i] = ir;   break;
            case 'g': *o[i] = ig;   break;
            case 'b': *o[i] = ib;   break;
            case 'a': *o[i] = ia;   break;
            case '1': *o[i] = F(1); break;
            default:                break;
        }
    }
}

namespace lowp {
#if defined(JUMPER_IS_SCALAR) || defined(SK_DISABLE_LOWP_RASTER_PIPELINE)
    // If we're not compiled by Clang, or otherwise switched into scalar mode (old Clang, manually),
    // we don't generate lowp stages.  All these nullptrs will tell SkJumper.cpp to always use the
    // highp float pipeline.
    #define M(st) static void (*st)(void) = nullptr;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
    static void (*just_return)(void) = nullptr;

    static void start_pipeline(size_t,size_t,size_t,size_t, void**) {}

#else  // We are compiling vector code with Clang... let's make some lowp stages!

#if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
    using U8  = uint8_t  __attribute__((ext_vector_type(16)));
    using U16 = uint16_t __attribute__((ext_vector_type(16)));
    using I16 =  int16_t __attribute__((ext_vector_type(16)));
    using I32 =  int32_t __attribute__((ext_vector_type(16)));
    using U32 = uint32_t __attribute__((ext_vector_type(16)));
    using F   = float    __attribute__((ext_vector_type(16)));
#else
    using U8  = uint8_t  __attribute__((ext_vector_type(8)));
    using U16 = uint16_t __attribute__((ext_vector_type(8)));
    using I16 =  int16_t __attribute__((ext_vector_type(8)));
    using I32 =  int32_t __attribute__((ext_vector_type(8)));
    using U32 = uint32_t __attribute__((ext_vector_type(8)));
    using F   = float    __attribute__((ext_vector_type(8)));
#endif

static const size_t N = sizeof(U16) / sizeof(uint16_t);

// Once again, some platforms benefit from a restricted Stage calling convention,
// but others can pass tons and tons of registers and we're happy to exploit that.
// It's exactly the same decision and implementation strategy as the F stages above.
#if JUMPER_NARROW_STAGES
    struct Params {
        size_t dx, dy, tail;
        U16 dr,dg,db,da;
    };
    using Stage = void(ABI*)(Params*, void** program, U16 r, U16 g, U16 b, U16 a);
#else
    // We pass program as the second argument so that load_and_inc() will find it in %rsi on x86-64.
    using Stage = void (ABI*)(size_t tail, void** program, size_t dx, size_t dy,
                              U16  r, U16  g, U16  b, U16  a,
                              U16 dr, U16 dg, U16 db, U16 da);
#endif

static void start_pipeline(const size_t x0,     const size_t y0,
                           const size_t xlimit, const size_t ylimit, void** program) {
    auto start = (Stage)load_and_inc(program);
    for (size_t dy = y0; dy < ylimit; dy++) {
    #if JUMPER_NARROW_STAGES
        Params params = { x0,dy,0, 0,0,0,0 };
        for (; params.dx + N <= xlimit; params.dx += N) {
            start(&params,program, 0,0,0,0);
        }
        if (size_t tail = xlimit - params.dx) {
            params.tail = tail;
            start(&params,program, 0,0,0,0);
        }
    #else
        size_t dx = x0;
        for (; dx + N <= xlimit; dx += N) {
            start(   0,program,dx,dy, 0,0,0,0, 0,0,0,0);
        }
        if (size_t tail = xlimit - dx) {
            start(tail,program,dx,dy, 0,0,0,0, 0,0,0,0);
        }
    #endif
    }
}

#if JUMPER_NARROW_STAGES
    static void ABI just_return(Params*, void**, U16,U16,U16,U16) {}
#else
    static void ABI just_return(size_t,void**,size_t,size_t, U16,U16,U16,U16, U16,U16,U16,U16) {}
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
    #define STAGE_GG(name, ...)                                                                \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F& x, F& y);          \
        static void ABI name(Params* params, void** program, U16 r, U16 g, U16 b, U16 a) {     \
            auto x = join<F>(r,g),                                                             \
                 y = join<F>(b,a);                                                             \
            name##_k(Ctx{program}, params->dx,params->dy,params->tail, x,y);                   \
            split(x, &r,&g);                                                                   \
            split(y, &b,&a);                                                                   \
            auto next = (Stage)load_and_inc(program);                                          \
            next(params,program, r,g,b,a);                                                     \
        }                                                                                      \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F& x, F& y)

    #define STAGE_GP(name, ...)                                                            \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F x, F y,         \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da);                              \
        static void ABI name(Params* params, void** program, U16 r, U16 g, U16 b, U16 a) { \
            auto x = join<F>(r,g),                                                         \
                 y = join<F>(b,a);                                                         \
            name##_k(Ctx{program}, params->dx,params->dy,params->tail, x,y, r,g,b,a,       \
                     params->dr,params->dg,params->db,params->da);                         \
            auto next = (Stage)load_and_inc(program);                                      \
            next(params,program, r,g,b,a);                                                 \
        }                                                                                  \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F x, F y,         \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da)

    #define STAGE_PP(name, ...)                                                            \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,                   \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da);                              \
        static void ABI name(Params* params, void** program, U16 r, U16 g, U16 b, U16 a) { \
            name##_k(Ctx{program}, params->dx,params->dy,params->tail, r,g,b,a,            \
                     params->dr,params->dg,params->db,params->da);                         \
            auto next = (Stage)load_and_inc(program);                                      \
            next(params,program, r,g,b,a);                                                 \
        }                                                                                  \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,                   \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da)
#else
    #define STAGE_GG(name, ...)                                                            \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F& x, F& y);      \
        static void ABI name(size_t tail, void** program, size_t dx, size_t dy,            \
                             U16  r, U16  g, U16  b, U16  a,                               \
                             U16 dr, U16 dg, U16 db, U16 da) {                             \
            auto x = join<F>(r,g),                                                         \
                 y = join<F>(b,a);                                                         \
            name##_k(Ctx{program}, dx,dy,tail, x,y);                                       \
            split(x, &r,&g);                                                               \
            split(y, &b,&a);                                                               \
            auto next = (Stage)load_and_inc(program);                                      \
            next(tail,program,dx,dy, r,g,b,a, dr,dg,db,da);                                \
        }                                                                                  \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F& x, F& y)

    #define STAGE_GP(name, ...)                                                            \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F x, F y,         \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da);                              \
        static void ABI name(size_t tail, void** program, size_t dx, size_t dy,            \
                             U16  r, U16  g, U16  b, U16  a,                               \
                             U16 dr, U16 dg, U16 db, U16 da) {                             \
            auto x = join<F>(r,g),                                                         \
                 y = join<F>(b,a);                                                         \
            name##_k(Ctx{program}, dx,dy,tail, x,y, r,g,b,a, dr,dg,db,da);                 \
            auto next = (Stage)load_and_inc(program);                                      \
            next(tail,program,dx,dy, r,g,b,a, dr,dg,db,da);                                \
        }                                                                                  \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F x, F y,         \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da)

    #define STAGE_PP(name, ...)                                                            \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,                   \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da);                              \
        static void ABI name(size_t tail, void** program, size_t dx, size_t dy,            \
                             U16  r, U16  g, U16  b, U16  a,                               \
                             U16 dr, U16 dg, U16 db, U16 da) {                             \
            name##_k(Ctx{program}, dx,dy,tail, r,g,b,a, dr,dg,db,da);                      \
            auto next = (Stage)load_and_inc(program);                                      \
            next(tail,program,dx,dy, r,g,b,a, dr,dg,db,da);                                \
        }                                                                                  \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,                   \
                         U16&  r, U16&  g, U16&  b, U16&  a,                               \
                         U16& dr, U16& dg, U16& db, U16& da)
#endif

// ~~~~~~ Commonly used helper functions ~~~~~~ //

SI U16 div255(U16 v) {
#if 0
    return (v+127)/255;  // The ideal rounding divide by 255.
#elif 1 && defined(JUMPER_IS_NEON)
    // With NEON we can compute (v+127)/255 as (v + ((v+128)>>8) + 128)>>8
    // just as fast as we can do the approximation below, so might as well be correct!
    // First we compute v + ((v+128)>>8), then one more round of (...+128)>>8 to finish up.
    return vrshrq_n_u16(vrsraq_n_u16(v, v, 8), 8);
#else
    return (v+255)/256;  // A good approximation of (v+127)/255.
#endif
}

SI U16 inv(U16 v) { return 255-v; }

SI U16 if_then_else(I16 c, U16 t, U16 e) { return (t & c) | (e & ~c); }
SI U32 if_then_else(I32 c, U32 t, U32 e) { return (t & c) | (e & ~c); }

SI U16 max(U16 x, U16 y) { return if_then_else(x < y, y, x); }
SI U16 min(U16 x, U16 y) { return if_then_else(x < y, x, y); }
SI U16 max(U16 x, U16 y, U16 z) { return max(x, max(y, z)); }
SI U16 min(U16 x, U16 y, U16 z) { return min(x, min(y, z)); }

SI U16 from_float(float f) { return f * 255.0f + 0.5f; }

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
    return bit_cast<F>( (bit_cast<I32>(t) & c) | (bit_cast<I32>(e) & ~c) );
}
SI F max(F x, F y) { return if_then_else(x < y, y, x); }
SI F min(F x, F y) { return if_then_else(x < y, x, y); }

SI F mad(F f, F m, F a) { return f*m+a; }
SI U32 trunc_(F x) { return (U32)cast<I32>(x); }

SI F rcp(F x) {
#if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
    __m256 lo,hi;
    split(x, &lo,&hi);
    return join<F>(_mm256_rcp_ps(lo), _mm256_rcp_ps(hi));
#elif defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    __m128 lo,hi;
    split(x, &lo,&hi);
    return join<F>(_mm_rcp_ps(lo), _mm_rcp_ps(hi));
#elif defined(JUMPER_IS_NEON)
    auto rcp = [](float32x4_t v) {
        auto est = vrecpeq_f32(v);
        return vrecpsq_f32(v,est)*est;
    };
    float32x4_t lo,hi;
    split(x, &lo,&hi);
    return join<F>(rcp(lo), rcp(hi));
#else
    return 1.0f / x;
#endif
}
SI F sqrt_(F x) {
#if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
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
#elif defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
    __m256 lo,hi;
    split(x, &lo,&hi);
    return join<F>(_mm256_floor_ps(lo), _mm256_floor_ps(hi));
#elif defined(JUMPER_IS_SSE41) || defined(JUMPER_IS_AVX)
    __m128 lo,hi;
    split(x, &lo,&hi);
    return join<F>(_mm_floor_ps(lo), _mm_floor_ps(hi));
#else
    F roundtrip = cast<F>(cast<I32>(x));
    return roundtrip - if_then_else(roundtrip > x, F(1), F(0));
#endif
}
SI F fract(F x) { return x - floor_(x); }
SI F abs_(F x) { return bit_cast<F>( bit_cast<I32>(x) & 0x7fffffff ); }

// ~~~~~~ Basic / misc. stages ~~~~~~ //

STAGE_GG(seed_shader, Ctx::None) {
    static const float iota[] = {
        0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f,
        8.5f, 9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f,
    };
    x = cast<F>(I32(dx)) + sk_unaligned_load<F>(iota);
    y = cast<F>(I32(dy)) + 0.5f;
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
    auto X = mad(x,m[0], mad(y,m[2], m[4])),
         Y = mad(x,m[1], mad(y,m[3], m[5]));
    x = X;
    y = Y;
}
STAGE_GG(matrix_perspective, const float* m) {
    // N.B. Unlike the other matrix_ stages, this matrix is row-major.
    auto X = mad(x,m[0], mad(y,m[1], m[2])),
         Y = mad(x,m[3], mad(y,m[4], m[5])),
         Z = mad(x,m[6], mad(y,m[7], m[8]));
    x = X * rcp(Z);
    y = Y * rcp(Z);
}

STAGE_PP(uniform_color, const SkRasterPipeline_UniformColorCtx* c) {
    r = c->rgba[0];
    g = c->rgba[1];
    b = c->rgba[2];
    a = c->rgba[3];
}
STAGE_PP(black_color, Ctx::None) { r = g = b =   0; a = 255; }
STAGE_PP(white_color, Ctx::None) { r = g = b = 255; a = 255; }

STAGE_PP(set_rgb, const float rgb[3]) {
    r = from_float(rgb[0]);
    g = from_float(rgb[1]);
    b = from_float(rgb[2]);
}

STAGE_PP(clamp_0, Ctx::None) { /*definitely a noop*/ }
STAGE_PP(clamp_1, Ctx::None) { /*_should_ be a noop*/ }

STAGE_PP(clamp_a, Ctx::None) {
    r = min(r, a);
    g = min(g, a);
    b = min(b, a);
}

STAGE_PP(clamp_gamut, Ctx::None) {
    // It shouldn't be possible to get out-of-gamut
    // colors when working in lowp.
}

STAGE_PP(premul, Ctx::None) {
    r = div255(r * a);
    g = div255(g * a);
    b = div255(b * a);
}
STAGE_PP(premul_dst, Ctx::None) {
    dr = div255(dr * da);
    dg = div255(dg * da);
    db = div255(db * da);
}

STAGE_PP(force_opaque    , Ctx::None) {  a = 255; }
STAGE_PP(force_opaque_dst, Ctx::None) { da = 255; }

STAGE_PP(swap_rb, Ctx::None) {
    auto tmp = r;
    r = b;
    b = tmp;
}
STAGE_PP(swap_rb_dst, Ctx::None) {
    auto tmp = dr;
    dr = db;
    db = tmp;
}

STAGE_PP(move_src_dst, Ctx::None) {
    dr = r;
    dg = g;
    db = b;
    da = a;
}

STAGE_PP(move_dst_src, Ctx::None) {
    r = dr;
    g = dg;
    b = db;
    a = da;
}

// ~~~~~~ Blend modes ~~~~~~ //

// The same logic applied to all 4 channels.
#define BLEND_MODE(name)                                 \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da); \
    STAGE_PP(name, Ctx::None) {                          \
        r = name##_channel(r,dr,a,da);                   \
        g = name##_channel(g,dg,a,da);                   \
        b = name##_channel(b,db,a,da);                   \
        a = name##_channel(a,da,a,da);                   \
    }                                                    \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da)

    BLEND_MODE(clear)    { return 0; }
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
#undef BLEND_MODE

// The same logic applied to color, and srcover for alpha.
#define BLEND_MODE(name)                                 \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da); \
    STAGE_PP(name, Ctx::None) {                          \
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
    auto clamp = [](F v, F limit) {
        limit = bit_cast<F>( bit_cast<U32>(limit) - 1 );  // Exclusive -> inclusive.
        return min(max(0, v), limit);
    };
    x = clamp(x, ctx->width);
    y = clamp(y, ctx->height);

    *ptr = (const T*)ctx->pixels;
    return trunc_(y)*ctx->stride + trunc_(x);
}

template <typename V, typename T>
SI V load(const T* ptr, size_t tail) {
    V v = 0;
    switch (tail & (N-1)) {
        case  0: memcpy(&v, ptr, sizeof(v)); break;
    #if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
        case 15: v[14] = ptr[14];
        case 14: v[13] = ptr[13];
        case 13: v[12] = ptr[12];
        case 12: memcpy(&v, ptr, 12*sizeof(T)); break;
        case 11: v[10] = ptr[10];
        case 10: v[ 9] = ptr[ 9];
        case  9: v[ 8] = ptr[ 8];
        case  8: memcpy(&v, ptr,  8*sizeof(T)); break;
    #endif
        case  7: v[ 6] = ptr[ 6];
        case  6: v[ 5] = ptr[ 5];
        case  5: v[ 4] = ptr[ 4];
        case  4: memcpy(&v, ptr,  4*sizeof(T)); break;
        case  3: v[ 2] = ptr[ 2];
        case  2: memcpy(&v, ptr,  2*sizeof(T)); break;
        case  1: v[ 0] = ptr[ 0];
    }
    return v;
}
template <typename V, typename T>
SI void store(T* ptr, size_t tail, V v) {
    switch (tail & (N-1)) {
        case  0: memcpy(ptr, &v, sizeof(v)); break;
    #if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
        case 15: ptr[14] = v[14];
        case 14: ptr[13] = v[13];
        case 13: ptr[12] = v[12];
        case 12: memcpy(ptr, &v, 12*sizeof(T)); break;
        case 11: ptr[10] = v[10];
        case 10: ptr[ 9] = v[ 9];
        case  9: ptr[ 8] = v[ 8];
        case  8: memcpy(ptr, &v,  8*sizeof(T)); break;
    #endif
        case  7: ptr[ 6] = v[ 6];
        case  6: ptr[ 5] = v[ 5];
        case  5: ptr[ 4] = v[ 4];
        case  4: memcpy(ptr, &v,  4*sizeof(T)); break;
        case  3: ptr[ 2] = v[ 2];
        case  2: memcpy(ptr, &v,  2*sizeof(T)); break;
        case  1: ptr[ 0] = v[ 0];
    }
}

#if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
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

        return join<U32>(_mm256_i32gather_epi32(ptr, lo, 4),
                         _mm256_i32gather_epi32(ptr, hi, 4));
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
#if 1 && defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
    // Swap the middle 128-bit lanes to make _mm256_packus_epi32() in cast_U16() work out nicely.
    __m256i _01,_23;
    split(rgba, &_01, &_23);
    __m256i _02 = _mm256_permute2x128_si256(_01,_23, 0x20),
            _13 = _mm256_permute2x128_si256(_01,_23, 0x31);
    rgba = join<U32>(_02, _13);

    auto cast_U16 = [](U32 v) -> U16 {
        __m256i _02,_13;
        split(v, &_02,&_13);
        return _mm256_packus_epi32(_02,_13);
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

SI void load_8888_(const uint32_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
#if 1 && defined(JUMPER_IS_NEON)
    uint8x8x4_t rgba;
    switch (tail & (N-1)) {
        case 0: rgba = vld4_u8     ((const uint8_t*)(ptr+0)         ); break;
        case 7: rgba = vld4_lane_u8((const uint8_t*)(ptr+6), rgba, 6);
        case 6: rgba = vld4_lane_u8((const uint8_t*)(ptr+5), rgba, 5);
        case 5: rgba = vld4_lane_u8((const uint8_t*)(ptr+4), rgba, 4);
        case 4: rgba = vld4_lane_u8((const uint8_t*)(ptr+3), rgba, 3);
        case 3: rgba = vld4_lane_u8((const uint8_t*)(ptr+2), rgba, 2);
        case 2: rgba = vld4_lane_u8((const uint8_t*)(ptr+1), rgba, 1);
        case 1: rgba = vld4_lane_u8((const uint8_t*)(ptr+0), rgba, 0);
    }
    *r = cast<U16>(rgba.val[0]);
    *g = cast<U16>(rgba.val[1]);
    *b = cast<U16>(rgba.val[2]);
    *a = cast<U16>(rgba.val[3]);
#else
    from_8888(load<U32>(ptr, tail), r,g,b,a);
#endif
}
SI void store_8888_(uint32_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
#if 1 && defined(JUMPER_IS_NEON)
    uint8x8x4_t rgba = {{
        cast<U8>(r),
        cast<U8>(g),
        cast<U8>(b),
        cast<U8>(a),
    }};
    switch (tail & (N-1)) {
        case 0: vst4_u8     ((uint8_t*)(ptr+0), rgba   ); break;
        case 7: vst4_lane_u8((uint8_t*)(ptr+6), rgba, 6);
        case 6: vst4_lane_u8((uint8_t*)(ptr+5), rgba, 5);
        case 5: vst4_lane_u8((uint8_t*)(ptr+4), rgba, 4);
        case 4: vst4_lane_u8((uint8_t*)(ptr+3), rgba, 3);
        case 3: vst4_lane_u8((uint8_t*)(ptr+2), rgba, 2);
        case 2: vst4_lane_u8((uint8_t*)(ptr+1), rgba, 1);
        case 1: vst4_lane_u8((uint8_t*)(ptr+0), rgba, 0);
    }
#else
    store(ptr, tail, cast<U32>(r | (g<<8)) <<  0
                   | cast<U32>(b | (a<<8)) << 16);
#endif
}

STAGE_PP(load_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    load_8888_(ptr_at_xy<const uint32_t>(ctx, dx,dy), tail, &r,&g,&b,&a);
}
STAGE_PP(load_8888_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    load_8888_(ptr_at_xy<const uint32_t>(ctx, dx,dy), tail, &dr,&dg,&db,&da);
}
STAGE_PP(store_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    store_8888_(ptr_at_xy<uint32_t>(ctx, dx,dy), tail, r,g,b,a);
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
SI void load_565_(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
    from_565(load<U16>(ptr, tail), r,g,b);
}
SI void store_565_(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b) {
    // Round from [0,255] to [0,31] or [0,63], as if x * (31/255.0f) + 0.5f.
    // (Don't feel like you need to find some fundamental truth in these...
    // they were brute-force searched.)
    U16 R = (r *  9 + 36) / 74,   //  9/74 ≈ 31/255, plus 36/74, about half.
        G = (g * 21 + 42) / 85,   // 21/85 = 63/255 exactly.
        B = (b *  9 + 36) / 74;
    // Pack them back into 15|rrrrr gggggg bbbbb|0.
    store(ptr, tail, R << 11
                   | G <<  5
                   | B <<  0);
}

STAGE_PP(load_565, const SkRasterPipeline_MemoryCtx* ctx) {
    load_565_(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &r,&g,&b);
    a = 255;
}
STAGE_PP(load_565_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    load_565_(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &dr,&dg,&db);
    da = 255;
}
STAGE_PP(store_565, const SkRasterPipeline_MemoryCtx* ctx) {
    store_565_(ptr_at_xy<uint16_t>(ctx, dx,dy), tail, r,g,b);
}
STAGE_GP(gather_565, const SkRasterPipeline_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    from_565(gather<U16>(ptr, ix), &r, &g, &b);
    a = 255;
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
SI void load_4444_(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
    from_4444(load<U16>(ptr, tail), r,g,b,a);
}
SI void store_4444_(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
    // Round from [0,255] to [0,15], producing the same value as (x*(15/255.0f) + 0.5f).
    U16 R = (r + 8) / 17,
        G = (g + 8) / 17,
        B = (b + 8) / 17,
        A = (a + 8) / 17;
    // Pack them back into 15|rrrr gggg bbbb aaaa|0.
    store(ptr, tail, R << 12
                   | G <<  8
                   | B <<  4
                   | A <<  0);
}

STAGE_PP(load_4444, const SkRasterPipeline_MemoryCtx* ctx) {
    load_4444_(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &r,&g,&b,&a);
}
STAGE_PP(load_4444_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    load_4444_(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &dr,&dg,&db,&da);
}
STAGE_PP(store_4444, const SkRasterPipeline_MemoryCtx* ctx) {
    store_4444_(ptr_at_xy<uint16_t>(ctx, dx,dy), tail, r,g,b,a);
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

SI void load_88_(const uint16_t* ptr, size_t tail, U16* r, U16* g) {
#if 1 && defined(JUMPER_IS_NEON)
    uint8x8x2_t rg;
    switch (tail & (N-1)) {
        case 0: rg = vld2_u8     ((const uint8_t*)(ptr+0)         ); break;
        case 7: rg = vld2_lane_u8((const uint8_t*)(ptr+6), rg, 6);
        case 6: rg = vld2_lane_u8((const uint8_t*)(ptr+5), rg, 5);
        case 5: rg = vld2_lane_u8((const uint8_t*)(ptr+4), rg, 4);
        case 4: rg = vld2_lane_u8((const uint8_t*)(ptr+3), rg, 3);
        case 3: rg = vld2_lane_u8((const uint8_t*)(ptr+2), rg, 2);
        case 2: rg = vld2_lane_u8((const uint8_t*)(ptr+1), rg, 1);
        case 1: rg = vld2_lane_u8((const uint8_t*)(ptr+0), rg, 0);
    }
    *r = cast<U16>(rg.val[0]);
    *g = cast<U16>(rg.val[1]);
#else
    from_88(load<U16>(ptr, tail), r,g);
#endif
}

SI void store_88_(uint16_t* ptr, size_t tail, U16 r, U16 g) {
#if 1 && defined(JUMPER_IS_NEON)
    uint8x8x2_t rg = {{
        cast<U8>(r),
        cast<U8>(g),
    }};
    switch (tail & (N-1)) {
        case 0: vst2_u8     ((uint8_t*)(ptr+0), rg   ); break;
        case 7: vst2_lane_u8((uint8_t*)(ptr+6), rg, 6);
        case 6: vst2_lane_u8((uint8_t*)(ptr+5), rg, 5);
        case 5: vst2_lane_u8((uint8_t*)(ptr+4), rg, 4);
        case 4: vst2_lane_u8((uint8_t*)(ptr+3), rg, 3);
        case 3: vst2_lane_u8((uint8_t*)(ptr+2), rg, 2);
        case 2: vst2_lane_u8((uint8_t*)(ptr+1), rg, 1);
        case 1: vst2_lane_u8((uint8_t*)(ptr+0), rg, 0);
    }
#else
    store(ptr, tail, cast<U16>(r | (g<<8)) <<  0);
#endif
}

STAGE_PP(load_rg88, const SkRasterPipeline_MemoryCtx* ctx) {
    b = 0;
    a = 1;
    load_88_(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &r,&g);
}
STAGE_PP(store_rg88, const SkRasterPipeline_MemoryCtx* ctx) {
    store_88_(ptr_at_xy<uint16_t>(ctx, dx, dy), tail, r, g);
}

// ~~~~~~ 8-bit memory loads and stores ~~~~~~ //

SI U16 load_8(const uint8_t* ptr, size_t tail) {
    return cast<U16>(load<U8>(ptr, tail));
}
SI void store_8(uint8_t* ptr, size_t tail, U16 v) {
    store(ptr, tail, cast<U8>(v));
}

STAGE_PP(load_a8, const SkRasterPipeline_MemoryCtx* ctx) {
    r = g = b = 0;
    a = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
}
STAGE_PP(load_a8_dst, const SkRasterPipeline_MemoryCtx* ctx) {
    dr = dg = db = 0;
    da = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
}
STAGE_PP(store_a8, const SkRasterPipeline_MemoryCtx* ctx) {
    store_8(ptr_at_xy<uint8_t>(ctx, dx,dy), tail, a);
}
STAGE_GP(gather_a8, const SkRasterPipeline_GatherCtx* ctx) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    r = g = b = 0;
    a = cast<U16>(gather<U8>(ptr, ix));
}

STAGE_PP(alpha_to_gray, Ctx::None) {
    r = g = b = a;
    a = 255;
}
STAGE_PP(alpha_to_gray_dst, Ctx::None) {
    dr = dg = db = da;
    da = 255;
}
STAGE_PP(luminance_to_alpha, Ctx::None) {
    a = (r*54 + g*183 + b*19)/256;  // 0.2126, 0.7152, 0.0722 with 256 denominator.
    r = g = b = 0;
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
STAGE_PP(lerp_native, const uint16_t scales[]) {
    auto c = sk_unaligned_load<U16>(scales);
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

STAGE_PP(scale_u8, const SkRasterPipeline_MemoryCtx* ctx) {
    U16 c = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
    r = div255( r * c );
    g = div255( g * c );
    b = div255( b * c );
    a = div255( a * c );
}
STAGE_PP(lerp_u8, const SkRasterPipeline_MemoryCtx* ctx) {
    U16 c = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

// Derive alpha's coverage from rgb coverage and the values of src and dst alpha.
SI U16 alpha_coverage_from_rgb_coverage(U16 a, U16 da, U16 cr, U16 cg, U16 cb) {
    return if_then_else(a < da, min(cr,cg,cb)
                              , max(cr,cg,cb));
}
STAGE_PP(scale_565, const SkRasterPipeline_MemoryCtx* ctx) {
    U16 cr,cg,cb;
    load_565_(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &cr,&cg,&cb);
    U16 ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = div255( r * cr );
    g = div255( g * cg );
    b = div255( b * cb );
    a = div255( a * ca );
}
STAGE_PP(lerp_565, const SkRasterPipeline_MemoryCtx* ctx) {
    U16 cr,cg,cb;
    load_565_(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &cr,&cg,&cb);
    U16 ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = lerp(da, a, ca);
}

STAGE_PP(emboss, const SkRasterPipeline_EmbossCtx* ctx) {
    U16 mul = load_8(ptr_at_xy<const uint8_t>(&ctx->mul, dx,dy), tail),
        add = load_8(ptr_at_xy<const uint8_t>(&ctx->add, dx,dy), tail);

    r = min(div255(r*mul) + add, a);
    g = min(div255(g*mul) + add, a);
    b = min(div255(b*mul) + add, a);
}


// ~~~~~~ Gradient stages ~~~~~~ //

// Clamp x to [0,1], both sides inclusive (think, gradients).
// Even repeat and mirror funnel through a clamp to handle bad inputs like +Inf, NaN.
SI F clamp_01(F v) { return min(max(0, v), 1); }

STAGE_GG(clamp_x_1 , Ctx::None) { x = clamp_01(x); }
STAGE_GG(repeat_x_1, Ctx::None) { x = clamp_01(x - floor_(x)); }
STAGE_GG(mirror_x_1, Ctx::None) {
    auto two = [](F x){ return x+x; };
    x = clamp_01(abs_( (x-1.0f) - two(floor_((x-1.0f)*0.5f)) - 1.0f ));
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
STAGE_PP(check_decal_mask, SkRasterPipeline_DecalTileCtx* ctx) {
    auto mask = sk_unaligned_load<U16>(ctx->mask);
    r = r & mask;
    g = g & mask;
    b = b & mask;
    a = a & mask;
}

SI void round_F_to_U16(F    R, F    G, F    B, F    A, bool interpolatedInPremul,
                       U16* r, U16* g, U16* b, U16* a) {
    auto round = [](F x) { return cast<U16>(x * 255.0f + 0.5f); };

    F limit = interpolatedInPremul ? A
                                   : 1;
    *r = round(min(max(0,R), limit));
    *g = round(min(max(0,G), limit));
    *b = round(min(max(0,B), limit));
    *a = round(A);  // we assume alpha is already in [0,1].
}

SI void gradient_lookup(const SkRasterPipeline_GradientCtx* c, U32 idx, F t,
                        U16* r, U16* g, U16* b, U16* a) {

    F fr, fg, fb, fa, br, bg, bb, ba;
#if defined(JUMPER_IS_HSW) || defined(JUMPER_IS_AVX512)
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
                   c->interpolatedInPremul,
                   r,g,b,a);
}

STAGE_GP(gradient, const SkRasterPipeline_GradientCtx* c) {
    auto t = x;
    U32 idx = 0;

    // N.B. The loop starts at 1 because idx 0 is the color to use before the first stop.
    for (size_t i = 1; i < c->stopCount; i++) {
        idx += if_then_else(t >= c->ts[i], U32(1), U32(0));
    }

    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE_GP(evenly_spaced_gradient, const SkRasterPipeline_GradientCtx* c) {
    auto t = x;
    auto idx = trunc_(t * (c->stopCount-1));
    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE_GP(evenly_spaced_2_stop_gradient, const SkRasterPipeline_EvenlySpaced2StopGradientCtx* c) {
    auto t = x;
    round_F_to_U16(mad(t, c->f[0], c->b[0]),
                   mad(t, c->f[1], c->b[1]),
                   mad(t, c->f[2], c->b[2]),
                   mad(t, c->f[3], c->b[3]),
                   c->interpolatedInPremul,
                   &r,&g,&b,&a);
}

STAGE_GG(xy_to_unit_angle, Ctx::None) {
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
STAGE_GG(xy_to_radius, Ctx::None) {
    x = sqrt_(x*x + y*y);
}

// ~~~~~~ Compound stages ~~~~~~ //

STAGE_PP(srcover_rgba_8888, const SkRasterPipeline_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    load_8888_(ptr, tail, &dr,&dg,&db,&da);
    r = r + div255( dr*inv(a) );
    g = g + div255( dg*inv(a) );
    b = b + div255( db*inv(a) );
    a = a + div255( da*inv(a) );
    store_8888_(ptr, tail, r,g,b,a);
}

#if defined(SK_DISABLE_LOWP_BILERP_CLAMP_CLAMP_STAGE)
    static void(*bilerp_clamp_8888)(void) = nullptr;
#else
STAGE_GP(bilerp_clamp_8888, const SkRasterPipeline_GatherCtx* ctx) {
    // (cx,cy) are the center of our sample.
    F cx = x,
      cy = y;

    // All sample points are at the same fractional offset (fx,fy).
    // They're the 4 corners of a logical 1x1 pixel surrounding (x,y) at (0.5,0.5) offsets.
    F fx = fract(cx + 0.5f),
      fy = fract(cy + 0.5f);

    // We'll accumulate the color of all four samples into {r,g,b,a} directly.
    r = g = b = a = 0;

    // The first three sample points will calculate their area using math
    // just like in the float code above, but the fourth will take up all the rest.
    //
    // Logically this is the same as doing the math for the fourth pixel too,
    // but rounding error makes this a better strategy, keeping opaque opaque, etc.
    //
    // We can keep up to 8 bits of fractional precision without overflowing 16-bit,
    // so our "1.0" area is 256.
    const uint16_t bias = 256;
    U16 remaining = bias;

    for (float dy = -0.5f; dy <= +0.5f; dy += 1.0f)
    for (float dx = -0.5f; dx <= +0.5f; dx += 1.0f) {
        // (x,y) are the coordinates of this sample point.
        F x = cx + dx,
          y = cy + dy;

        // ix_and_ptr() will clamp to the image's bounds for us.
        const uint32_t* ptr;
        U32 ix = ix_and_ptr(&ptr, ctx, x,y);

        U16 sr,sg,sb,sa;
        from_8888(gather<U32>(ptr, ix), &sr,&sg,&sb,&sa);

        // In bilinear interpolation, the 4 pixels at +/- 0.5 offsets from the sample pixel center
        // are combined in direct proportion to their area overlapping that logical query pixel.
        // At positive offsets, the x-axis contribution to that rectangle is fx,
        // or (1-fx) at negative x.  Same deal for y.
        F sx = (dx > 0) ? fx : 1.0f - fx,
          sy = (dy > 0) ? fy : 1.0f - fy;

        U16 area = (dy == 0.5f && dx == 0.5f) ? remaining
                                              : cast<U16>(sx * sy * bias);
        for (size_t i = 0; i < N; i++) {
            SkASSERT(remaining[i] >= area[i]);
        }
        remaining -= area;

        r += sr * area;
        g += sg * area;
        b += sb * area;
        a += sa * area;
    }

    r = (r + bias/2) / bias;
    g = (g + bias/2) / bias;
    b = (b + bias/2) / bias;
    a = (a + bias/2) / bias;
}
#endif

// ~~~~~~ GrSwizzle stage ~~~~~~ //

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
            case '1': *o[i] = U16(255); break;
            default:                    break;
        }
    }
}

// Now we'll add null stand-ins for stages we haven't implemented in lowp.
// If a pipeline uses these stages, it'll boot it out of lowp into highp.
#define NOT_IMPLEMENTED(st) static void (*st)(void) = nullptr;
    NOT_IMPLEMENTED(callback)
    NOT_IMPLEMENTED(interpreter)
    NOT_IMPLEMENTED(unbounded_set_rgb)
    NOT_IMPLEMENTED(unbounded_uniform_color)
    NOT_IMPLEMENTED(unpremul)
    NOT_IMPLEMENTED(dither)  // TODO
    NOT_IMPLEMENTED(from_srgb)
    NOT_IMPLEMENTED(to_srgb)
    NOT_IMPLEMENTED(load_16161616)
    NOT_IMPLEMENTED(store_16161616)
    NOT_IMPLEMENTED(load_a16)
    NOT_IMPLEMENTED(store_a16)
    NOT_IMPLEMENTED(load_rg1616)
    NOT_IMPLEMENTED(store_rg1616)
    NOT_IMPLEMENTED(load_f16)
    NOT_IMPLEMENTED(load_f16_dst)
    NOT_IMPLEMENTED(store_f16)
    NOT_IMPLEMENTED(gather_f16)
    NOT_IMPLEMENTED(load_af16)
    NOT_IMPLEMENTED(store_af16)
    NOT_IMPLEMENTED(load_rgf16)
    NOT_IMPLEMENTED(store_rgf16)
    NOT_IMPLEMENTED(load_f32)
    NOT_IMPLEMENTED(load_f32_dst)
    NOT_IMPLEMENTED(store_f32)
    NOT_IMPLEMENTED(gather_f32)
    NOT_IMPLEMENTED(load_rgf32)
    NOT_IMPLEMENTED(store_rgf32)
    NOT_IMPLEMENTED(load_1010102)
    NOT_IMPLEMENTED(load_1010102_dst)
    NOT_IMPLEMENTED(store_1010102)
    NOT_IMPLEMENTED(gather_1010102)
    NOT_IMPLEMENTED(store_u16_be)
    NOT_IMPLEMENTED(byte_tables)  // TODO
    NOT_IMPLEMENTED(colorburn)
    NOT_IMPLEMENTED(colordodge)
    NOT_IMPLEMENTED(softlight)
    NOT_IMPLEMENTED(hue)
    NOT_IMPLEMENTED(saturation)
    NOT_IMPLEMENTED(color)
    NOT_IMPLEMENTED(luminosity)
    NOT_IMPLEMENTED(matrix_3x3)
    NOT_IMPLEMENTED(matrix_3x4)
    NOT_IMPLEMENTED(matrix_4x5)  // TODO
    NOT_IMPLEMENTED(matrix_4x3)  // TODO
    NOT_IMPLEMENTED(parametric)
    NOT_IMPLEMENTED(gamma_)
    NOT_IMPLEMENTED(rgb_to_hsl)
    NOT_IMPLEMENTED(hsl_to_rgb)
    NOT_IMPLEMENTED(gauss_a_to_rgba)  // TODO
    NOT_IMPLEMENTED(mirror_x)         // TODO
    NOT_IMPLEMENTED(repeat_x)         // TODO
    NOT_IMPLEMENTED(mirror_y)         // TODO
    NOT_IMPLEMENTED(repeat_y)         // TODO
    NOT_IMPLEMENTED(negate_x)
    NOT_IMPLEMENTED(bilinear_nx)      // TODO
    NOT_IMPLEMENTED(bilinear_ny)      // TODO
    NOT_IMPLEMENTED(bilinear_px)      // TODO
    NOT_IMPLEMENTED(bilinear_py)      // TODO
    NOT_IMPLEMENTED(bicubic_n3x)      // TODO
    NOT_IMPLEMENTED(bicubic_n1x)      // TODO
    NOT_IMPLEMENTED(bicubic_p1x)      // TODO
    NOT_IMPLEMENTED(bicubic_p3x)      // TODO
    NOT_IMPLEMENTED(bicubic_n3y)      // TODO
    NOT_IMPLEMENTED(bicubic_n1y)      // TODO
    NOT_IMPLEMENTED(bicubic_p1y)      // TODO
    NOT_IMPLEMENTED(bicubic_p3y)      // TODO
    NOT_IMPLEMENTED(save_xy)          // TODO
    NOT_IMPLEMENTED(accumulate)       // TODO
    NOT_IMPLEMENTED(xy_to_2pt_conical_well_behaved)
    NOT_IMPLEMENTED(xy_to_2pt_conical_strip)
    NOT_IMPLEMENTED(xy_to_2pt_conical_focal_on_circle)
    NOT_IMPLEMENTED(xy_to_2pt_conical_smaller)
    NOT_IMPLEMENTED(xy_to_2pt_conical_greater)
    NOT_IMPLEMENTED(alter_2pt_conical_compensate_focal)
    NOT_IMPLEMENTED(alter_2pt_conical_unswap)
    NOT_IMPLEMENTED(mask_2pt_conical_nan)
    NOT_IMPLEMENTED(mask_2pt_conical_degenerates)
    NOT_IMPLEMENTED(apply_vector_mask)
#undef NOT_IMPLEMENTED

#endif//defined(JUMPER_IS_SCALAR) controlling whether we build lowp stages
}  // namespace lowp

}  // namespace SK_OPTS_NS

#endif//SkRasterPipeline_opts_DEFINED
