/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBlitRow_opts_SSE2.h"
#include "SkBitmapProcState_opts_SSE2.h"
#include "SkColorPriv.h"
#include "SkDither.h"
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
        __m128i ag_mask = _mm_set1_epi32(0xFF00FF00);

        // Move scale factors to upper byte of word
        __m128i src_scale_wide = _mm_set1_epi16(src_scale << 8);
        __m128i dst_scale_wide = _mm_set1_epi16(dst_scale << 8);
        while (count >= 4) {
            // Load 4 pixels each of src and dest.
            __m128i src_pixel = _mm_loadu_si128(s);
            __m128i dst_pixel = _mm_load_si128(d);

            // Interleave Atom port 0/1 operations based on the execution port
            // constraints that multiply can only be executed on port 0 (while
            // boolean operations can be executed on either port 0 or port 1)
            // because GCC currently doesn't do a good job scheduling
            // instructions based on these constraints.

            // Get red and blue pixels into lower byte of each word.
            // (0, r, 0, b, 0, r, 0, b, 0, r, 0, b, 0, r, 0, b)
            __m128i src_rb = _mm_and_si128(rb_mask, src_pixel);

            // Multiply by scale.
            // (4 x (0, rs.h, 0, bs.h))
            // where rs.h stands for the higher byte of r * scale, and
            // bs.h the higher byte of b * scale.
            src_rb = _mm_mulhi_epu16(src_rb, src_scale_wide);

            // Get alpha and green pixels into higher byte of each word.
            // (a, 0, g, 0, a, 0, g, 0, a, 0, g, 0, a, 0, g, 0)
            __m128i src_ag = _mm_and_si128(ag_mask, src_pixel);

            // Multiply by scale.
            // (4 x (as.h, as.l, gs.h, gs.l))
            src_ag = _mm_mulhi_epu16(src_ag, src_scale_wide);

            // Clear the lower byte of the a*scale and g*scale results
            // (4 x (as.h, 0, gs.h, 0))
            src_ag = _mm_and_si128(src_ag, ag_mask);

            // Operations the destination pixels are the same as on the
            // source pixels. See the comments above.
            __m128i dst_rb = _mm_and_si128(rb_mask, dst_pixel);
            dst_rb = _mm_mulhi_epu16(dst_rb, dst_scale_wide);
            __m128i dst_ag = _mm_and_si128(ag_mask, dst_pixel);
            dst_ag = _mm_mulhi_epu16(dst_ag, dst_scale_wide);
            dst_ag = _mm_and_si128(dst_ag, ag_mask);

            // Combine back into RGBA.
            // (4 x (as.h, rs.h, gs.h, bs.h))
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
        __m128i src_scale_wide = _mm_set1_epi16(src_scale << 8);
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
            // After the following two statements, the dst_alpha looks like
            // (0, a0, 0, a0, 0, a1, 0, a1, 0, a2, 0, a2, 0, a3, 0, a3)
            __m128i dst_alpha = _mm_shufflehi_epi16(src_ag, 0xF5);
            dst_alpha = _mm_shufflelo_epi16(dst_alpha, 0xF5);

            // dst_alpha = dst_alpha * src_scale
            // Because src_scales are in the higher byte of each word and
            // we use mulhi here, the resulting alpha values are already
            // in the right place and don't need to be divided by 256.
            // (0, sa0, 0, sa0, 0, sa1, 0, sa1, 0, sa2, 0, sa2, 0, sa3, 0, sa3)
            dst_alpha = _mm_mulhi_epu16(dst_alpha, src_scale_wide);

            // Subtract alphas from 256, to get 1..256
            dst_alpha = _mm_sub_epi16(c_256, dst_alpha);

            // Multiply red and blue by dst pixel alpha.
            dst_rb = _mm_mullo_epi16(dst_rb, dst_alpha);
            // Multiply alpha and green by dst pixel alpha.
            dst_ag = _mm_mullo_epi16(dst_ag, dst_alpha);

            // Multiply red and blue by global alpha.
            // (4 x (0, rs.h, 0, bs.h))
            // where rs.h stands for the higher byte of r * src_scale,
            // and bs.h the higher byte of b * src_scale.
            // Again, because we use mulhi, the resuling red and blue
            // values are already in the right place and don't need to
            // be divided by 256.
            src_rb = _mm_mulhi_epu16(src_rb, src_scale_wide);
            // Multiply alpha and green by global alpha.
            // (4 x (0, as.h, 0, gs.h))
            src_ag = _mm_mulhi_epu16(src_ag, src_scale_wide);

            // Divide by 256.
            dst_rb = _mm_srli_epi16(dst_rb, 8);

            // Mask out low bits (goodies already in the right place; no need to divide)
            dst_ag = _mm_andnot_si128(rb_mask, dst_ag);
            // Shift alpha and green to higher byte of each word.
            // (4 x (as.h, 0, gs.h, 0))
            src_ag = _mm_slli_epi16(src_ag, 8);

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
        return;
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

void SkARGB32_A8_BlitMask_SSE2(void* device, size_t dstRB, const void* maskPtr,
                               size_t maskRB, SkColor origColor,
                               int width, int height) {
    SkPMColor color = SkPreMultiplyColor(origColor);
    size_t dstOffset = dstRB - (width << 2);
    size_t maskOffset = maskRB - width;
    SkPMColor* dst = (SkPMColor *)device;
    const uint8_t* mask = (const uint8_t*)maskPtr;
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

// The following (left) shifts cause the top 5 bits of the mask components to
// line up with the corresponding components in an SkPMColor.
// Note that the mask's RGB16 order may differ from the SkPMColor order.
#define SK_R16x5_R32x5_SHIFT (SK_R32_SHIFT - SK_R16_SHIFT - SK_R16_BITS + 5)
#define SK_G16x5_G32x5_SHIFT (SK_G32_SHIFT - SK_G16_SHIFT - SK_G16_BITS + 5)
#define SK_B16x5_B32x5_SHIFT (SK_B32_SHIFT - SK_B16_SHIFT - SK_B16_BITS + 5)

#if SK_R16x5_R32x5_SHIFT == 0
    #define SkPackedR16x5ToUnmaskedR32x5_SSE2(x) (x)
#elif SK_R16x5_R32x5_SHIFT > 0
    #define SkPackedR16x5ToUnmaskedR32x5_SSE2(x) (_mm_slli_epi32(x, SK_R16x5_R32x5_SHIFT))
#else
    #define SkPackedR16x5ToUnmaskedR32x5_SSE2(x) (_mm_srli_epi32(x, -SK_R16x5_R32x5_SHIFT))
#endif

#if SK_G16x5_G32x5_SHIFT == 0
    #define SkPackedG16x5ToUnmaskedG32x5_SSE2(x) (x)
#elif SK_G16x5_G32x5_SHIFT > 0
    #define SkPackedG16x5ToUnmaskedG32x5_SSE2(x) (_mm_slli_epi32(x, SK_G16x5_G32x5_SHIFT))
#else
    #define SkPackedG16x5ToUnmaskedG32x5_SSE2(x) (_mm_srli_epi32(x, -SK_G16x5_G32x5_SHIFT))
#endif

#if SK_B16x5_B32x5_SHIFT == 0
    #define SkPackedB16x5ToUnmaskedB32x5_SSE2(x) (x)
#elif SK_B16x5_B32x5_SHIFT > 0
    #define SkPackedB16x5ToUnmaskedB32x5_SSE2(x) (_mm_slli_epi32(x, SK_B16x5_B32x5_SHIFT))
#else
    #define SkPackedB16x5ToUnmaskedB32x5_SSE2(x) (_mm_srli_epi32(x, -SK_B16x5_B32x5_SHIFT))
#endif

static __m128i SkBlendLCD16_SSE2(__m128i &srci, __m128i &dst,
                                 __m128i &mask, __m128i &scale) {
    // Get the R,G,B of each 16bit mask pixel, we want all of them in 5 bits.
    __m128i r = _mm_and_si128(SkPackedR16x5ToUnmaskedR32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_R32_SHIFT));

    __m128i g = _mm_and_si128(SkPackedG16x5ToUnmaskedG32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_G32_SHIFT));
    
    __m128i b = _mm_and_si128(SkPackedB16x5ToUnmaskedB32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_B32_SHIFT));
            
    // Pack the 4 16bit mask pixels into 4 32bit pixels, (p0, p1, p2, p3)
    mask = _mm_or_si128(_mm_or_si128(r, g), b);

    // Interleave R,G,B into the lower byte of word. 
    __m128i maskLo, maskHi;
    maskLo = _mm_unpacklo_epi8(mask, _mm_setzero_si128());
    maskHi = _mm_unpackhi_epi8(mask, _mm_setzero_si128());

    // Upscale to 0..32
    maskLo = _mm_add_epi16(maskLo, _mm_srli_epi16(maskLo, 4));
    maskHi = _mm_add_epi16(maskHi, _mm_srli_epi16(maskHi, 4));

    maskLo = _mm_mullo_epi16(maskLo, scale);
    maskHi = _mm_mullo_epi16(maskHi, scale);

    maskLo = _mm_srli_epi16(maskLo, 8);
    maskHi = _mm_srli_epi16(maskHi, 8);

    // Interleave R,G,B into the lower byte of the word.
    __m128i dstLo = _mm_unpacklo_epi8(dst, _mm_setzero_si128());
    __m128i dstHi = _mm_unpackhi_epi8(dst, _mm_setzero_si128());

    maskLo = _mm_mullo_epi16(maskLo, _mm_sub_epi16(srci, dstLo));
    maskHi = _mm_mullo_epi16(maskHi, _mm_sub_epi16(srci, dstHi));

    maskLo = _mm_srai_epi16(maskLo, 5);
    maskHi = _mm_srai_epi16(maskHi, 5);

    // Add two pixels into result.
    __m128i resultLo = _mm_add_epi16(dstLo, maskLo);
    __m128i resultHi = _mm_add_epi16(dstHi, maskHi);

    // Pack into 4 32bit dst pixels
    return _mm_packus_epi16(resultLo, resultHi);
}

