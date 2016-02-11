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

// TODO: NEON versions
static inline Sk4f SkHalfToFloat_01(uint64_t hs) {
#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    // Load our 16-bit floats into the bottom 16 bits of each 32-bit lane, with zeroes on top.
    __m128i h = _mm_unpacklo_epi16(_mm_loadl_epi64((const __m128i*)&hs), _mm_setzero_si128());

    // Fork into two paths, depending on whether the 16-bit float is denormalized.
    __m128 is_denorm = _mm_castsi128_ps(_mm_cmplt_epi32(h, _mm_set1_epi32(0x0400)));

    // TODO: figure out, explain
    const __m128 half = _mm_set1_ps(0.5f);
    __m128 denorm = _mm_sub_ps(_mm_or_ps(_mm_castsi128_ps(h), half), half);

    // If we're normalized, just shift ourselves so the exponent/mantissa dividing line
    // is correct, then re-bias the exponent from 15 to 127.
    __m128 norm = _mm_castsi128_ps(_mm_add_epi32(_mm_slli_epi32(h, 13),
                                                 _mm_set1_epi32((127-15) << 23)));

    return _mm_or_ps(_mm_and_ps   (is_denorm, denorm),
                     _mm_andnot_ps(is_denorm, norm));
#else
    float fs[4];
    for (int i = 0; i < 4; i++) {
        fs[i] = SkHalfToFloat(hs >> (i*16));
    }
    return Sk4f::Load(fs);
#endif
}

static inline uint64_t SkFloatToHalf_01(const Sk4f& fs) {
#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    // Scale our floats down by a tiny power of 2 to pull up our mantissa bits,
    // then shift back down to 16-bit float layout.  This doesn't round, so can be 1 bit small.
    // TODO: understand better.  Why this scale factor?
    const __m128 scale = _mm_castsi128_ps(_mm_set1_epi32(15 << 23));
    __m128i h = _mm_srli_epi32(_mm_castps_si128(_mm_mul_ps(fs.fVec, scale)), 13);

    uint64_t r;
    _mm_storel_epi64((__m128i*)&r, _mm_packs_epi32(h,h));
    return r;
#else
    SkHalf hs[4];
    for (int i = 0; i < 4; i++) {
        hs[i] = SkFloatToHalf(fs[i]);
    }
    return (uint64_t)hs[3] << 48
         | (uint64_t)hs[2] << 32
         | (uint64_t)hs[1] << 16
         | (uint64_t)hs[0] <<  0;
#endif
}

#endif
