/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow_opts_SSE4.h"

// Some compilers can't compile SSSE3 or SSE4 intrinsics.  We give them stub methods.
// The stubs should never be called, so we make them crash just to confirm that.
#if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_SSE41
void S32A_Opaque_BlitRow32_SSE4(SkPMColor* SK_RESTRICT, const SkPMColor* SK_RESTRICT, int, U8CPU) {
    sk_throw();
}

#else

#include <smmintrin.h>      // SSE4.1 intrinsics
#include "SkColorPriv.h"
#include "SkColor_opts_SSE2.h"

void S32A_Opaque_BlitRow32_SSE4(SkPMColor* SK_RESTRICT dst,
                                const SkPMColor* SK_RESTRICT src,
                                int count,
                                U8CPU alpha) {
    SkASSERT(alpha == 255);
    // As long as we can, we'll work on 16 pixel pairs at once.
    int count16 = count / 16;
    __m128i* dst4 = (__m128i*)dst;
    const __m128i* src4 = (const __m128i*)src;

    for (int i = 0; i < count16 * 4; i += 4) {
        // Load 16 source pixels.
        __m128i s0 = _mm_loadu_si128(src4+i+0),
                s1 = _mm_loadu_si128(src4+i+1),
                s2 = _mm_loadu_si128(src4+i+2),
                s3 = _mm_loadu_si128(src4+i+3);

        const __m128i alphaMask = _mm_set1_epi32(0xFF << SK_A32_SHIFT);
        const __m128i ORed = _mm_or_si128(s3, _mm_or_si128(s2, _mm_or_si128(s1, s0)));
        if (_mm_testz_si128(ORed, alphaMask)) {
            // All 16 source pixels are fully transparent.  There's nothing to do!
            continue;
        }
        const __m128i ANDed = _mm_and_si128(s3, _mm_and_si128(s2, _mm_and_si128(s1, s0)));
        if (_mm_testc_si128(ANDed, alphaMask)) {
            // All 16 source pixels are fully opaque.  There's no need to read dst or blend it.
            _mm_storeu_si128(dst4+i+0, s0);
            _mm_storeu_si128(dst4+i+1, s1);
            _mm_storeu_si128(dst4+i+2, s2);
            _mm_storeu_si128(dst4+i+3, s3);
            continue;
        }
        // The general slow case: do the blend for all 16 pixels.
        _mm_storeu_si128(dst4+i+0, SkPMSrcOver_SSE2(s0, _mm_loadu_si128(dst4+i+0)));
        _mm_storeu_si128(dst4+i+1, SkPMSrcOver_SSE2(s1, _mm_loadu_si128(dst4+i+1)));
        _mm_storeu_si128(dst4+i+2, SkPMSrcOver_SSE2(s2, _mm_loadu_si128(dst4+i+2)));
        _mm_storeu_si128(dst4+i+3, SkPMSrcOver_SSE2(s3, _mm_loadu_si128(dst4+i+3)));
    }

    // Wrap up the last <= 15 pixels.
    for (int i = count16*16; i < count; i++) {
        // This check is not really necessarily, but it prevents pointless autovectorization.
        if (src[i] & 0xFF000000) {
            dst[i] = SkPMSrcOver(src[i], dst[i]);
        }
    }
}

#endif