static __m128i SkBlendLCD16Opaque_SSE2(__m128i &srci, __m128i &dst,
                                       __m128i &mask) {
    // Get the R,G,B of each 16bit mask pixel, we want all of them in 5 bits.
    __m128i r = _mm_and_si128(SkPackedR16x5ToUnmaskedR32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_R32_SHIFT));

    __m128i g = _mm_and_si128(SkPackedG16x5ToUnmaskedG32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_G32_SHIFT));
    
    __m128i b = _mm_and_si128(SkPackedB16x5ToUnmaskedB32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_B32_SHIFT));

    // Pack the 4 16bit mask pixels into 4 32bit pixels, (p0, p1, p2, p3)
    mask = _mm_or_si128(_mm_or_si128(r, g), b);

    // Interleave R,G,B into the lower byte of word. 
    __m128i maskLo, maskHi;
    maskLo = _mm_unpacklo_epi8(mask, _mm_setzero_si128());
    maskHi = _mm_unpackhi_epi8(mask, _mm_setzero_si128());

    // Upscale to 0..32
    maskLo = _mm_add_epi16(maskLo, _mm_srli_epi16(maskLo, 4));
    maskHi = _mm_add_epi16(maskHi, _mm_srli_epi16(maskHi, 4));

    // Interleave R,G,B into the lower byte of the word.
    __m128i dstLo = _mm_unpacklo_epi8(dst, _mm_setzero_si128());
    __m128i dstHi = _mm_unpackhi_epi8(dst, _mm_setzero_si128());

    maskLo = _mm_mullo_epi16(maskLo, _mm_sub_epi16(srci, dstLo));
    maskHi = _mm_mullo_epi16(maskHi, _mm_sub_epi16(srci, dstHi));

    maskLo = _mm_srai_epi16(maskLo, 5);
    maskHi = _mm_srai_epi16(maskHi, 5);

    // Add two pixels into result.
    __m128i resultLo = _mm_add_epi16(dstLo, maskLo);
    __m128i resultHi = _mm_add_epi16(dstHi, maskHi);

    // Pack into 4 32bit dst pixels
    return _mm_packus_epi16(resultLo, resultHi);
}

void SkBlitLCD16Row_SSE2(SkPMColor dst[], const uint16_t src[],
                         SkColor color, int width, SkPMColor) {
    if (width <= 0) {
        return;
    }

    int srcA = SkColorGetA(color);
    int srcR = SkColorGetR(color);
    int srcG = SkColorGetG(color);
    int srcB = SkColorGetB(color);
    
    srcA = SkAlpha255To256(srcA);

    if (width >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkBlendLCD16(srcA, srcR, srcG, srcB, *dst, *src);
            src++;
            dst++;
            width--;
        }

        __m128i *d = reinterpret_cast<__m128i*>(dst);
        __m128i srci = _mm_set1_epi32(SkPackARGB32(0xFF, srcR, srcG, srcB));
        srci = _mm_unpacklo_epi8(srci, _mm_setzero_si128());
        __m128i scale = _mm_set1_epi16(srcA);
        while (width >= 4) {
            __m128i dst_pixel = _mm_load_si128(d);
            __m128i mask_pixel = _mm_loadl_epi64(
                                     reinterpret_cast<const __m128i*>(src));

            // Check whether mask_pixels are equal to 0 and get the highest bit
            // of each byte of result, if mask pixes are all zero, we will get
            // pack_cmp to 0xFFFF
            int pack_cmp = _mm_movemask_epi8(_mm_cmpeq_epi16(mask_pixel,
                                             _mm_setzero_si128()));

            // if mask pixels are not all zero, we will blend the dst pixels
            if (pack_cmp != 0xFFFF) {
                // Unpack 4 16bit mask pixels to 
                // (p0, 0, p1, 0, p2, 0, p3, 0)
                mask_pixel = _mm_unpacklo_epi16(mask_pixel,
                                                _mm_setzero_si128());

                // Process 4 32bit dst pixels
                __m128i result = SkBlendLCD16_SSE2(srci, dst_pixel,
                                                   mask_pixel, scale); 
                _mm_store_si128(d, result);
            }

            d++;
            src += 4;
            width -= 4;
        }

        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (width > 0) {
        *dst = SkBlendLCD16(srcA, srcR, srcG, srcB, *dst, *src);
        src++;
        dst++;
        width--;        
    }
}

