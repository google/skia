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

#include "SkBlitRow_opts_SSE2.h"
#include "SkColorPriv.h"
#include "SkUtils.h"

#include <emmintrin.h>

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
    uint32_t dst_scale = 256 - src_scale;

    if (count >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkAlphaMulQ(*src, src_scale) + SkAlphaMulQ(*dst, dst_scale);
            src++;
            dst++;
            count--;
        }

        const __m128i *s = reinterpret_cast<const __m128i*>(src);
        __m128i *d = reinterpret_cast<__m128i*>(dst);
        __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);
        __m128i src_scale_wide = _mm_set1_epi16(src_scale);
        __m128i dst_scale_wide = _mm_set1_epi16(dst_scale);
        while (count >= 4) {
            // Load 4 pixels each of src and dest.
            __m128i src_pixel = _mm_loadu_si128(s);
            __m128i dst_pixel = _mm_load_si128(d);

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
            _mm_store_si128(d, result);
            s++;
            d++;
            count -= 4;
        }
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (count > 0) {
        *dst = SkAlphaMulQ(*src, src_scale) + SkAlphaMulQ(*dst, dst_scale);
        src++;
        dst++;
        count--;
    }
}

void S32A_Opaque_BlitRow32_SSE2(SkPMColor* SK_RESTRICT dst,
                                const SkPMColor* SK_RESTRICT src,
                                int count, U8CPU alpha) {
    SkASSERT(alpha == 255);
    if (count <= 0) {
        return;
    }

    if (count >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkPMSrcOver(*src, *dst);
            src++;
            dst++;
            count--;
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
            __m128i dst_pixel = _mm_load_si128(d);

            __m128i dst_rb = _mm_and_si128(rb_mask, dst_pixel);
            __m128i dst_ag = _mm_srli_epi16(dst_pixel, 8);
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
            _mm_store_si128(d, result);
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
            __m128i dst_pixel = _mm_load_si128(d);

            __m128i dst_rb = _mm_and_si128(rb_mask, dst_pixel);
            __m128i dst_ag = _mm_srli_epi16(dst_pixel, 8);

            // (a0, g0, a1, g1, a2, g2, a3, g3)  (low byte of each word)
            __m128i alpha = _mm_srli_epi16(src_pixel, 8);

            // (a0, a0, a1, a1, a2, g2, a3, g3)
            alpha = _mm_shufflehi_epi16(alpha, 0xF5);

            // (a0, a0, a1, a1, a2, a2, a3, a3)
            alpha = _mm_shufflelo_epi16(alpha, 0xF5);

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
            _mm_store_si128(d, result);
            s++;
            d++;
            count -= 4;
        }
#endif
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (count > 0) {
        *dst = SkPMSrcOver(*src, *dst);
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

        uint32_t src_scale = SkAlpha255To256(alpha);

        const __m128i *s = reinterpret_cast<const __m128i*>(src);
        __m128i *d = reinterpret_cast<__m128i*>(dst);
        __m128i src_scale_wide = _mm_set1_epi16(src_scale);
        __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);
        __m128i c_256 = _mm_set1_epi16(256);  // 8 copies of 256 (16-bit)
        while (count >= 4) {
            // Load 4 pixels each of src and dest.
            __m128i src_pixel = _mm_loadu_si128(s);
            __m128i dst_pixel = _mm_load_si128(d);

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

/* SSE2 version of Color32()
 * portable version is in core/SkBlitRow_D32.cpp
 */
void Color32_SSE2(SkPMColor dst[], const SkPMColor src[], int count,
                  SkPMColor color) {

    if (count <= 0) {
        return;
    }

    if (0 == color) {
        if (src != dst) {
            memcpy(dst, src, count * sizeof(SkPMColor));
        }
    }

    unsigned colorA = SkGetPackedA32(color);
    if (255 == colorA) {
        sk_memset32(dst, color, count);
    } else {
        unsigned scale = 256 - SkAlpha255To256(colorA);

        if (count >= 4) {
            SkASSERT(((size_t)dst & 0x03) == 0);
            while (((size_t)dst & 0x0F) != 0) {
                *dst = color + SkAlphaMulQ(*src, scale);
                src++;
                dst++;
                count--;
            }

            const __m128i *s = reinterpret_cast<const __m128i*>(src);
            __m128i *d = reinterpret_cast<__m128i*>(dst);
            __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);
            __m128i src_scale_wide = _mm_set1_epi16(scale);
            __m128i color_wide = _mm_set1_epi32(color);
            while (count >= 4) {
                // Load 4 pixels each of src and dest.
                __m128i src_pixel = _mm_loadu_si128(s);

                // Get red and blue pixels into lower byte of each word.
                __m128i src_rb = _mm_and_si128(rb_mask, src_pixel);

                // Get alpha and green into lower byte of each word.
                __m128i src_ag = _mm_srli_epi16(src_pixel, 8);

                // Multiply by scale.
                src_rb = _mm_mullo_epi16(src_rb, src_scale_wide);
                src_ag = _mm_mullo_epi16(src_ag, src_scale_wide);

                // Divide by 256.
                src_rb = _mm_srli_epi16(src_rb, 8);
                src_ag = _mm_andnot_si128(rb_mask, src_ag);

                // Combine back into RGBA.
                src_pixel = _mm_or_si128(src_rb, src_ag);

                // Add color to result.
                __m128i result = _mm_add_epi8(color_wide, src_pixel);

                // Store result.
                _mm_store_si128(d, result);
                s++;
                d++;
                count -= 4;
            }
            src = reinterpret_cast<const SkPMColor*>(s);
            dst = reinterpret_cast<SkPMColor*>(d);
         }

        while (count > 0) {
            *dst = color + SkAlphaMulQ(*src, scale);
            src += 1;
            dst += 1;
            count--;
        }
    }
}

void SkARGB32_BlitMask_SSE2(void* device, size_t dstRB,
                            SkBitmap::Config dstConfig, const uint8_t* mask,
                            size_t maskRB, SkColor origColor,
                            int width, int height)
{
    SkPMColor color = SkPreMultiplyColor(origColor);
    size_t dstOffset = dstRB - (width << 2);
    size_t maskOffset = maskRB - width;
    SkPMColor* dst = (SkPMColor *)device;
    do {
        int count = width;
        if (count >= 4) {
            while (((size_t)dst & 0x0F) != 0 && (count > 0)) {
                *dst = SkBlendARGB32(color, *dst, *mask);
                mask++;
                dst++;
                count--;
            }
            __m128i *d = reinterpret_cast<__m128i*>(dst);
            __m128i rb_mask = _mm_set1_epi32(0x00FF00FF);
            __m128i c_256 = _mm_set1_epi16(256);
            __m128i c_1 = _mm_set1_epi16(1);
            __m128i src_pixel = _mm_set1_epi32(color);
            while (count >= 4) {
                // Load 4 pixels each of src and dest.
                __m128i dst_pixel = _mm_load_si128(d);

                //set the aphla value
                __m128i src_scale_wide =  _mm_set_epi8(0, *(mask+3),\
                                0, *(mask+3),0, \
                                *(mask+2),0, *(mask+2),\
                                0,*(mask+1), 0,*(mask+1),\
                                0, *mask,0,*mask);

                //call SkAlpha255To256()
                src_scale_wide = _mm_add_epi16(src_scale_wide, c_1);

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
                __m128i tmp_src_pixel = _mm_or_si128(src_rb, src_ag);

                // Add two pixels into result.
                __m128i result = _mm_add_epi8(tmp_src_pixel, dst_pixel);
                _mm_store_si128(d, result);
                // load the next 4 pixel
                mask = mask + 4;
                d++;
                count -= 4;
            }
            dst = reinterpret_cast<SkPMColor *>(d);
        }
        while(count > 0) {
            *dst= SkBlendARGB32(color, *dst, *mask);
            dst += 1;
            mask++;
            count --;
        }
        dst = (SkPMColor *)((char*)dst + dstOffset);
        mask += maskOffset;
    } while (--height != 0);
}
