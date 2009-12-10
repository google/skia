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
#include "SkBitmapProcState_opts_SSE2.h"
#include "SkUtils.h"

void S32_opaque_D32_filter_DX_SSE2(const SkBitmapProcState& s,
                                   const uint32_t* xy,
                                   int count, uint32_t* colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    SkASSERT(s.fBitmap->config() == SkBitmap::kARGB_8888_Config);
    SkASSERT(s.fAlphaScale == 256);

    const char* srcAddr = static_cast<const char*>(s.fBitmap->getPixels());
    unsigned rb = s.fBitmap->rowBytes();
    uint32_t XY = *xy++;
    unsigned y0 = XY >> 14;
    const uint32_t* row0 = reinterpret_cast<const uint32_t*>(srcAddr + (y0 >> 4) * rb);
    const uint32_t* row1 = reinterpret_cast<const uint32_t*>(srcAddr + (XY & 0x3FFF) * rb);
    unsigned subY = y0 & 0xF;

    // ( 0,  0,  0,  0,  0,  0,  0, 16)
    __m128i sixteen = _mm_cvtsi32_si128(16);

    // ( 0,  0,  0,  0, 16, 16, 16, 16)
    sixteen = _mm_shufflelo_epi16(sixteen, 0);

    // ( 0,  0,  0,  0,  0,  0,  0,  y)
    __m128i allY = _mm_cvtsi32_si128(subY);

    // ( 0,  0,  0,  0,  y,  y,  y,  y)
    allY = _mm_shufflelo_epi16(allY, 0);

    // ( 0,  0,  0,  0, 16-y, 16-y, 16-y, 16-y)
    __m128i negY = _mm_sub_epi16(sixteen, allY);

    // (16-y, 16-y, 16-y, 16-y, y, y, y, y)
    allY = _mm_unpacklo_epi64(allY, negY);

    // (16, 16, 16, 16, 16, 16, 16, 16 )
    sixteen = _mm_shuffle_epi32(sixteen, 0);

    // ( 0,  0,  0,  0,  0,  0,  0,  0)
    __m128i zero = _mm_setzero_si128();
    do {
        uint32_t XX = *xy++;    // x0:14 | 4 | x1:14
        unsigned x0 = XX >> 18;
        unsigned x1 = XX & 0x3FFF;

        // (0, 0, 0, 0, 0, 0, 0, x)
        __m128i allX = _mm_cvtsi32_si128((XX >> 14) & 0x0F);
        
        // (0, 0, 0, 0, x, x, x, x)
        allX = _mm_shufflelo_epi16(allX, 0);

        // (x, x, x, x, x, x, x, x)
        allX = _mm_shuffle_epi32(allX, 0);

        // (16-x, 16-x, 16-x, 16-x, 16-x, 16-x, 16-x)
        __m128i negX = _mm_sub_epi16(sixteen, allX);

        // Load 4 samples (pixels).
        __m128i a00 = _mm_cvtsi32_si128(row0[x0]);
        __m128i a01 = _mm_cvtsi32_si128(row0[x1]);
        __m128i a10 = _mm_cvtsi32_si128(row1[x0]);
        __m128i a11 = _mm_cvtsi32_si128(row1[x1]);

        // (0, 0, a00, a10)
        __m128i a00a10 = _mm_unpacklo_epi32(a10, a00);

        // Expand to 16 bits per component.
        a00a10 = _mm_unpacklo_epi8(a00a10, zero);

        // ((a00 * (16-y)), (a10 * y)).
        a00a10 = _mm_mullo_epi16(a00a10, allY);

        // (a00 * (16-y) * (16-x), a10 * y * (16-x)).
        a00a10 = _mm_mullo_epi16(a00a10, negX);

        // (0, 0, a01, a10)
        __m128i a01a11 = _mm_unpacklo_epi32(a11, a01);

        // Expand to 16 bits per component.
        a01a11 = _mm_unpacklo_epi8(a01a11, zero);

        // (a01 * (16-y)), (a11 * y)
        a01a11 = _mm_mullo_epi16(a01a11, allY);

        // (a01 * (16-y) * x), (a11 * y * x)
        a01a11 = _mm_mullo_epi16(a01a11, allX);

        // (a00*w00 + a01*w01, a10*w10 + a11*w11)
        __m128i sum = _mm_add_epi16(a00a10, a01a11);

        // (DC, a00*w00 + a01*w01)
        __m128i shifted = _mm_shuffle_epi32(sum, 0xEE);

        // (DC, a00*w00 + a01*w01 + a10*w10 + a11*w11)
        sum = _mm_add_epi16(sum, shifted);

        // Divide each 16 bit component by 256.
        sum = _mm_srli_epi16(sum, 8);

        // Pack lower 4 16 bit values of sum into lower 4 bytes.
        sum = _mm_packus_epi16(sum, zero);

        // Extract low int and store.
        *colors++ = _mm_cvtsi128_si32(sum);
    } while (--count > 0);
}

