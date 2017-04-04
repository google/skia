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

    using F   = float;
    using I32 =  int32_t;
    using U32 = uint32_t;
    using U16 = uint16_t;
    using U8  = uint8_t;

    SI F   mad(F f, F m, F a)   { return f*m+a; }
    SI F   min(F a, F b)        { return fminf(a,b); }
    SI F   max(F a, F b)        { return fmaxf(a,b); }
    SI F   abs_  (F v)          { return fabsf(v); }
    SI F   floor_(F v)          { return floorf(v); }
    SI F   rcp   (F v)          { return 1.0f / v; }
    SI F   rsqrt (F v)          { return 1.0f / sqrtf(v); }
    SI U32 round (F v, F scale) { return (uint32_t)lrintf(v*scale); }
    SI U16 pack(U32 v)          { return (U16)v; }
    SI U8  pack(U16 v)          { return  (U8)v; }

    SI F if_then_else(I32 c, F t, F e) { return c ? t : e; }

    SI F gather(const float* p, U32 ix) { return p[ix]; }

    SI void load4(const void* vptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        auto ptr = (const uint16_t*)vptr;
        *r = ptr[0];
        *g = ptr[1];
        *b = ptr[2];
        *a = ptr[3];
    }
    SI void store4(void* vptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        auto ptr = (uint16_t*)vptr;
        ptr[0] = r;
        ptr[1] = g;
        ptr[2] = b;
        ptr[3] = a;
    }

    SI F from_half(U16 h) {
        if ((int16_t)h < 0x0400) { h = 0; }   // Flush denorm and negative to zero.
        return bit_cast<F>(h << 13)           // Line up the mantissa,
             * bit_cast<F>(U32(0x77800000));  // then fix up the exponent.
    }
    SI U16 to_half(F f) {
        return bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
            >> 13;                                                // then line up the mantissa.
    }

#elif defined(__aarch64__)
    #include <arm_neon.h>

    // Since we know we're using Clang, we can use its vector extensions.
    using F   = float    __attribute__((ext_vector_type(4)));
    using I32 =  int32_t __attribute__((ext_vector_type(4)));
    using U32 = uint32_t __attribute__((ext_vector_type(4)));
    using U16 = uint16_t __attribute__((ext_vector_type(4)));
    using U8  = uint8_t  __attribute__((ext_vector_type(4)));

    // We polyfill a few routines that Clang doesn't build into ext_vector_types.
    SI F   mad(F f, F m, F a)                    { return vfmaq_f32(a,f,m);        }
    SI F   min(F a, F b)                         { return vminq_f32(a,b);          }
    SI F   max(F a, F b)                         { return vmaxq_f32(a,b);          }
    SI F   abs_  (F v)                           { return vabsq_f32(v);            }
    SI F   floor_(F v)                           { return vrndmq_f32(v);           }
    SI F   rcp   (F v) { auto e = vrecpeq_f32 (v); return vrecpsq_f32 (v,e  ) * e; }
    SI F   rsqrt (F v) { auto e = vrsqrteq_f32(v); return vrsqrtsq_f32(v,e*e) * e; }
    SI U32 round (F v, F scale)                  { return vcvtnq_u32_f32(v*scale); }
    SI U16 pack(U32 v)                           { return __builtin_convertvector(v, U16); }
    SI U8  pack(U16 v)                           { return __builtin_convertvector(v,  U8); }

    SI F if_then_else(I32 c, F t, F e) { return vbslq_f32((U32)c,t,e); }

    SI F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]}; }

    SI void load4(const void* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        uint16x4x4_t rgba = vld4_u16((const uint16_t*)ptr);
        *r = rgba.val[0];
        *g = rgba.val[1];
        *b = rgba.val[2];
        *a = rgba.val[3];
    }
    SI void store4(void* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        uint16x4x4_t rgba = {{r,g,b,a}};
        vst4_u16((uint16_t*)ptr, rgba);
    }

    SI F from_half(U16 h) { return vcvt_f32_f16(h); }
    SI U16 to_half(F   f) { return vcvt_f16_f32(f); }

