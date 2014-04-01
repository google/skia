/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColor_opts_SSE2_DEFINED
#define SkColor_opts_SSE2_DEFINED

#include <emmintrin.h>

static inline __m128i SkMul16ShiftRound_SSE(__m128i a, __m128i b, int shift) {
    __m128i prod = _mm_mullo_epi16(a, b);
    prod = _mm_add_epi16(prod, _mm_set1_epi16(1 << (shift - 1)));
    prod = _mm_add_epi16(prod, _mm_srli_epi16(prod, shift));
    prod = _mm_srli_epi16(prod, shift);

    return prod;
}

static inline __m128i SkPackRGB16_SSE(__m128i r, __m128i g, __m128i b) {
    r = _mm_slli_epi16(r, SK_R16_SHIFT);
    g = _mm_slli_epi16(g, SK_G16_SHIFT);
    b = _mm_slli_epi16(b, SK_B16_SHIFT);

    __m128i c = _mm_or_si128(r, g);
    return _mm_or_si128(c, b);
}

#endif//SkColor_opts_SSE2_DEFINED
