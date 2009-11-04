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

#include "SkBlitRow.h"
#include "SkColorPriv.h"
#include "SkDither.h"

#include <emmintrin.h>

#ifdef _MSC_VER
static void getcpuid(int info_type, int info[4])
{
    __asm {
        mov    eax, [info_type]
        cpuid
        mov    edi, [info]
        mov    [edi], eax
        mov    [edi+4], ebx
        mov    [edi+8], ecx
        mov    [edi+12], edx
    }
}
#else
static void getcpuid(int info_type, int info[4])
{
    asm("cpuid": "=a" (info[0]), "=b" (info[1]), "=c" (info[2]), "=d" (info[3])
               : "a"(info_type)
               :
       );
}
#endif

/* SSE2 version of S32_Blend_BlitRow32()
 * portable version is in core/SkBlitRow_D32.cpp
 */
static void S32_Blend_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                                     const SkPMColor* SK_RESTRICT src,
                                     int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count <= 0) {
        return;
    }

    uint32_t src_scale = SkAlpha255To256(alpha);
    uint32_t dst_scale = 256 - src_scale;

    const __m128i *s = reinterpret_cast<const __m128i*>(src);
    __m128i *d = reinterpret_cast<__m128i*>(dst);
    __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);
    __m128i src_scale_wide = _mm_set1_epi16(src_scale);
    __m128i dst_scale_wide = _mm_set1_epi16(dst_scale);
    while (count >= 4) {
        // Load 4 pixels each of src and dest.
        __m128i src_pixel = _mm_loadu_si128(s);
        __m128i dst_pixel = _mm_loadu_si128(d);

        // Get red and blue pixels into lower byte of each word.
        __m128i dst_rb = _mm_and_si128(rb_mask, dst_pixel);
        __m128i src_rb = _mm_and_si128(rb_mask, src_pixel);

        // Get alpha and green into lower byte of each word.
        __m128i dst_ag = _mm_srli_epi16(dst_pixel, 8);
        __m128i src_ag = _mm_srli_epi16(src_pixel, 8);

        // Multiply by scale.
        src_rb = _mm_mullo_epi16(src_rb, src_scale_wide);
        src_ag = _mm_mullo_epi16(src_ag, src_scale_wide);
        dst_rb = _mm_mullo_epi16(dst_rb, dst_scale_wide);
        dst_ag = _mm_mullo_epi16(dst_ag, dst_scale_wide);

        // Divide by 256.
        src_rb = _mm_srli_epi16(src_rb, 8);
        dst_rb = _mm_srli_epi16(dst_rb, 8);
        src_ag = _mm_andnot_si128(rb_mask, src_ag);
        dst_ag = _mm_andnot_si128(rb_mask, dst_ag);

        // Combine back into RGBA.
        src_pixel = _mm_or_si128(src_rb, src_ag);
        dst_pixel = _mm_or_si128(dst_rb, dst_ag);

        // Add result
        __m128i result = _mm_add_epi8(src_pixel, dst_pixel);
        _mm_storeu_si128(d, result);
        s++;
        d++;
        count -= 4;
    }

    src = reinterpret_cast<const SkPMColor*>(s);
    dst = reinterpret_cast<SkPMColor*>(d);
   while (count > 0) {
        *dst = SkAlphaMulQ(*src, src_scale) + SkAlphaMulQ(*dst, dst_scale);
        src++;
        dst++;
        count--;
    }
}

