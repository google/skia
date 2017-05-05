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
#include "SkMSAN.h"
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
