/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emmintrin.h>
#include "SkBitmapProcState_opts_SSE2.h"
#include "SkBlitRow_opts_SSE2.h"
#include "SkColorPriv.h"
#include "SkColor_opts_SSE2.h"
#include "SkDither.h"
#include "SkUtils.h"

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
        while (count > 0) {
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

static __m128i SkBlendLCD16_SSE2(__m128i &src, __m128i &dst,
                                 __m128i &mask, __m128i &srcA) {
    // In the following comments, the components of src, dst and mask are
    // abbreviated as (s)rc, (d)st, and (m)ask. Color components are marked
    // by an R, G, B, or A suffix. Components of one of the four pixels that
    // are processed in parallel are marked with 0, 1, 2, and 3. "d1B", for
    // example is the blue channel of the second destination pixel. Memory
    // layout is shown for an ARGB byte order in a color value.

    // src and srcA store 8-bit values interleaved with zeros.
    // src  = (0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
    // srcA = (srcA, 0, srcA, 0, srcA, 0, srcA, 0,
    //         srcA, 0, srcA, 0, srcA, 0, srcA, 0)
    // mask stores 16-bit values (compressed three channels) interleaved with zeros.
    // Lo and Hi denote the low and high bytes of a 16-bit value, respectively.
    // mask = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
    //         m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0)

    // Get the R,G,B of each 16bit mask pixel, we want all of them in 5 bits.
    // r = (0, m0R, 0, 0, 0, m1R, 0, 0, 0, m2R, 0, 0, 0, m3R, 0, 0)
    __m128i r = _mm_and_si128(SkPackedR16x5ToUnmaskedR32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_R32_SHIFT));

    // g = (0, 0, m0G, 0, 0, 0, m1G, 0, 0, 0, m2G, 0, 0, 0, m3G, 0)
    __m128i g = _mm_and_si128(SkPackedG16x5ToUnmaskedG32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_G32_SHIFT));

    // b = (0, 0, 0, m0B, 0, 0, 0, m1B, 0, 0, 0, m2B, 0, 0, 0, m3B)
    __m128i b = _mm_and_si128(SkPackedB16x5ToUnmaskedB32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_B32_SHIFT));

    // Pack the 4 16bit mask pixels into 4 32bit pixels, (p0, p1, p2, p3)
    // Each component (m0R, m0G, etc.) is then a 5-bit value aligned to an
    // 8-bit position
    // mask = (0, m0R, m0G, m0B, 0, m1R, m1G, m1B,
    //         0, m2R, m2G, m2B, 0, m3R, m3G, m3B)
    mask = _mm_or_si128(_mm_or_si128(r, g), b);

    // Interleave R,G,B into the lower byte of word.
    // i.e. split the sixteen 8-bit values from mask into two sets of eight
    // 16-bit values, padded by zero.
    __m128i maskLo, maskHi;
    // maskLo = (0, 0, m0R, 0, m0G, 0, m0B, 0, 0, 0, m1R, 0, m1G, 0, m1B, 0)
    maskLo = _mm_unpacklo_epi8(mask, _mm_setzero_si128());
    // maskHi = (0, 0, m2R, 0, m2G, 0, m2B, 0, 0, 0, m3R, 0, m3G, 0, m3B, 0)
    maskHi = _mm_unpackhi_epi8(mask, _mm_setzero_si128());

    // Upscale from 0..31 to 0..32
    // (allows to replace division by left-shift further down)
    // Left-shift each component by 4 and add the result back to that component,
    // mapping numbers in the range 0..15 to 0..15, and 16..31 to 17..32
    maskLo = _mm_add_epi16(maskLo, _mm_srli_epi16(maskLo, 4));
    maskHi = _mm_add_epi16(maskHi, _mm_srli_epi16(maskHi, 4));

    // Multiply each component of maskLo and maskHi by srcA
    maskLo = _mm_mullo_epi16(maskLo, srcA);
    maskHi = _mm_mullo_epi16(maskHi, srcA);

    // Left shift mask components by 8 (divide by 256)
    maskLo = _mm_srli_epi16(maskLo, 8);
    maskHi = _mm_srli_epi16(maskHi, 8);

    // Interleave R,G,B into the lower byte of the word
    // dstLo = (0, 0, d0R, 0, d0G, 0, d0B, 0, 0, 0, d1R, 0, d1G, 0, d1B, 0)
    __m128i dstLo = _mm_unpacklo_epi8(dst, _mm_setzero_si128());
    // dstLo = (0, 0, d2R, 0, d2G, 0, d2B, 0, 0, 0, d3R, 0, d3G, 0, d3B, 0)
    __m128i dstHi = _mm_unpackhi_epi8(dst, _mm_setzero_si128());

    // mask = (src - dst) * mask
    maskLo = _mm_mullo_epi16(maskLo, _mm_sub_epi16(src, dstLo));
    maskHi = _mm_mullo_epi16(maskHi, _mm_sub_epi16(src, dstHi));

    // mask = (src - dst) * mask >> 5
    maskLo = _mm_srai_epi16(maskLo, 5);
    maskHi = _mm_srai_epi16(maskHi, 5);

    // Add two pixels into result.
    // result = dst + ((src - dst) * mask >> 5)
    __m128i resultLo = _mm_add_epi16(dstLo, maskLo);
    __m128i resultHi = _mm_add_epi16(dstHi, maskHi);

    // Pack into 4 32bit dst pixels.
    // resultLo and resultHi contain eight 16-bit components (two pixels) each.
    // Merge into one SSE regsiter with sixteen 8-bit values (four pixels),
    // clamping to 255 if necessary.
    return _mm_packus_epi16(resultLo, resultHi);
}