static void S32A_Opaque_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                                       const SkPMColor* SK_RESTRICT src,
                                       int count, U8CPU alpha) {
    SkASSERT(alpha == 255);
    if (count <= 0) {
        return;
    }
    const __m128i *s = reinterpret_cast<const __m128i*>(src);
    __m128i *d = reinterpret_cast<__m128i*>(dst);
#ifdef SK_USE_ACCURATE_BLENDING
    __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);
    __m128i c_128 = _mm_set1_epi16(128);  // 8 copies of 128 (16-bit)
    __m128i c_255 = _mm_set1_epi16(255);  // 8 copies of 255 (16-bit)
    while (count >= 4) {
        // Load 4 pixels
        __m128i src_pixel = _mm_loadu_si128(s);
        __m128i dst_pixel = _mm_loadu_si128(d);

        __m128i dst_rb = _mm_and_si128(rb_mask, dst_pixel);
        __m128i dst_ag = _mm_andnot_si128(rb_mask, dst_pixel);
        dst_ag = _mm_srli_epi16(dst_ag, 8);
        // Shift alphas down to lower 8 bits of each quad.
        __m128i alpha = _mm_srli_epi32(src_pixel, 24);

        // Copy alpha to upper 3rd byte of each quad
        alpha = _mm_or_si128(alpha, _mm_slli_epi32(alpha, 16));

        // Subtract alphas from 255, to get 0..255
        alpha = _mm_sub_epi16(c_255, alpha);

        // Multiply by red and blue by src alpha.
        dst_rb = _mm_mullo_epi16(dst_rb, alpha);
        // Multiply by alpha and green by src alpha.
        dst_ag = _mm_mullo_epi16(dst_ag, alpha);

        // dst_rb_low = (dst_rb >> 8)
        __m128i dst_rb_low = _mm_srli_epi16(dst_rb, 8);
        __m128i dst_ag_low = _mm_srli_epi16(dst_ag, 8);

        // dst_rb = (dst_rb + dst_rb_low + 128) >> 8
        dst_rb = _mm_add_epi16(dst_rb, dst_rb_low);
        dst_rb = _mm_add_epi16(dst_rb, c_128);
        dst_rb = _mm_srli_epi16(dst_rb, 8);

        // dst_ag = (dst_ag + dst_ag_low + 128) & ag_mask
        dst_ag = _mm_add_epi16(dst_ag, dst_ag_low);
        dst_ag = _mm_add_epi16(dst_ag, c_128);
        dst_ag = _mm_andnot_si128(rb_mask, dst_ag);

        // Combine back into RGBA.
        dst_pixel = _mm_or_si128(dst_rb, dst_ag);

        // Add result
        __m128i result = _mm_add_epi8(src_pixel, dst_pixel);
        _mm_storeu_si128(d, result);
        s++;
        d++;
        count -= 4;
    }
#else
    __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);
    __m128i c_256 = _mm_set1_epi16(0x0100);  // 8 copies of 256 (16-bit)
    while (count >= 4) {
        // Load 4 pixels
        __m128i src_pixel = _mm_loadu_si128(s);
        __m128i dst_pixel = _mm_loadu_si128(d);

        __m128i dst_rb = _mm_and_si128(rb_mask, dst_pixel);
        __m128i dst_ag = _mm_andnot_si128(rb_mask, dst_pixel);
        dst_ag = _mm_srli_epi16(dst_ag, 8);
        // Shift alphas down to lower 8 bits of each quad.
        __m128i alpha = _mm_srli_epi32(src_pixel, 24);

        // Copy alpha to upper 3rd byte of each quad
        alpha = _mm_or_si128(alpha, _mm_slli_epi32(alpha, 16));

        // Subtract alphas from 256, to get 1..256
        alpha = _mm_sub_epi16(c_256, alpha);

        // Multiply by red and blue by src alpha.
        dst_rb = _mm_mullo_epi16(dst_rb, alpha);
        // Multiply by alpha and green by src alpha.
        dst_ag = _mm_mullo_epi16(dst_ag, alpha);

        // Divide by 256.
        dst_rb = _mm_srli_epi16(dst_rb, 8);

        // Mask out high bits (already in the right place)
        dst_ag = _mm_andnot_si128(rb_mask, dst_ag);

        // Combine back into RGBA.
        dst_pixel = _mm_or_si128(dst_rb, dst_ag);

        // Add result
        __m128i result = _mm_add_epi8(src_pixel, dst_pixel);
        _mm_storeu_si128(d, result);
        s++;
        d++;
        count -= 4;
    }
#endif

    src = reinterpret_cast<const SkPMColor*>(s);
    dst = reinterpret_cast<SkPMColor*>(d);
    while (count > 0) {
        *dst = SkPMSrcOver(*src, *dst);
        src++;
        dst++;
        count--;
    }
}

