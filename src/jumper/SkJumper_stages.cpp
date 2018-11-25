/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkJumper_misc.h"

// A little wrapper macro to name Stages differently depending on the instruction set.
// That lets us link together several options.
#if !defined(JUMPER_IS_OFFLINE)
    #define WRAP(name) sk_##name
#elif defined(__AVX512F__)
    #define WRAP(name) sk_##name##_skx
#elif defined(__AVX2__)
    #define WRAP(name) sk_##name##_hsw
#elif defined(__AVX__)
    #define WRAP(name) sk_##name##_avx
#elif defined(__SSE4_1__)
    #define WRAP(name) sk_##name##_sse41
#elif defined(__SSE2__)
    #define WRAP(name) sk_##name##_sse2
#endif

// Every function in this file should be marked static and inline using SI (see SkJumper_misc.h).

#if !defined(__clang__)
    #define JUMPER_IS_SCALAR
#elif defined(__ARM_NEON)
    #define JUMPER_IS_NEON
#elif defined(__AVX512F__)
    #define JUMPER_IS_AVX512
#elif defined(__AVX2__) && defined(__F16C__) && defined(__FMA__)
    #define JUMPER_IS_HSW
#elif defined(__AVX__)
    #define JUMPER_IS_AVX
#elif defined(__SSE4_1__)
    #define JUMPER_IS_SSE41
#elif defined(__SSE2__)
    #define JUMPER_IS_SSE2
#else
    #define JUMPER_IS_SCALAR
#endif

// Older Clangs seem to crash when generating non-optimized NEON code for ARMv7.
#if defined(__clang__) && !defined(__OPTIMIZE__) && defined(__arm__)
    // Apple Clang 9 and vanilla Clang 5 are fine, and may even be conservative.
    #if defined(__apple_build_version__) && __clang_major__ < 9
        #define JUMPER_IS_SCALAR
    #elif __clang_major__ < 5
        #define JUMPER_IS_SCALAR
    #endif
#endif

#if defined(JUMPER_IS_SCALAR)
    // This path should lead to portable scalar code.
    #include <math.h>

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
    #include <arm_neon.h>

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

    #if defined(__aarch64__)
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
    #include <immintrin.h>

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
        return unaligned_load<U8>(&r);
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
    #include <immintrin.h>

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
        return unaligned_load<U16>(&p);  // We have two copies.  Return (the lower) one.
    }
    SI U8 pack(U16 v) {
        auto r = widen_cast<__m128i>(v);
        r = _mm_packus_epi16(r,r);
        return unaligned_load<U8>(&r);
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

        *r = unaligned_load<U16>(&R);
        *g = unaligned_load<U16>(&G);
        *b = unaligned_load<U16>(&B);
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

        *r = unaligned_load<U16>((uint16_t*)&rg + 0);
        *g = unaligned_load<U16>((uint16_t*)&rg + 4);
        *b = unaligned_load<U16>((uint16_t*)&ba + 0);
        *a = unaligned_load<U16>((uint16_t*)&ba + 4);
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
    SI U32 trunc_(F   v) { return (U32)v; }
    SI U32 expand(U16 v) { return (U32)v; }
    SI U32 expand(U8  v) { return (U32)v; }
#else
    SI F   cast  (U32 v) { return      __builtin_convertvector((I32)v,   F); }
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
    return unaligned_load<U16>(&v);
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
    return if_then_else(x == 0, 0
                              , approx_pow2(approx_log2(x) * y));
}

