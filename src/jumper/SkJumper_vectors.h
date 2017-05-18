/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_vectors_DEFINED
#define SkJumper_vectors_DEFINED

#include "SkJumper.h"
#include "SkJumper_misc.h"

// This file contains vector types that SkJumper_stages.cpp uses to define stages.

// Every function in this file should be marked static and inline using SI (see SkJumper_misc.h).

#if !defined(JUMPER)
    // This path should lead to portable code that can be compiled directly into Skia.
    // (All other paths are compiled offline by Clang into SkJumper_generated.S.)
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
    SI U32 round (F v, F scale) { return (uint32_t)lrintf(v*scale); }
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

#elif defined(__aarch64__)
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
    SI F   mad(F f, F m, F a)                    { return vfmaq_f32(a,f,m);        }
    SI F   min(F a, F b)                         { return vminq_f32(a,b);          }
    SI F   max(F a, F b)                         { return vmaxq_f32(a,b);          }
    SI F   abs_  (F v)                           { return vabsq_f32(v);            }
    SI F   floor_(F v)                           { return vrndmq_f32(v);           }
    SI F   rcp   (F v) { auto e = vrecpeq_f32 (v); return vrecpsq_f32 (v,e  ) * e; }
    SI F   rsqrt (F v) { auto e = vrsqrteq_f32(v); return vrsqrtsq_f32(v,e*e) * e; }
    SI F    sqrt_(F v)                           { return vsqrtq_f32(v); }
    SI U32 round (F v, F scale)                  { return vcvtnq_u32_f32(v*scale); }
    SI U16 pack(U32 v)                           { return __builtin_convertvector(v, U16); }
    SI U8  pack(U16 v)                           { return __builtin_convertvector(v,  U8); }

    SI F if_then_else(I32 c, F t, F e) { return vbslq_f32((U32)c,t,e); }

    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]};
    }

    SI void load3(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
        uint16x4x3_t rgb = vld3_u16(ptr);
        *r = rgb.val[0];
        *g = rgb.val[1];
        *b = rgb.val[2];
    }
    SI void load4(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        uint16x4x4_t rgba = vld4_u16(ptr);
        *r = rgba.val[0];
        *g = rgba.val[1];
        *b = rgba.val[2];
        *a = rgba.val[3];
    }
    SI void store4(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        vst4_u16(ptr, (uint16x4x4_t{{r,g,b,a}}));
    }

    SI void load4(const float* ptr, size_t tail, F* r, F* g, F* b, F* a) {
        float32x4x4_t rgba = vld4q_f32(ptr);
        *r = rgba.val[0];
        *g = rgba.val[1];
        *b = rgba.val[2];
        *a = rgba.val[3];
    }
    SI void store4(float* ptr, size_t tail, F r, F g, F b, F a) {
        vst4q_f32(ptr, (float32x4x4_t{{r,g,b,a}}));
    }

#elif defined(__arm__)
    #if defined(__thumb2__) || !defined(__ARM_ARCH_7A__) || !defined(__ARM_VFPV4__)
        #error On ARMv7, compile with -march=armv7-a -mfpu=neon-vfp4, without -mthumb.
    #endif
    #include <arm_neon.h>

    // We can pass {s0-s15} as arguments under AAPCS-VFP.  We'll slice that as 8 d-registers.
    template <typename T> using V = T __attribute__((ext_vector_type(2)));
    using F   = V<float   >;
    using I32 = V< int32_t>;
    using U64 = V<uint64_t>;
    using U32 = V<uint32_t>;
    using U16 = V<uint16_t>;
    using U8  = V<uint8_t >;

    SI F   mad(F f, F m, F a)                  { return vfma_f32(a,f,m);        }
    SI F   min(F a, F b)                       { return vmin_f32(a,b);          }
    SI F   max(F a, F b)                       { return vmax_f32(a,b);          }
    SI F   abs_ (F v)                          { return vabs_f32(v);            }
    SI F   rcp  (F v) { auto e = vrecpe_f32 (v); return vrecps_f32 (v,e  ) * e; }
    SI F   rsqrt(F v) { auto e = vrsqrte_f32(v); return vrsqrts_f32(v,e*e) * e; }
    SI U32 round(F v, F scale)                 { return vcvt_u32_f32(mad(v,scale,0.5f)); }
    SI U16 pack(U32 v)                         { return __builtin_convertvector(v, U16); }
    SI U8  pack(U16 v)                         { return __builtin_convertvector(v,  U8); }

    SI F sqrt_(F v) {
        auto e = vrsqrte_f32(v);  // Estimate and two refinement steps for e = rsqrt(v).
        e *= vrsqrts_f32(v,e*e);
        e *= vrsqrts_f32(v,e*e);
        return v*e;               // sqrt(v) == v*rsqrt(v).
    }

    SI F if_then_else(I32 c, F t, F e) { return vbsl_f32((U32)c,t,e); }

    SI F floor_(F v) {
        F roundtrip = vcvt_f32_s32(vcvt_s32_f32(v));
        return roundtrip - if_then_else(roundtrip > v, 1, 0);
    }

    template <typename T>
    SI V<T> gather(const T* p, U32 ix) {
        return {p[ix[0]], p[ix[1]]};
    }

    SI void load3(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
        uint16x4x3_t rgb;
        rgb = vld3_lane_u16(ptr + 0, rgb, 0);
        rgb = vld3_lane_u16(ptr + 3, rgb, 1);
        *r = unaligned_load<U16>(rgb.val+0);
        *g = unaligned_load<U16>(rgb.val+1);
        *b = unaligned_load<U16>(rgb.val+2);
    }
    SI void load4(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        uint16x4x4_t rgba;
        rgba = vld4_lane_u16(ptr + 0, rgba, 0);
        rgba = vld4_lane_u16(ptr + 4, rgba, 1);
        *r = unaligned_load<U16>(rgba.val+0);
        *g = unaligned_load<U16>(rgba.val+1);
        *b = unaligned_load<U16>(rgba.val+2);
        *a = unaligned_load<U16>(rgba.val+3);
    }
    SI void store4(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        uint16x4x4_t rgba = {{
            widen_cast<uint16x4_t>(r),
            widen_cast<uint16x4_t>(g),
            widen_cast<uint16x4_t>(b),
            widen_cast<uint16x4_t>(a),
        }};
        vst4_lane_u16(ptr + 0, rgba, 0);
        vst4_lane_u16(ptr + 4, rgba, 1);
    }

    SI void load4(const float* ptr, size_t tail, F* r, F* g, F* b, F* a) {
        float32x2x4_t rgba = vld4_f32(ptr);
        *r = rgba.val[0];
        *g = rgba.val[1];
        *b = rgba.val[2];
        *a = rgba.val[3];
    }
    SI void store4(float* ptr, size_t tail, F r, F g, F b, F a) {
        vst4_f32(ptr, (float32x2x4_t{{r,g,b,a}}));
    }