void SkBlitLCD16OpaqueRow_SSE2(SkPMColor dst[], const uint16_t src[],
                               SkColor color, int width, SkPMColor opaqueDst) {
    if (width <= 0) {
        return;
    }

    int srcR = SkColorGetR(color);
    int srcG = SkColorGetG(color);
    int srcB = SkColorGetB(color);

    if (width >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkBlendLCD16Opaque(srcR, srcG, srcB, *dst, *src, opaqueDst);
            src++;
            dst++;
            width--;
        }

        __m128i *d = reinterpret_cast<__m128i*>(dst);
        __m128i srci = _mm_set1_epi32(SkPackARGB32(0xFF, srcR, srcG, srcB));
        srci = _mm_unpacklo_epi8(srci, _mm_setzero_si128());
        while (width >= 4) {
            __m128i dst_pixel = _mm_load_si128(d);
            __m128i mask_pixel = _mm_loadl_epi64(
                                     reinterpret_cast<const __m128i*>(src));

            // Check whether mask_pixels are equal to 0 and get the highest bit
            // of each byte of result, if mask pixes are all zero, we will get
            // pack_cmp to 0xFFFF
            int pack_cmp = _mm_movemask_epi8(_mm_cmpeq_epi16(mask_pixel,
                                             _mm_setzero_si128()));

            // if mask pixels are not all zero, we will blend the dst pixels
            if (pack_cmp != 0xFFFF) {
                // Unpack 4 16bit mask pixels to 
                // (p0, 0, p1, 0, p2, 0, p3, 0)
                mask_pixel = _mm_unpacklo_epi16(mask_pixel,
                                                _mm_setzero_si128());

                // Process 4 32bit dst pixels
                __m128i result = SkBlendLCD16Opaque_SSE2(srci, dst_pixel,
                                                         mask_pixel); 
                _mm_store_si128(d, result);
            }

            d++;
            src += 4;
            width -= 4;
        }

        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (width > 0) {
        *dst = SkBlendLCD16Opaque(srcR, srcG, srcB, *dst, *src, opaqueDst);
        src++;
        dst++;
        width--;        
    }
}

/* SSE2 version of S32A_D565_Blend()
 * portable version is in core/SkBlitRow_D16.cpp
 */

void S32A_D565_Blend_SSE2(uint16_t* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src, int count,
                               U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 > alpha);

    while (((size_t)src & 0xF ) && count-- > 0 )
    {
        SkPMColor sc = *src++;
        if (sc) {
            uint16_t dc = *dst;
            unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
            unsigned dr = SkMulS16(SkPacked32ToR16(sc), alpha) + SkMulS16(SkGetPackedR16(dc), dst_scale);
            unsigned dg = SkMulS16(SkPacked32ToG16(sc), alpha) + SkMulS16(SkGetPackedG16(dc), dst_scale);
            unsigned db = SkMulS16(SkPacked32ToB16(sc), alpha) + SkMulS16(SkGetPackedB16(dc), dst_scale);
            *dst = SkPackRGB16(SkDiv255Round(dr), SkDiv255Round(dg), SkDiv255Round(db));
        }
        dst++;
     }

    __m128i v_alpha = _mm_set1_epi16(alpha);
    __m128i v_128 = _mm_set1_epi16(128);

    while (count >= 8) {

        __m128i src_pixel_l = _mm_load_si128((const __m128i*)(src));
        __m128i src_pixel_h = _mm_load_si128((const __m128i*)(src+4));

        // compute dst_scale
        __m128i dst_scale = _mm_srli_epi32(src_pixel_l, 24);
        dst_scale = _mm_packs_epi32(dst_scale, _mm_srli_epi32(src_pixel_h, 24));

        dst_scale = _mm_mullo_epi16(dst_scale, v_alpha);
        dst_scale = _mm_add_epi16(dst_scale, v_128);
        dst_scale = _mm_add_epi16(dst_scale, _mm_srli_epi16(dst_scale, 8));
        dst_scale = _mm_srli_epi16(dst_scale, 8);
        dst_scale = _mm_sub_epi16(_mm_set1_epi16(255), dst_scale);

        // unsigned dr = SkMulS16(SkPacked32ToR16(sc), alpha) + SkMulS16(SkGetPackedR16(dc), dst_scale);
        // unsigned dg = SkMulS16(SkPacked32ToG16(sc), alpha) + SkMulS16(SkGetPackedG16(dc), dst_scale);
        // unsigned db = SkMulS16(SkPacked32ToB16(sc), alpha) + SkMulS16(SkGetPackedB16(dc), dst_scale);

        __m128i dst_pixel = _mm_loadu_si128((__m128i *)dst);

        // SkMulS16(SkPacked32ToR16(sc), alpha)
        __m128i src_r_l = _mm_srli_epi32(src_pixel_l, 3);
        __m128i src_r_h = _mm_srli_epi32(src_pixel_h, 3);
        src_r_l = _mm_and_si128(src_r_l, _mm_set1_epi32(0x1F));
        src_r_h = _mm_and_si128(src_r_h, _mm_set1_epi32(0x1F));
        src_r_l = _mm_packs_epi32(src_r_l, src_r_h);
        src_r_l = _mm_mullo_epi16(src_r_l, v_alpha);

        // SkMulS16(SkGetPackedR16(dc), dst_scale)
        __m128i dst_r = _mm_srli_epi16(dst_pixel, SK_R16_SHIFT);
        dst_r = _mm_mullo_epi16(dst_r, dst_scale);
        dst_r = _mm_add_epi16(dst_r, src_r_l);
        dst_r = _mm_add_epi16(dst_r, v_128);
        // add and shift
        dst_r = _mm_add_epi16(dst_r, _mm_srli_epi16(dst_r, 8));
        dst_r = _mm_srli_epi16(dst_r, 8);

        // SkMulS16(SkPacked32ToG16(sc), alpha)

        __m128i src_g_l = _mm_srli_epi32(src_pixel_l, 10);
        __m128i src_g_h = _mm_srli_epi32(src_pixel_h, 10);
        src_g_l = _mm_and_si128(src_g_l, _mm_set1_epi32(0x3F));
        src_g_h = _mm_and_si128(src_g_h, _mm_set1_epi32(0x3F));
        src_g_l = _mm_packs_epi32(src_g_l, src_g_h);
        src_g_l = _mm_mullo_epi16(src_g_l, v_alpha);

        // SkMulS16(SkGetPackedG16(dc), dst_scale)
        __m128i dst_g = _mm_srli_epi16(dst_pixel, SK_G16_SHIFT);
        dst_g = _mm_and_si128(dst_g, _mm_set1_epi16(SK_G16_MASK));
        dst_g = _mm_mullo_epi16(dst_g, dst_scale);
        dst_g = _mm_add_epi16(dst_g, src_g_l);
        dst_g = _mm_add_epi16(dst_g, v_128);
        // add and shift
        dst_g = _mm_add_epi16(dst_g, _mm_srli_epi16(dst_g, 8));
        dst_g = _mm_srli_epi16(dst_g, 8);

        // SkMulS16(SkPacked32ToB16(sc), alpha)
        __m128i src_b_l = _mm_srli_epi32(src_pixel_l, 19);
        __m128i src_b_h = _mm_srli_epi32(src_pixel_h, 19);
        src_b_l = _mm_and_si128(src_b_l, _mm_set1_epi32(0x1F));
        src_b_h = _mm_and_si128(src_b_h, _mm_set1_epi32(0x1F));
        src_b_l = _mm_packs_epi32(src_b_l, src_b_h);
        src_b_l = _mm_mullo_epi16(src_b_l, v_alpha);

        // SkMulS16(SkGetPackedB16(dc), dst_scale);

        __m128i dst_b = _mm_and_si128(dst_pixel, _mm_set1_epi16(SK_B16_MASK));
        dst_b = _mm_mullo_epi16(dst_b, dst_scale);
        dst_b = _mm_add_epi16(dst_b, src_b_l);
        dst_b = _mm_add_epi16(dst_b, v_128);
        // add and shift
        dst_b = _mm_add_epi16(dst_b, _mm_srli_epi16(dst_b, 8));
        dst_b = _mm_srli_epi16(dst_b, 8);

        // *dst = SkPackRGB16(dr, dg, db);
        dst_r = _mm_slli_epi16(dst_r, SK_R16_SHIFT);
        dst_r = _mm_or_si128(dst_r, dst_b);
        dst_g = _mm_slli_epi16(dst_g, SK_G16_SHIFT);
        dst_r = _mm_or_si128(dst_r, dst_g);

        _mm_storeu_si128((__m128i *)dst, dst_r);

        dst += 8;
        src += 8;
        count -= 8;
}

    while (count-- > 0 )
    {
        SkPMColor sc = *src++;
        if (sc) {
            uint16_t dc = *dst;
            unsigned dst_scale = 255 - SkMulDiv255Round(SkGetPackedA32(sc), alpha);
            unsigned dr = SkMulS16(SkPacked32ToR16(sc), alpha) + SkMulS16(SkGetPackedR16(dc), dst_scale);
            unsigned dg = SkMulS16(SkPacked32ToG16(sc), alpha) + SkMulS16(SkGetPackedG16(dc), dst_scale);
            unsigned db = SkMulS16(SkPacked32ToB16(sc), alpha) + SkMulS16(SkGetPackedB16(dc), dst_scale);
            *dst = SkPackRGB16(SkDiv255Round(dr), SkDiv255Round(dg), SkDiv255Round(db));
        }
        dst++;
     }

}