static __m128i SkBlendLCD16Opaque_SSE2(__m128i &src, __m128i &dst,
                                       __m128i &mask) {
    // In the following comments, the components of src, dst and mask are
    // abbreviated as (s)rc, (d)st, and (m)ask. Color components are marked
    // by an R, G, B, or A suffix. Components of one of the four pixels that
    // are processed in parallel are marked with 0, 1, 2, and 3. "d1B", for
    // example is the blue channel of the second destination pixel. Memory
    // layout is shown for an ARGB byte order in a color value.

    // src and srcA store 8-bit values interleaved with zeros.
    // src  = (0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
    // mask stores 16-bit values (shown as high and low bytes) interleaved with
    // zeros
    // mask = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
    //         m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0)

    // Get the R,G,B of each 16bit mask pixel, we want all of them in 5 bits.
    // r = (0, m0R, 0, 0, 0, m1R, 0, 0, 0, m2R, 0, 0, 0, m3R, 0, 0)
    __m128i r = _mm_and_si128(SkPackedR16x5ToUnmaskedR32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_R32_SHIFT));

    // g = (0, 0, m0G, 0, 0, 0, m1G, 0, 0, 0, m2G, 0, 0, 0, m3G, 0)
    __m128i g = _mm_and_si128(SkPackedG16x5ToUnmaskedG32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_G32_SHIFT));

    // b = (0, 0, 0, m0B, 0, 0, 0, m1B, 0, 0, 0, m2B, 0, 0, 0, m3B)
    __m128i b = _mm_and_si128(SkPackedB16x5ToUnmaskedB32x5_SSE2(mask),
                              _mm_set1_epi32(0x1F << SK_B32_SHIFT));

    // Pack the 4 16bit mask pixels into 4 32bit pixels, (p0, p1, p2, p3)
    // Each component (m0R, m0G, etc.) is then a 5-bit value aligned to an
    // 8-bit position
    // mask = (0, m0R, m0G, m0B, 0, m1R, m1G, m1B,
    //         0, m2R, m2G, m2B, 0, m3R, m3G, m3B)
    mask = _mm_or_si128(_mm_or_si128(r, g), b);

    // Interleave R,G,B into the lower byte of word.
    // i.e. split the sixteen 8-bit values from mask into two sets of eight
    // 16-bit values, padded by zero.
    __m128i maskLo, maskHi;
    // maskLo = (0, 0, m0R, 0, m0G, 0, m0B, 0, 0, 0, m1R, 0, m1G, 0, m1B, 0)
    maskLo = _mm_unpacklo_epi8(mask, _mm_setzero_si128());
    // maskHi = (0, 0, m2R, 0, m2G, 0, m2B, 0, 0, 0, m3R, 0, m3G, 0, m3B, 0)
    maskHi = _mm_unpackhi_epi8(mask, _mm_setzero_si128());

    // Upscale from 0..31 to 0..32
    // (allows to replace division by left-shift further down)
    // Left-shift each component by 4 and add the result back to that component,
    // mapping numbers in the range 0..15 to 0..15, and 16..31 to 17..32
    maskLo = _mm_add_epi16(maskLo, _mm_srli_epi16(maskLo, 4));
    maskHi = _mm_add_epi16(maskHi, _mm_srli_epi16(maskHi, 4));

    // Interleave R,G,B into the lower byte of the word
    // dstLo = (0, 0, d0R, 0, d0G, 0, d0B, 0, 0, 0, d1R, 0, d1G, 0, d1B, 0)
    __m128i dstLo = _mm_unpacklo_epi8(dst, _mm_setzero_si128());
    // dstLo = (0, 0, d2R, 0, d2G, 0, d2B, 0, 0, 0, d3R, 0, d3G, 0, d3B, 0)
    __m128i dstHi = _mm_unpackhi_epi8(dst, _mm_setzero_si128());

    // mask = (src - dst) * mask
    maskLo = _mm_mullo_epi16(maskLo, _mm_sub_epi16(src, dstLo));
    maskHi = _mm_mullo_epi16(maskHi, _mm_sub_epi16(src, dstHi));

    // mask = (src - dst) * mask >> 5
    maskLo = _mm_srai_epi16(maskLo, 5);
    maskHi = _mm_srai_epi16(maskHi, 5);

    // Add two pixels into result.
    // result = dst + ((src - dst) * mask >> 5)
    __m128i resultLo = _mm_add_epi16(dstLo, maskLo);
    __m128i resultHi = _mm_add_epi16(dstHi, maskHi);

    // Pack into 4 32bit dst pixels and force opaque.
    // resultLo and resultHi contain eight 16-bit components (two pixels) each.
    // Merge into one SSE regsiter with sixteen 8-bit values (four pixels),
    // clamping to 255 if necessary. Set alpha components to 0xFF.
    return _mm_or_si128(_mm_packus_epi16(resultLo, resultHi),
                        _mm_set1_epi32(SK_A32_MASK << SK_A32_SHIFT));
}

