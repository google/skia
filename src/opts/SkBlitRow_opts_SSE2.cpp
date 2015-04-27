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

        while (count >= 4) {
            // Load 4 pixels each of src and dest.
            __m128i src_pixel = _mm_loadu_si128(s);
            __m128i dst_pixel = _mm_load_si128(d);

            src_pixel = SkAlphaMulQ_SSE2(src_pixel, src_scale);
            dst_pixel = SkAlphaMulQ_SSE2(dst_pixel, dst_scale);

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

#ifdef SK_USE_ACCURATE_BLENDING
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
        src = reinterpret_cast<const SkPMColor*>(s);
        dst = reinterpret_cast<SkPMColor*>(d);
    }

    while (count > 0) {
        *dst = SkPMSrcOver(*src, *dst);
        src++;
        dst++;
        count--;
    }
#else
    int count16 = count / 16;
    __m128i* dst4 = (__m128i*)dst;
    const __m128i* src4 = (const __m128i*)src;

    for (int i = 0; i < count16 * 4; i += 4) {
        // Load 16 source pixels.
        __m128i s0 = _mm_loadu_si128(src4+i+0),
                s1 = _mm_loadu_si128(src4+i+1),
                s2 = _mm_loadu_si128(src4+i+2),
                s3 = _mm_loadu_si128(src4+i+3);

        const __m128i alphaMask = _mm_set1_epi32(0xFF << SK_A32_SHIFT);
        const __m128i ORed = _mm_or_si128(s3, _mm_or_si128(s2, _mm_or_si128(s1, s0)));
        __m128i cmp = _mm_cmpeq_epi8(_mm_and_si128(ORed, alphaMask), _mm_setzero_si128());
        if (0xffff == _mm_movemask_epi8(cmp)) {
            // All 16 source pixels are fully transparent. There's nothing to do!
            continue;
        }
        const __m128i ANDed = _mm_and_si128(s3, _mm_and_si128(s2, _mm_and_si128(s1, s0)));
        cmp = _mm_cmpeq_epi8(_mm_and_si128(ANDed, alphaMask), alphaMask);
        if (0xffff == _mm_movemask_epi8(cmp)) {
            // All 16 source pixels are fully opaque. There's no need to read dst or blend it.
            _mm_storeu_si128(dst4+i+0, s0);
            _mm_storeu_si128(dst4+i+1, s1);
            _mm_storeu_si128(dst4+i+2, s2);
            _mm_storeu_si128(dst4+i+3, s3);
            continue;
        }
        // The general slow case: do the blend for all 16 pixels.
        _mm_storeu_si128(dst4+i+0, SkPMSrcOver_SSE2(s0, _mm_loadu_si128(dst4+i+0)));
        _mm_storeu_si128(dst4+i+1, SkPMSrcOver_SSE2(s1, _mm_loadu_si128(dst4+i+1)));
        _mm_storeu_si128(dst4+i+2, SkPMSrcOver_SSE2(s2, _mm_loadu_si128(dst4+i+2)));
        _mm_storeu_si128(dst4+i+3, SkPMSrcOver_SSE2(s3, _mm_loadu_si128(dst4+i+3)));
    }

    // Wrap up the last <= 15 pixels.
    SkASSERT(count - (count16*16) <= 15);
    for (int i = count16*16; i < count; i++) {
        // This check is not really necessarily, but it prevents pointless autovectorization.
        if (src[i] & 0xFF000000) {
            dst[i] = SkPMSrcOver(src[i], dst[i]);
        }
    }
#endif
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