#elif defined(__AVX__)
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
    #if defined(__FMA__)
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
    #if defined(__AVX2__)
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
            if (tail > 0) { _0 = load_rgb(ptr +  0); }
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
            _0 = _01; _1 = _mm_srli_si128(_01, 6),
            _2 = _23; _3 = _mm_srli_si128(_23, 6),
            _4 = _45; _5 = _mm_srli_si128(_45, 6),
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

#elif defined(__SSE2__)
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
    #if defined(__SSE4_1__)
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
    #if defined(__SSE4_1__)
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
        // Load slightly weirdly to make sure we don't load past the end of 4x48 bits.
        auto _01 =                _mm_loadu_si128((const __m128i*)(ptr + 0))    ,
             _23 = _mm_srli_si128(_mm_loadu_si128((const __m128i*)(ptr + 4)), 4);

        // Each _N holds R,G,B for pixel N in its lower 3 lanes (upper 5 are ignored).
        auto _0 = _01, _1 = _mm_srli_si128(_01, 6),
             _2 = _23, _3 = _mm_srli_si128(_23, 6);

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
        auto _01 = _mm_loadu_si128(((__m128i*)ptr) + 0),
             _23 = _mm_loadu_si128(((__m128i*)ptr) + 1);

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
        _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi32(rg, ba));
        _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi32(rg, ba));
    }

    SI void load4(const float* ptr, size_t tail, F* r, F* g, F* b, F* a) {
        auto _0 = _mm_loadu_ps(ptr+ 0),
             _1 = _mm_loadu_ps(ptr+ 4),
             _2 = _mm_loadu_ps(ptr+ 8),
             _3 = _mm_loadu_ps(ptr+12);
        _MM_TRANSPOSE4_PS(_0,_1,_2,_3);
        *r = _0;
        *g = _1;
        *b = _2;
        *a = _3;
    }
    SI void store4(float* ptr, size_t tail, F r, F g, F b, F a) {
        _MM_TRANSPOSE4_PS(r,g,b,a);
        _mm_storeu_ps(ptr+ 0, r);
        _mm_storeu_ps(ptr+ 4, g);
        _mm_storeu_ps(ptr+ 8, b);
        _mm_storeu_ps(ptr+12, a);
    }
#endif

// We need to be a careful with casts.
// (F)x means cast x to float in the portable path, but bit_cast x to float in the others.
// These named casts and bit_cast() are always what they seem to be.
#if defined(JUMPER)
    SI F   cast  (U32 v) { return      __builtin_convertvector((I32)v,   F); }
    SI U32 trunc_(F   v) { return (U32)__builtin_convertvector(     v, I32); }
    SI U32 expand(U16 v) { return      __builtin_convertvector(     v, U32); }
    SI U32 expand(U8  v) { return      __builtin_convertvector(     v, U32); }
#else
    SI F   cast  (U32 v) { return   (F)v; }
    SI U32 trunc_(F   v) { return (U32)v; }
    SI U32 expand(U16 v) { return (U32)v; }
    SI U32 expand(U8  v) { return (U32)v; }
#endif

template <typename V>
SI V if_then_else(I32 c, V t, V e) {
    return bit_cast<V>(if_then_else(c, bit_cast<F>(t), bit_cast<F>(e)));
}

SI U16 bswap(U16 x) {
#if defined(JUMPER) && defined(__SSE2__) && !defined(__AVX__)
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
    return approx_pow2(approx_log2(x) * y);
}

SI F from_half(U16 h) {
#if defined(JUMPER) && defined(__aarch64__)
    return vcvt_f32_f16(h);

#elif defined(JUMPER) && defined(__arm__)
    auto v = widen_cast<uint16x4_t>(h);
    return vget_low_f32(vcvt_f32_f16(v));

#elif defined(JUMPER) && defined(__AVX2__)
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
#if defined(JUMPER) && defined(__aarch64__)
    return vcvt_f16_f32(f);

#elif defined(JUMPER) && defined(__arm__)
    auto v = widen_cast<float32x4_t>(f);
    uint16x4_t h = vcvt_f16_f32(v);
    return unaligned_load<U16>(&h);

#elif defined(JUMPER) && defined(__AVX2__)
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



#endif//SkJumper_vectors_DEFINED