void SkBlitLCD16Row_SSE2(SkPMColor dst[], const uint16_t mask[],
                         SkColor src, int width, SkPMColor) {
    if (width <= 0) {
        return;
    }

    int srcA = SkColorGetA(src);
    int srcR = SkColorGetR(src);
    int srcG = SkColorGetG(src);
    int srcB = SkColorGetB(src);

    srcA = SkAlpha255To256(srcA);

    if (width >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkBlendLCD16(srcA, srcR, srcG, srcB, *dst, *mask);
            mask++;
            dst++;
            width--;
        }

        __m128i *d = reinterpret_cast<__m128i*>(dst);
        // Set alpha to 0xFF and replicate source four times in SSE register.
        __m128i src_sse = _mm_set1_epi32(SkPackARGB32(0xFF, srcR, srcG, srcB));
        // Interleave with zeros to get two sets of four 16-bit values.
        src_sse = _mm_unpacklo_epi8(src_sse, _mm_setzero_si128());
        // Set srcA_sse to contain eight copies of srcA, padded with zero.
        // src_sse=(0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
        __m128i srcA_sse = _mm_set1_epi16(srcA);
        while (width >= 4) {
            // Load four destination pixels into dst_sse.
            __m128i dst_sse = _mm_load_si128(d);
            // Load four 16-bit masks into lower half of mask_sse.
            __m128i mask_sse = _mm_loadl_epi64(
                                   reinterpret_cast<const __m128i*>(mask));

            // Check whether masks are equal to 0 and get the highest bit
            // of each byte of result, if masks are all zero, we will get
            // pack_cmp to 0xFFFF
            int pack_cmp = _mm_movemask_epi8(_mm_cmpeq_epi16(mask_sse,
                                             _mm_setzero_si128()));

            // if mask pixels are not all zero, we will blend the dst pixels
            if (pack_cmp != 0xFFFF) {
                // Unpack 4 16bit mask pixels to
                // mask_sse = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
                //             m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0)
                mask_sse = _mm_unpacklo_epi16(mask_sse,
                                              _mm_setzero_si128());

                // Process 4 32bit dst pixels
                __m128i result = SkBlendLCD16_SSE2(src_sse, dst_sse,
                                                   mask_sse, srcA_sse);
                _mm_store_si128(d, result);
            }

            d++;
            mask += 4;
            width -= 4;
        }

        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (width > 0) {
        *dst = SkBlendLCD16(srcA, srcR, srcG, srcB, *dst, *mask);
        mask++;
        dst++;
        width--;
    }
}