void S32_alpha_D32_filter_DX_SSE2(const SkBitmapProcState& s,
                                  const uint32_t* xy,
                                  int count, uint32_t* colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    SkASSERT(s.fBitmap->config() == SkBitmap::kARGB_8888_Config);
    SkASSERT(s.fAlphaScale < 256);

    const char* srcAddr = static_cast<const char*>(s.fBitmap->getPixels());
    unsigned rb = s.fBitmap->rowBytes();
    uint32_t XY = *xy++;
    unsigned y0 = XY >> 14;
    const uint32_t* row0 = reinterpret_cast<const uint32_t*>(srcAddr + (y0 >> 4) * rb);
    const uint32_t* row1 = reinterpret_cast<const uint32_t*>(srcAddr + (XY & 0x3FFF) * rb);
    unsigned subY = y0 & 0xF;

    // ( 0,  0,  0,  0,  0,  0,  0, 16)
    __m128i sixteen = _mm_cvtsi32_si128(16);

    // ( 0,  0,  0,  0, 16, 16, 16, 16)
    sixteen = _mm_shufflelo_epi16(sixteen, 0);

    // ( 0,  0,  0,  0,  0,  0,  0,  y)
    __m128i allY = _mm_cvtsi32_si128(subY);

    // ( 0,  0,  0,  0,  y,  y,  y,  y)
    allY = _mm_shufflelo_epi16(allY, 0);

    // ( 0,  0,  0,  0, 16-y, 16-y, 16-y, 16-y)
    __m128i negY = _mm_sub_epi16(sixteen, allY);

    // (16-y, 16-y, 16-y, 16-y, y, y, y, y)
    allY = _mm_unpacklo_epi64(allY, negY);

    // (16, 16, 16, 16, 16, 16, 16, 16 )
    sixteen = _mm_shuffle_epi32(sixteen, 0);

    // ( 0,  0,  0,  0,  0,  0,  0,  0)
    __m128i zero = _mm_setzero_si128();

    // ( alpha, alpha, alpha, alpha, alpha, alpha, alpha, alpha )
    __m128i alpha = _mm_set1_epi16(s.fAlphaScale);

    do {
        uint32_t XX = *xy++;    // x0:14 | 4 | x1:14
        unsigned x0 = XX >> 18;
        unsigned x1 = XX & 0x3FFF;

        // (0, 0, 0, 0, 0, 0, 0, x)
        __m128i allX = _mm_cvtsi32_si128((XX >> 14) & 0x0F);
        
        // (0, 0, 0, 0, x, x, x, x)
        allX = _mm_shufflelo_epi16(allX, 0);

        // (x, x, x, x, x, x, x, x)
        allX = _mm_shuffle_epi32(allX, 0);

        // (16-x, 16-x, 16-x, 16-x, 16-x, 16-x, 16-x)
        __m128i negX = _mm_sub_epi16(sixteen, allX);

        // Load 4 samples (pixels).
        __m128i a00 = _mm_cvtsi32_si128(row0[x0]);
        __m128i a01 = _mm_cvtsi32_si128(row0[x1]);
        __m128i a10 = _mm_cvtsi32_si128(row1[x0]);
        __m128i a11 = _mm_cvtsi32_si128(row1[x1]);

        // (0, 0, a00, a10)
        __m128i a00a10 = _mm_unpacklo_epi32(a10, a00);

        // Expand to 16 bits per component.
        a00a10 = _mm_unpacklo_epi8(a00a10, zero);

        // ((a00 * (16-y)), (a10 * y)).
        a00a10 = _mm_mullo_epi16(a00a10, allY);

        // (a00 * (16-y) * (16-x), a10 * y * (16-x)).
        a00a10 = _mm_mullo_epi16(a00a10, negX);

        // (0, 0, a01, a10)
        __m128i a01a11 = _mm_unpacklo_epi32(a11, a01);

        // Expand to 16 bits per component.
        a01a11 = _mm_unpacklo_epi8(a01a11, zero);

        // (a01 * (16-y)), (a11 * y)
        a01a11 = _mm_mullo_epi16(a01a11, allY);

        // (a01 * (16-y) * x), (a11 * y * x)
        a01a11 = _mm_mullo_epi16(a01a11, allX);

        // (a00*w00 + a01*w01, a10*w10 + a11*w11)
        __m128i sum = _mm_add_epi16(a00a10, a01a11);

        // (DC, a00*w00 + a01*w01)
        __m128i shifted = _mm_shuffle_epi32(sum, 0xEE);

        // (DC, a00*w00 + a01*w01 + a10*w10 + a11*w11)
        sum = _mm_add_epi16(sum, shifted);

        // Divide each 16 bit component by 256.
        sum = _mm_srli_epi16(sum, 8);

        // Multiply by alpha.
        sum = _mm_mullo_epi16(sum, alpha);

        // Divide each 16 bit component by 256.
        sum = _mm_srli_epi16(sum, 8);

        // Pack lower 4 16 bit values of sum into lower 4 bytes.
        sum = _mm_packus_epi16(sum, zero);

        // Extract low int and store.
        *colors++ = _mm_cvtsi128_si32(sum);
    } while (--count > 0);
}
