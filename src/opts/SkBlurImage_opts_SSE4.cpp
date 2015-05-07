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
    // ARGB -> 0000 0000 0000 ARGB
    __m128i widened = _mm_cvtsi32_si128(a);
    // SSE4.1 has xxxx xxxx xxxx ARGB -> 000A 000R 000G 000B as a one-stop-shop instruction.
    // It can even work from memory, so a smart compiler probably merges in the _mm_cvtsi32_si128().
    return _mm_cvtepu8_epi32(widened);
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
    for (int y = 0; y < height; ++y) {
        __m128i sum = _mm_setzero_si128();
        const SkPMColor* p = src;
        for (int i = 0; i < rightBorder; ++i) {
            sum = _mm_add_epi32(sum, expand(*p));
            p += srcStrideX;
        }

        const SkPMColor* sptr = src;
        SkColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            // TODO(mtklein): We are working in 8.24 here. Drop to 8.8 when the kernel is narrow?

            // Multiply each component by scale (i.e. divide by kernel size) and add half to round.
            __m128i result = _mm_mullo_epi32(sum, scale);
            result = _mm_add_epi32(result, half);

            // Now pack the top byte of each 32-bit lane back down into one 32-bit color.
            // Axxx Rxxx Gxxx Bxxx -> xxxx xxxx xxxx ARGB
            const char _ = 0;  // Don't care what ends up in these bytes.  Happens to be byte 0.
            result = _mm_shuffle_epi8(result, _mm_set_epi8(_,_,_,_, _,_,_,_, _,_,_,_, 15,11,7,3));

            *dptr = _mm_cvtsi128_si32(result);

            // TODO(mtklein): experiment with breaking this loop into 3 parts
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
                // TODO(mtklein): experiment with moving this prefetch forward
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
                                    SkBoxBlurProc* boxBlurXY,
                                    SkBoxBlurProc* boxBlurYX) {
    *boxBlurX = SkBoxBlur_SSE4<kX, kX>;
    *boxBlurXY = SkBoxBlur_SSE4<kX, kY>;
    *boxBlurYX = SkBoxBlur_SSE4<kY, kX>;
    return true;
}

#else // SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41

bool SkBoxBlurGetPlatformProcs_SSE4(SkBoxBlurProc* boxBlurX,
                                    SkBoxBlurProc* boxBlurXY,
                                    SkBoxBlurProc* boxBlurYX) {
    sk_throw();
    return false;
}


#endif