void SkBlitLCD16OpaqueRow_SSE2(SkPMColor dst[], const uint16_t mask[],
                               SkColor src, int width, SkPMColor opaqueDst) {
    if (width <= 0) {
        return;
    }

    int srcR = SkColorGetR(src);
    int srcG = SkColorGetG(src);
    int srcB = SkColorGetB(src);

    if (width >= 4) {
        SkASSERT(((size_t)dst & 0x03) == 0);
        while (((size_t)dst & 0x0F) != 0) {
            *dst = SkBlendLCD16Opaque(srcR, srcG, srcB, *dst, *mask, opaqueDst);
            mask++;
            dst++;
            width--;
        }

        __m128i *d = reinterpret_cast<__m128i*>(dst);
        // Set alpha to 0xFF and replicate source four times in SSE register.
        __m128i src_sse = _mm_set1_epi32(SkPackARGB32(0xFF, srcR, srcG, srcB));
        // Set srcA_sse to contain eight copies of srcA, padded with zero.
        // src_sse=(0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
        src_sse = _mm_unpacklo_epi8(src_sse, _mm_setzero_si128());
        while (width >= 4) {
            // Load four destination pixels into dst_sse.
            __m128i dst_sse = _mm_load_si128(d);
            // Load four 16-bit masks into lower half of mask_sse.
            __m128i mask_sse = _mm_loadl_epi64(
                                   reinterpret_cast<const __m128i*>(mask));

            // Check whether masks are equal to 0 and get the highest bit
            // of each byte of result, if masks are all zero, we will get
            // pack_cmp to 0xFFFF
            int pack_cmp = _mm_movemask_epi8(_mm_cmpeq_epi16(mask_sse,
                                             _mm_setzero_si128()));

            // if mask pixels are not all zero, we will blend the dst pixels
            if (pack_cmp != 0xFFFF) {
                // Unpack 4 16bit mask pixels to
                // mask_sse = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
                //             m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0)
                mask_sse = _mm_unpacklo_epi16(mask_sse,
                                              _mm_setzero_si128());

                // Process 4 32bit dst pixels
                __m128i result = SkBlendLCD16Opaque_SSE2(src_sse, dst_sse,
                                                         mask_sse);
                _mm_store_si128(d, result);
            }

            d++;
            mask += 4;
            width -= 4;
        }

        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (width > 0) {
        *dst = SkBlendLCD16Opaque(srcR, srcG, srcB, *dst, *mask, opaqueDst);
        mask++;
        dst++;
        width--;
    }
}

/* SSE2 version of S32_D565_Opaque()
 * portable version is in core/SkBlitRow_D16.cpp
 */
void S32_D565_Opaque_SSE2(uint16_t* SK_RESTRICT dst,
                          const SkPMColor* SK_RESTRICT src, int count,
                          U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count <= 0) {
        return;
    }

    if (count >= 8) {
        while (((size_t)dst & 0x0F) != 0) {
            SkPMColor c = *src++;
            SkPMColorAssert(c);

            *dst++ = SkPixel32ToPixel16_ToU16(c);
            count--;
        }

        const __m128i* s = reinterpret_cast<const __m128i*>(src);
        __m128i* d = reinterpret_cast<__m128i*>(dst);
        __m128i r16_mask = _mm_set1_epi32(SK_R16_MASK);
        __m128i g16_mask = _mm_set1_epi32(SK_G16_MASK);
        __m128i b16_mask = _mm_set1_epi32(SK_B16_MASK);

        while (count >= 8) {
            // Load 8 pixels of src.
            __m128i src_pixel1 = _mm_loadu_si128(s++);
            __m128i src_pixel2 = _mm_loadu_si128(s++);

            // Calculate result r.
            __m128i r1 = _mm_srli_epi32(src_pixel1,
                                        SK_R32_SHIFT + (8 - SK_R16_BITS));
            r1 = _mm_and_si128(r1, r16_mask);
            __m128i r2 = _mm_srli_epi32(src_pixel2,
                                        SK_R32_SHIFT + (8 - SK_R16_BITS));
            r2 = _mm_and_si128(r2, r16_mask);
            __m128i r = _mm_packs_epi32(r1, r2);

            // Calculate result g.
            __m128i g1 = _mm_srli_epi32(src_pixel1,
                                        SK_G32_SHIFT + (8 - SK_G16_BITS));
            g1 = _mm_and_si128(g1, g16_mask);
            __m128i g2 = _mm_srli_epi32(src_pixel2,
                                        SK_G32_SHIFT + (8 - SK_G16_BITS));
            g2 = _mm_and_si128(g2, g16_mask);
            __m128i g = _mm_packs_epi32(g1, g2);

            // Calculate result b.
            __m128i b1 = _mm_srli_epi32(src_pixel1,
                                        SK_B32_SHIFT + (8 - SK_B16_BITS));
            b1 = _mm_and_si128(b1, b16_mask);
            __m128i b2 = _mm_srli_epi32(src_pixel2,
                                        SK_B32_SHIFT + (8 - SK_B16_BITS));
            b2 = _mm_and_si128(b2, b16_mask);
            __m128i b = _mm_packs_epi32(b1, b2);

            // Store 8 16-bit colors in dst.
            __m128i d_pixel = SkPackRGB16_SSE2(r, g, b);
            _mm_store_si128(d++, d_pixel);
            count -= 8;
        }
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<uint16_t*>(d);
    }

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            *dst++ = SkPixel32ToPixel16_ToU16(c);
        } while (--count != 0);
    }
}