/* SSE2 version of S32A_D565_Opaque()
 * portable version is in core/SkBlitRow_D16.cpp
 */

void S32A_D565_Opaque_SSE2(uint16_t* SK_RESTRICT dst,
                               const SkPMColor* SK_RESTRICT src, int count,
                               U8CPU alpha, int /*x*/, int /*y*/) {
    while(((intptr_t)src & 0xf) && (count-- > 0)){
                *dst = SkSrcOver32To16(*src, *dst);
        src++;
        dst++;
    }

    while (count >= 8) {
        __m128i src_pixel1 = _mm_load_si128((const __m128i*)(src));
                __m128i src_pixel2 = _mm_load_si128((const __m128i*)(src + 4));

        /* unpack src_r */
        __m128i src_r1 = _mm_slli_epi32(src_pixel1, 24);
        src_r1 = _mm_srli_epi32(src_r1, 24);
        __m128i src_r2 = _mm_slli_epi32(src_pixel2, 24);
        src_r2 = _mm_srli_epi32(src_r2, 24);
        __m128i src_r = _mm_packs_epi32 (src_r1, src_r2);

        /* unpack src_g */
        __m128i src_g1 = _mm_slli_epi32(src_pixel1, 16);
        src_g1 = _mm_srli_epi32(src_g1, 24);
        __m128i src_g2 = _mm_slli_epi32(src_pixel2, 16);
        src_g2 = _mm_srli_epi32(src_g2, 24);
        __m128i src_g = _mm_packs_epi32 (src_g1, src_g2);

        /* unpack src_b */
        __m128i src_b1 = _mm_slli_epi32(src_pixel1, 8);
        src_b1 = _mm_srli_epi32(src_b1, 24);
        __m128i src_b2 = _mm_slli_epi32(src_pixel2, 8);
        src_b2 = _mm_srli_epi32(src_b2, 24);
        __m128i src_b = _mm_packs_epi32 (src_b1, src_b2);

        /* compute isav */
        __m128i isav1 = _mm_srli_epi32(src_pixel1, 24);
        isav1 = _mm_sub_epi32(_mm_set1_epi32(255), isav1);
        __m128i isav2 = _mm_srli_epi32(src_pixel2, 24);
        isav2 = _mm_sub_epi32(_mm_set1_epi32(255), isav2);
        __m128i isav = _mm_packs_epi32 (isav1, isav2);

        __m128i dst_pixel = _mm_loadu_si128((__m128i *)dst);

        // get dst red
        __m128i dst_r = _mm_srli_epi16(dst_pixel, SK_R16_SHIFT);
        dst_r = _mm_and_si128(dst_r, _mm_set1_epi16(SK_R16_MASK));

        // get dst green
        __m128i dst_g = _mm_srli_epi16(dst_pixel, SK_G16_SHIFT);
        dst_g = _mm_and_si128(dst_g, _mm_set1_epi16(SK_G16_MASK));

        // get dst blue
        __m128i dst_b = _mm_srli_epi16(dst_pixel, SK_B16_SHIFT);
        dst_b = _mm_and_si128(dst_b, _mm_set1_epi16(SK_B16_MASK));

        // interleave blending of r,g,b
        // dr = (sr + SkMul16ShiftRound(dr,isa,SK_R16_BITS)) >> (8 - SK_R16_BITS);
        // dg = (sg + SkMul16ShiftRound(dg,isa,SK_G16_BITS)) >> (8 - SK_G16_BITS);
        // db = (sb + SkMul16ShiftRound(db,isa,SK_B16_BITS)) >> (8 - SK_B16_BITS);

        dst_r = _mm_mullo_epi16(dst_r, isav);
        dst_g = _mm_mullo_epi16(dst_g, isav);
        dst_b = _mm_mullo_epi16(dst_b, isav);

        dst_r = _mm_add_epi16(dst_r, _mm_set1_epi16(1 << (SK_R16_BITS - 1)));
        dst_g = _mm_add_epi16(dst_g, _mm_set1_epi16(1 << (SK_G16_BITS - 1)));
        dst_b = _mm_add_epi16(dst_b, _mm_set1_epi16(1 << (SK_B16_BITS - 1)));
        dst_r = _mm_add_epi16(dst_r, _mm_srli_epi16(dst_r, SK_R16_BITS));
        dst_g = _mm_add_epi16(dst_g, _mm_srli_epi16(dst_g, SK_G16_BITS));
        dst_r = _mm_srli_epi16(dst_r, SK_R16_BITS);

        dst_b = _mm_add_epi16(dst_b, _mm_srli_epi16(dst_b, SK_B16_BITS));
        dst_g = _mm_srli_epi16(dst_g, SK_G16_BITS);
        dst_r = _mm_add_epi16(dst_r, src_r);

        dst_b = _mm_srli_epi16(dst_b, SK_B16_BITS);
        dst_g = _mm_add_epi16(dst_g, src_g);

        dst_r = _mm_srli_epi16(dst_r, (8 - SK_R16_BITS));
        dst_b = _mm_add_epi16(dst_b, src_b);
        dst_g = _mm_srli_epi16(dst_g, (8 - SK_G16_BITS));
        dst_b = _mm_srli_epi16(dst_b, (8 - SK_B16_BITS));

        //	*dst = SkPackRGB16(dr, dg, db);
        //	SkToU16((r << SK_R16_SHIFT) | (g << SK_G16_SHIFT) | (b << SK_B16_SHIFT)
        dst_r = _mm_slli_epi16(dst_r, SK_R16_SHIFT);
        dst_g = _mm_slli_epi16(dst_g, SK_G16_SHIFT);
        dst_r = _mm_or_si128(dst_r, dst_g);
        dst_b = _mm_slli_epi16(dst_b, SK_B16_SHIFT);
        dst_r = _mm_or_si128(dst_r, dst_b);

        _mm_storeu_si128((__m128i *)dst, dst_r);

        dst += 8;
        src += 8;
        count -= 8;
    }

    while (count-- > 0) {
        *dst = SkSrcOver32To16(*src, *dst);
        src++;
        dst++;
    }
}