#elif defined(__arm__)
    #if defined(__thumb2__) || !defined(__ARM_ARCH_7A__) || !defined(__ARM_VFPV4__)
        #error On ARMv7, compile with -march=armv7-a -mfpu=neon-vfp4, without -mthumb.
    #endif
    #include <arm_neon.h>

    // We can pass {s0-s15} as arguments under AAPCS-VFP.  We'll slice that as 8 d-registers.
    using F   = float    __attribute__((ext_vector_type(2)));
    using I32 =  int32_t __attribute__((ext_vector_type(2)));
    using U32 = uint32_t __attribute__((ext_vector_type(2)));
    using U16 = uint16_t __attribute__((ext_vector_type(2)));
    using U8  = uint8_t  __attribute__((ext_vector_type(2)));

    SI F   mad(F f, F m, F a)                  { return vfma_f32(a,f,m);        }
    SI F   min(F a, F b)                       { return vmin_f32(a,b);          }
    SI F   max(F a, F b)                       { return vmax_f32(a,b);          }
    SI F   abs_ (F v)                          { return vabs_f32(v);            }
    SI F   rcp  (F v) { auto e = vrecpe_f32 (v); return vrecps_f32 (v,e  ) * e; }
    SI F   rsqrt(F v) { auto e = vrsqrte_f32(v); return vrsqrts_f32(v,e*e) * e; }
    SI U32 round(F v, F scale)                 { return vcvt_u32_f32(mad(v,scale,0.5f)); }
    SI U16 pack(U32 v)                         { return __builtin_convertvector(v, U16); }
    SI U8  pack(U16 v)                         { return __builtin_convertvector(v,  U8); }

    SI F if_then_else(I32 c, F t, F e) { return vbsl_f32((U32)c,t,e); }

    SI F floor_(F v) {
        F roundtrip = vcvt_f32_s32(vcvt_s32_f32(v));
        return roundtrip - if_then_else(roundtrip > v, 1.0_f, 0);
    }

    SI F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]]}; }

    SI void load4(const void* vptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
        auto ptr = (const uint16_t*)vptr;
        uint16x4x4_t rgba;
        rgba = vld4_lane_u16(ptr + 0, rgba, 0);
        rgba = vld4_lane_u16(ptr + 4, rgba, 1);
        *r = unaligned_load<U16>(rgba.val+0);
        *g = unaligned_load<U16>(rgba.val+1);
        *b = unaligned_load<U16>(rgba.val+2);
        *a = unaligned_load<U16>(rgba.val+3);
    }
    SI void store4(void* vptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        auto ptr = (uint16_t*)vptr;
        uint16x4x4_t rgba = {{
            widen_cast<uint16x4_t>(r),
            widen_cast<uint16x4_t>(g),
            widen_cast<uint16x4_t>(b),
            widen_cast<uint16x4_t>(a),
        }};
        vst4_lane_u16(ptr + 0, rgba, 0);
        vst4_lane_u16(ptr + 4, rgba, 1);
    }

    SI F from_half(U16 h) {
        auto v = widen_cast<uint16x4_t>(h);
        return vget_low_f32(vcvt_f32_f16(v));
    }
    SI U16 to_half(F f) {
        auto v = widen_cast<float32x4_t>(f);
        uint16x4_t h = vcvt_f16_f32(v);
        return unaligned_load<U16>(&h);
    }

