/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBlurImage_opts_SSE4.h"
#include "SkColorPriv.h"
#include "SkRect.h"

/* With the exception of the compilers that don't support it, we always build the
 * SSE4 functions and enable the caller to determine SSE4 support.  However for
 * compilers that do not support SSE4x we provide a stub implementation.
 */
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41

#include <smmintrin.h>

namespace {
enum BlurDirection {
    kX, kY
};

/* Helper function to spread the components of a 32-bit integer into the
 * lower 8 bits of each 32-bit element of an SSE register.
 */
inline __m128i expand(int a) {
    const __m128i zero = _mm_setzero_si128();

    // 0 0 0 0   0 0 0 0   0 0 0 0   A R G B
    __m128i result = _mm_cvtsi32_si128(a);

    // 0 0 0 0   0 0 0 0   0 A 0 R   0 G 0 B
    result = _mm_unpacklo_epi8(result, zero);

    // 0 0 0 A   0 0 0 R   0 0 0 G   0 0 0 B
    return _mm_unpacklo_epi16(result, zero);
}

template<BlurDirection srcDirection, BlurDirection dstDirection>
void SkBoxBlur_SSE4(const SkPMColor* src, int srcStride, SkPMColor* dst, int kernelSize,
                    int leftOffset, int rightOffset, int width, int height)
{
    const int rightBorder = SkMin32(rightOffset + 1, width);
    const int srcStrideX = srcDirection == kX ? 1 : srcStride;
    const int dstStrideX = dstDirection == kX ? 1 : height;
    const int srcStrideY = srcDirection == kX ? srcStride : 1;
    const int dstStrideY = dstDirection == kX ? width : 1;
    const __m128i scale = _mm_set1_epi32((1 << 24) / kernelSize);
    const __m128i half = _mm_set1_epi32(1 << 23);
    const __m128i zero = _mm_setzero_si128();
    for (int y = 0; y < height; ++y) {
        __m128i sum = zero;
        const SkPMColor* p = src;
        for (int i = 0; i < rightBorder; ++i) {
            sum = _mm_add_epi32(sum, expand(*p));
            p += srcStrideX;
        }

        const SkPMColor* sptr = src;
        SkColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            __m128i result = _mm_mullo_epi32(sum, scale);

            // sumA*scale+.5 sumB*scale+.5 sumG*scale+.5 sumB*scale+.5
            result = _mm_add_epi32(result, half);

            // 0 0 0 A   0 0 0 R   0 0 0 G   0 0 0 B
            result = _mm_srli_epi32(result, 24);

            // 0 0 0 0   0 0 0 0   0 A 0 R   0 G 0 B
            result = _mm_packs_epi32(result, zero);

            // 0 0 0 0   0 0 0 0   0 0 0 0   A R G B
            result = _mm_packus_epi16(result, zero);
            *dptr = _mm_cvtsi128_si32(result);
            if (x >= leftOffset) {
                SkColor l = *(sptr - leftOffset * srcStrideX);
                sum = _mm_sub_epi32(sum, expand(l));
            }
            if (x + rightOffset + 1 < width) {
                SkColor r = *(sptr + (rightOffset + 1) * srcStrideX);
                sum = _mm_add_epi32(sum, expand(r));
            }
            sptr += srcStrideX;
            if (srcDirection == kY) {
                _mm_prefetch(reinterpret_cast<const char*>(sptr + (rightOffset + 1) * srcStrideX),
                             _MM_HINT_T0);
            }
            dptr += dstStrideX;
        }
        src += srcStrideY;
        dst += dstStrideY;
    }
}

} // namespace

bool SkBoxBlurGetPlatformProcs_SSE4(SkBoxBlurProc* boxBlurX,
                                    SkBoxBlurProc* boxBlurY,
                                    SkBoxBlurProc* boxBlurXY,
                                    SkBoxBlurProc* boxBlurYX) {
    *boxBlurX = SkBoxBlur_SSE4<kX, kX>;
    *boxBlurY = SkBoxBlur_SSE4<kY, kY>;
    *boxBlurXY = SkBoxBlur_SSE4<kX, kY>;
    *boxBlurYX = SkBoxBlur_SSE4<kY, kX>;
    return true;
}

#else // SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41

bool SkBoxBlurGetPlatformProcs_SSE4(SkBoxBlurProc* boxBlurX,
                                    SkBoxBlurProc* boxBlurY,
                                    SkBoxBlurProc* boxBlurXY,
                                    SkBoxBlurProc* boxBlurYX) {
    sk_throw();
    return false;
}


#endif