/* SSE2 version of S32A_D565_Opaque_Dither()
 * portable version is in core/SkBlitRow_D16.cpp
 */

void S32A_D565_Opaque_Dither_SSE2(uint16_t* SK_RESTRICT dst, const SkPMColor* SK_RESTRICT src,int count,
        U8CPU alpha, int x, int y)
{

#ifdef ENABLE_DITHER_MATRIX_4X4
    const uint8_t* dither_scan = gDitherMatrix_3Bit_4X4[(y) & 3];
#else
    uint16_t dither_scan = gDitherMatrix_3Bit_16[(y) & 3];
#endif

    __m128i dither_v, dither_p, maskFF;
    unsigned short dither_temp[4];

    // if we are using SIMD, load dither_scan (or precursor) into 8 ints
    if(count >= 8) {

#ifdef ENABLE_DITHER_MATRIX_4X4
    // #define DITHER_VALUE(x) dither_scan[(x) & 3]
        dither_temp[0] = dither_scan[x & 3];
        dither_temp[1] = dither_scan[x+1 & 3];
        dither_temp[2] = dither_scan[x+2 & 3];
        dither_temp[3] = dither_scan[x+3 & 3];
        dither_p = _mm_loadl_epi64((__m128i *) dither_temp);
        dither_p = _mm_shuffle_epi32(dither_p, 0x44);
#else
        dither_temp[0] = ((dither_scan >> ((x & 3) << 2)) & 0xF);
        dither_temp[1] = ((dither_scan >> (((x+1) & 3) << 2)) & 0xF);
        dither_temp[2] = ((dither_scan >> (((x+2) & 3) << 2)) & 0xF);
        dither_temp[3] = ((dither_scan >> (((x+3) & 3) << 2)) & 0xF);
        dither_p = _mm_loadl_epi64((__m128i *) dither_temp);
        dither_p = _mm_shuffle_epi32(dither_p, 0x44);
#endif
         maskFF = _mm_set1_epi32(0xFF);
     }

    while (count >= 8) {

        __m128i src_pixel_l = _mm_loadu_si128((const __m128i*)(src));
        __m128i src_pixel_h = _mm_loadu_si128((const __m128i*)(src+4));

        __m128i alpha_v = _mm_srli_epi32(src_pixel_l, 24);
        alpha_v = _mm_packs_epi32(alpha_v, _mm_srli_epi32(src_pixel_h, 24));

        dither_v = _mm_mullo_epi16(dither_p, _mm_add_epi16(alpha_v, _mm_set1_epi16(1)));
        dither_v = _mm_srli_epi16(dither_v, 8);

        // sr = (sr + d - (sr >> 5))
        __m128i src_r_l = _mm_and_si128(src_pixel_l, maskFF);
        __m128i src_r_h = _mm_and_si128(src_pixel_h, maskFF);
        src_r_l = _mm_packs_epi32(src_r_l, src_r_h);

        __m128i srvs = _mm_srli_epi16(src_r_l, 5);
        src_r_l = _mm_add_epi16(src_r_l, dither_v);
        src_r_l = _mm_sub_epi16(src_r_l, srvs);
        src_r_l = _mm_slli_epi16(src_r_l, 2);

        // sg = (sg + (d >> 1) - (sg >> 6))
        __m128i src_g_l = _mm_srli_epi32(src_pixel_l, 8);
        src_g_l = _mm_and_si128(src_g_l, maskFF);
        __m128i src_g_h = _mm_srli_epi32(src_pixel_h, 8);
        src_g_h = _mm_and_si128(src_g_h, maskFF);
        src_g_l = _mm_packs_epi32(src_g_l, src_g_h);

        __m128i sgvs = _mm_srli_epi16(src_g_l, 6);
        src_g_l = _mm_add_epi16(src_g_l,_mm_srli_epi16(dither_v, 1));
        src_g_l = _mm_sub_epi16(src_g_l, sgvs);
        src_g_l = _mm_slli_epi16(src_g_l, 3);

        //sb = (sb + d - (sb >> 5))
        __m128i src_b_l = _mm_srli_epi32(src_pixel_l, 16);
        src_b_l = _mm_and_si128(src_b_l, maskFF);
        __m128i src_b_h = _mm_srli_epi32(src_pixel_h, 16);
        src_b_h = _mm_and_si128(src_b_h, maskFF);
        src_b_l = _mm_packs_epi32(src_b_l, src_b_h);

        __m128i sbvs = _mm_srli_epi16(src_b_l, 5);
        src_b_l = _mm_add_epi16(src_b_l, dither_v);
        src_b_l = _mm_sub_epi16(src_b_l, sbvs);
        src_b_l = _mm_slli_epi16(src_b_l, 2);

        // unpack dst
        __m128i dst_pixel = _mm_loadu_si128((__m128i *)dst);

        // get dst red
        __m128i dst_r = _mm_srli_epi16(dst_pixel, SK_R16_SHIFT);
        dst_r = _mm_and_si128(dst_r, _mm_set1_epi16(SK_R16_MASK));

        // get dst green
        __m128i dst_g = _mm_srli_epi16(dst_pixel, SK_G16_SHIFT);
        dst_g = _mm_and_si128(dst_g, _mm_set1_epi16(SK_G16_MASK));

        // get dst blue
        __m128i dst_b = _mm_and_si128(dst_pixel, _mm_set1_epi16(SK_R16_MASK));

        //dst_expanded = dst_expanded * (SkAlpha255To256(255 - a) >> 3)
        __m128i avtemp = _mm_sub_epi16(_mm_set1_epi16(256), alpha_v);
        avtemp = _mm_srli_epi16(avtemp, 3);

        dst_r = _mm_mullo_epi16(dst_r, avtemp);
        dst_g = _mm_mullo_epi16(dst_g, avtemp);
        dst_b = _mm_mullo_epi16(dst_b, avtemp);

        // *dst = SkCompact_rgb_16((src_expanded + dst_expanded) >> 5)
        // (src_expanded + dst_expanded) >> 5

        dst_r = _mm_add_epi16(dst_r, src_r_l);
        dst_r = _mm_srli_epi16(dst_r, 5);

        dst_g = _mm_add_epi16(dst_g, src_g_l);
        dst_g = _mm_srli_epi16(dst_g, 5);

        dst_b = _mm_add_epi16(dst_b, src_b_l);
        dst_b = _mm_srli_epi16(dst_b, 5);

        dst_r = _mm_slli_epi16(dst_r, SK_R16_SHIFT);
        dst_g = _mm_slli_epi16(dst_g, SK_G16_SHIFT);
        dst_r = _mm_or_si128(dst_r, dst_g);
        dst_r = _mm_or_si128(dst_r, dst_b);

        _mm_storeu_si128((__m128i *)dst, dst_r);

        src += 8;
        dst += 8;
        x += 8;
        count -= 8;
    }

	if (count > 0) {
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            if (c) {
                unsigned a = SkGetPackedA32(c);
                int d = SkAlphaMul(DITHER_VALUE(x), SkAlpha255To256(a));
                unsigned sr = SkGetPackedR32(c);
                unsigned sg = SkGetPackedG32(c);
                unsigned sb = SkGetPackedB32(c);
                sr = SkDITHER_R32_FOR_565(sr, d);
                sg = SkDITHER_G32_FOR_565(sg, d);
                sb = SkDITHER_B32_FOR_565(sb, d);

                uint32_t src_expanded = (sg << 24) | (sr << 13) | (sb << 2);
                uint32_t dst_expanded = SkExpand_rgb_16(*dst);
                dst_expanded = dst_expanded * (SkAlpha255To256(255 - a) >> 3);
                // now src and dst expanded are in g:11 r:10 x:1 b:10
                *dst = SkCompact_rgb_16((src_expanded + dst_expanded) >> 5);
            }
            dst += 1;
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

/* SSE2 version of S32_D565_Opaque()
 * portable version is in core/SkBlitRow_D16.cpp
 */

void S32_D565_Opaque_SSE2(uint16_t* SK_RESTRICT dst, const SkPMColor* SK_RESTRICT src,int count,
        U8CPU alpha, int x, int y)
{

    __m128i mask_RB, mask_G;

    if(count >= 8) {
         mask_RB = _mm_set1_epi32(0x1F);
         mask_G  = _mm_set1_epi32(0x3F);
     }

    while (count >= 8) {

        __m128i src_pixel_l = _mm_loadu_si128((const __m128i*)(src));
        __m128i src_pixel_h = _mm_loadu_si128((const __m128i*)(src+4));

        __m128i src_r_l = _mm_srli_epi32(src_pixel_l, 3);
        src_r_l = _mm_and_si128(src_r_l, mask_RB);
        __m128i src_r_h = _mm_srli_epi32(src_pixel_h, 3);
        src_r_h = _mm_and_si128(src_r_h, mask_RB);
        src_r_l = _mm_packs_epi32(src_r_l, src_r_h);

        __m128i src_g_l = _mm_srli_epi32(src_pixel_l, 10);
        src_g_l = _mm_and_si128(src_g_l, mask_G);
        __m128i src_g_h = _mm_srli_epi32(src_pixel_h, 10);
        src_g_h = _mm_and_si128(src_g_h, mask_G);
        src_g_l = _mm_packs_epi32(src_g_l, src_g_h);

        __m128i src_b_l = _mm_srli_epi32(src_pixel_l, 19);
        src_b_l = _mm_and_si128(src_b_l, mask_RB);
        __m128i src_b_h = _mm_srli_epi32(src_pixel_h, 19);
        src_b_h = _mm_and_si128(src_b_h, mask_RB);
        src_b_l = _mm_packs_epi32(src_b_l, src_b_h);

        src_r_l = _mm_slli_epi16(src_r_l, SK_R16_SHIFT);
        src_g_l = _mm_slli_epi16(src_g_l, SK_G16_SHIFT);
        src_r_l = _mm_or_si128(src_r_l, src_g_l);
        src_r_l = _mm_or_si128(src_r_l, src_b_l);

        _mm_storeu_si128((__m128i *)dst, src_r_l);

        src += 8;
        dst += 8;
        count -= 8;
    }

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);
            *dst++ = SkPixel32ToPixel16_ToU16(c);
        } while (--count != 0);
    }
}