#elif defined(__AVX__)
    #include <immintrin.h>

    // These are __m256 and __m256i, but friendlier and strongly-typed.
    using F   = float    __attribute__((ext_vector_type(8)));
    using I32 =  int32_t __attribute__((ext_vector_type(8)));
    using U32 = uint32_t __attribute__((ext_vector_type(8)));
    using U16 = uint16_t __attribute__((ext_vector_type(8)));
    using U8  = uint8_t  __attribute__((ext_vector_type(8)));

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

    SI F gather(const float* p, U32 ix) {
    #if defined(__AVX2__)
        return _mm256_i32gather_ps(p, ix, 4);
    #else
        return { p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]],
                 p[ix[4]], p[ix[5]], p[ix[6]], p[ix[7]], };
    #endif
    }

    SI void load4(const void* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
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
    SI void store4(void* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
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

    SI F from_half(U16 h) {
    #if defined(__AVX2__)
        return _mm256_cvtph_ps(h);
    #else
        // This technique would slow down ~10x for denorm inputs, so we flush them to zero.
        // With a signed comparison this conveniently also flushes negative half floats to zero.
        h = _mm_andnot_si128(_mm_cmplt_epi16(h, _mm_set1_epi32(0x04000400_i)), h);

        U32 w = _mm256_setr_m128i(_mm_unpacklo_epi16(h, _mm_setzero_si128()),
                                  _mm_unpackhi_epi16(h, _mm_setzero_si128()));
        return bit_cast<F>(w << 13)             // Line up the mantissa,
             * bit_cast<F>(U32(0x77800000_i));  // then fix up the exponent.
    #endif
    }
    SI U16 to_half(F f) {
    #if defined(__AVX2__)
        return _mm256_cvtps_ph(f, _MM_FROUND_CUR_DIRECTION);
    #else
        return pack(bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
                    >> 13);                                            // then line up the mantissa.
    #endif
    }

#elif defined(__SSE2__)
    #include <immintrin.h>

    using F   = float    __attribute__((ext_vector_type(4)));
    using I32 =  int32_t __attribute__((ext_vector_type(4)));
    using U32 = uint32_t __attribute__((ext_vector_type(4)));
    using U16 = uint16_t __attribute__((ext_vector_type(4)));
    using U8  = uint8_t  __attribute__((ext_vector_type(4)));

    SI F   mad(F f, F m, F a)  { return f*m+a;              }
    SI F   min(F a, F b)       { return _mm_min_ps(a,b);    }
    SI F   max(F a, F b)       { return _mm_max_ps(a,b);    }
    SI F   abs_(F v)           { return _mm_and_ps(v, 0-v); }
    SI F   rcp  (F v)          { return _mm_rcp_ps  (v);    }
    SI F   rsqrt(F v)          { return _mm_rsqrt_ps(v);    }
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
        return roundtrip - if_then_else(roundtrip > v, 1.0_f, 0);
    #endif
    }

    SI F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]}; }

    SI void load4(const void* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
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
    SI void store4(const void* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
        auto rg = _mm_unpacklo_epi16(widen_cast<__m128i>(r), widen_cast<__m128i>(g)),
             ba = _mm_unpacklo_epi16(widen_cast<__m128i>(b), widen_cast<__m128i>(a));
        _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi32(rg, ba));
        _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi32(rg, ba));
    }

    SI F from_half(U16 h) {
        auto v = widen_cast<__m128i>(h);

        // Same deal as AVX: flush denorms and negatives to zero.
        v = _mm_andnot_si128(_mm_cmplt_epi16(v, _mm_set1_epi32(0x04000400_i)), v);

        U32 w = _mm_unpacklo_epi16(v, _mm_setzero_si128());
        return bit_cast<F>(w << 13)             // Line up the mantissa,
             * bit_cast<F>(U32(0x77800000_i));  // then fix up the exponent.
    }
    SI U16 to_half(F f) {
        return pack(bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
                    >> 13);                                            // then line up the mantissa.
    }
#endif

// We need to be a careful with casts.
// (F)x means cast x to float in the portable path, but bit_cast x to float in the others.
// These named casts and bit_cast() are always what they seem to be.
#if defined(JUMPER)
    SI F   cast  (U32 v) { return __builtin_convertvector((I32)v, F);   }
    SI U32 expand(U16 v) { return __builtin_convertvector(     v, U32); }
    SI U32 expand(U8  v) { return __builtin_convertvector(     v, U32); }
#else
    SI F   cast  (U32 v) { return   (F)v; }
    SI U32 expand(U16 v) { return (U32)v; }
    SI U32 expand(U8  v) { return (U32)v; }
#endif

#endif//SkJumper_vectors_DEFINED