/* SSE2 version of S32A_D565_Opaque()
 * portable version is in core/SkBlitRow_D16.cpp
 */
void S32A_D565_Opaque_SSE2(uint16_t* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src,
                           int count, U8CPU alpha, int /*x*/, int /*y*/) {
    SkASSERT(255 == alpha);

    if (count <= 0) {
        return;
    }

    if (count >= 8) {
        // Make dst 16 bytes alignment
        while (((size_t)dst & 0x0F) != 0) {
            SkPMColor c = *src++;
            if (c) {
              *dst = SkSrcOver32To16(c, *dst);
            }
            dst += 1;
            count--;
        }

        const __m128i* s = reinterpret_cast<const __m128i*>(src);
        __m128i* d = reinterpret_cast<__m128i*>(dst);
        __m128i var255 = _mm_set1_epi16(255);
        __m128i r16_mask = _mm_set1_epi16(SK_R16_MASK);
        __m128i g16_mask = _mm_set1_epi16(SK_G16_MASK);
        __m128i b16_mask = _mm_set1_epi16(SK_B16_MASK);

        while (count >= 8) {
            // Load 8 pixels of src.
            __m128i src_pixel1 = _mm_loadu_si128(s++);
            __m128i src_pixel2 = _mm_loadu_si128(s++);

            // Check whether src pixels are equal to 0 and get the highest bit
            // of each byte of result, if src pixels are all zero, src_cmp1 and
            // src_cmp2 will be 0xFFFF.
            int src_cmp1 = _mm_movemask_epi8(_mm_cmpeq_epi16(src_pixel1,
                                             _mm_setzero_si128()));
            int src_cmp2 = _mm_movemask_epi8(_mm_cmpeq_epi16(src_pixel2,
                                             _mm_setzero_si128()));
            if (src_cmp1 == 0xFFFF && src_cmp2 == 0xFFFF) {
                d++;
                count -= 8;
                continue;
            }

            // Load 8 pixels of dst.
            __m128i dst_pixel = _mm_load_si128(d);

            // Extract A from src.
            __m128i sa1 = _mm_slli_epi32(src_pixel1, (24 - SK_A32_SHIFT));
            sa1 = _mm_srli_epi32(sa1, 24);
            __m128i sa2 = _mm_slli_epi32(src_pixel2, (24 - SK_A32_SHIFT));
            sa2 = _mm_srli_epi32(sa2, 24);
            __m128i sa = _mm_packs_epi32(sa1, sa2);

            // Extract R from src.
            __m128i sr1 = _mm_slli_epi32(src_pixel1, (24 - SK_R32_SHIFT));
            sr1 = _mm_srli_epi32(sr1, 24);
            __m128i sr2 = _mm_slli_epi32(src_pixel2, (24 - SK_R32_SHIFT));
            sr2 = _mm_srli_epi32(sr2, 24);
            __m128i sr = _mm_packs_epi32(sr1, sr2);

            // Extract G from src.
            __m128i sg1 = _mm_slli_epi32(src_pixel1, (24 - SK_G32_SHIFT));
            sg1 = _mm_srli_epi32(sg1, 24);
            __m128i sg2 = _mm_slli_epi32(src_pixel2, (24 - SK_G32_SHIFT));
            sg2 = _mm_srli_epi32(sg2, 24);
            __m128i sg = _mm_packs_epi32(sg1, sg2);

            // Extract B from src.
            __m128i sb1 = _mm_slli_epi32(src_pixel1, (24 - SK_B32_SHIFT));
            sb1 = _mm_srli_epi32(sb1, 24);
            __m128i sb2 = _mm_slli_epi32(src_pixel2, (24 - SK_B32_SHIFT));
            sb2 = _mm_srli_epi32(sb2, 24);
            __m128i sb = _mm_packs_epi32(sb1, sb2);

            // Extract R G B from dst.
            __m128i dr = _mm_srli_epi16(dst_pixel, SK_R16_SHIFT);
            dr = _mm_and_si128(dr, r16_mask);
            __m128i dg = _mm_srli_epi16(dst_pixel, SK_G16_SHIFT);
            dg = _mm_and_si128(dg, g16_mask);
            __m128i db = _mm_srli_epi16(dst_pixel, SK_B16_SHIFT);
            db = _mm_and_si128(db, b16_mask);

            __m128i isa = _mm_sub_epi16(var255, sa); // 255 -sa

            // Calculate R G B of result.
            // Original algorithm is in SkSrcOver32To16().
            dr = _mm_add_epi16(sr, SkMul16ShiftRound_SSE2(dr, isa, SK_R16_BITS));
            dr = _mm_srli_epi16(dr, 8 - SK_R16_BITS);
            dg = _mm_add_epi16(sg, SkMul16ShiftRound_SSE2(dg, isa, SK_G16_BITS));
            dg = _mm_srli_epi16(dg, 8 - SK_G16_BITS);
            db = _mm_add_epi16(sb, SkMul16ShiftRound_SSE2(db, isa, SK_B16_BITS));
            db = _mm_srli_epi16(db, 8 - SK_B16_BITS);

            // Pack R G B into 16-bit color.
            __m128i d_pixel = SkPackRGB16_SSE2(dr, dg, db);

            // Store 8 16-bit colors in dst.
            _mm_store_si128(d++, d_pixel);
            count -= 8;
        }

        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<uint16_t*>(d);
    }

    if (count > 0) {
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);
            if (c) {
                *dst = SkSrcOver32To16(c, *dst);
            }
            dst += 1;
        } while (--count != 0);
    }
}