void Color32A_D565_SSE2(uint16_t dst[], SkPMColor src, int count, int x, int y) {
    SkASSERT(count > 0);

    uint32_t src_expand = (SkGetPackedG32(src) << 24) |
                          (SkGetPackedR32(src) << 13) |
                          (SkGetPackedB32(src) << 2);
    unsigned scale = SkAlpha255To256(0xFF - SkGetPackedA32(src)) >> 3;

    // Check if we have enough pixels to run SIMD
    if (count >= (int)(8 + (((16 - (size_t)dst) & 0x0F) >> 1))) {
        __m128i* dst_wide;
        const __m128i src_R_wide = _mm_set1_epi16(SkGetPackedR32(src) << 2);
        const __m128i src_G_wide = _mm_set1_epi16(SkGetPackedG32(src) << 3);
        const __m128i src_B_wide = _mm_set1_epi16(SkGetPackedB32(src) << 2);
        const __m128i scale_wide = _mm_set1_epi16(scale);
        const __m128i mask_blue  = _mm_set1_epi16(SK_B16_MASK);
        const __m128i mask_green = _mm_set1_epi16(SK_G16_MASK << SK_G16_SHIFT);

        // Align dst to an even 16 byte address (0-7 pixels)
        while (((((size_t)dst) & 0x0F) != 0) && (count > 0)) {
            *dst = SkBlend32_RGB16(src_expand, *dst, scale);
            dst += 1;
            count--;
        }

        dst_wide = reinterpret_cast<__m128i*>(dst);
        do {
            // Load eight RGB565 pixels
            __m128i pixels = _mm_load_si128(dst_wide);

            // Mask out sub-pixels
            __m128i pixel_R = _mm_srli_epi16(pixels, SK_R16_SHIFT);
            __m128i pixel_G = _mm_slli_epi16(pixels, SK_R16_BITS);
            pixel_G = _mm_srli_epi16(pixel_G, SK_R16_BITS + SK_B16_BITS);
            __m128i pixel_B = _mm_and_si128(pixels, mask_blue);

            // Scale with alpha
            pixel_R = _mm_mullo_epi16(pixel_R, scale_wide);
            pixel_G = _mm_mullo_epi16(pixel_G, scale_wide);
            pixel_B = _mm_mullo_epi16(pixel_B, scale_wide);

            // Add src_X_wide and shift down again
            pixel_R = _mm_add_epi16(pixel_R, src_R_wide);
            pixel_R = _mm_srli_epi16(pixel_R, 5);
            pixel_G = _mm_add_epi16(pixel_G, src_G_wide);
            pixel_B = _mm_add_epi16(pixel_B, src_B_wide);
            pixel_B = _mm_srli_epi16(pixel_B, 5);

            // Combine into RGB565 and store
            pixel_R = _mm_slli_epi16(pixel_R, SK_R16_SHIFT);
            pixel_G = _mm_and_si128(pixel_G, mask_green);
            pixels = _mm_or_si128(pixel_R, pixel_G);
            pixels = _mm_or_si128(pixels, pixel_B);
            _mm_store_si128(dst_wide, pixels);
            count -= 8;
            dst_wide++;
        } while (count >= 8);

        dst = reinterpret_cast<uint16_t*>(dst_wide);
    }

    // Small loop to handle remaining pixels.
    while (count > 0) {
        *dst = SkBlend32_RGB16(src_expand, *dst, scale);
        dst += 1;
        count--;
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
            __m128i src_pixel = _mm_set1_epi32(color);
            while (count >= 4) {
                // Load 4 dst pixels
                __m128i dst_pixel = _mm_load_si128(d);

                // Set the alpha value
                __m128i alpha_wide = _mm_cvtsi32_si128(*reinterpret_cast<const uint32_t*>(mask));
                alpha_wide = _mm_unpacklo_epi8(alpha_wide, _mm_setzero_si128());
                alpha_wide = _mm_unpacklo_epi16(alpha_wide, _mm_setzero_si128());

                __m128i result = SkBlendARGB32_SSE2(src_pixel, dst_pixel, alpha_wide);
                _mm_store_si128(d, result);
                // Load the next 4 dst pixels and alphas
                mask = mask + 4;
                d++;
                count -= 4;
            }
            dst = reinterpret_cast<SkPMColor*>(d);
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

        while (count >= 8) {
            // Load 8 pixels of src.
            __m128i src_pixel1 = _mm_loadu_si128(s++);
            __m128i src_pixel2 = _mm_loadu_si128(s++);

            __m128i d_pixel = SkPixel32ToPixel16_ToU16_SSE2(src_pixel1, src_pixel2);
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