static void S32A_Blend_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                                      const SkPMColor* SK_RESTRICT src,
                                      int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count <= 0) {
        return;
    }

    uint32_t src_scale = SkAlpha255To256(alpha);

    const __m128i *s = reinterpret_cast<const __m128i*>(src);
    __m128i *d = reinterpret_cast<__m128i*>(dst);
    __m128i src_scale_wide = _mm_set1_epi16(src_scale);
    __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);
    __m128i c_256 = _mm_set1_epi16(256);  // 8 copies of 256 (16-bit)
    while (count >= 4) {
        // Load 4 pixels each of src and dest.
        __m128i src_pixel = _mm_loadu_si128(s);
        __m128i dst_pixel = _mm_loadu_si128(d);

        // Get red and blue pixels into lower byte of each word.
        __m128i dst_rb = _mm_and_si128(rb_mask, dst_pixel);
        __m128i src_rb = _mm_and_si128(rb_mask, src_pixel);

        // Get alpha and green into lower byte of each word.
        __m128i dst_ag = _mm_srli_epi16(dst_pixel, 8);
        __m128i src_ag = _mm_srli_epi16(src_pixel, 8);

        // Put per-pixel alpha in low byte of each word.
        __m128i dst_alpha = _mm_shufflehi_epi16(src_ag, 0xF5);
        dst_alpha = _mm_shufflelo_epi16(dst_alpha, 0xF5);

        // dst_alpha = dst_alpha * src_scale
        dst_alpha = _mm_mullo_epi16(dst_alpha, src_scale_wide);

        // Divide by 256.
        dst_alpha = _mm_srli_epi16(dst_alpha, 8);

        // Subtract alphas from 256, to get 1..256
        dst_alpha = _mm_sub_epi16(c_256, dst_alpha);

        // Multiply red and blue by dst pixel alpha.
        dst_rb = _mm_mullo_epi16(dst_rb, dst_alpha);
        // Multiply alpha and green by dst pixel alpha.
        dst_ag = _mm_mullo_epi16(dst_ag, dst_alpha);

        // Multiply red and blue by global alpha.
        src_rb = _mm_mullo_epi16(src_rb, src_scale_wide);
        // Multiply alpha and green by global alpha.
        src_ag = _mm_mullo_epi16(src_ag, src_scale_wide);

        // Divide by 256.
        dst_rb = _mm_srli_epi16(dst_rb, 8);
        src_rb = _mm_srli_epi16(src_rb, 8);

        // Mask out low bits (goodies already in the right place; no need to divide)
        dst_ag = _mm_andnot_si128(rb_mask, dst_ag);
        src_ag = _mm_andnot_si128(rb_mask, src_ag);

        // Combine back into RGBA.
        dst_pixel = _mm_or_si128(dst_rb, dst_ag);
        src_pixel = _mm_or_si128(src_rb, src_ag);

        // Add two pixels into result.
        __m128i result = _mm_add_epi8(src_pixel, dst_pixel);
        _mm_storeu_si128(d, result);
        s++;
        d++;
        count -= 4;
    }
    src = reinterpret_cast<const SkPMColor*>(s);
    dst = reinterpret_cast<SkPMColor*>(d);
    while (count > 0) {
        *dst = SkBlendARGB32(*src, *dst, alpha);
        src++;
        dst++;
        count--;
    }
}

///////////////////////////////////////////////////////////////////////////////

static const SkBlitRow::Proc32 platform_32_procs[] = {
    NULL,                               // S32_Opaque,
    S32_Blend_BlitRow32_SSE2,           // S32_Blend,
    S32A_Opaque_BlitRow32_SSE2,         // S32A_Opaque
    S32A_Blend_BlitRow32_SSE2,          // S32A_Blend,
};

const SkBlitRow::Proc SkBlitRow::PlatformProcs4444(unsigned flags) {
    return NULL;
}

const SkBlitRow::Proc SkBlitRow::PlatformProcs565(unsigned flags) {
    return NULL;
}

const SkBlitRow::Proc32 SkBlitRow::PlatformProcs32(unsigned flags) {
    static bool once;
    static bool hasSSE2;
    if (!once) {
        int cpu_info[4] = { 0 };
        getcpuid(1, cpu_info);
        hasSSE2 = (cpu_info[3] & (1<<26)) != 0;
        once = true;
    }
    if (hasSSE2) {
        return platform_32_procs[flags];
    } else {
        return NULL;
    }
}