void S32_D565_Opaque_Dither_SSE2(uint16_t* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src,
                                 int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);

    if (count <= 0) {
        return;
    }

    if (count >= 8) {
        while (((size_t)dst & 0x0F) != 0) {
            DITHER_565_SCAN(y);
            SkPMColor c = *src++;
            SkPMColorAssert(c);

            unsigned dither = DITHER_VALUE(x);
            *dst++ = SkDitherRGB32To565(c, dither);
            DITHER_INC_X(x);
            count--;
        }

        unsigned short dither_value[8];
        __m128i dither;
#ifdef ENABLE_DITHER_MATRIX_4X4
        const uint8_t* dither_scan = gDitherMatrix_3Bit_4X4[(y) & 3];
        dither_value[0] = dither_value[4] = dither_scan[(x) & 3];
        dither_value[1] = dither_value[5] = dither_scan[(x + 1) & 3];
        dither_value[2] = dither_value[6] = dither_scan[(x + 2) & 3];
        dither_value[3] = dither_value[7] = dither_scan[(x + 3) & 3];
#else
        const uint16_t dither_scan = gDitherMatrix_3Bit_16[(y) & 3];
        dither_value[0] = dither_value[4] = (dither_scan
                                             >> (((x) & 3) << 2)) & 0xF;
        dither_value[1] = dither_value[5] = (dither_scan
                                             >> (((x + 1) & 3) << 2)) & 0xF;
        dither_value[2] = dither_value[6] = (dither_scan
                                             >> (((x + 2) & 3) << 2)) & 0xF;
        dither_value[3] = dither_value[7] = (dither_scan
                                             >> (((x + 3) & 3) << 2)) & 0xF;
#endif
        dither = _mm_loadu_si128((__m128i*) dither_value);

        const __m128i* s = reinterpret_cast<const __m128i*>(src);
        __m128i* d = reinterpret_cast<__m128i*>(dst);

        while (count >= 8) {
            // Load 8 pixels of src.
            __m128i src_pixel1 = _mm_loadu_si128(s++);
            __m128i src_pixel2 = _mm_loadu_si128(s++);

            // Extract R from src.
            __m128i sr1 = _mm_slli_epi32(src_pixel1, (24 - SK_R32_SHIFT));
            sr1 = _mm_srli_epi32(sr1, 24);
            __m128i sr2 = _mm_slli_epi32(src_pixel2, (24 - SK_R32_SHIFT));
            sr2 = _mm_srli_epi32(sr2, 24);
            __m128i sr = _mm_packs_epi32(sr1, sr2);

            // SkDITHER_R32To565(sr, dither)
            __m128i sr_offset = _mm_srli_epi16(sr, 5);
            sr = _mm_add_epi16(sr, dither);
            sr = _mm_sub_epi16(sr, sr_offset);
            sr = _mm_srli_epi16(sr, SK_R32_BITS - SK_R16_BITS);

            // Extract G from src.
            __m128i sg1 = _mm_slli_epi32(src_pixel1, (24 - SK_G32_SHIFT));
            sg1 = _mm_srli_epi32(sg1, 24);
            __m128i sg2 = _mm_slli_epi32(src_pixel2, (24 - SK_G32_SHIFT));
            sg2 = _mm_srli_epi32(sg2, 24);
            __m128i sg = _mm_packs_epi32(sg1, sg2);

            // SkDITHER_R32To565(sg, dither)
            __m128i sg_offset = _mm_srli_epi16(sg, 6);
            sg = _mm_add_epi16(sg, _mm_srli_epi16(dither, 1));
            sg = _mm_sub_epi16(sg, sg_offset);
            sg = _mm_srli_epi16(sg, SK_G32_BITS - SK_G16_BITS);

            // Extract B from src.
            __m128i sb1 = _mm_slli_epi32(src_pixel1, (24 - SK_B32_SHIFT));
            sb1 = _mm_srli_epi32(sb1, 24);
            __m128i sb2 = _mm_slli_epi32(src_pixel2, (24 - SK_B32_SHIFT));
            sb2 = _mm_srli_epi32(sb2, 24);
            __m128i sb = _mm_packs_epi32(sb1, sb2);

            // SkDITHER_R32To565(sb, dither)
            __m128i sb_offset = _mm_srli_epi16(sb, 5);
            sb = _mm_add_epi16(sb, dither);
            sb = _mm_sub_epi16(sb, sb_offset);
            sb = _mm_srli_epi16(sb, SK_B32_BITS - SK_B16_BITS);

            // Pack and store 16-bit dst pixel.
            __m128i d_pixel = SkPackRGB16_SSE2(sr, sg, sb);
            _mm_store_si128(d++, d_pixel);

            count -= 8;
            x += 8;
        }

        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<uint16_t*>(d);
    }

    if (count > 0) {
        DITHER_565_SCAN(y);
        do {
            SkPMColor c = *src++;
            SkPMColorAssert(c);

            unsigned dither = DITHER_VALUE(x);
            *dst++ = SkDitherRGB32To565(c, dither);
            DITHER_INC_X(x);
        } while (--count != 0);
    }
}