SI F from_half(U16 h) {
#if defined(__aarch64__) && !defined(SK_BUILD_FOR_GOOGLE3)  // Temporary workaround for some Google3 builds.
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
#if defined(__aarch64__) && !defined(SK_BUILD_FOR_GOOGLE3)  // Temporary workaround for some Google3 builds.
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

#if defined(__i386__) || defined(_M_IX86) || defined(__arm__)
    // On 32-bit x86 we've only got 8 xmm registers, so we keep the 4 hottest (r,g,b,a)
    // in registers and the d-registers on the stack (giving us 4 temporary registers).
    // General-purpose registers are also tight, so we put most of those on the stack too.
    // On ARMv7, we do the same so that we can make the r,g,b,a vectors wider.
    struct Params {
        size_t dx, dy, tail;
        F dr,dg,db,da;
    };
    using Stage = void(ABI*)(Params*, void** program, F r, F g, F b, F a);

#else
    // We keep program the second argument, so that it's passed in rsi for load_and_inc().
    using Stage = void(ABI*)(size_t tail, void** program, size_t dx, size_t dy, F,F,F,F, F,F,F,F);
#endif


extern "C" MAYBE_MSABI void WRAP(start_pipeline)(size_t dx, size_t dy, size_t xlimit, size_t ylimit,
                                                 void** program) {
    auto start = (Stage)load_and_inc(program);
    const size_t x0 = dx;
    for (; dy < ylimit; dy++) {
    #if defined(__i386__) || defined(_M_IX86) || defined(__arm__)
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

#if defined(__i386__) || defined(_M_IX86) || defined(__arm__)
    #define STAGE(name, ...)                                                          \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,              \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);         \
        extern "C" ABI void WRAP(name)(Params* params, void** program,                \
                                       F r, F g, F b, F a) {                          \
            name##_k(Ctx{program},params->dx,params->dy,params->tail, r,g,b,a,        \
                     params->dr, params->dg, params->db, params->da);                 \
            auto next = (Stage)load_and_inc(program);                                 \
            next(params,program, r,g,b,a);                                            \
        }                                                                             \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,              \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#else
    #define STAGE(name, ...)                                                              \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,                  \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);             \
        extern "C" ABI void WRAP(name)(size_t tail, void** program, size_t dx, size_t dy, \
                                       F r, F g, F b, F a, F dr, F dg, F db, F da) {      \
            name##_k(Ctx{program},dx,dy,tail, r,g,b,a, dr,dg,db,da);                      \
            auto next = (Stage)load_and_inc(program);                                     \
            next(tail,program,dx,dy, r,g,b,a, dr,dg,db,da);                               \
        }                                                                                 \
        SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,                  \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#endif


// just_return() is a simple no-op stage that only exists to end the chain,
// returning back up to start_pipeline(), and from there to the caller.
#if defined(__i386__) || defined(_M_IX86) || defined(__arm__)
    extern "C" ABI void WRAP(just_return)(Params*, void**, F,F,F,F) {}
#else
    extern "C" ABI void WRAP(just_return)(size_t, void**, size_t,size_t, F,F,F,F, F,F,F,F) {}
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
    return unaligned_load<V>(src);
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
    unaligned_store(dst, v);
}

SI F from_byte(U8 b) {
    return cast(expand(b)) * (1/255.0f);
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
SI void from_1010102(U32 rgba, F* r, F* g, F* b, F* a) {
    *r = cast((rgba      ) & 0x3ff) * (1/1023.0f);
    *g = cast((rgba >> 10) & 0x3ff) * (1/1023.0f);
    *b = cast((rgba >> 20) & 0x3ff) * (1/1023.0f);
    *a = cast((rgba >> 30)        ) * (1/   3.0f);
}

// Used by load_ and store_ stages to get to the right (dx,dy) starting point of contiguous memory.
template <typename T>
SI T* ptr_at_xy(const SkJumper_MemoryCtx* ctx, size_t dx, size_t dy) {
    return (T*)ctx->pixels + dy*ctx->stride + dx;
}

// clamp v to [0,limit).
SI F clamp(F v, F limit) {
    F inclusive = bit_cast<F>( bit_cast<U32>(limit) - 1 );  // Exclusive -> inclusive.
    return min(max(0, v), inclusive);
}

// Used by gather_ stages to calculate the base pointer and a vector of indices to load.
template <typename T>
SI U32 ix_and_ptr(T** ptr, const SkJumper_GatherCtx* ctx, F x, F y) {
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

// Now finally, normal Stages!

STAGE(seed_shader, const float* iota) {
    // It's important for speed to explicitly cast(dx) and cast(dy),
    // which has the effect of splatting them to vectors before converting to floats.
    // On Intel this breaks a data dependency on previous loop iterations' registers.
    r = cast(dx) + unaligned_load<F>(iota);
    g = cast(dy) + 0.5f;
    b = 1.0f;
    a = 0;
    dr = dg = db = da = 0;
}

STAGE(dither, const float* rate) {
    // Get [(dx,dy), (dx+1,dy), (dx+2,dy), ...] loaded up in integer vectors.
    uint32_t iota[] = {0,1,2,3,4,5,6,7};
    U32 X = dx + unaligned_load<U32>(iota),
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
STAGE(uniform_color, const SkJumper_UniformColorCtx* c) {
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
STAGE(load_rgba, const float* ptr) {
    r = unaligned_load<F>(ptr + 0*N);
    g = unaligned_load<F>(ptr + 1*N);
    b = unaligned_load<F>(ptr + 2*N);
    a = unaligned_load<F>(ptr + 3*N);
}

// store registers r,g,b,a into context (mirrors load_rgba)
STAGE(store_rgba, float* ptr) {
    unaligned_store(ptr + 0*N, r);
    unaligned_store(ptr + 1*N, g);
    unaligned_store(ptr + 2*N, b);
    unaligned_store(ptr + 3*N, a);
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

STAGE(srcover_rgba_8888, const SkJumper_MemoryCtx* ctx) {
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

STAGE(srcover_bgra_8888, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 dst = load<U32>(ptr, tail);
    db = cast((dst      ) & 0xff);
    dg = cast((dst >>  8) & 0xff);
    dr = cast((dst >> 16) & 0xff);
    da = cast((dst >> 24)       );
    // {dr,dg,db,da} are in [0,255]
    // { r, g, b, a} are in [0,  1] (but may be out of gamut)

    r = mad(dr, inv(a), r*255.0f);
    g = mad(dg, inv(a), g*255.0f);
    b = mad(db, inv(a), b*255.0f);
    a = mad(da, inv(a), a*255.0f);
    // { r, g, b, a} are now in [0,255]  (but may be out of gamut)

    // to_unorm() clamps back to gamut.  Scaling by 1 since we're already 255-biased.
    dst = to_unorm(b, 1, 255)
        | to_unorm(g, 1, 255) <<  8
        | to_unorm(r, 1, 255) << 16
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

STAGE(clamp_a_dst, Ctx::None) {
    da = min(da, 1.0f);
    dr = min(dr, da);
    dg = min(dg, da);
    db = min(db, da);
}

STAGE(set_rgb, const float* rgb) {
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];
}
STAGE(swap_rb, Ctx::None) {
    auto tmp = r;
    r = b;
    b = tmp;
}
STAGE(invert, Ctx::None) {
    r = inv(r);
    g = inv(g);
    b = inv(b);
    a = inv(a);
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

SI F from_srgb(F s) {
    auto lo = s * (1/12.92f);
    auto hi = mad(s*s, mad(s, 0.3000f, 0.6975f), 0.0025f);
    return if_then_else(s < 0.055f, lo, hi);
}

STAGE(from_srgb, Ctx::None) {
    r = from_srgb(r);
    g = from_srgb(g);
    b = from_srgb(b);
}
STAGE(from_srgb_dst, Ctx::None) {
    dr = from_srgb(dr);
    dg = from_srgb(dg);
    db = from_srgb(db);
}
STAGE(to_srgb, Ctx::None) {
    auto fn = [&](F l) {
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
        return if_then_else(l < 0.00465985f, lo, hi);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

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
STAGE(scale_u8, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    auto scales = load<U8>(ptr, tail);
    auto c = from_byte(scales);

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(scale_565, const SkJumper_MemoryCtx* ctx) {
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
STAGE(lerp_u8, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    auto scales = load<U8>(ptr, tail);
    auto c = from_byte(scales);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_565, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    F cr,cg,cb;
    from_565(load<U16>(ptr, tail), &cr, &cg, &cb);

    F ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = lerp(da, a, ca);
}

STAGE(load_tables, const SkJumper_LoadTablesCtx* c) {
    auto px = load<U32>((const uint32_t*)c->src + dx, tail);
    r = gather(c->r, (px      ) & 0xff);
    g = gather(c->g, (px >>  8) & 0xff);
    b = gather(c->b, (px >> 16) & 0xff);
    a = cast(        (px >> 24)) * (1/255.0f);
}
STAGE(load_tables_u16_be, const SkJumper_LoadTablesCtx* c) {
    auto ptr = (const uint16_t*)c->src + 4*dx;

    U16 R,G,B,A;
    load4(ptr, tail, &R,&G,&B,&A);

    // c->src is big-endian, so & 0xff grabs the 8 most signficant bits.
    r = gather(c->r, expand(R) & 0xff);
    g = gather(c->g, expand(G) & 0xff);
    b = gather(c->b, expand(B) & 0xff);
    a = (1/65535.0f) * cast(expand(bswap(A)));
}
STAGE(load_tables_rgb_u16_be, const SkJumper_LoadTablesCtx* c) {
    auto ptr = (const uint16_t*)c->src + 3*dx;

    U16 R,G,B;
    load3(ptr, tail, &R,&G,&B);

    // c->src is big-endian, so & 0xff grabs the 8 most signficant bits.
    r = gather(c->r, expand(R) & 0xff);
    g = gather(c->g, expand(G) & 0xff);
    b = gather(c->b, expand(B) & 0xff);
    a = 1.0f;
}

STAGE(byte_tables, const void* ctx) {  // TODO: rename Tables SkJumper_ByteTablesCtx
    struct Tables { const uint8_t *r, *g, *b, *a; };
    auto tables = (const Tables*)ctx;

    r = from_byte(gather(tables->r, to_unorm(r, 255)));
    g = from_byte(gather(tables->g, to_unorm(g, 255)));
    b = from_byte(gather(tables->b, to_unorm(b, 255)));
    a = from_byte(gather(tables->a, to_unorm(a, 255)));
}

STAGE(byte_tables_rgb, const void* ctx) {  // TODO: rename Tables SkJumper_ByteTablesRGBCtx
    struct Tables { const uint8_t *r, *g, *b; int n; };
    auto tables = (const Tables*)ctx;

    int scale = tables->n - 1;
    r = from_byte(gather(tables->r, to_unorm(r, scale)));
    g = from_byte(gather(tables->g, to_unorm(g, scale)));
    b = from_byte(gather(tables->b, to_unorm(b, scale)));
}

SI F table(F v, const SkJumper_TableCtx* ctx) {
    return gather(ctx->table, to_unorm(v, ctx->size - 1));
}
STAGE(table_r, const SkJumper_TableCtx* ctx) { r = table(r, ctx); }
STAGE(table_g, const SkJumper_TableCtx* ctx) { g = table(g, ctx); }
STAGE(table_b, const SkJumper_TableCtx* ctx) { b = table(b, ctx); }
STAGE(table_a, const SkJumper_TableCtx* ctx) { a = table(a, ctx); }

SI F parametric(F v, const SkJumper_ParametricTransferFunction* ctx) {
    F r = if_then_else(v <= ctx->D, mad(ctx->C, v, ctx->F)
                                  , approx_powf(mad(ctx->A, v, ctx->B), ctx->G) + ctx->E);
    return min(max(r, 0), 1.0f);  // Clamp to [0,1], with argument order mattering to handle NaN.
}
STAGE(parametric_r, const SkJumper_ParametricTransferFunction* ctx) { r = parametric(r, ctx); }
STAGE(parametric_g, const SkJumper_ParametricTransferFunction* ctx) { g = parametric(g, ctx); }
STAGE(parametric_b, const SkJumper_ParametricTransferFunction* ctx) { b = parametric(b, ctx); }
STAGE(parametric_a, const SkJumper_ParametricTransferFunction* ctx) { a = parametric(a, ctx); }

STAGE(gamma, const float* G) {
    r = approx_powf(r, *G);
    g = approx_powf(g, *G);
    b = approx_powf(b, *G);
}
STAGE(gamma_dst, const float* G) {
    dr = approx_powf(dr, *G);
    dg = approx_powf(dg, *G);
    db = approx_powf(db, *G);
}

STAGE(lab_to_xyz, Ctx::None) {
    F L = r * 100.0f,
      A = g * 255.0f - 128.0f,
      B = b * 255.0f - 128.0f;

    F Y = (L + 16.0f) * (1/116.0f),
      X = Y + A*(1/500.0f),
      Z = Y - B*(1/200.0f);

    X = if_then_else(X*X*X > 0.008856f, X*X*X, (X - (16/116.0f)) * (1/7.787f));
    Y = if_then_else(Y*Y*Y > 0.008856f, Y*Y*Y, (Y - (16/116.0f)) * (1/7.787f));
    Z = if_then_else(Z*Z*Z > 0.008856f, Z*Z*Z, (Z - (16/116.0f)) * (1/7.787f));

    // Adjust to D50 illuminant.
    r = X * 0.96422f;
    g = Y           ;
    b = Z * 0.82521f;
}

STAGE(load_a8, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    r = g = b = 0.0f;
    a = from_byte(load<U8>(ptr, tail));
}
STAGE(load_a8_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    dr = dg = db = 0.0f;
    da = from_byte(load<U8>(ptr, tail));
}
STAGE(gather_a8, const SkJumper_GatherCtx* ctx) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = g = b = 0.0f;
    a = from_byte(gather(ptr, ix));
}
STAGE(store_a8, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint8_t>(ctx, dx,dy);

    U8 packed = pack(pack(to_unorm(a, 255)));
    store(ptr, packed, tail);
}

STAGE(load_g8, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    r = g = b = from_byte(load<U8>(ptr, tail));
    a = 1.0f;
}
STAGE(load_g8_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, dx,dy);

    dr = dg = db = from_byte(load<U8>(ptr, tail));
    da = 1.0f;
}
STAGE(gather_g8, const SkJumper_GatherCtx* ctx) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = g = b = from_byte(gather(ptr, ix));
    a = 1.0f;
}

STAGE(load_565, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    from_565(load<U16>(ptr, tail), &r,&g,&b);
    a = 1.0f;
}
STAGE(load_565_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);

    from_565(load<U16>(ptr, tail), &dr,&dg,&db);
    da = 1.0f;
}
STAGE(gather_565, const SkJumper_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_565(gather(ptr, ix), &r,&g,&b);
    a = 1.0f;
}
STAGE(store_565, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);

    U16 px = pack( to_unorm(r, 31) << 11
                 | to_unorm(g, 63) <<  5
                 | to_unorm(b, 31)      );
    store(ptr, px, tail);
}

STAGE(load_4444, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    from_4444(load<U16>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_4444_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, dx,dy);
    from_4444(load<U16>(ptr, tail), &dr,&dg,&db,&da);
}
STAGE(gather_4444, const SkJumper_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_4444(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_4444, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, dx,dy);
    U16 px = pack( to_unorm(r, 15) << 12
                 | to_unorm(g, 15) <<  8
                 | to_unorm(b, 15) <<  4
                 | to_unorm(a, 15)      );
    store(ptr, px, tail);
}

STAGE(load_8888, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_8888(load<U32>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_8888_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_8888(load<U32>(ptr, tail), &dr,&dg,&db,&da);
}
STAGE(gather_8888, const SkJumper_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_8888(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_8888, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 px = to_unorm(r, 255)
           | to_unorm(g, 255) <<  8
           | to_unorm(b, 255) << 16
           | to_unorm(a, 255) << 24;
    store(ptr, px, tail);
}

STAGE(load_bgra, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_8888(load<U32>(ptr, tail), &b,&g,&r,&a);
}
STAGE(load_bgra_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_8888(load<U32>(ptr, tail), &db,&dg,&dr,&da);
}
STAGE(gather_bgra, const SkJumper_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_8888(gather(ptr, ix), &b,&g,&r,&a);
}
STAGE(store_bgra, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 px = to_unorm(b, 255)
           | to_unorm(g, 255) <<  8
           | to_unorm(r, 255) << 16
           | to_unorm(a, 255) << 24;
    store(ptr, px, tail);
}

STAGE(load_1010102, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_1010102(load<U32>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_1010102_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, dx,dy);
    from_1010102(load<U32>(ptr, tail), &dr,&dg,&db,&da);
}
STAGE(gather_1010102, const SkJumper_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_1010102(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_1010102, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    U32 px = to_unorm(r, 1023)
           | to_unorm(g, 1023) << 10
           | to_unorm(b, 1023) << 20
           | to_unorm(a,    3) << 30;
    store(ptr, px, tail);
}

STAGE(load_f16, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx,dy);

    U16 R,G,B,A;
    load4((const uint16_t*)ptr,tail, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}
STAGE(load_f16_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, dx,dy);

    U16 R,G,B,A;
    load4((const uint16_t*)ptr,tail, &R,&G,&B,&A);
    dr = from_half(R);
    dg = from_half(G);
    db = from_half(B);
    da = from_half(A);
}
STAGE(gather_f16, const SkJumper_GatherCtx* ctx) {
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
STAGE(store_f16, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint64_t>(ctx, dx,dy);
    store4((uint16_t*)ptr,tail, to_half(r)
                              , to_half(g)
                              , to_half(b)
                              , to_half(a));
}

STAGE(load_u16_be, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, 4*dx,dy);

    U16 R,G,B,A;
    load4(ptr,tail, &R,&G,&B,&A);

    r = (1/65535.0f) * cast(expand(bswap(R)));
    g = (1/65535.0f) * cast(expand(bswap(G)));
    b = (1/65535.0f) * cast(expand(bswap(B)));
    a = (1/65535.0f) * cast(expand(bswap(A)));
}
STAGE(load_rgb_u16_be, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, 3*dx,dy);

    U16 R,G,B;
    load3(ptr,tail, &R,&G,&B);

    r = (1/65535.0f) * cast(expand(bswap(R)));
    g = (1/65535.0f) * cast(expand(bswap(G)));
    b = (1/65535.0f) * cast(expand(bswap(B)));
    a = 1.0f;
}
STAGE(store_u16_be, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, 4*dx,dy);

    U16 R = bswap(pack(to_unorm(r, 65535))),
        G = bswap(pack(to_unorm(g, 65535))),
        B = bswap(pack(to_unorm(b, 65535))),
        A = bswap(pack(to_unorm(a, 65535)));

    store4(ptr,tail, R,G,B,A);
}

STAGE(load_f32, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const float>(ctx, 4*dx,dy);
    load4(ptr,tail, &r,&g,&b,&a);
}
STAGE(load_f32_dst, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<const float>(ctx, 4*dx,dy);
    load4(ptr,tail, &dr,&dg,&db,&da);
}
STAGE(store_f32, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<float>(ctx, 4*dx,dy);
    store4(ptr,tail, r,g,b,a);
}

SI F exclusive_repeat(F v, const SkJumper_TileCtx* ctx) {
    return v - floor_(v*ctx->invScale)*ctx->scale;
}
SI F exclusive_mirror(F v, const SkJumper_TileCtx* ctx) {
    auto limit = ctx->scale;
    auto invLimit = ctx->invScale;
    return abs_( (v-limit) - (limit+limit)*floor_((v-limit)*(invLimit*0.5f)) - limit );
}
// Tile x or y to [0,limit) == [0,limit - 1 ulp] (think, sampling from images).
// The gather stages will hard clamp the output of these stages to [0,limit)...
// we just need to do the basic repeat or mirroring.
STAGE(repeat_x, const SkJumper_TileCtx* ctx) { r = exclusive_repeat(r, ctx); }
STAGE(repeat_y, const SkJumper_TileCtx* ctx) { g = exclusive_repeat(g, ctx); }
STAGE(mirror_x, const SkJumper_TileCtx* ctx) { r = exclusive_mirror(r, ctx); }
STAGE(mirror_y, const SkJumper_TileCtx* ctx) { g = exclusive_mirror(g, ctx); }

// Clamp x to [0,1], both sides inclusive (think, gradients).
// Even repeat and mirror funnel through a clamp to handle bad inputs like +Inf, NaN.
SI F clamp_01(F v) { return min(max(0, v), 1); }

STAGE( clamp_x_1, Ctx::None) { r = clamp_01(r); }
STAGE(repeat_x_1, Ctx::None) { r = clamp_01(r - floor_(r)); }
STAGE(mirror_x_1, Ctx::None) { r = clamp_01(abs_( (r-1.0f) - two(floor_((r-1.0f)*0.5f)) - 1.0f )); }

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
STAGE(matrix_3x4, const float* m) {
    auto R = mad(r,m[0], mad(g,m[3], mad(b,m[6], m[ 9]))),
         G = mad(r,m[1], mad(g,m[4], mad(b,m[7], m[10]))),
         B = mad(r,m[2], mad(g,m[5], mad(b,m[8], m[11])));
    r = R;
    g = G;
    b = B;
}
STAGE(matrix_4x5, const float* m) {
    auto R = mad(r,m[0], mad(g,m[4], mad(b,m[ 8], mad(a,m[12], m[16])))),
         G = mad(r,m[1], mad(g,m[5], mad(b,m[ 9], mad(a,m[13], m[17])))),
         B = mad(r,m[2], mad(g,m[6], mad(b,m[10], mad(a,m[14], m[18])))),
         A = mad(r,m[3], mad(g,m[7], mad(b,m[11], mad(a,m[15], m[19]))));
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

SI void gradient_lookup(const SkJumper_GradientCtx* c, U32 idx, F t,
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

STAGE(evenly_spaced_gradient, const SkJumper_GradientCtx* c) {
    auto t = r;
    auto idx = trunc_(t * (c->stopCount-1));
    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE(gradient, const SkJumper_GradientCtx* c) {
    auto t = r;
    U32 idx = 0;

    // N.B. The loop starts at 1 because idx 0 is the color to use before the first stop.
    for (size_t i = 1; i < c->stopCount; i++) {
        idx += if_then_else(t >= c->ts[i], U32(1), U32(0));
    }

    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE(evenly_spaced_2_stop_gradient, const void* ctx) {
    // TODO: Rename Ctx SkJumper_EvenlySpaced2StopGradientCtx.
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

STAGE(xy_to_2pt_conical_strip, const SkJumper_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = x + sqrt_(ctx->fP0 - y*y); // ctx->fP0 = r0 * r0
}

STAGE(xy_to_2pt_conical_focal_on_circle, Ctx::None) {
    F x = r, y = g, &t = r;
    t = x + y*y / x; // (x^2 + y^2) / x
}

STAGE(xy_to_2pt_conical_well_behaved, const SkJumper_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = sqrt_(x*x + y*y) - x * ctx->fP0; // ctx->fP0 = 1/r1
}

STAGE(xy_to_2pt_conical_greater, const SkJumper_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = sqrt_(x*x - y*y) - x * ctx->fP0; // ctx->fP0 = 1/r1
}

STAGE(xy_to_2pt_conical_smaller, const SkJumper_2PtConicalCtx* ctx) {
    F x = r, y = g, &t = r;
    t = -sqrt_(x*x - y*y) - x * ctx->fP0; // ctx->fP0 = 1/r1
}

STAGE(alter_2pt_conical_compensate_focal, const SkJumper_2PtConicalCtx* ctx) {
    F& t = r;
    t = t + ctx->fP1; // ctx->fP1 = f
}

STAGE(alter_2pt_conical_unswap, Ctx::None) {
    F& t = r;
    t = 1 - t;
}

STAGE(mask_2pt_conical_nan, SkJumper_2PtConicalCtx* c) {
    F& t = r;
    auto is_degenerate = (t != t); // NaN
    t = if_then_else(is_degenerate, F(0), t);
    unaligned_store(&c->fMask, if_then_else(is_degenerate, U32(0), U32(0xffffffff)));
}

STAGE(mask_2pt_conical_degenerates, SkJumper_2PtConicalCtx* c) {
    F& t = r;
    auto is_degenerate = (t <= 0) | (t != t);
    t = if_then_else(is_degenerate, F(0), t);
    unaligned_store(&c->fMask, if_then_else(is_degenerate, U32(0), U32(0xffffffff)));
}

STAGE(apply_vector_mask, const uint32_t* ctx) {
    const U32 mask = unaligned_load<U32>(ctx);
    r = bit_cast<F>(bit_cast<U32>(r) & mask);
    g = bit_cast<F>(bit_cast<U32>(g) & mask);
    b = bit_cast<F>(bit_cast<U32>(b) & mask);
    a = bit_cast<F>(bit_cast<U32>(a) & mask);
}

STAGE(save_xy, SkJumper_SamplerCtx* c) {
    // Whether bilinear or bicubic, all sample points are at the same fractional offset (fx,fy).
    // They're either the 4 corners of a logical 1x1 pixel or the 16 corners of a 3x3 grid
    // surrounding (x,y) at (0.5,0.5) off-center.
    F fx = fract(r + 0.5f),
      fy = fract(g + 0.5f);

    // Samplers will need to load x and fx, or y and fy.
    unaligned_store(c->x,  r);
    unaligned_store(c->y,  g);
    unaligned_store(c->fx, fx);
    unaligned_store(c->fy, fy);
}

STAGE(accumulate, const SkJumper_SamplerCtx* c) {
    // Bilinear and bicubic filters are both separable, so we produce independent contributions
    // from x and y, multiplying them together here to get each pixel's total scale factor.
    auto scale = unaligned_load<F>(c->scalex)
               * unaligned_load<F>(c->scaley);
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
SI void bilinear_x(SkJumper_SamplerCtx* ctx, F* x) {
    *x = unaligned_load<F>(ctx->x) + (kScale * 0.5f);
    F fx = unaligned_load<F>(ctx->fx);

    F scalex;
    if (kScale == -1) { scalex = 1.0f - fx; }
    if (kScale == +1) { scalex =        fx; }
    unaligned_store(ctx->scalex, scalex);
}
template <int kScale>
SI void bilinear_y(SkJumper_SamplerCtx* ctx, F* y) {
    *y = unaligned_load<F>(ctx->y) + (kScale * 0.5f);
    F fy = unaligned_load<F>(ctx->fy);

    F scaley;
    if (kScale == -1) { scaley = 1.0f - fy; }
    if (kScale == +1) { scaley =        fy; }
    unaligned_store(ctx->scaley, scaley);
}

STAGE(bilinear_nx, SkJumper_SamplerCtx* ctx) { bilinear_x<-1>(ctx, &r); }
STAGE(bilinear_px, SkJumper_SamplerCtx* ctx) { bilinear_x<+1>(ctx, &r); }
STAGE(bilinear_ny, SkJumper_SamplerCtx* ctx) { bilinear_y<-1>(ctx, &g); }
STAGE(bilinear_py, SkJumper_SamplerCtx* ctx) { bilinear_y<+1>(ctx, &g); }


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
SI void bicubic_x(SkJumper_SamplerCtx* ctx, F* x) {
    *x = unaligned_load<F>(ctx->x) + (kScale * 0.5f);
    F fx = unaligned_load<F>(ctx->fx);

    F scalex;
    if (kScale == -3) { scalex = bicubic_far (1.0f - fx); }
    if (kScale == -1) { scalex = bicubic_near(1.0f - fx); }
    if (kScale == +1) { scalex = bicubic_near(       fx); }
    if (kScale == +3) { scalex = bicubic_far (       fx); }
    unaligned_store(ctx->scalex, scalex);
}
template <int kScale>
SI void bicubic_y(SkJumper_SamplerCtx* ctx, F* y) {
    *y = unaligned_load<F>(ctx->y) + (kScale * 0.5f);
    F fy = unaligned_load<F>(ctx->fy);

    F scaley;
    if (kScale == -3) { scaley = bicubic_far (1.0f - fy); }
    if (kScale == -1) { scaley = bicubic_near(1.0f - fy); }
    if (kScale == +1) { scaley = bicubic_near(       fy); }
    if (kScale == +3) { scaley = bicubic_far (       fy); }
    unaligned_store(ctx->scaley, scaley);
}

STAGE(bicubic_n3x, SkJumper_SamplerCtx* ctx) { bicubic_x<-3>(ctx, &r); }
STAGE(bicubic_n1x, SkJumper_SamplerCtx* ctx) { bicubic_x<-1>(ctx, &r); }
STAGE(bicubic_p1x, SkJumper_SamplerCtx* ctx) { bicubic_x<+1>(ctx, &r); }
STAGE(bicubic_p3x, SkJumper_SamplerCtx* ctx) { bicubic_x<+3>(ctx, &r); }

STAGE(bicubic_n3y, SkJumper_SamplerCtx* ctx) { bicubic_y<-3>(ctx, &g); }
STAGE(bicubic_n1y, SkJumper_SamplerCtx* ctx) { bicubic_y<-1>(ctx, &g); }
STAGE(bicubic_p1y, SkJumper_SamplerCtx* ctx) { bicubic_y<+1>(ctx, &g); }
STAGE(bicubic_p3y, SkJumper_SamplerCtx* ctx) { bicubic_y<+3>(ctx, &g); }

STAGE(callback, SkJumper_CallbackCtx* c) {
    store4(c->rgba,0, r,g,b,a);
    c->fn(c, tail ? tail : N);
    load4(c->read_from,0, &r,&g,&b,&a);
}

// Our general strategy is to recursively interpolate each dimension,
// accumulating the index to sample at, and our current pixel stride to help accumulate the index.
template <int dim>
SI void color_lookup_table(const SkJumper_ColorLookupTableCtx* ctx,
                           F& r, F& g, F& b, F a, U32 index, U32 stride) {
    // We'd logically like to sample this dimension at x.
    int limit = ctx->limits[dim-1];
    F src;
    switch(dim) {
        case 1: src = r; break;
        case 2: src = g; break;
        case 3: src = b; break;
        case 4: src = a; break;
    }
    F x = src * (limit - 1);

    // We can't index an array by a float (darn) so we have to snap to nearby integers lo and hi.
    U32 lo = trunc_(x          ),
        hi = trunc_(x + 0.9999f);

    // Recursively sample at lo and hi.
    F lr = r, lg = g, lb = b,
      hr = r, hg = g, hb = b;
    color_lookup_table<dim-1>(ctx, lr,lg,lb,a, stride*lo + index, stride*limit);
    color_lookup_table<dim-1>(ctx, hr,hg,hb,a, stride*hi + index, stride*limit);

    // Linearly interpolate those colors based on their distance to x.
    F t = x - cast(lo);
    r = lerp(lr, hr, t);
    g = lerp(lg, hg, t);
    b = lerp(lb, hb, t);
}

// Bottom out our recursion at 0 dimensions, i.e. just return the colors at index.
template<>
inline void color_lookup_table<0>(const SkJumper_ColorLookupTableCtx* ctx,
                                  F& r, F& g, F& b, F a, U32 index, U32 stride) {
    r = gather(ctx->table, 3*index+0);
    g = gather(ctx->table, 3*index+1);
    b = gather(ctx->table, 3*index+2);
}

STAGE(clut_3D, const SkJumper_ColorLookupTableCtx* ctx) {
    color_lookup_table<3>(ctx, r,g,b,a, 0,1);
    // This 3D color lookup table leaves alpha alone.
}
STAGE(clut_4D, const SkJumper_ColorLookupTableCtx* ctx) {
    color_lookup_table<4>(ctx, r,g,b,a, 0,1);
    // "a" was really CMYK's K, so we just set alpha opaque.
    a = 1.0f;
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
STAGE(bilerp_clamp_8888, SkJumper_GatherCtx* ctx) {
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
