/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMath_opts_SSE2_DEFINED
#define SkMath_opts_SSE2_DEFINED

#include <emmintrin.h>

// Because no _mm_div_epi32() in SSE2, we use float division to emulate.
// When using this function, make sure a and b don't exceed float's precision.
static inline __m128i shim_mm_div_epi32(const __m128i& a, const __m128i& b) {
    __m128 x = _mm_cvtepi32_ps(a);
    __m128 y = _mm_cvtepi32_ps(b);
    return _mm_cvttps_epi32(_mm_div_ps(x, y));
}

// Portable version of SkSqrtBits is in SkMath.cpp.
static inline __m128i SkSqrtBits_SSE2(const __m128i& x, int count) {
    __m128i root =  _mm_setzero_si128();
    __m128i remHi = _mm_setzero_si128();
    __m128i remLo = x;
    __m128i one128 = _mm_set1_epi32(1);

    do {
        root = _mm_slli_epi32(root, 1);

        remHi = _mm_or_si128(_mm_slli_epi32(remHi, 2),
                             _mm_srli_epi32(remLo, 30));
        remLo = _mm_slli_epi32(remLo, 2);

        __m128i testDiv = _mm_slli_epi32(root, 1);
        testDiv = _mm_add_epi32(testDiv, _mm_set1_epi32(1));

        __m128i cmp = _mm_cmplt_epi32(remHi, testDiv);
        __m128i remHi1 = _mm_and_si128(cmp, remHi);
        __m128i root1 = _mm_and_si128(cmp, root);
        __m128i remHi2 = _mm_andnot_si128(cmp, _mm_sub_epi32(remHi, testDiv));
        __m128i root2 = _mm_andnot_si128(cmp, _mm_add_epi32(root, one128));

        remHi = _mm_or_si128(remHi1, remHi2);
        root = _mm_or_si128(root1, root2);
    } while (--count >= 0);

    return root;
}

#endif // SkMath_opts_SSE2_DEFINED
