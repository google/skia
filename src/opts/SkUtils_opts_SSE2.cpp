/*
 **
 ** Copyright 2009, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License"); 
 ** you may not use this file except in compliance with the License. 
 ** You may obtain a copy of the License at 
 **
 **     http://www.apache.org/licenses/LICENSE-2.0 
 **
 ** Unless required by applicable law or agreed to in writing, software 
 ** distributed under the License is distributed on an "AS IS" BASIS, 
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 ** See the License for the specific language governing permissions and 
 ** limitations under the License.
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
            _mm_store_si128(d++, value_wide);
            _mm_store_si128(d++, value_wide);
            _mm_store_si128(d++, value_wide);
            _mm_store_si128(d++, value_wide);
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
            _mm_store_si128(d++, value_wide);
            _mm_store_si128(d++, value_wide);
            _mm_store_si128(d++, value_wide);
            _mm_store_si128(d++, value_wide);
            count -= 16;
        }
        dst = reinterpret_cast<uint32_t*>(d);
    }
    while (count > 0) {
        *dst++ = value;
        --count;
    }
}
