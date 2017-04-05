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
        __m128i r;
        memcpy(&r, &v, sizeof(v));
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
