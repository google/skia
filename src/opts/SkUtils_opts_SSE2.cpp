/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emmintrin.h>
#include "SkUtils_opts_SSE2.h"

void sk_memset16_SSE2(uint16_t *dst, uint16_t value, int count)
{
    SkASSERT(dst != NULL && count >= 0);

    // dst must be 2-byte aligned.
    SkASSERT((((size_t) dst) & 0x01) == 0);

    if (count >= 32) {
        while (((size_t)dst) & 0x0F) {
            *dst++ = value;
            --count;
        }
        __m128i *d = reinterpret_cast<__m128i*>(dst);
        __m128i value_wide = _mm_set1_epi16(value);
        while (count >= 32) {
            _mm_store_si128(d    , value_wide);
            _mm_store_si128(d + 1, value_wide);
            _mm_store_si128(d + 2, value_wide);
            _mm_store_si128(d + 3, value_wide);
            d += 4;
            count -= 32;
        }
        dst = reinterpret_cast<uint16_t*>(d);
    }
    while (count > 0) {
        *dst++ = value;
        --count;
    }
}

void sk_memset32_SSE2(uint32_t *dst, uint32_t value, int count)
{
    SkASSERT(dst != NULL && count >= 0);

    // dst must be 4-byte aligned.
    SkASSERT((((size_t) dst) & 0x03) == 0);

    if (count >= 16) {
        while (((size_t)dst) & 0x0F) {
            *dst++ = value;
            --count;
        }
        __m128i *d = reinterpret_cast<__m128i*>(dst);
        __m128i value_wide = _mm_set1_epi32(value);
        while (count >= 16) {
            _mm_store_si128(d    , value_wide);
            _mm_store_si128(d + 1, value_wide);
            _mm_store_si128(d + 2, value_wide);
            _mm_store_si128(d + 3, value_wide);
            d += 4;
            count -= 16;
        }
        dst = reinterpret_cast<uint32_t*>(d);
    }
    while (count > 0) {
        *dst++ = value;
        --count;
    }
}

void sk_memcpy32_SSE2(uint32_t *dst, const uint32_t *src, int count)
{
    if (count >= 16) {
        while (((size_t)dst) & 0x0F) {
            *dst++ = *src++;
            --count;
        }
        __m128i *dst128 = reinterpret_cast<__m128i*>(dst);
        const __m128i *src128 = reinterpret_cast<const __m128i*>(src);
        while (count >= 16) {
            __m128i a =  _mm_loadu_si128(src128++);
            __m128i b =  _mm_loadu_si128(src128++);
            __m128i c =  _mm_loadu_si128(src128++);
            __m128i d =  _mm_loadu_si128(src128++);

            _mm_store_si128(dst128++, a);
            _mm_store_si128(dst128++, b);
            _mm_store_si128(dst128++, c);
            _mm_store_si128(dst128++, d);
            count -= 16;
        }
        dst = reinterpret_cast<uint32_t*>(dst128);
        src = reinterpret_cast<const uint32_t*>(src128);
    }
    while (count > 0) {
        *dst++ = *src++;
        --count;
    }
}