/* SSE2 version of S32_D565_Blend()
 * portable version is in core/SkBlitRow_D16.cpp
 */

void S32_D565_Blend_SSE2(uint16_t* SK_RESTRICT dst, const SkPMColor* SK_RESTRICT src,int count,
        U8CPU alpha, int x, int y)
{

    __m128i alpha_scale, mask_RB, mask_G;

    if(count >= 8) {
         alpha_scale = _mm_set1_epi16(SkAlpha255To256(alpha));
         mask_RB = _mm_set1_epi32(0x1F);
         mask_G  = _mm_set1_epi32(0x3F);
     }

    while (count >= 8) {

        __m128i src_pixel_l = _mm_loadu_si128((const __m128i*)(src));
        __m128i src_pixel_h = _mm_loadu_si128((const __m128i*)(src+4));

        __m128i src_r_l = _mm_srli_epi32(src_pixel_l, 3);
        src_r_l = _mm_and_si128(src_r_l, mask_RB);
        __m128i src_r_h = _mm_srli_epi32(src_pixel_h, 3);
        src_r_h = _mm_and_si128(src_r_h, mask_RB);
        src_r_l = _mm_packs_epi32(src_r_l, src_r_h);

        __m128i src_g_l = _mm_srli_epi32(src_pixel_l, 10);
        src_g_l = _mm_and_si128(src_g_l, mask_G);
        __m128i src_g_h = _mm_srli_epi32(src_pixel_h, 10);
        src_g_h = _mm_and_si128(src_g_h, mask_G);
        src_g_l = _mm_packs_epi32(src_g_l, src_g_h);

        __m128i src_b_l = _mm_srli_epi32(src_pixel_l, 19);
        src_b_l = _mm_and_si128(src_b_l, mask_RB);
        __m128i src_b_h = _mm_srli_epi32(src_pixel_h, 19);
        src_b_h = _mm_and_si128(src_b_h, mask_RB);
        src_b_l = _mm_packs_epi32(src_b_l, src_b_h);

        __m128i dst_pixel = _mm_loadu_si128((__m128i *)dst);
        __m128i dst_r = _mm_srli_epi16(dst_pixel, SK_R16_SHIFT);
        src_r_l = _mm_sub_epi16(src_r_l, dst_r);
        src_r_l = _mm_mullo_epi16(src_r_l, alpha_scale);
        src_r_l = _mm_srli_epi16(src_r_l, 8);
        src_r_l = _mm_add_epi16(src_r_l, dst_r);
        src_r_l = _mm_and_si128(src_r_l, _mm_set1_epi16(SK_R16_MASK));

        __m128i dst_g = _mm_srli_epi16(dst_pixel, SK_G16_SHIFT);
        dst_g = _mm_and_si128(dst_g, _mm_set1_epi16(SK_G16_MASK));
        src_g_l = _mm_sub_epi16(src_g_l, dst_g);
        src_g_l = _mm_mullo_epi16(src_g_l, alpha_scale);
        src_g_l = _mm_srli_epi16(src_g_l, 8);
        src_g_l = _mm_add_epi16(src_g_l, dst_g);
        src_g_l = _mm_and_si128(src_g_l, _mm_set1_epi16(SK_G16_MASK));

        __m128i dst_b = _mm_and_si128(dst_pixel, _mm_set1_epi16(SK_B16_MASK));
        src_b_l = _mm_sub_epi16(src_b_l, dst_b);
        src_b_l = _mm_mullo_epi16(src_b_l, alpha_scale);
        src_b_l = _mm_srli_epi16(src_b_l, 8);
        src_b_l = _mm_add_epi16(src_b_l, dst_b);
        src_b_l = _mm_and_si128(src_b_l, _mm_set1_epi16(SK_B16_MASK));

        src_r_l = _mm_slli_epi16(src_r_l, SK_R16_SHIFT);
        src_g_l = _mm_slli_epi16(src_g_l, SK_G16_SHIFT);
        src_r_l = _mm_or_si128(src_r_l, src_g_l);
        src_r_l = _mm_or_si128(src_r_l, src_b_l);

        _mm_storeu_si128((__m128i *)dst, src_r_l);

        src += 8;
        dst += 8;
        count -= 8;
    }


    if (count > 0) {
        int scale = SkAlpha255To256(alpha);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);
            uint16_t d = *dst;
            *dst++ = SkPackRGB16(
                    SkAlphaBlend(SkPacked32ToR16(c), SkGetPackedR16(d), scale),
                    SkAlphaBlend(SkPacked32ToG16(c), SkGetPackedG16(d), scale),
                    SkAlphaBlend(SkPacked32ToB16(c), SkGetPackedB16(d), scale));
        } while (--count != 0);
    }
}

