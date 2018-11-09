/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emmintrin.h>
#include "SkBitmapProcState_opts_SSE2.h"
#include "SkBlitRow_opts_SSE2.h"
#include "SkColorData.h"
#include "SkColor_opts_SSE2.h"
#include "SkMSAN.h"
#include "SkUTF.h"

/* SSE2 version of S32_Blend_BlitRow32()
 * portable version is in core/SkBlitRow_D32.cpp
 */
void S32_Blend_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src,
                              int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count <= 0) {
        return;
    }

    uint32_t src_scale = SkAlpha255To256(alpha);

    if (count >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkPMLerp(*src, *dst, src_scale);
            src++;
            dst++;
            count--;
        }

        const __m128i *s = reinterpret_cast<const __m128i*>(src);
        __m128i *d = reinterpret_cast<__m128i*>(dst);

        while (count >= 4) {
            // Load 4 pixels each of src and dest.
            __m128i src_pixel = _mm_loadu_si128(s);
            __m128i dst_pixel = _mm_load_si128(d);

            __m128i result = SkPMLerp_SSE2(src_pixel, dst_pixel, src_scale);
            _mm_store_si128(d, result);
            s++;
            d++;
            count -= 4;
        }
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (count > 0) {
        *dst = SkPMLerp(*src, *dst, src_scale);
        src++;
        dst++;
        count--;
    }
}

void S32A_Blend_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                               const SkPMColor* SK_RESTRICT src,
                               int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count <= 0) {
        return;
    }

    if (count >= 4) {
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkBlendARGB32(*src, *dst, alpha);
            src++;
            dst++;
            count--;
        }

        const __m128i *s = reinterpret_cast<const __m128i*>(src);
        __m128i *d = reinterpret_cast<__m128i*>(dst);
        while (count >= 4) {
            // Load 4 pixels each of src and dest.
            __m128i src_pixel = _mm_loadu_si128(s);
            __m128i dst_pixel = _mm_load_si128(d);

            __m128i result = SkBlendARGB32_SSE2(src_pixel, dst_pixel, alpha);
            _mm_store_si128(d, result);
            s++;
            d++;
            count -= 4;
        }
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (count > 0) {
        *dst = SkBlendARGB32(*src, *dst, alpha);
        src++;
        dst++;
        count--;
    }
}
