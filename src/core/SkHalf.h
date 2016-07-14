/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHalf_DEFINED
#define SkHalf_DEFINED

#include "SkNx.h"
#include "SkTypes.h"

// 16-bit floating point value
// format is 1 bit sign, 5 bits exponent, 10 bits mantissa
// only used for storage
typedef uint16_t SkHalf;

#define SK_HalfMin      0x0400   // 2^-24  (minimum positive normal value)
#define SK_HalfMax      0x7bff   // 65504
#define SK_HalfEpsilon  0x1400   // 2^-10

// convert between half and single precision floating point
float SkHalfToFloat(SkHalf h);
SkHalf SkFloatToHalf(float f);

// Convert between half and single precision floating point, but pull any dirty
// trick we can to make it faster as long as it's correct enough for values in [0,1].
static inline     Sk4f SkHalfToFloat_01(uint64_t);
static inline uint64_t SkFloatToHalf_01(const Sk4f&);

// ~~~~~~~~~~~ impl ~~~~~~~~~~~~~~ //

// Like the serial versions in SkHalf.cpp, these are based on
// https://fgiesen.wordpress.com/2012/03/28/half-to-float-done-quic/

// GCC 4.9 lacks the intrinsics to use ARMv8 f16<->f32 instructions, so we use inline assembly.

static inline Sk4f SkHalfToFloat_01(uint64_t hs) {
#if !defined(SKNX_NO_SIMD) && defined(SK_CPU_ARM64)
    float32x4_t fs;
    asm ("fmov  %d[fs], %[hs]        \n"   // vcreate_f16(hs)
         "fcvtl %[fs].4s, %[fs].4h   \n"   // vcvt_f32_f16(...)
        : [fs] "=w" (fs)                   // =w: write-only NEON register
        : [hs] "r" (hs));                  //  r: read-only 64-bit general register
    return fs;

#elif !defined(SKNX_NO_SIMD) && defined(SK_ARM_HAS_NEON)
    // NEON makes this pretty easy:
    //   - denormals are 10-bit * 2^-14 == 24-bit fixed point;
    //   - handle normals the same way as in SSE: align mantissa, then rebias exponent.
    uint32x4_t h = vmovl_u16(vcreate_u16(hs)),
               is_denorm = vcltq_u32(h, vdupq_n_u32(1<<10));
    float32x4_t denorm = vcvtq_n_f32_u32(h, 24),
                  norm = vreinterpretq_f32_u32(vaddq_u32(vshlq_n_u32(h, 13),
                                                         vdupq_n_u32((127-15) << 23)));
    return vbslq_f32(is_denorm, denorm, norm);

#elif !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    // If our input is a normal 16-bit float, things are pretty easy:
    //   - shift left by 13 to put the mantissa in the right place;
    //   - the exponent is wrong, but it just needs to be rebiased;
    //   - re-bias the exponent from 15-bias to 127-bias by adding (127-15).

    // If our input is denormalized, we're going to do the same steps, plus a few more fix ups:
    //   - the input is h = K*2^-14, for some 10-bit fixed point K in [0,1);
    //   - by shifting left 13 and adding (127-15) to the exponent, we constructed the float value
    //     2^-15*(1+K);
    //   - we'd need to subtract 2^-15 and multiply by 2 to get back to K*2^-14, or equivallently
    //     multiply by 2 then subtract 2^-14.
    //
    //   - We'll work that multiply by 2 into the rebias, by adding 1 more to the exponent.
    //   - Conveniently, this leaves that rebias constant 2^-14, exactly what we want to subtract.

    __m128i h = _mm_unpacklo_epi16(_mm_loadl_epi64((const __m128i*)&hs), _mm_setzero_si128());
    const __m128i is_denorm = _mm_cmplt_epi32(h, _mm_set1_epi32(1<<10));

    __m128i rebias = _mm_set1_epi32((127-15) << 23);
    rebias = _mm_add_epi32(rebias, _mm_and_si128(is_denorm, _mm_set1_epi32(1<<23)));

    __m128i f = _mm_add_epi32(_mm_slli_epi32(h, 13), rebias);
    return _mm_sub_ps(_mm_castsi128_ps(f),
                      _mm_castsi128_ps(_mm_and_si128(is_denorm, rebias)));
#else
    float fs[4];
    for (int i = 0; i < 4; i++) {
        fs[i] = SkHalfToFloat(hs >> (i*16));
    }
    return Sk4f::Load(fs);
#endif
}

static inline uint64_t SkFloatToHalf_01(const Sk4f& fs) {
    uint64_t r;
#if !defined(SKNX_NO_SIMD) && defined(SK_CPU_ARM64)
    float32x4_t vec = fs.fVec;
    asm ("fcvtn %[vec].4h, %[vec].4s  \n"   // vcvt_f16_f32(vec)
         "fmov  %[r], %d[vec]         \n"   // vst1_f16(&r, ...)
        : [r] "=r" (r)                      // =r: write-only 64-bit general register
        , [vec] "+w" (vec));                // +w: read-write NEON register

// TODO: ARMv7 NEON float->half?

#elif !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    // Scale down from 127-bias to 15-bias, then cut off bottom 13 mantissa bits.
    // This doesn't round, so it can be 1 bit too small.
    const __m128 rebias = _mm_castsi128_ps(_mm_set1_epi32((127 - (127-15)) << 23));
    __m128i h = _mm_srli_epi32(_mm_castps_si128(_mm_mul_ps(fs.fVec, rebias)), 13);
    _mm_storel_epi64((__m128i*)&r, _mm_packs_epi32(h,h));

#else
    SkHalf hs[4];
    for (int i = 0; i < 4; i++) {
        hs[i] = SkFloatToHalf(fs[i]);
    }
    r = (uint64_t)hs[3] << 48
      | (uint64_t)hs[2] << 32
      | (uint64_t)hs[1] << 16
      | (uint64_t)hs[0] <<  0;
#endif
    return r;
}

#endif