/* SSE2 version of S32_D565_Opaque_Dither()
 * portable version is in core/SkBlitRow_D16.cpp
 */

void S32_D565_Opaque_Dither_SSE2(uint16_t* SK_RESTRICT dst, const SkPMColor* SK_RESTRICT src,int count,
        U8CPU alpha, int x, int y)
{
#ifdef ENABLE_DITHER_MATRIX_4X4
    const uint8_t* dither_scan = gDitherMatrix_3Bit_4X4[(y) & 3];
#else
    uint16_t dither_scan = gDitherMatrix_3Bit_16[(y) & 3];
#endif

    __m128i dither_v, maskFF;
    unsigned short dither_temp[4];

    // if we are using SIMD, load dither_scan (or precursor) into 8 ints
    if(count >= 8) {

#ifdef ENABLE_DITHER_MATRIX_4X4
    // #define DITHER_VALUE(x) dither_scan[(x) & 3]
        dither_temp[0] = dither_scan[x & 3];
        dither_temp[1] = dither_scan[x+1 & 3];
        dither_temp[2] = dither_scan[x+2 & 3];
        dither_temp[3] = dither_scan[x+3 & 3];
        dither_v = _mm_loadl_epi64((__m128i *) dither_temp);
        dither_v = _mm_shuffle_epi32(dither_v, 0x44);
#else
        dither_temp[0] = ((dither_scan >> ((x & 3) << 2)) & 0xF);
        dither_temp[1] = ((dither_scan >> (((x+1) & 3) << 2)) & 0xF);
        dither_temp[2] = ((dither_scan >> (((x+2) & 3) << 2)) & 0xF);
        dither_temp[3] = ((dither_scan >> (((x+3) & 3) << 2)) & 0xF);
        dither_v = _mm_loadl_epi64((__m128i *) dither_temp);
        dither_v = _mm_shuffle_epi32(dither_v, 0x44);
#endif
         maskFF = _mm_set1_epi32(0xFF);
     }

    while (count >= 8) {

        __m128i src_pixel_l = _mm_loadu_si128((const __m128i*)(src));
        __m128i src_pixel_h = _mm_loadu_si128((const __m128i*)(src+4));

        // sr = (sr + d - (sr >> 5))
        __m128i src_r_l = _mm_and_si128(src_pixel_l, maskFF);
        __m128i src_r_h = _mm_and_si128(src_pixel_h, maskFF);
        src_r_l = _mm_packs_epi32(src_r_l, src_r_h);

        __m128i srvs = _mm_srli_epi16(src_r_l, 5);
        src_r_l = _mm_add_epi16(src_r_l, dither_v);
        src_r_l = _mm_sub_epi16(src_r_l, srvs);
        src_r_l = _mm_srli_epi16(src_r_l, 3);

        // sg = (sg + (d >> 1) - (sg >> 6))
        __m128i src_g_l = _mm_srli_epi32(src_pixel_l, 8);
        src_g_l = _mm_and_si128(src_g_l, maskFF);
        __m128i src_g_h = _mm_srli_epi32(src_pixel_h, 8);
        src_g_h = _mm_and_si128(src_g_h, maskFF);
        src_g_l = _mm_packs_epi32(src_g_l, src_g_h);

        __m128i sgvs = _mm_srli_epi16(src_g_l, 6);
        src_g_l = _mm_add_epi16(src_g_l,_mm_srli_epi16(dither_v, 1));
        src_g_l = _mm_sub_epi16(src_g_l, sgvs);
        src_g_l = _mm_srli_epi16(src_g_l, 2);

        //sb = (sb + d - (sb >> 5))
        __m128i src_b_l = _mm_srli_epi32(src_pixel_l, 16);
        src_b_l = _mm_and_si128(src_b_l, maskFF);
        __m128i src_b_h = _mm_srli_epi32(src_pixel_h, 16);
        src_b_h = _mm_and_si128(src_b_h, maskFF);
        src_b_l = _mm_packs_epi32(src_b_l, src_b_h);

        __m128i sbvs = _mm_srli_epi16(src_b_l, 5);
        src_b_l = _mm_add_epi16(src_b_l, dither_v);
        src_b_l = _mm_sub_epi16(src_b_l, sbvs);
        src_b_l = _mm_srli_epi16(src_b_l, 3);

        src_r_l = _mm_slli_epi16(src_r_l, SK_R16_SHIFT);
        src_g_l = _mm_slli_epi16(src_g_l, SK_G16_SHIFT);
        src_r_l = _mm_or_si128(src_r_l, src_g_l);
        src_r_l = _mm_or_si128(src_r_l, src_b_l);

        _mm_storeu_si128((__m128i *)dst, src_r_l);

        src += 8;
        dst += 8;
        x += 8;
        count -= 8;
    }

    if (count > 0) {
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);

            unsigned dither = DITHER_VALUE(x);
            *dst++ = SkDitherRGB32To565(c, dither);
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

/* SSE2 version of S32_D565_Blend_Dither()
 * portable version is in core/SkBlitRow_D16.cpp
 */

void S32_D565_Blend_Dither_SSE2(uint16_t* SK_RESTRICT dst, const SkPMColor* SK_RESTRICT src,int count,
        U8CPU alpha, int x, int y)
{
#ifdef ENABLE_DITHER_MATRIX_4X4
    const uint8_t* dither_scan = gDitherMatrix_3Bit_4X4[(y) & 3];
#else
    uint16_t dither_scan = gDitherMatrix_3Bit_16[(y) & 3];
#endif

    __m128i dither_v, maskFF, alpha_scale;
    unsigned short dither_temp[4];

    // if we are using SIMD, load dither_scan (or precursor) into 8 ints
    if(count >= 8) {

#ifdef ENABLE_DITHER_MATRIX_4X4
    // #define DITHER_VALUE(x) dither_scan[(x) & 3]
        dither_temp[0] = dither_scan[x & 3];
        dither_temp[1] = dither_scan[x+1 & 3];
        dither_temp[2] = dither_scan[x+2 & 3];
        dither_temp[3] = dither_scan[x+3 & 3];
        dither_v = _mm_loadl_epi64((__m128i *) dither_temp);
        dither_v = _mm_shuffle_epi32(dither_v, 0x44);
#else
        dither_temp[0] = ((dither_scan >> ((x & 3) << 2)) & 0xF);
        dither_temp[1] = ((dither_scan >> (((x+1) & 3) << 2)) & 0xF);
        dither_temp[2] = ((dither_scan >> (((x+2) & 3) << 2)) & 0xF);
        dither_temp[3] = ((dither_scan >> (((x+3) & 3) << 2)) & 0xF);
        dither_v = _mm_loadl_epi64((__m128i *) dither_temp);
        dither_v = _mm_shuffle_epi32(dither_v, 0x44);
#endif
         maskFF = _mm_set1_epi32(0xFF);
         alpha_scale = _mm_set1_epi16(SkAlpha255To256(alpha));
     }

    while (count >= 8) {

        __m128i src_pixel_l = _mm_loadu_si128((const __m128i*)(src));
        __m128i src_pixel_h = _mm_loadu_si128((const __m128i*)(src+4));

        // sr = (sr + d - (sr >> 5))
        __m128i src_r_l = _mm_and_si128(src_pixel_l, maskFF);
        __m128i src_r_h = _mm_and_si128(src_pixel_h, maskFF);
        src_r_l = _mm_packs_epi32(src_r_l, src_r_h);

        __m128i srvs = _mm_srli_epi16(src_r_l, 5);
        src_r_l = _mm_add_epi16(src_r_l, dither_v);
        src_r_l = _mm_sub_epi16(src_r_l, srvs);
        src_r_l = _mm_srli_epi16(src_r_l, 3);

        // sg = (sg + (d >> 1) - (sg >> 6))
        __m128i src_g_l = _mm_srli_epi32(src_pixel_l, 8);
        src_g_l = _mm_and_si128(src_g_l, maskFF);
        __m128i src_g_h = _mm_srli_epi32(src_pixel_h, 8);
        src_g_h = _mm_and_si128(src_g_h, maskFF);
        src_g_l = _mm_packs_epi32(src_g_l, src_g_h);

        __m128i sgvs = _mm_srli_epi16(src_g_l, 6);
        src_g_l = _mm_add_epi16(src_g_l,_mm_srli_epi16(dither_v, 1));
        src_g_l = _mm_sub_epi16(src_g_l, sgvs);
        src_g_l = _mm_srli_epi16(src_g_l, 2);

        //sb = (sb + d - (sb >> 5))
        __m128i src_b_l = _mm_srli_epi32(src_pixel_l, 16);
        src_b_l = _mm_and_si128(src_b_l, maskFF);
        __m128i src_b_h = _mm_srli_epi32(src_pixel_h, 16);
        src_b_h = _mm_and_si128(src_b_h, maskFF);
        src_b_l = _mm_packs_epi32(src_b_l, src_b_h);

        __m128i sbvs = _mm_srli_epi16(src_b_l, 5);
        src_b_l = _mm_add_epi16(src_b_l, dither_v);
        src_b_l = _mm_sub_epi16(src_b_l, sbvs);
        src_b_l = _mm_srli_epi16(src_b_l, 3);

        __m128i dst_pixel = _mm_loadu_si128((__m128i *)dst);
        __m128i dst_r = _mm_srli_epi16(dst_pixel, SK_R16_SHIFT);
        src_r_l = _mm_sub_epi16(src_r_l, dst_r);
        src_r_l = _mm_mullo_epi16(src_r_l, alpha_scale);
        src_r_l = _mm_srli_epi16(src_r_l, 8);
        src_r_l = _mm_add_epi16(src_r_l, dst_r);
        src_r_l = _mm_and_si128(src_r_l, _mm_set1_epi16(SK_R16_MASK));

        __m128i dst_g = _mm_srli_epi16(dst_pixel, SK_G16_SHIFT);
        dst_g = _mm_and_si128(dst_g, _mm_set1_epi16(SK_G16_MASK));
        src_g_l = _mm_sub_epi16(src_g_l, dst_g);
        src_g_l = _mm_mullo_epi16(src_g_l, alpha_scale);
        src_g_l = _mm_srli_epi16(src_g_l, 8);
        src_g_l = _mm_add_epi16(src_g_l, dst_g);
        src_g_l = _mm_and_si128(src_g_l, _mm_set1_epi16(SK_G16_MASK));

        __m128i dst_b = _mm_and_si128(dst_pixel, _mm_set1_epi16(SK_B16_MASK));
        src_b_l = _mm_sub_epi16(src_b_l, dst_b);
        src_b_l = _mm_mullo_epi16(src_b_l, alpha_scale);
        src_b_l = _mm_srli_epi16(src_b_l, 8);
        src_b_l = _mm_add_epi16(src_b_l, dst_b);
        src_b_l = _mm_and_si128(src_b_l, _mm_set1_epi16(SK_B16_MASK));

        src_r_l = _mm_slli_epi16(src_r_l, SK_R16_SHIFT);
        src_g_l = _mm_slli_epi16(src_g_l, SK_G16_SHIFT);
        src_r_l = _mm_or_si128(src_r_l, src_g_l);
        src_r_l = _mm_or_si128(src_r_l, src_b_l);

        _mm_storeu_si128((__m128i *)dst, src_r_l);

        src += 8;
        dst += 8;
        x += 8;
        count -= 8;
    }

    if (count > 0) {
        int scale = SkAlpha255To256(alpha);
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            SkASSERT(SkGetPackedA32(c) == 255);

            int dither = DITHER_VALUE(x);
            int sr = SkGetPackedR32(c);
            int sg = SkGetPackedG32(c);
            int sb = SkGetPackedB32(c);
            sr = SkDITHER_R32To565(sr, dither);
            sg = SkDITHER_G32To565(sg, dither);
            sb = SkDITHER_B32To565(sb, dither);

            uint16_t d = *dst;
            *dst++ = SkPackRGB16(SkAlphaBlend(sr, SkGetPackedR16(d), scale),
                                 SkAlphaBlend(sg, SkGetPackedG16(d), scale),
                                 SkAlphaBlend(sb, SkGetPackedB16(d), scale));
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}