/* SSE2 version of S32A_D565_Opaque_Dither()
 * portable version is in core/SkBlitRow_D16.cpp
 */
void S32A_D565_Opaque_Dither_SSE2(uint16_t* SK_RESTRICT dst,
                                  const SkPMColor* SK_RESTRICT src,
                                  int count, U8CPU alpha, int x, int y) {
    SkASSERT(255 == alpha);

    if (count <= 0) {
        return;
    }

    if (count >= 8) {
        while (((size_t)dst & 0x0F) != 0) {
            DITHER_565_SCAN(y);
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
            count--;
        }

        unsigned short dither_value[8];
        __m128i dither, dither_cur;
#ifdef ENABLE_DITHER_MATRIX_4X4
        const uint8_t* dither_scan = gDitherMatrix_3Bit_4X4[(y) & 3];
        dither_value[0] = dither_value[4] = dither_scan[(x) & 3];
        dither_value[1] = dither_value[5] = dither_scan[(x + 1) & 3];
        dither_value[2] = dither_value[6] = dither_scan[(x + 2) & 3];
        dither_value[3] = dither_value[7] = dither_scan[(x + 3) & 3];
#else
        const uint16_t dither_scan = gDitherMatrix_3Bit_16[(y) & 3];
        dither_value[0] = dither_value[4] = (dither_scan
                                             >> (((x) & 3) << 2)) & 0xF;
        dither_value[1] = dither_value[5] = (dither_scan
                                             >> (((x + 1) & 3) << 2)) & 0xF;
        dither_value[2] = dither_value[6] = (dither_scan
                                             >> (((x + 2) & 3) << 2)) & 0xF;
        dither_value[3] = dither_value[7] = (dither_scan
                                             >> (((x + 3) & 3) << 2)) & 0xF;
#endif
        dither = _mm_loadu_si128((__m128i*) dither_value);

        const __m128i* s = reinterpret_cast<const __m128i*>(src);
        __m128i* d = reinterpret_cast<__m128i*>(dst);
        __m128i var256 = _mm_set1_epi16(256);
        __m128i r16_mask = _mm_set1_epi16(SK_R16_MASK);
        __m128i g16_mask = _mm_set1_epi16(SK_G16_MASK);
        __m128i b16_mask = _mm_set1_epi16(SK_B16_MASK);

        while (count >= 8) {
            // Load 8 pixels of src and dst.
            __m128i src_pixel1 = _mm_loadu_si128(s++);
            __m128i src_pixel2 = _mm_loadu_si128(s++);
            __m128i dst_pixel = _mm_load_si128(d);

            // Extract A from src.
            __m128i sa1 = _mm_slli_epi32(src_pixel1, (24 - SK_A32_SHIFT));
            sa1 = _mm_srli_epi32(sa1, 24);
            __m128i sa2 = _mm_slli_epi32(src_pixel2, (24 - SK_A32_SHIFT));
            sa2 = _mm_srli_epi32(sa2, 24);
            __m128i sa = _mm_packs_epi32(sa1, sa2);

            // Calculate current dither value.
            dither_cur = _mm_mullo_epi16(dither,
                                         _mm_add_epi16(sa, _mm_set1_epi16(1)));
            dither_cur = _mm_srli_epi16(dither_cur, 8);

            // Extract R from src.
            __m128i sr1 = _mm_slli_epi32(src_pixel1, (24 - SK_R32_SHIFT));
            sr1 = _mm_srli_epi32(sr1, 24);
            __m128i sr2 = _mm_slli_epi32(src_pixel2, (24 - SK_R32_SHIFT));
            sr2 = _mm_srli_epi32(sr2, 24);
            __m128i sr = _mm_packs_epi32(sr1, sr2);

            // SkDITHER_R32_FOR_565(sr, d)
            __m128i sr_offset = _mm_srli_epi16(sr, 5);
            sr = _mm_add_epi16(sr, dither_cur);
            sr = _mm_sub_epi16(sr, sr_offset);

            // Expand sr.
            sr = _mm_slli_epi16(sr, 2);

            // Extract G from src.
            __m128i sg1 = _mm_slli_epi32(src_pixel1, (24 - SK_G32_SHIFT));
            sg1 = _mm_srli_epi32(sg1, 24);
            __m128i sg2 = _mm_slli_epi32(src_pixel2, (24 - SK_G32_SHIFT));
            sg2 = _mm_srli_epi32(sg2, 24);
            __m128i sg = _mm_packs_epi32(sg1, sg2);

            // sg = SkDITHER_G32_FOR_565(sg, d).
            __m128i sg_offset = _mm_srli_epi16(sg, 6);
            sg = _mm_add_epi16(sg, _mm_srli_epi16(dither_cur, 1));
            sg = _mm_sub_epi16(sg, sg_offset);

            // Expand sg.
            sg = _mm_slli_epi16(sg, 3);

            // Extract B from src.
            __m128i sb1 = _mm_slli_epi32(src_pixel1, (24 - SK_B32_SHIFT));
            sb1 = _mm_srli_epi32(sb1, 24);
            __m128i sb2 = _mm_slli_epi32(src_pixel2, (24 - SK_B32_SHIFT));
            sb2 = _mm_srli_epi32(sb2, 24);
            __m128i sb = _mm_packs_epi32(sb1, sb2);

            // sb = SkDITHER_B32_FOR_565(sb, d).
            __m128i sb_offset = _mm_srli_epi16(sb, 5);
            sb = _mm_add_epi16(sb, dither_cur);
            sb = _mm_sub_epi16(sb, sb_offset);

            // Expand sb.
            sb = _mm_slli_epi16(sb, 2);

            // Extract R G B from dst.
            __m128i dr = _mm_srli_epi16(dst_pixel, SK_R16_SHIFT);
            dr = _mm_and_si128(dr, r16_mask);
            __m128i dg = _mm_srli_epi16(dst_pixel, SK_G16_SHIFT);
            dg = _mm_and_si128(dg, g16_mask);
            __m128i db = _mm_srli_epi16(dst_pixel, SK_B16_SHIFT);
            db = _mm_and_si128(db, b16_mask);

            // SkAlpha255To256(255 - a) >> 3
            __m128i isa = _mm_sub_epi16(var256, sa);
            isa = _mm_srli_epi16(isa, 3);

            dr = _mm_mullo_epi16(dr, isa);
            dr = _mm_add_epi16(dr, sr);
            dr = _mm_srli_epi16(dr, 5);

            dg = _mm_mullo_epi16(dg, isa);
            dg = _mm_add_epi16(dg, sg);
            dg = _mm_srli_epi16(dg, 5);

            db = _mm_mullo_epi16(db, isa);
            db = _mm_add_epi16(db, sb);
            db = _mm_srli_epi16(db, 5);

            // Package and store dst pixel.
            __m128i d_pixel = SkPackRGB16_SSE2(dr, dg, db);
            _mm_store_si128(d++, d_pixel);

            count -= 8;
            x += 8;
        }

        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<uint16_t*>(d);
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
