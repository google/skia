/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorType.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkUtils.h"
#include "src/base/SkVx.h"
#include "src/core/SkBlitMask.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkMask.h"
#include "src/core/SkMemset.h"
#include "src/shaders/SkShaderBase.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

static inline int upscale_31_to_32(int value) {
    SkASSERT((unsigned)value <= 31);
    return value + (value >> 4);
}

static inline int blend_32(int src, int dst, int scale) {
    SkASSERT((unsigned)src <= 0xFF);
    SkASSERT((unsigned)dst <= 0xFF);
    SkASSERT((unsigned)scale <= 32);
    return dst + ((src - dst) * scale >> 5);
}

static inline SkPMColor blend_lcd16(int srcA, int srcR, int srcG, int srcB,
                                     SkPMColor dst, uint16_t mask) {
    if (mask == 0) {
        return dst;
    }

    /*  We want all of these in 5bits, hence the shifts in case one of them
     *  (green) is 6bits.
     */
    int maskR = SkGetPackedR16(mask) >> (SK_R16_BITS - 5);
    int maskG = SkGetPackedG16(mask) >> (SK_G16_BITS - 5);
    int maskB = SkGetPackedB16(mask) >> (SK_B16_BITS - 5);

    // Now upscale them to 0..32, so we can use blend32
    maskR = upscale_31_to_32(maskR);
    maskG = upscale_31_to_32(maskG);
    maskB = upscale_31_to_32(maskB);

    // srcA has been upscaled to 256 before passed into this function
    maskR = maskR * srcA >> 8;
    maskG = maskG * srcA >> 8;
    maskB = maskB * srcA >> 8;

    int dstA = SkGetPackedA32(dst);
    int dstR = SkGetPackedR32(dst);
    int dstG = SkGetPackedG32(dst);
    int dstB = SkGetPackedB32(dst);

    // Subtract 1 from srcA to bring it back to [0-255] to compare against dstA, alpha needs to
    // use either the min or the max of the LCD coverages. See https:/skbug.com/40037823
    int maskA = (srcA-1) < dstA ? std::min(maskR, std::min(maskG, maskB))
                                : std::max(maskR, std::max(maskG, maskB));

    return SkPackARGB32(blend_32(0xFF, dstA, maskA),
                        blend_32(srcR, dstR, maskR),
                        blend_32(srcG, dstG, maskG),
                        blend_32(srcB, dstB, maskB));
}

static inline SkPMColor blend_lcd16_opaque(int srcR, int srcG, int srcB,
                                           SkPMColor dst, uint16_t mask,
                                           SkPMColor opaqueDst) {
    if (mask == 0) {
        return dst;
    }

    if (0xFFFF == mask) {
        return opaqueDst;
    }

    /*  We want all of these in 5bits, hence the shifts in case one of them
     *  (green) is 6bits.
     */
    int maskR = SkGetPackedR16(mask) >> (SK_R16_BITS - 5);
    int maskG = SkGetPackedG16(mask) >> (SK_G16_BITS - 5);
    int maskB = SkGetPackedB16(mask) >> (SK_B16_BITS - 5);

    // Now upscale them to 0..32, so we can use blend32
    maskR = upscale_31_to_32(maskR);
    maskG = upscale_31_to_32(maskG);
    maskB = upscale_31_to_32(maskB);

    int dstA = SkGetPackedA32(dst);
    int dstR = SkGetPackedR32(dst);
    int dstG = SkGetPackedG32(dst);
    int dstB = SkGetPackedB32(dst);

    // Opaque src alpha always uses the max of the LCD coverages.
    int maskA = std::max(maskR, std::max(maskG, maskB));

    // LCD blitting is only supported if the dst is known/required
    // to be opaque
    return SkPackARGB32(blend_32(0xFF, dstA, maskA),
                        blend_32(srcR, dstR, maskR),
                        blend_32(srcG, dstG, maskG),
                        blend_32(srcB, dstB, maskB));
}


// TODO: rewrite at least the SSE code here.  It's miserable.

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <emmintrin.h>

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

    static __m128i blend_lcd16_sse2(__m128i &src, __m128i &dst, __m128i &mask, __m128i &srcA) {
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

        // a needs to be either the min or the max of the LCD coverages, depending on srcA < dstA
        __m128i aMin = _mm_min_epu8(_mm_slli_epi32(r, SK_A32_SHIFT - SK_R32_SHIFT),
                       _mm_min_epu8(_mm_slli_epi32(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                    _mm_slli_epi32(b, SK_A32_SHIFT - SK_B32_SHIFT)));
        __m128i aMax = _mm_max_epu8(_mm_slli_epi32(r, SK_A32_SHIFT - SK_R32_SHIFT),
                       _mm_max_epu8(_mm_slli_epi32(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                    _mm_slli_epi32(b, SK_A32_SHIFT - SK_B32_SHIFT)));
        // srcA has been biased to [0-256], so compare srcA against (dstA+1)
        __m128i a = _mm_cmplt_epi32(srcA,
                                    _mm_and_si128(
                                            _mm_add_epi32(dst, _mm_set1_epi32(1 << SK_A32_SHIFT)),
                                            _mm_set1_epi32(SK_A32_MASK)));
        // a = if_then_else(a, aMin, aMax) == (aMin & a) | (aMax & ~a)
        a = _mm_or_si128(_mm_and_si128(a, aMin), _mm_andnot_si128(a, aMax));

        // Pack the 4 16bit mask pixels into 4 32bit pixels, (p0, p1, p2, p3)
        // Each component (m0R, m0G, etc.) is then a 5-bit value aligned to an
        // 8-bit position
        // mask = (m0A, m0R, m0G, m0B, m1A, m1R, m1G, m1B,
        //         m2A, m2R, m2G, m2B, m3A, m3R, m3G, m3B)
        mask = _mm_or_si128(_mm_or_si128(a, r), _mm_or_si128(g, b));

        // Interleave R,G,B into the lower byte of word.
        // i.e. split the sixteen 8-bit values from mask into two sets of eight
        // 16-bit values, padded by zero.
        __m128i maskLo, maskHi;
        // maskLo = (m0A, 0, m0R, 0, m0G, 0, m0B, 0, m1A, 0, m1R, 0, m1G, 0, m1B, 0)
        maskLo = _mm_unpacklo_epi8(mask, _mm_setzero_si128());
        // maskHi = (m2A, 0, m2R, 0, m2G, 0, m2B, 0, m3A, 0, m3R, 0, m3G, 0, m3B, 0)
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
        // dstLo = (d0A, 0, d0R, 0, d0G, 0, d0B, 0, d1A, 0, d1R, 0, d1G, 0, d1B, 0)
        __m128i dstLo = _mm_unpacklo_epi8(dst, _mm_setzero_si128());
        // dstLo = (d2A, 0, d2R, 0, d2G, 0, d2B, 0, d3A, 0, d3R, 0, d3G, 0, d3B, 0)
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

    static __m128i blend_lcd16_opaque_sse2(__m128i &src, __m128i &dst, __m128i &mask) {
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

        // a = max(r, g, b) since opaque src alpha uses max of LCD coverages
        __m128i a = _mm_max_epu8(_mm_slli_epi32(r, SK_A32_SHIFT - SK_R32_SHIFT),
                    _mm_max_epu8(_mm_slli_epi32(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                 _mm_slli_epi32(b, SK_A32_SHIFT - SK_B32_SHIFT)));

        // Pack the 4 16bit mask pixels into 4 32bit pixels, (p0, p1, p2, p3)
        // Each component (m0R, m0G, etc.) is then a 5-bit value aligned to an
        // 8-bit position
        // mask = (m0A, m0R, m0G, m0B, m1A, m1R, m1G, m1B,
        //         m2A, m2R, m2G, m2B, m3A, m3R, m3G, m3B)
        mask = _mm_or_si128(_mm_or_si128(a, r), _mm_or_si128(g, b));

        // Interleave R,G,B into the lower byte of word.
        // i.e. split the sixteen 8-bit values from mask into two sets of eight
        // 16-bit values, padded by zero.
        __m128i maskLo, maskHi;
        // maskLo = (m0A, 0, m0R, 0, m0G, 0, m0B, 0, m1A, 0, m1R, 0, m1G, 0, m1B, 0)
        maskLo = _mm_unpacklo_epi8(mask, _mm_setzero_si128());
        // maskHi = (m2A, 0, m2R, 0, m2G, 0, m2B, 0, m3A, 0, m3R, 0, m3G, 0, m3B, 0)
        maskHi = _mm_unpackhi_epi8(mask, _mm_setzero_si128());

        // Upscale from 0..31 to 0..32
        // (allows to replace division by left-shift further down)
        // Left-shift each component by 4 and add the result back to that component,
        // mapping numbers in the range 0..15 to 0..15, and 16..31 to 17..32
        maskLo = _mm_add_epi16(maskLo, _mm_srli_epi16(maskLo, 4));
        maskHi = _mm_add_epi16(maskHi, _mm_srli_epi16(maskHi, 4));

        // Interleave R,G,B into the lower byte of the word
        // dstLo = (d0A, 0, d0R, 0, d0G, 0, d0B, 0, d1A, 0, d1R, 0, d1G, 0, d1B, 0)
        __m128i dstLo = _mm_unpacklo_epi8(dst, _mm_setzero_si128());
        // dstLo = (d2A, 0, d2R, 0, d2G, 0, d2B, 0, d3A, 0, d3R, 0, d3G, 0, d3B, 0)
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

        // Merge into one SSE regsiter with sixteen 8-bit values (four pixels),
        // clamping to 255 if necessary.
        return _mm_packus_epi16(resultLo, resultHi);
    }

    void blit_row_lcd16(SkPMColor dst[], const uint16_t mask[], SkColor src, int width, SkPMColor) {
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
                *dst = blend_lcd16(srcA, srcR, srcG, srcB, *dst, *mask);
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
                __m128i mask_sse = _mm_loadu_si64(mask);

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
                    __m128i result = blend_lcd16_sse2(src_sse, dst_sse, mask_sse, srcA_sse);
                    _mm_store_si128(d, result);
                }

                d++;
                mask += 4;
                width -= 4;
            }

            dst = reinterpret_cast<SkPMColor*>(d);
        }

        while (width > 0) {
            *dst = blend_lcd16(srcA, srcR, srcG, srcB, *dst, *mask);
            mask++;
            dst++;
            width--;
        }
    }

    void blit_row_lcd16_opaque(SkPMColor dst[], const uint16_t mask[],
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
                *dst = blend_lcd16_opaque(srcR, srcG, srcB, *dst, *mask, opaqueDst);
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
                __m128i mask_sse = _mm_loadu_si64(mask);

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
                    __m128i result = blend_lcd16_opaque_sse2(src_sse, dst_sse, mask_sse);
                    _mm_store_si128(d, result);
                }

                d++;
                mask += 4;
                width -= 4;
            }

            dst = reinterpret_cast<SkPMColor*>(d);
        }

        while (width > 0) {
            *dst = blend_lcd16_opaque(srcR, srcG, srcB, *dst, *mask, opaqueDst);
            mask++;
            dst++;
            width--;
        }
    }

#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>

    #define NEON_A (SK_A32_SHIFT / 8)
    #define NEON_R (SK_R32_SHIFT / 8)
    #define NEON_G (SK_G32_SHIFT / 8)
    #define NEON_B (SK_B32_SHIFT / 8)

    static inline uint8x8_t blend_32_neon(uint8x8_t src, uint8x8_t dst, uint16x8_t scale) {
        int16x8_t src_wide, dst_wide;

        src_wide = vreinterpretq_s16_u16(vmovl_u8(src));
        dst_wide = vreinterpretq_s16_u16(vmovl_u8(dst));

        src_wide = (src_wide - dst_wide) * vreinterpretq_s16_u16(scale);

        dst_wide += vshrq_n_s16(src_wide, 5);

        return vmovn_u16(vreinterpretq_u16_s16(dst_wide));
    }

    void blit_row_lcd16_opaque(SkPMColor dst[], const uint16_t src[],
                               SkColor color, int width,
                               SkPMColor opaqueDst) {
        int colR = SkColorGetR(color);
        int colG = SkColorGetG(color);
        int colB = SkColorGetB(color);

        uint8x8_t vcolA = vdup_n_u8(0xFF);
        uint8x8_t vcolR = vdup_n_u8(colR);
        uint8x8_t vcolG = vdup_n_u8(colG);
        uint8x8_t vcolB = vdup_n_u8(colB);

        while (width >= 8) {
            uint8x8x4_t vdst;
            uint16x8_t vmask;
            uint16x8_t vmaskR, vmaskG, vmaskB, vmaskA;

            vdst = vld4_u8((uint8_t*)dst);
            vmask = vld1q_u16(src);

            // Get all the color masks on 5 bits
            vmaskR = vshrq_n_u16(vmask, SK_R16_SHIFT);
            vmaskG = vshrq_n_u16(vshlq_n_u16(vmask, SK_R16_BITS),
                                 SK_B16_BITS + SK_R16_BITS + 1);
            vmaskB = vmask & vdupq_n_u16(SK_B16_MASK);

            // Upscale to 0..32
            vmaskR = vmaskR + vshrq_n_u16(vmaskR, 4);
            vmaskG = vmaskG + vshrq_n_u16(vmaskG, 4);
            vmaskB = vmaskB + vshrq_n_u16(vmaskB, 4);
            // Opaque srcAlpha always uses the max of the 3 LCD coverage values
            vmaskA = vmaxq_u16(vmaskR, vmaxq_u16(vmaskG, vmaskB));

            vdst.val[NEON_R] = blend_32_neon(vcolR, vdst.val[NEON_R], vmaskR);
            vdst.val[NEON_G] = blend_32_neon(vcolG, vdst.val[NEON_G], vmaskG);
            vdst.val[NEON_B] = blend_32_neon(vcolB, vdst.val[NEON_B], vmaskB);
            vdst.val[NEON_A] = blend_32_neon(vcolA, vdst.val[NEON_A], vmaskA);

            vst4_u8((uint8_t*)dst, vdst);

            dst += 8;
            src += 8;
            width -= 8;
        }

        // Leftovers
        for (int i = 0; i < width; i++) {
            dst[i] = blend_lcd16_opaque(colR, colG, colB, dst[i], src[i], opaqueDst);
        }
    }

    void blit_row_lcd16(SkPMColor dst[], const uint16_t src[],
                        SkColor color, int width, SkPMColor) {
        int colA = SkColorGetA(color);
        int colR = SkColorGetR(color);
        int colG = SkColorGetG(color);
        int colB = SkColorGetB(color);

        // srcA in [0-255] to compare vs dstA
        uint16x8_t vcolACmp = vdupq_n_u16(colA);
        colA = SkAlpha255To256(colA);

        uint16x8_t vcolA = vdupq_n_u16(colA); // srcA in [0-256] to combine with coverage
        uint8x8_t vcolR = vdup_n_u8(colR);
        uint8x8_t vcolG = vdup_n_u8(colG);
        uint8x8_t vcolB = vdup_n_u8(colB);

        while (width >= 8) {
            uint8x8x4_t vdst;
            uint16x8_t vmask;
            uint16x8_t vmaskR, vmaskG, vmaskB, vmaskA;

            vdst = vld4_u8((uint8_t*)dst);
            vmask = vld1q_u16(src);

            // Get all the color masks on 5 bits
            vmaskR = vshrq_n_u16(vmask, SK_R16_SHIFT);
            vmaskG = vshrq_n_u16(vshlq_n_u16(vmask, SK_R16_BITS),
                                 SK_B16_BITS + SK_R16_BITS + 1);
            vmaskB = vmask & vdupq_n_u16(SK_B16_MASK);

            // Upscale to 0..32
            vmaskR = vmaskR + vshrq_n_u16(vmaskR, 4);
            vmaskG = vmaskG + vshrq_n_u16(vmaskG, 4);
            vmaskB = vmaskB + vshrq_n_u16(vmaskB, 4);

            vmaskR = vshrq_n_u16(vmaskR * vcolA, 8);
            vmaskG = vshrq_n_u16(vmaskG * vcolA, 8);
            vmaskB = vshrq_n_u16(vmaskB * vcolA, 8);

            // Select either the min or the max of the RGB mask values, depending on if the src
            // alpha is less than the dst alpha.
            vmaskA = vbslq_u16(vcleq_u16(vcolACmp, vmovl_u8(vdst.val[NEON_A])), // srcA < dstA
                               vminq_u16(vmaskR, vminq_u16(vmaskG, vmaskB)),    // ? min(r,g,b)
                               vmaxq_u16(vmaskR, vmaxq_u16(vmaskG, vmaskB)));   // : max(r,g,b)

            vdst.val[NEON_R] = blend_32_neon(vcolR, vdst.val[NEON_R], vmaskR);
            vdst.val[NEON_G] = blend_32_neon(vcolG, vdst.val[NEON_G], vmaskG);
            vdst.val[NEON_B] = blend_32_neon(vcolB, vdst.val[NEON_B], vmaskB);
            // vmaskA already includes vcolA so blend against 0xFF
            vdst.val[NEON_A] = blend_32_neon(vdup_n_u8(0xFF), vdst.val[NEON_A], vmaskA);
            vst4_u8((uint8_t*)dst, vdst);

            dst += 8;
            src += 8;
            width -= 8;
        }

        for (int i = 0; i < width; i++) {
            dst[i] = blend_lcd16(colA, colR, colG, colB, dst[i], src[i]);
        }
    }

#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX

    // The following (left) shifts cause the top 5 bits of the mask components to
    // line up with the corresponding components in an SkPMColor.
    // Note that the mask's RGB16 order may differ from the SkPMColor order.
    #define SK_R16x5_R32x5_SHIFT (SK_R32_SHIFT - SK_R16_SHIFT - SK_R16_BITS + 5)
    #define SK_G16x5_G32x5_SHIFT (SK_G32_SHIFT - SK_G16_SHIFT - SK_G16_BITS + 5)
    #define SK_B16x5_B32x5_SHIFT (SK_B32_SHIFT - SK_B16_SHIFT - SK_B16_BITS + 5)

    #if SK_R16x5_R32x5_SHIFT == 0
        #define SkPackedR16x5ToUnmaskedR32x5_LASX(x) (x)
    #elif SK_R16x5_R32x5_SHIFT > 0
        #define SkPackedR16x5ToUnmaskedR32x5_LASX(x) (__lasx_xvslli_w(x, SK_R16x5_R32x5_SHIFT))
    #else
        #define SkPackedR16x5ToUnmaskedR32x5_LASX(x) (__lasx_xvsrli_w(x, -SK_R16x5_R32x5_SHIFT))
    #endif

    #if SK_G16x5_G32x5_SHIFT == 0
        #define SkPackedG16x5ToUnmaskedG32x5_LASX(x) (x)
    #elif SK_G16x5_G32x5_SHIFT > 0
        #define SkPackedG16x5ToUnmaskedG32x5_LASX(x) (__lasx_xvslli_w(x, SK_G16x5_G32x5_SHIFT))
    #else
        #define SkPackedG16x5ToUnmaskedG32x5_LASX(x) (__lasx_xvsrli_w(x, -SK_G16x5_G32x5_SHIFT))
    #endif

    #if SK_B16x5_B32x5_SHIFT == 0
        #define SkPackedB16x5ToUnmaskedB32x5_LASX(x) (x)
    #elif SK_B16x5_B32x5_SHIFT > 0
        #define SkPackedB16x5ToUnmaskedB32x5_LASX(x) (__lasx_xvslli_w(x, SK_B16x5_B32x5_SHIFT))
    #else
        #define SkPackedB16x5ToUnmaskedB32x5_LASX(x) (__lasx_xvsrli_w(x, -SK_B16x5_B32x5_SHIFT))
    #endif

    static __m256i blend_lcd16_lasx(__m256i &src, __m256i &dst, __m256i &mask, __m256i &srcA) {
        // In the following comments, the components of src, dst and mask are
        // abbreviated as (s)rc, (d)st, and (m)ask. Color components are marked
        // by an R, G, B, or A suffix. Components of one of the four pixels that
        // are processed in parallel are marked with 0, 1, 2, and 3. "d1B", for
        // example is the blue channel of the second destination pixel. Memory
        // layout is shown for an ARGB byte order in a color value.

        // src and srcA store 8-bit values interleaved with zeros.
        // src  = (0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0,
        //         0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
        // srcA = (srcA, 0, srcA, 0, srcA, 0, srcA, 0,
        //         srcA, 0, srcA, 0, srcA, 0, srcA, 0,
        //         srcA, 0, srcA, 0, srcA, 0, srcA, 0,
        //         srcA, 0, srcA, 0, srcA, 0, srcA, 0)
        // mask stores 16-bit values (compressed three channels) interleaved with zeros.
        // Lo and Hi denote the low and high bytes of a 16-bit value, respectively.
        // mask = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
        //         m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0,
        //         m4RGBLo, m4RGBHi, 0, 0, m5RGBLo, m5RGBHi, 0, 0,
        //         m6RGBLo, m6RGBHi, 0, 0, m7RGBLo, m7RGBHi, 0, 0)

        __m256i xv_zero = __lasx_xvldi(0);

        // Get the R,G,B of each 16bit mask pixel, we want all of them in 5 bits.
        // r = (0, m0R, 0, 0, 0, m1R, 0, 0, 0, m2R, 0, 0, 0, m3R, 0, 0,
        //      0, m4R, 0, 0, 0, m5R, 0, 0, 0, m6R, 0, 0, 0, m7R, 0, 0)
        __m256i r = __lasx_xvand_v(SkPackedR16x5ToUnmaskedR32x5_LASX(mask),
                                   __lasx_xvreplgr2vr_w(0x1F << SK_R32_SHIFT));

        // g = (0, 0, m0G, 0, 0, 0, m1G, 0, 0, 0, m2G, 0, 0, 0, m3G, 0)
        //      0, 0, m4G, 0, 0, 0, m5G, 0, 0, 0, m6G, 0, 0, 0, m7R, 0)
        __m256i g = __lasx_xvand_v(SkPackedG16x5ToUnmaskedG32x5_LASX(mask),
                                   __lasx_xvreplgr2vr_w(0x1F << SK_G32_SHIFT));

        // b = (0, 0, 0, m0B, 0, 0, 0, m1B, 0, 0, 0, m2B, 0, 0, 0, m3B)
        //      0, 0, 0, m4B, 0, 0, 0, m5B, 0, 0, 0, m6B, 0, 0, 0, m7B)
        __m256i b = __lasx_xvand_v(SkPackedB16x5ToUnmaskedB32x5_LASX(mask),
                                   __lasx_xvreplgr2vr_w(0x1F << SK_B32_SHIFT));

        // a needs to be either the min or the max of the LCD coverages, depending on srcA < dstA
        __m256i aMin = __lasx_xvmin_b(__lasx_xvslli_w(r, SK_A32_SHIFT - SK_R32_SHIFT),
                       __lasx_xvmin_b(__lasx_xvslli_w(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                      __lasx_xvslli_w(b, SK_A32_SHIFT - SK_B32_SHIFT)));
        __m256i aMax = __lasx_xvmax_b(__lasx_xvslli_w(r, SK_A32_SHIFT - SK_R32_SHIFT),
                       __lasx_xvmax_b(__lasx_xvslli_w(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                      __lasx_xvslli_w(b, SK_A32_SHIFT - SK_B32_SHIFT)));
        // srcA has been biased to [0-256], so compare srcA against (dstA+1)
        __m256i a = __lasx_xvmskltz_w(srcA -
                                    __lasx_xvand_v(
                                           __lasx_xvadd_w(dst,
                                                          __lasx_xvreplgr2vr_w(1 << SK_A32_SHIFT)),
                                           __lasx_xvreplgr2vr_w(SK_A32_MASK)));
        // a = if_then_else(a, aMin, aMax) == (aMin & a) | (aMax & ~a)
        a = __lasx_xvor_v(__lasx_xvand_v(a, aMin), __lasx_xvandn_v(a, aMax));

        // Pack the 8 16bit mask pixels into 8 32bit pixels, (p0, p1, p2, p3)
        // Each component (m0R, m0G, etc.) is then a 5-bit value aligned to an
        // 8-bit position
        // mask = (m0A, m0R, m0G, m0B, m1R, m1R, m1G, m1B,
        //         m2A, m2R, m2G, m2B, m3R, m3R, m3G, m3B,
        //         m4A, m4R, m4G, m4B, m5R, m5R, m5G, m5B,
        //         m6A, m6R, m6G, m6B, m7R, m7R, m7G, m7B)
        mask = __lasx_xvor_v(__lasx_xvor_v(a, r), __lasx_xvor_v(g, b));

        // Interleave R,G,B into the lower byte of word.
        // i.e. split the sixteen 8-bit values from mask into two sets of sixteen
        // 16-bit values, padded by zero.
        __m256i maskLo, maskHi;
        // maskLo = (m0A, 0, m0R, 0, m0G, 0, m0B, 0, m1A, 0, m1R, 0, m1G, 0, m1B, 0,
        //           m2A, 0, m2R, 0, m2G, 0, m2B, 0, m3A, 0, m3R, 0, m3G, 0, m3B, 0)
        maskLo = __lasx_xvilvl_b(xv_zero, mask);
        // maskHi = (m4A, 0, m4R, 0, m4G, 0, m4B, 0, m5A, 0, m5R, 0, m5G, 0, m5B, 0,
        //           m6A, 0, m6R, 0, m6G, 0, m6B, 0, m7A, 0, m7R, 0, m7G, 0, m7B, 0)
        maskHi = __lasx_xvilvh_b(xv_zero, mask);

        // Upscale from 0..31 to 0..32
        // (allows to replace division by left-shift further down)
        // Left-shift each component by 4 and add the result back to that component,
        // mapping numbers in the range 0..15 to 0..15, and 16..31 to 17..32
        maskLo = __lasx_xvadd_h(maskLo, __lasx_xvsrli_h(maskLo, 4));
        maskHi = __lasx_xvadd_h(maskHi, __lasx_xvsrli_h(maskHi, 4));

        // Multiply each component of maskLo and maskHi by srcA
        maskLo = __lasx_xvmul_h(maskLo, srcA);
        maskHi = __lasx_xvmul_h(maskHi, srcA);

        // Left shift mask components by 8 (divide by 256)
        maskLo = __lasx_xvsrli_h(maskLo, 8);
        maskHi = __lasx_xvsrli_h(maskHi, 8);

        // Interleave R,G,B into the lower byte of the word
        // dstLo = (d0A, 0, d0R, 0, d0G, 0, d0B, 0, d1A, 0, d1R, 0, d1G, 0, d1B, 0)
        //          d2A, 0, d2R, 0, d2G, 0, d2B, 0, d3A, 0, d3R, 0, d3G, 0, d3B, 0)
        __m256i dstLo = __lasx_xvilvl_b(xv_zero, dst);
        // dstLo = (d4A, 0, d4R, 0, d4G, 0, d4B, 0, d5A, 0, d5R, 0, d5G, 0, d5B, 0)
        //          d6A, 0, d6R, 0, d6G, 0, d6B, 0, d7A, 0, d7R, 0, d7G, 0, d7B, 0)
        __m256i dstHi = __lasx_xvilvh_b(xv_zero, dst);

        // mask = (src - dst) * mask
        maskLo = __lasx_xvmul_h(maskLo, __lasx_xvsub_h(src, dstLo));
        maskHi = __lasx_xvmul_h(maskHi, __lasx_xvsub_h(src, dstHi));

        // mask = (src - dst) * mask >> 5
        maskLo = __lasx_xvsrai_h(maskLo, 5);
        maskHi = __lasx_xvsrai_h(maskHi, 5);

        // Add two pixels into result.
        // result = dst + ((src - dst) * mask >> 5)
        __m256i resultLo = __lasx_xvadd_h(dstLo, maskLo);
        __m256i resultHi = __lasx_xvadd_h(dstHi, maskHi);

        // Pack into 8 32bit dst pixels.
        // resultLo and resultHi contain sixteen 16-bit components (four pixels) each.
        // Merge into one LASX regsiter with 32 8-bit values (eight pixels),
        // clamping to 255 if necessary.
        __m256i tmpl = __lasx_xvsat_hu(resultLo, 7);
        __m256i tmph = __lasx_xvsat_hu(resultHi, 7);
        return __lasx_xvpickev_b(tmph, tmpl);
    }

    static __m256i blend_lcd16_opaque_lasx(__m256i &src, __m256i &dst, __m256i &mask) {
        // In the following comments, the components of src, dst and mask are
        // abbreviated as (s)rc, (d)st, and (m)ask. Color components are marked
        // by an R, G, B, or A suffix. Components of one of the four pixels that
        // are processed in parallel are marked with 0, 1, 2, and 3. "d1B", for
        // example is the blue channel of the second destination pixel. Memory
        // layout is shown for an ARGB byte order in a color value.

        // src and srcA store 8-bit values interleaved with zeros.
        // src  = (0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0,
        //         0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
        // mask stores 16-bit values (shown as high and low bytes) interleaved with
        // zeros
        // mask = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
        //         m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0,
        //         m4RGBLo, m4RGBHi, 0, 0, m5RGBLo, m5RGBHi, 0, 0,
        //         m6RGBLo, m6RGBHi, 0, 0, m7RGBLo, m7RGBHi, 0, 0)

        __m256i xv_zero = __lasx_xvldi(0);

        // Get the R,G,B of each 16bit mask pixel, we want all of them in 5 bits.
        // r = (0, m0R, 0, 0, 0, m1R, 0, 0, 0, m2R, 0, 0, 0, m3R, 0, 0,
        //      0, m4R, 0, 0, 0, m5R, 0, 0, 0, m6R, 0, 0, 0, m7R, 0, 0)
        __m256i r = __lasx_xvand_v(SkPackedR16x5ToUnmaskedR32x5_LASX(mask),
                                   __lasx_xvreplgr2vr_w(0x1F << SK_R32_SHIFT));

        // g = (0, 0, m0G, 0, 0, 0, m1G, 0, 0, 0, m2G, 0, 0, 0, m3G, 0,
        //      0, 0, m4G, 0, 0, 0, m5G, 0, 0, 0, m6G, 0, 0, 0, m7G, 0)
        __m256i g = __lasx_xvand_v(SkPackedG16x5ToUnmaskedG32x5_LASX(mask),
                                   __lasx_xvreplgr2vr_w(0x1F << SK_G32_SHIFT));

        // b = (0, 0, 0, m0B, 0, 0, 0, m1B, 0, 0, 0, m2B, 0, 0, 0, m3B,
        //      0, 0, 0, m4B, 0, 0, 0, m5B, 0, 0, 0, m6B, 0, 0, 0, m7B)
        __m256i b = __lasx_xvand_v(SkPackedB16x5ToUnmaskedB32x5_LASX(mask),
                                   __lasx_xvreplgr2vr_w(0x1F << SK_B32_SHIFT));

        // a = max(r, g, b) since opaque src alpha uses max of LCD coverages
        __m256i a = __lasx_xvmax_b(__lasx_xvslli_w(r, SK_A32_SHIFT - SK_R32_SHIFT),
                    __lasx_xvmax_b(__lasx_xvslli_w(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                   __lasx_xvslli_w(b, SK_A32_SHIFT - SK_B32_SHIFT)));

        // Pack the 8 16bit mask pixels into 8 32bit pixels, (p0, p1, p2, p3,
        // p4, p5, p6, p7)
        // Each component (m0R, m0G, etc.) is then a 5-bit value aligned to an
        // 8-bit position
        // mask = (m0A, m0R, m0G, m0B, m1A, m1R, m1G, m1B,
        //         m2A, m2R, m2G, m2B, m3A, m3R, m3G, m3B,
        //         m4A, m4R, m4G, m4B, m5A, m5R, m5G, m5B,
        //         m6A, m6R, m6G, m6B, m7A, m7R, m7G, m7B)
        mask = __lasx_xvor_v(__lasx_xvor_v(a, r), __lasx_xvor_v(g, b));

        // Interleave R,G,B into the lower byte of word.
        // i.e. split the 32 8-bit values from mask into two sets of sixteen
        // 16-bit values, padded by zero.
        __m256i maskLo, maskHi;
        // maskLo = (m0A, 0, m0R, 0, m0G, 0, m0B, 0, m1A, 0, m1R, 0, m1G, 0, m1B, 0,
        //           m2A, 0, m2R, 0, m2G, 0, m2B, 0, m3A, 0, m3R, 0, m3G, 0, m3B, 0)
        maskLo = __lasx_xvilvl_b(xv_zero, mask);
        // maskHi = (m4A, 0, m4R, 0, m4G, 0, m4B, 0, m5A, 0, m5R, 0, m5G, 0, m5B, 0,
        //           m6A, 0, m6R, 0, m6G, 0, m6B, 0, m7A, 0, m7R, 0, m7G, 0, m7B, 0)
        maskHi = __lasx_xvilvh_b(xv_zero, mask);

        // Upscale from 0..31 to 0..32
        // (allows to replace division by left-shift further down)
        // Left-shift each component by 4 and add the result back to that component,
        // mapping numbers in the range 0..15 to 0..15, and 16..31 to 17..32
        maskLo = __lasx_xvadd_h(maskLo, __lasx_xvsrli_h(maskLo, 4));
        maskHi = __lasx_xvadd_h(maskHi, __lasx_xvsrli_h(maskHi, 4));

        // Interleave R,G,B into the lower byte of the word
        // dstLo = (d0A, 0, d0R, 0, d0G, 0, d0B, 0, d1A, 0, d1R, 0, d1G, 0, d1B, 0,
        //          d2A, 0, d2R, 0, d2G, 0, d2B, 0, d3A, 0, d3R, 0, d3G, 0, d3B, 0)
        __m256i dstLo = __lasx_xvilvl_b(xv_zero, dst);
        // dstLo = (d4A, 0, d4R, 0, d4G, 0, d4B, 0, d5A, 0, d5R, 0, d5G, 0, d5B, 0,
        // dstLo = (d6A, 0, d6R, 0, d6G, 0, d6B, 0, d7A, 0, d7R, 0, d7G, 0, d7B, 0)
        __m256i dstHi = __lasx_xvilvh_b(xv_zero, dst);

        // mask = (src - dst) * mask
        maskLo = __lasx_xvmul_h(maskLo, __lasx_xvsub_h(src, dstLo));
        maskHi = __lasx_xvmul_h(maskHi, __lasx_xvsub_h(src, dstHi));

        // mask = (src - dst) * mask >> 5
        maskLo = __lasx_xvsrai_h(maskLo, 5);
        maskHi = __lasx_xvsrai_h(maskHi, 5);

        // Add two pixels into result.
        // result = dst + ((src - dst) * mask >> 5)
        __m256i resultLo = __lasx_xvadd_h(dstLo, maskLo);
        __m256i resultHi = __lasx_xvadd_h(dstHi, maskHi);

        // Merge into one SSE regsiter with 32 8-bit values (eight pixels),
        // clamping to 255 if necessary.
        __m256i tmpl = __lasx_xvsat_hu(resultLo, 7);
        __m256i tmph = __lasx_xvsat_hu(resultHi, 7);

        return __lasx_xvpickev_b(tmph, tmpl);
    }

    void blit_row_lcd16(SkPMColor dst[], const uint16_t mask[], SkColor src, int width, SkPMColor) {
        if (width <= 0) {
            return;
        }

        int srcA = SkColorGetA(src);
        int srcR = SkColorGetR(src);
        int srcG = SkColorGetG(src);
        int srcB = SkColorGetB(src);
        __m256i xv_zero = __lasx_xvldi(0);

        srcA = SkAlpha255To256(srcA);
        if (width >= 8) {
            SkASSERT(((size_t)dst & 0x03) == 0);
            while (((size_t)dst & 0x0F) != 0) {
                *dst = blend_lcd16(srcA, srcR, srcG, srcB, *dst, *mask);
                mask++;
                dst++;
                width--;
            }

            __m256i *d = reinterpret_cast<__m256i*>(dst);
            // Set alpha to 0xFF and replicate source eight times in LASX register.
            unsigned int skpackargb32 = SkPackARGB32(0xFF, srcR, srcG, srcB);
            __m256i src_lasx = __lasx_xvreplgr2vr_w(skpackargb32);
            // Interleave with zeros to get two sets of eight 16-bit values.
            src_lasx = __lasx_xvilvl_b(xv_zero, src_lasx);
            // Set srcA_lasx to contain sixteen copies of srcA, padded with zero.
            // src_lasx=(0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0,
            //           0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
            __m256i srcA_lasx = __lasx_xvreplgr2vr_h(srcA);

            while (width >= 8) {
                // Load eight destination pixels into dst_lasx.
                __m256i dst_lasx = __lasx_xvld(d, 0);
                // Load eight 16-bit masks into lower half of mask_lasx.
                __m256i mask_lasx = __lasx_xvld(mask, 0);
                mask_lasx = (__m256i){mask_lasx[0], 0, mask_lasx[1], 0};

                int pack_cmp = __lasx_xbz_v(mask_lasx);
                // if mask pixels are not all zero, we will blend the dst pixels
                if (pack_cmp != 1) {
                    // Unpack 8 16bit mask pixels to
                    // mask_lasx = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
                    //              m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0,
                    //              m4RGBLo, m4RGBHi, 0, 0, m5RGBLo, m5RGBHi, 0, 0,
                    //              m6RGBLo, m6RGBHi, 0, 0, m7RGBLo, m7RGBHi, 0, 0)
                    mask_lasx = __lasx_xvilvl_h(xv_zero, mask_lasx);

                    // Process 8 32bit dst pixels
                    __m256i result = blend_lcd16_lasx(src_lasx, dst_lasx, mask_lasx, srcA_lasx);
                    __lasx_xvst(result, d, 0);
                }
                d++;
                mask += 8;
                width -= 8;
            }
            dst = reinterpret_cast<SkPMColor*>(d);
        }

        while (width > 0) {
            *dst = blend_lcd16(srcA, srcR, srcG, srcB, *dst, *mask);
            mask++;
            dst++;
            width--;
        }
    }

    void blit_row_lcd16_opaque(SkPMColor dst[], const uint16_t mask[],
                               SkColor src, int width, SkPMColor opaqueDst) {
        if (width <= 0) {
            return;
        }

        int srcR = SkColorGetR(src);
        int srcG = SkColorGetG(src);
        int srcB = SkColorGetB(src);
        __m256i xv_zero = __lasx_xvldi(0);

        if (width >= 8) {
            SkASSERT(((size_t)dst & 0x03) == 0);
            while (((size_t)dst & 0x0F) != 0) {
                *dst = blend_lcd16_opaque(srcR, srcG, srcB, *dst, *mask, opaqueDst);
                mask++;
                dst++;
                width--;
            }

            __m256i *d = reinterpret_cast<__m256i*>(dst);
            // Set alpha to 0xFF and replicate source four times in LASX register.
            unsigned int sk_pack_argb32 = SkPackARGB32(0xFF, srcR, srcG, srcB);
            __m256i src_lasx = __lasx_xvreplgr2vr_w(sk_pack_argb32);
            // Set srcA_lasx to contain sixteen copies of srcA, padded with zero.
            // src_lasx=(0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0,
            //           0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
            src_lasx = __lasx_xvilvl_b(xv_zero, src_lasx);

            while (width >= 8) {
                // Load eight destination pixels into dst_lasx.
                __m256i dst_lasx = __lasx_xvld(d, 0);
                // Load eight 16-bit masks into lower half of mask_lasx.
                __m256i mask_lasx = __lasx_xvld(mask, 0);
                mask_lasx = (__m256i){mask_lasx[0], 0, mask_lasx[1], 0};

                int32_t pack_cmp = __lasx_xbz_v(mask_lasx);
                // if mask pixels are not all zero, we will blend the dst pixels
                if (pack_cmp != 1) {
                    // Unpack 8 16bit mask pixels to
                    // mask_lasx = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
                    //              m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0,
                    //              m4RGBLo, m4RGBHi, 0, 0, m5RGBLo, m5RGBHi, 0, 0,
                    //              m6RGBLo, m6RGBHi, 0, 0, m7RGBLo, m7RGBHi, 0, 0)
                    mask_lasx = __lasx_xvilvl_h(xv_zero, mask_lasx);
                    // Process 8 32bit dst pixels
                    __m256i result = blend_lcd16_opaque_lasx(src_lasx, dst_lasx, mask_lasx);
                    __lasx_xvst(result, d, 0);
                }
                d++;
                mask += 8;
                width -= 8;
            }

            dst = reinterpret_cast<SkPMColor*>(d);
        }

        while (width > 0) {
            *dst = blend_lcd16_opaque(srcR, srcG, srcB, *dst, *mask, opaqueDst);
            mask++;
            dst++;
            width--;
        }
    }

#elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX

    // The following (left) shifts cause the top 5 bits of the mask components to
    // line up with the corresponding components in an SkPMColor.
    // Note that the mask's RGB16 order may differ from the SkPMColor order.
    #define SK_R16x5_R32x5_SHIFT (SK_R32_SHIFT - SK_R16_SHIFT - SK_R16_BITS + 5)
    #define SK_G16x5_G32x5_SHIFT (SK_G32_SHIFT - SK_G16_SHIFT - SK_G16_BITS + 5)
    #define SK_B16x5_B32x5_SHIFT (SK_B32_SHIFT - SK_B16_SHIFT - SK_B16_BITS + 5)

    #if SK_R16x5_R32x5_SHIFT == 0
        #define SkPackedR16x5ToUnmaskedR32x5_LSX(x) (x)
    #elif SK_R16x5_R32x5_SHIFT > 0
        #define SkPackedR16x5ToUnmaskedR32x5_LSX(x) (__lsx_vslli_w(x, SK_R16x5_R32x5_SHIFT))
    #else
        #define SkPackedR16x5ToUnmaskedR32x5_LSX(x) (__lsx_vsrli_w(x, -SK_R16x5_R32x5_SHIFT))
    #endif

    #if SK_G16x5_G32x5_SHIFT == 0
        #define SkPackedG16x5ToUnmaskedG32x5_LSX(x) (x)
    #elif SK_G16x5_G32x5_SHIFT > 0
        #define SkPackedG16x5ToUnmaskedG32x5_LSX(x) (__lsx_vslli_w(x, SK_G16x5_G32x5_SHIFT))
    #else
        #define SkPackedG16x5ToUnmaskedG32x5_LSX(x) (__lsx_vsrli_w(x, -SK_G16x5_G32x5_SHIFT))
    #endif

    #if SK_B16x5_B32x5_SHIFT == 0
        #define SkPackedB16x5ToUnmaskedB32x5_LSX(x) (x)
    #elif SK_B16x5_B32x5_SHIFT > 0
        #define SkPackedB16x5ToUnmaskedB32x5_LSX(x) (__lsx_vslli_w(x, SK_B16x5_B32x5_SHIFT))
    #else
        #define SkPackedB16x5ToUnmaskedB32x5_LSX(x) (__lsx_vsrli_w(x, -SK_B16x5_B32x5_SHIFT))
    #endif

    static __m128i blend_lcd16_lsx(__m128i &src, __m128i &dst, __m128i &mask, __m128i &srcA) {
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

        __m128i v_zero = __lsx_vldi(0);

        // Get the R,G,B of each 16bit mask pixel, we want all of them in 5 bits.
        // r = (0, m0R, 0, 0, 0, m1R, 0, 0, 0, m2R, 0, 0, 0, m3R, 0, 0)
        __m128i r = __lsx_vand_v(SkPackedR16x5ToUnmaskedR32x5_LSX(mask),
                                 __lsx_vreplgr2vr_w(0x1F << SK_R32_SHIFT));

        // g = (0, 0, m0G, 0, 0, 0, m1G, 0, 0, 0, m2G, 0, 0, 0, m3G, 0)
        __m128i g = __lsx_vand_v(SkPackedG16x5ToUnmaskedG32x5_LSX(mask),
                                 __lsx_vreplgr2vr_w(0x1F << SK_G32_SHIFT));

        // b = (0, 0, 0, m0B, 0, 0, 0, m1B, 0, 0, 0, m2B, 0, 0, 0, m3B)
        __m128i b = __lsx_vand_v(SkPackedB16x5ToUnmaskedB32x5_LSX(mask),
                                 __lsx_vreplgr2vr_w(0x1F << SK_B32_SHIFT));

        // a needs to be either the min or the max of the LCD coverages, depending on srcA < dstA
        __m128i aMin = __lsx_vmin_b(__lsx_vslli_w(r, SK_A32_SHIFT - SK_R32_SHIFT),
                       __lsx_vmin_b(__lsx_vslli_w(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                    __lsx_vslli_w(b, SK_A32_SHIFT - SK_B32_SHIFT)));
        __m128i aMax = __lsx_vmax_b(__lsx_vslli_w(r, SK_A32_SHIFT - SK_R32_SHIFT),
                       __lsx_vmax_b(__lsx_vslli_w(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                    __lsx_vslli_w(b, SK_A32_SHIFT - SK_B32_SHIFT)));
        // srcA has been biased to [0-256], so compare srcA against (dstA+1)
        __m128i a = __lsx_vmskltz_w(srcA -
                                    __lsx_vand_v(
                                          __lsx_vadd_w(dst,
                                                       __lsx_vreplgr2vr_w(1 << SK_A32_SHIFT)),
                                          __lsx_vreplgr2vr_w(SK_A32_MASK)));
        // a = if_then_else(a, aMin, aMax) == (aMin & a) | (aMax & ~a)
        a = __lsx_vor_v(__lsx_vand_v(a, aMin), __lsx_vandn_v(a, aMax));

        // Pack the 4 16bit mask pixels into 4 32bit pixels, (p0, p1, p2, p3)
        // Each component (m0R, m0G, etc.) is then a 5-bit value aligned to an
        // 8-bit position
        // mask = (m0A, m0R, m0G, m0B, m1A, m1R, m1G, m1B,
        //         m2A, m2R, m2G, m2B, m3A, m3R, m3G, m3B)
        mask = __lsx_vor_v(__lsx_vor_v(a, r), __lsx_vor_v(g, b));

        // Interleave R,G,B into the lower byte of word.
        // i.e. split the sixteen 8-bit values from mask into two sets of eight
        // 16-bit values, padded by zero.
        __m128i maskLo, maskHi;
        // maskLo = (m0A, 0, m0R, 0, m0G, 0, m0B, 0, m1A, 0, m1R, 0, m1G, 0, m1B, 0)
        maskLo = __lsx_vilvl_b(v_zero, mask);
        // maskHi = (m2A, 0, m2R, 0, m2G, 0, m2B, 0, m3A, 0, m3R, 0, m3G, 0, m3B, 0)
        maskHi = __lsx_vilvh_b(v_zero, mask);

        // Upscale from 0..31 to 0..32
        // (allows to replace division by left-shift further down)
        // Left-shift each component by 4 and add the result back to that component,
        // mapping numbers in the range 0..15 to 0..15, and 16..31 to 17..32
        maskLo = __lsx_vadd_h(maskLo, __lsx_vsrli_h(maskLo, 4));
        maskHi = __lsx_vadd_h(maskHi, __lsx_vsrli_h(maskHi, 4));

        // Multiply each component of maskLo and maskHi by srcA
        maskLo = __lsx_vmul_h(maskLo, srcA);
        maskHi = __lsx_vmul_h(maskHi, srcA);

        // Left shift mask components by 8 (divide by 256)
        maskLo = __lsx_vsrli_h(maskLo, 8);
        maskHi = __lsx_vsrli_h(maskHi, 8);

        // Interleave R,G,B into the lower byte of the word
        // dstLo = (d0A, 0, d0R, 0, d0G, 0, d0B, 0, d1A, 0, d1R, 0, d1G, 0, d1B, 0)
        __m128i dstLo = __lsx_vilvl_b(v_zero, dst);
        // dstLo = (d2A, 0, d2R, 0, d2G, 0, d2B, 0, d3A, 0, d3R, 0, d3G, 0, d3B, 0)
        __m128i dstHi = __lsx_vilvh_b(v_zero, dst);

        // mask = (src - dst) * mask
        maskLo = __lsx_vmul_h(maskLo, __lsx_vsub_h(src, dstLo));
        maskHi = __lsx_vmul_h(maskHi, __lsx_vsub_h(src, dstHi));

        // mask = (src - dst) * mask >> 5
        maskLo = __lsx_vsrai_h(maskLo, 5);
        maskHi = __lsx_vsrai_h(maskHi, 5);

        // Add two pixels into result.
        // result = dst + ((src - dst) * mask >> 5)
        __m128i resultLo = __lsx_vadd_h(dstLo, maskLo);
        __m128i resultHi = __lsx_vadd_h(dstHi, maskHi);

        // Pack into 4 32bit dst pixels.
        // resultLo and resultHi contain eight 16-bit components (two pixels) each.
        // Merge into one LSX regsiter with sixteen 8-bit values (four pixels),
        // clamping to 255 if necessary.
        __m128i tmpl = __lsx_vsat_hu(resultLo, 7);
        __m128i tmph = __lsx_vsat_hu(resultHi, 7);
        return __lsx_vpickev_b(tmph, tmpl);
    }

    static __m128i blend_lcd16_opaque_lsx(__m128i &src, __m128i &dst, __m128i &mask) {
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

        __m128i v_zero = __lsx_vldi(0);

        // Get the R,G,B of each 16bit mask pixel, we want all of them in 5 bits.
        // r = (0, m0R, 0, 0, 0, m1R, 0, 0, 0, m2R, 0, 0, 0, m3R, 0, 0)
        __m128i r = __lsx_vand_v(SkPackedR16x5ToUnmaskedR32x5_LSX(mask),
                                 __lsx_vreplgr2vr_w(0x1F << SK_R32_SHIFT));

        // g = (0, 0, m0G, 0, 0, 0, m1G, 0, 0, 0, m2G, 0, 0, 0, m3G, 0)
        __m128i g = __lsx_vand_v(SkPackedG16x5ToUnmaskedG32x5_LSX(mask),
                                 __lsx_vreplgr2vr_w(0x1F << SK_G32_SHIFT));

        // b = (0, 0, 0, m0B, 0, 0, 0, m1B, 0, 0, 0, m2B, 0, 0, 0, m3B)
        __m128i b = __lsx_vand_v(SkPackedB16x5ToUnmaskedB32x5_LSX(mask),
                                 __lsx_vreplgr2vr_w(0x1F << SK_B32_SHIFT));

        // a = max(r, g, b) since opaque src alpha uses max of LCD coverages
        __m128i a = __lsx_vmax_b(__lsx_vslli_w(r, SK_A32_SHIFT - SK_R32_SHIFT),
                    __lsx_vmax_b(__lsx_vslli_w(g, SK_A32_SHIFT - SK_G32_SHIFT),
                                 __lsx_vslli_w(b, SK_A32_SHIFT - SK_B32_SHIFT)));

        // Pack the 4 16bit mask pixels into 4 32bit pixels, (p0, p1, p2, p3)
        // Each component (m0R, m0G, etc.) is then a 5-bit value aligned to an
        // 8-bit position
        // mask = (m0A, m0R, m0G, m0B, m1A, m1R, m1G, m1B,
        //         m2A, m2R, m2G, m2B, m3A, m3R, m3G, m3B)
        mask = __lsx_vor_v(__lsx_vor_v(a, r), __lsx_vor_v(g, b));

        // Interleave R,G,B into the lower byte of word.
        // i.e. split the sixteen 8-bit values from mask into two sets of eight
        // 16-bit values, padded by zero.
        __m128i maskLo, maskHi;
        // maskLo = (m0A, 0, m0R, 0, m0G, 0, m0B, 0, m1A, 0, m1R, 0, m1G, 0, m1B, 0)
        maskLo = __lsx_vilvl_b(v_zero, mask);
        // maskHi = (m2A, 0, m2R, 0, m2G, 0, m2B, 0, m3A, 0, m3R, 0, m3G, 0, m3B, 0)
        maskHi = __lsx_vilvh_b(v_zero, mask);

        // Upscale from 0..31 to 0..32
        // (allows to replace division by left-shift further down)
        // Left-shift each component by 4 and add the result back to that component,
        // mapping numbers in the range 0..15 to 0..15, and 16..31 to 17..32
        maskLo = __lsx_vadd_h(maskLo, __lsx_vsrli_h(maskLo, 4));
        maskHi = __lsx_vadd_h(maskHi, __lsx_vsrli_h(maskHi, 4));

        // Interleave R,G,B into the lower byte of the word
        // dstLo = (d0A, 0, d0R, 0, d0G, 0, d0B, 0, d1A, 0, d1R, 0, d1G, 0, d1B, 0)
        __m128i dstLo = __lsx_vilvl_b(v_zero, dst);
        // dstLo = (d2A, 0, d2R, 0, d2G, 0, d2B, 0, d3A, 0, d3R, 0, d3G, 0, d3B, 0)
        __m128i dstHi = __lsx_vilvh_b(v_zero, dst);

        // mask = (src - dst) * mask
        maskLo = __lsx_vmul_h(maskLo, __lsx_vsub_h(src, dstLo));
        maskHi = __lsx_vmul_h(maskHi, __lsx_vsub_h(src, dstHi));

        // mask = (src - dst) * mask >> 5
        maskLo = __lsx_vsrai_h(maskLo, 5);
        maskHi = __lsx_vsrai_h(maskHi, 5);

        // Add two pixels into result.
        // result = dst + ((src - dst) * mask >> 5)
        __m128i resultLo = __lsx_vadd_h(dstLo, maskLo);
        __m128i resultHi = __lsx_vadd_h(dstHi, maskHi);

        // Merge into one LSX regsiter with sixteen 8-bit values (four pixels),
        // clamping to 255 if necessary.
        __m128i tmpl = __lsx_vsat_hu(resultLo, 7);
        __m128i tmph = __lsx_vsat_hu(resultHi, 7);
        return __lsx_vpickev_b(tmph, tmpl);
    }

    void blit_row_lcd16(SkPMColor dst[], const uint16_t mask[], SkColor src, int width, SkPMColor) {
        if (width <= 0) {
            return;
        }

        int srcA = SkColorGetA(src);
        int srcR = SkColorGetR(src);
        int srcG = SkColorGetG(src);
        int srcB = SkColorGetB(src);
        __m128i v_zero = __lsx_vldi(0);

        srcA = SkAlpha255To256(srcA);
        if (width >= 4) {
            SkASSERT(((size_t)dst & 0x03) == 0);
            while (((size_t)dst & 0x0F) != 0) {
                *dst = blend_lcd16(srcA, srcR, srcG, srcB, *dst, *mask);
                mask++;
                dst++;
                width--;
            }

            __m128i *d = reinterpret_cast<__m128i*>(dst);
            // Set alpha to 0xFF and replicate source eight times in LSX register.
            unsigned int skpackargb32 = SkPackARGB32(0xFF, srcR, srcG, srcB);
            __m128i src_lsx = __lsx_vreplgr2vr_w(skpackargb32);
            // Interleave with zeros to get two sets of eight 16-bit values.
            src_lsx = __lsx_vilvl_b(v_zero, src_lsx);
            // Set srcA_lsx to contain eight copies of srcA, padded with zero.
            // src_lsx=(0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
            __m128i srcA_lsx = __lsx_vreplgr2vr_h(srcA);

            while (width >= 4) {
                // Load eight destination pixels into dst_lsx.
                __m128i dst_lsx = __lsx_vld(d, 0);
                // Load four 16-bit masks into lower half of mask_lsx.
                __m128i mask_lsx = __lsx_vldrepl_d((void *)mask, 0);
                mask_lsx =  __lsx_vilvl_d(v_zero, mask_lsx);

                int pack_cmp = __lsx_bz_v(mask_lsx);
                // if mask pixels are not all zero, we will blend the dst pixels
                if (pack_cmp != 1) {
                    // Unpack 4 16bit mask pixels to
                    // mask_lsx = (m0RGBLo, m0RGBHi, 0, 0, m1RGBLo, m1RGBHi, 0, 0,
                    //             m2RGBLo, m2RGBHi, 0, 0, m3RGBLo, m3RGBHi, 0, 0)
                    mask_lsx = __lsx_vilvl_h(v_zero, mask_lsx);

                    // Process 8 32bit dst pixels
                    __m128i result = blend_lcd16_lsx(src_lsx, dst_lsx, mask_lsx, srcA_lsx);
                    __lsx_vst(result, d, 0);
                }

                d++;
                mask += 4;
                width -= 4;
            }

            dst = reinterpret_cast<SkPMColor*>(d);
        }

        while (width > 0) {
            *dst = blend_lcd16(srcA, srcR, srcG, srcB, *dst, *mask);
            mask++;
            dst++;
            width--;
        }
    }

    void blit_row_lcd16_opaque(SkPMColor dst[], const uint16_t mask[],
                               SkColor src, int width, SkPMColor opaqueDst) {
        if (width <= 0) {
            return;
        }

        int srcR = SkColorGetR(src);
        int srcG = SkColorGetG(src);
        int srcB = SkColorGetB(src);
        __m128i v_zero = __lsx_vldi(0);

        if (width >= 4) {
            SkASSERT(((size_t)dst & 0x03) == 0);
            while (((size_t)dst & 0x0F) != 0) {
                *dst = blend_lcd16_opaque(srcR, srcG, srcB, *dst, *mask, opaqueDst);
                mask++;
                dst++;
                width--;
            }

            __m128i *d = reinterpret_cast<__m128i*>(dst);
            // Set alpha to 0xFF and replicate source four times in LSX register.
            unsigned int sk_pack_argb32 = SkPackARGB32(0xFF, srcR, srcG, srcB);
            __m128i src_lsx = __lsx_vreplgr2vr_w(sk_pack_argb32);
            // Set srcA_lsx to contain eight copies of srcA, padded with zero.
            // src_lsx=(0xFF, 0, sR, 0, sG, 0, sB, 0, 0xFF, 0, sR, 0, sG, 0, sB, 0)
            src_lsx = __lsx_vilvl_b(v_zero, src_lsx);

            while (width >= 4) {
                // Load four destination pixels into dst_lsx.
                __m128i dst_lsx = __lsx_vld(d, 0);
                // Load four 16-bit masks into lower half of mask_lsx.
                __m128i mask_lsx = __lsx_vldrepl_d((void *)(mask), 0);
                mask_lsx =  __lsx_vilvl_d(v_zero, mask_lsx);

                int pack_cmp = __lsx_bz_v(mask_lsx);
                // if mask pixels are not all zero, we will blend the dst pixels
                if (pack_cmp != 1) {
                    // Unpack 4 16bit mask pixels to
                    mask_lsx = __lsx_vilvl_h(v_zero, mask_lsx);

                    // Process 8 32bit dst pixels
                    __m128i result = blend_lcd16_opaque_lsx(src_lsx, dst_lsx, mask_lsx);
                    __lsx_vst(result, d, 0);
                }
                d++;
                mask += 4;
                width -= 4;
            }

            dst = reinterpret_cast<SkPMColor*>(d);
        }

        while (width > 0) {
            *dst = blend_lcd16_opaque(srcR, srcG, srcB, *dst, *mask, opaqueDst);
            mask++;
            dst++;
            width--;
        }
    }

#else

    static inline void blit_row_lcd16(SkPMColor dst[], const uint16_t mask[],
                                      SkColor src, int width, SkPMColor) {
        int srcA = SkColorGetA(src);
        int srcR = SkColorGetR(src);
        int srcG = SkColorGetG(src);
        int srcB = SkColorGetB(src);

        srcA = SkAlpha255To256(srcA);

        for (int i = 0; i < width; i++) {
            dst[i] = blend_lcd16(srcA, srcR, srcG, srcB, dst[i], mask[i]);
        }
    }

    static inline void blit_row_lcd16_opaque(SkPMColor dst[], const uint16_t mask[],
                                             SkColor src, int width,
                                             SkPMColor opaqueDst) {
        int srcR = SkColorGetR(src);
        int srcG = SkColorGetG(src);
        int srcB = SkColorGetB(src);

        for (int i = 0; i < width; i++) {
            dst[i] = blend_lcd16_opaque(srcR, srcG, srcB, dst[i], mask[i], opaqueDst);
        }
    }

#endif

static bool blit_color(const SkPixmap& device,
                       const SkMask& mask,
                       const SkIRect& clip,
                       SkColor color) {
    int x = clip.fLeft,
        y = clip.fTop;

    if (device.colorType() == kN32_SkColorType && mask.fFormat == SkMask::kA8_Format) {
        SkOpts::blit_mask_d32_a8(device.writable_addr32(x,y), device.rowBytes(),
                                 (const SkAlpha*)mask.getAddr(x,y), mask.fRowBytes,
                                 color, clip.width(), clip.height());
        return true;
    }

    if (device.colorType() == kN32_SkColorType && mask.fFormat == SkMask::kLCD16_Format) {
        auto dstRow  = device.writable_addr32(x,y);
        auto maskRow = (const uint16_t*)mask.getAddr(x,y);

        auto blit_row = blit_row_lcd16;
        SkPMColor opaqueDst = 0;  // ignored unless opaque

        if (0xff == SkColorGetA(color)) {
            blit_row  = blit_row_lcd16_opaque;
            opaqueDst = SkPreMultiplyColor(color);
        }

        for (int height = clip.height(); height --> 0; ) {
            blit_row(dstRow, maskRow, color, clip.width(), opaqueDst);

            dstRow  = (SkPMColor*)     ((      char*) dstRow + device.rowBytes());
            maskRow = (const uint16_t*)((const char*)maskRow +  mask.fRowBytes);
        }
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

static void SkARGB32_Blit32(const SkPixmap& device, const SkMask& mask,
                            const SkIRect& clip, SkPMColor srcColor) {
    U8CPU alpha = SkGetPackedA32(srcColor);
    unsigned flags = SkBlitRow::kSrcPixelAlpha_Flag32;
    if (alpha != 255) {
        flags |= SkBlitRow::kGlobalAlpha_Flag32;
    }
    SkBlitRow::Proc32 proc = SkBlitRow::Factory32(flags);

    int x = clip.fLeft;
    int y = clip.fTop;
    int width = clip.width();
    int height = clip.height();

    SkPMColor* dstRow = device.writable_addr32(x, y);
    const SkPMColor* srcRow = reinterpret_cast<const SkPMColor*>(mask.getAddr8(x, y));

    do {
        proc(dstRow, srcRow, width, alpha);
        dstRow = (SkPMColor*)((char*)dstRow + device.rowBytes());
        srcRow = (const SkPMColor*)((const char*)srcRow + mask.fRowBytes);
    } while (--height != 0);
}

//////////////////////////////////////////////////////////////////////////////////////

SkARGB32_Blitter::SkARGB32_Blitter(const SkPixmap& device, const SkPaint& paint)
        : INHERITED(device) {
    SkColor color = paint.getColor();
    fColor = color;

    fSrcA = SkColorGetA(color);
    unsigned scale = SkAlpha255To256(fSrcA);
    fSrcR = SkAlphaMul(SkColorGetR(color), scale);
    fSrcG = SkAlphaMul(SkColorGetG(color), scale);
    fSrcB = SkAlphaMul(SkColorGetB(color), scale);

    fPMColor = SkPackARGB32(fSrcA, fSrcR, fSrcG, fSrcB);
}

#if defined _WIN32  // disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

void SkARGB32_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t* device = fDevice.writable_addr32(x, y);
    SkBlitRow::Color32(device, width, fPMColor);
}

void SkARGB32_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                 const int16_t runs[]) {
    if (fSrcA == 0) {
        return;
    }

    uint32_t    color = fPMColor;
    uint32_t*   device = fDevice.writable_addr32(x, y);
    unsigned    opaqueMask = fSrcA; // if fSrcA is 0xFF, then we will catch the fast opaque case

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        unsigned aa = antialias[0];
        if (aa) {
            if ((opaqueMask & aa) == 255) {
                SkOpts::memset32(device, color, count);
            } else {
                uint32_t sc = SkAlphaMulQ(color, SkAlpha255To256(aa));
                SkBlitRow::Color32(device, count, sc);
            }
        }
        runs += count;
        antialias += count;
        device += count;
    }
}

void SkARGB32_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = SkBlendARGB32(fPMColor, device[0], a0);
    device[1] = SkBlendARGB32(fPMColor, device[1], a1);
}

void SkARGB32_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = SkBlendARGB32(fPMColor, device[0], a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = SkBlendARGB32(fPMColor, device[0], a1);
}

//////////////////////////////////////////////////////////////////////////////////////

#define solid_8_pixels(mask, dst, color)    \
    do {                                    \
        if (mask & 0x80) dst[0] = color;    \
        if (mask & 0x40) dst[1] = color;    \
        if (mask & 0x20) dst[2] = color;    \
        if (mask & 0x10) dst[3] = color;    \
        if (mask & 0x08) dst[4] = color;    \
        if (mask & 0x04) dst[5] = color;    \
        if (mask & 0x02) dst[6] = color;    \
        if (mask & 0x01) dst[7] = color;    \
    } while (0)

#define SK_BLITBWMASK_NAME                  SkARGB32_BlitBW
#define SK_BLITBWMASK_ARGS                  , SkPMColor color
#define SK_BLITBWMASK_BLIT8(mask, dst)      solid_8_pixels(mask, dst, color)
#define SK_BLITBWMASK_GETADDR               writable_addr32
#define SK_BLITBWMASK_DEVTYPE               uint32_t
#include "src/core/SkBlitBWMaskTemplate.h"

#define blend_8_pixels(mask, dst, sc, dst_scale)                            \
    do {                                                                    \
        if (mask & 0x80) { dst[0] = sc + SkAlphaMulQ(dst[0], dst_scale); }  \
        if (mask & 0x40) { dst[1] = sc + SkAlphaMulQ(dst[1], dst_scale); }  \
        if (mask & 0x20) { dst[2] = sc + SkAlphaMulQ(dst[2], dst_scale); }  \
        if (mask & 0x10) { dst[3] = sc + SkAlphaMulQ(dst[3], dst_scale); }  \
        if (mask & 0x08) { dst[4] = sc + SkAlphaMulQ(dst[4], dst_scale); }  \
        if (mask & 0x04) { dst[5] = sc + SkAlphaMulQ(dst[5], dst_scale); }  \
        if (mask & 0x02) { dst[6] = sc + SkAlphaMulQ(dst[6], dst_scale); }  \
        if (mask & 0x01) { dst[7] = sc + SkAlphaMulQ(dst[7], dst_scale); }  \
    } while (0)

#define SK_BLITBWMASK_NAME                  SkARGB32_BlendBW
#define SK_BLITBWMASK_ARGS                  , uint32_t sc, unsigned dst_scale
#define SK_BLITBWMASK_BLIT8(mask, dst)      blend_8_pixels(mask, dst, sc, dst_scale)
#define SK_BLITBWMASK_GETADDR               writable_addr32
#define SK_BLITBWMASK_DEVTYPE               uint32_t
#include "src/core/SkBlitBWMaskTemplate.h"

void SkARGB32_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));
    SkASSERT(fSrcA != 0xFF);

    if (fSrcA == 0) {
        return;
    }

    if (blit_color(fDevice, mask, clip, fColor)) {
        return;
    }

    switch (mask.fFormat) {
        case SkMask::kBW_Format:
            SkARGB32_BlendBW(fDevice, mask, clip, fPMColor, SkAlpha255To256(255 - fSrcA));
            break;
        case SkMask::kARGB32_Format:
            SkARGB32_Blit32(fDevice, mask, clip, fPMColor);
            break;
        default:
            SK_ABORT("Mask format not handled.");
    }
}

void SkARGB32_Opaque_Blitter::blitMask(const SkMask& mask,
                                       const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    if (blit_color(fDevice, mask, clip, fColor)) {
        return;
    }

    switch (mask.fFormat) {
        case SkMask::kBW_Format:
            SkARGB32_BlitBW(fDevice, mask, clip, fPMColor);
            break;
        case SkMask::kARGB32_Format:
            SkARGB32_Blit32(fDevice, mask, clip, fPMColor);
            break;
        default:
            SK_ABORT("Mask format not handled.");
    }
}

void SkARGB32_Opaque_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = SkFastFourByteInterp(fPMColor, device[0], a0);
    device[1] = SkFastFourByteInterp(fPMColor, device[1], a1);
}

void SkARGB32_Opaque_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = SkFastFourByteInterp(fPMColor, device[0], a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = SkFastFourByteInterp(fPMColor, device[0], a1);
}

///////////////////////////////////////////////////////////////////////////////

void SkARGB32_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (alpha == 0 || fSrcA == 0) {
        return;
    }

    uint32_t* device = fDevice.writable_addr32(x, y);
    uint32_t  color = fPMColor;

    if (alpha != 255) {
        color = SkAlphaMulQ(color, SkAlpha255To256(alpha));
    }

    unsigned dst_scale = SkAlpha255To256(255 - SkGetPackedA32(color));
    size_t rowBytes = fDevice.rowBytes();
    while (--height >= 0) {
        device[0] = color + SkAlphaMulQ(device[0], dst_scale);
        device = (uint32_t*)((char*)device + rowBytes);
    }
}

void SkARGB32_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width() && y + height <= fDevice.height());

    if (fSrcA == 0) {
        return;
    }

    uint32_t*   device = fDevice.writable_addr32(x, y);
    uint32_t    color = fPMColor;
    size_t      rowBytes = fDevice.rowBytes();

    if (SkGetPackedA32(fPMColor) == 0xFF) {
        SkOpts::rect_memset32(device, color, width, rowBytes, height);
    } else {
        while (height --> 0) {
            SkBlitRow::Color32(device, width, color);
            device = (uint32_t*)((char*)device + rowBytes);
        }
    }
}

#if defined _WIN32
#pragma warning ( pop )
#endif

///////////////////////////////////////////////////////////////////////

void SkARGB32_Black_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                       const int16_t runs[]) {
    uint32_t*   device = fDevice.writable_addr32(x, y);
    SkPMColor   black = (SkPMColor)(SK_A32_MASK << SK_A32_SHIFT);

    for (;;) {
        int count = runs[0];
        SkASSERT(count >= 0);
        if (count <= 0) {
            return;
        }
        unsigned aa = antialias[0];
        if (aa) {
            if (aa == 255) {
                SkOpts::memset32(device, black, count);
            } else {
                SkPMColor src = aa << SK_A32_SHIFT;
                unsigned dst_scale = 256 - aa;
                int n = count;
                do {
                    --n;
                    device[n] = src + SkAlphaMulQ(device[n], dst_scale);
                } while (n > 0);
            }
        }
        runs += count;
        antialias += count;
        device += count;
    }
}

void SkARGB32_Black_Blitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x + 1, y);)

    device[0] = (a0 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a0);
    device[1] = (a1 << SK_A32_SHIFT) + SkAlphaMulQ(device[1], 256 - a1);
}

void SkARGB32_Black_Blitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    uint32_t* device = fDevice.writable_addr32(x, y);
    SkDEBUGCODE((void)fDevice.writable_addr32(x, y + 1);)

    device[0] = (a0 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a0);
    device = (uint32_t*)((char*)device + fDevice.rowBytes());
    device[0] = (a1 << SK_A32_SHIFT) + SkAlphaMulQ(device[0], 256 - a1);
}

///////////////////////////////////////////////////////////////////////////////

SkARGB32_Shader_Blitter::SkARGB32_Shader_Blitter(const SkPixmap& device,
        const SkPaint& paint, SkShaderBase::Context* shaderContext)
    : INHERITED(device, paint, shaderContext)
{
    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * (sizeof(SkPMColor)));

    SkASSERT(paint.isSrcOver());

    int flags = 0;
    if (!(shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag)) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag32;
    }
    // we call this on the output from the shader
    fProc32 = SkBlitRow::Factory32(flags);
    // we call this on the output from the shader + alpha from the aa buffer
    fProc32Blend = SkBlitRow::Factory32(flags | SkBlitRow::kGlobalAlpha_Flag32);

    fShadeDirectlyIntoDevice =
            SkToBool(shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag);
}

SkARGB32_Shader_Blitter::~SkARGB32_Shader_Blitter() {
    sk_free(fBuffer);
}

void SkARGB32_Shader_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t* device = fDevice.writable_addr32(x, y);

    if (fShadeDirectlyIntoDevice) {
        fShaderContext->shadeSpan(x, y, device, width);
    } else {
        SkPMColor*  span = fBuffer;
        fShaderContext->shadeSpan(x, y, span, width);
        fProc32(device, span, width, 255);
    }
}

void SkARGB32_Shader_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 &&
             x + width <= fDevice.width() && y + height <= fDevice.height());

    uint32_t*  device = fDevice.writable_addr32(x, y);
    size_t     deviceRB = fDevice.rowBytes();
    auto*      shaderContext = fShaderContext;
    SkPMColor* span = fBuffer;

    if (fShadeDirectlyIntoDevice) {
        do {
            shaderContext->shadeSpan(x, y, device, width);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    } else {
        SkBlitRow::Proc32 proc = fProc32;
        do {
            shaderContext->shadeSpan(x, y, span, width);
            proc(device, span, width, 255);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    }
}

void SkARGB32_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                        const int16_t runs[]) {
    SkPMColor* span = fBuffer;
    uint32_t*  device = fDevice.writable_addr32(x, y);
    auto*      shaderContext = fShaderContext;

    if (fShadeDirectlyIntoDevice || (shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag)) {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                if (aa == 255) {
                    // cool, have the shader draw right into the device
                    shaderContext->shadeSpan(x, y, device, count);
                } else {
                    shaderContext->shadeSpan(x, y, span, count);
                    fProc32Blend(device, span, count, aa);
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    } else {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                break;
            }
            int aa = *antialias;
            if (aa) {
                shaderContext->shadeSpan(x, y, span, count);
                if (aa == 255) {
                    fProc32(device, span, count, 255);
                } else {
                    fProc32Blend(device, span, count, aa);
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    }
}

using U32  = skvx::Vec< 4, uint32_t>;
using U8x4 = skvx::Vec<16, uint8_t>;
using U8   = skvx::Vec< 4, uint8_t>;

static void drive(SkPMColor* dst, const SkPMColor* src, const uint8_t* cov, int n,
                  U8x4 (*kernel)(U8x4,U8x4,U8x4)) {

    auto apply = [kernel](U32 dst, U32 src, U8 cov) -> U32 {
        U8x4 cov_splat = skvx::shuffle<0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3>(cov);
        return sk_bit_cast<U32>(kernel(sk_bit_cast<U8x4>(dst),
                                       sk_bit_cast<U8x4>(src),
                                       cov_splat));
    };
    while (n >= 4) {
        apply(U32::Load(dst), U32::Load(src), U8::Load(cov)).store(dst);
        dst += 4;
        src += 4;
        cov += 4;
        n   -= 4;
    }
    while (n --> 0) {
        *dst = apply(U32{*dst}, U32{*src}, U8{*cov})[0];
        dst++;
        src++;
        cov++;
    }
}

static void blend_row_A8(SkPMColor* dst, const void* mask, const SkPMColor* src, int n) {
    auto cov = (const uint8_t*)mask;
    drive(dst, src, cov, n, [](U8x4 d, U8x4 s, U8x4 c) {
        U8x4 s_aa  = skvx::approx_scale(s, c),
             alpha = skvx::shuffle<3,3,3,3, 7,7,7,7, 11,11,11,11, 15,15,15,15>(s_aa);
        return s_aa + skvx::approx_scale(d, 255 - alpha);
    });
}

static void blend_row_A8_opaque(SkPMColor* dst, const void* mask, const SkPMColor* src, int n) {
    auto cov = (const uint8_t*)mask;
    drive(dst, src, cov, n, [](U8x4 d, U8x4 s, U8x4 c) {
        return skvx::div255( skvx::cast<uint16_t>(s) * skvx::cast<uint16_t>(  c  )
                           + skvx::cast<uint16_t>(d) * skvx::cast<uint16_t>(255-c));
    });
}

static void blend_row_lcd16(SkPMColor* dst, const void* vmask, const SkPMColor* src, int n) {
    auto src_alpha_blend = [](int s, int d, int sa, int m) {
        return d + SkAlphaMul(s - SkAlphaMul(sa, d), m);
    };

    auto upscale_31_to_255 = [](int v) {
        return (v << 3) | (v >> 2);
    };

    auto mask = (const uint16_t*)vmask;
    for (int i = 0; i < n; ++i) {
        uint16_t m = mask[i];
        if (0 == m) {
            continue;
        }

        SkPMColor s = src[i];
        SkPMColor d = dst[i];

        int srcA = SkGetPackedA32(s);
        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        srcA += srcA >> 7;

        // We're ignoring the least significant bit of the green coverage channel here.
        int maskR = SkGetPackedR16(m) >> (SK_R16_BITS - 5);
        int maskG = SkGetPackedG16(m) >> (SK_G16_BITS - 5);
        int maskB = SkGetPackedB16(m) >> (SK_B16_BITS - 5);

        // Scale up to 8-bit coverage to work with SkAlphaMul() in src_alpha_blend().
        maskR = upscale_31_to_255(maskR);
        maskG = upscale_31_to_255(maskG);
        maskB = upscale_31_to_255(maskB);

        // This LCD blit routine only works if the destination is opaque.
        dst[i] = SkPackARGB32(0xFF,
                              src_alpha_blend(srcR, SkGetPackedR32(d), srcA, maskR),
                              src_alpha_blend(srcG, SkGetPackedG32(d), srcA, maskG),
                              src_alpha_blend(srcB, SkGetPackedB32(d), srcA, maskB));
    }
}

static void blend_row_LCD16_opaque(SkPMColor* dst, const void* vmask, const SkPMColor* src, int n) {
    auto mask = (const uint16_t*)vmask;

    for (int i = 0; i < n; ++i) {
        uint16_t m = mask[i];
        if (0 == m) {
            continue;
        }

        SkPMColor s = src[i];
        SkPMColor d = dst[i];

        int srcR = SkGetPackedR32(s);
        int srcG = SkGetPackedG32(s);
        int srcB = SkGetPackedB32(s);

        // We're ignoring the least significant bit of the green coverage channel here.
        int maskR = SkGetPackedR16(m) >> (SK_R16_BITS - 5);
        int maskG = SkGetPackedG16(m) >> (SK_G16_BITS - 5);
        int maskB = SkGetPackedB16(m) >> (SK_B16_BITS - 5);

        // Now upscale them to 0..32, so we can use blend_32.
        maskR = upscale_31_to_32(maskR);
        maskG = upscale_31_to_32(maskG);
        maskB = upscale_31_to_32(maskB);

        // This LCD blit routine only works if the destination is opaque.
        dst[i] = SkPackARGB32(0xFF,
                              blend_32(srcR, SkGetPackedR32(d), maskR),
                              blend_32(srcG, SkGetPackedG32(d), maskG),
                              blend_32(srcB, SkGetPackedB32(d), maskB));
    }
}

void SkARGB32_Shader_Blitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    void (*blend_row)(SkPMColor*, const void* mask, const SkPMColor*, int) = nullptr;

    bool opaque = (fShaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag);

    if (mask.fFormat == SkMask::kA8_Format && opaque) {
        blend_row = blend_row_A8_opaque;
    } else if (mask.fFormat == SkMask::kA8_Format) {
        blend_row = blend_row_A8;
    } else if (mask.fFormat == SkMask::kLCD16_Format && opaque) {
        blend_row = blend_row_LCD16_opaque;
    } else if (mask.fFormat == SkMask::kLCD16_Format) {
        blend_row = blend_row_lcd16;
    } else {
        this->INHERITED::blitMask(mask, clip);
        return;
    }

    const int x = clip.fLeft;
    const int width = clip.width();
    int y = clip.fTop;
    int height = clip.height();

    char* dstRow = (char*)fDevice.writable_addr32(x, y);
    const size_t dstRB = fDevice.rowBytes();
    const uint8_t* maskRow = (const uint8_t*)mask.getAddr(x, y);
    const size_t maskRB = mask.fRowBytes;

    SkPMColor* span = fBuffer;
    SkASSERT(blend_row);
    do {
        fShaderContext->shadeSpan(x, y, span, width);
        blend_row(reinterpret_cast<SkPMColor*>(dstRow), maskRow, span, width);
        dstRow += dstRB;
        maskRow += maskRB;
        y += 1;
    } while (--height > 0);
}

void SkARGB32_Shader_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());

    uint32_t* device = fDevice.writable_addr32(x, y);
    size_t    deviceRB = fDevice.rowBytes();

    if (fShadeDirectlyIntoDevice) {
        if (255 == alpha) {
            do {
                fShaderContext->shadeSpan(x, y, device, 1);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        } else {
            do {
                SkPMColor c;
                fShaderContext->shadeSpan(x, y, &c, 1);
                *device = SkFourByteInterp(c, *device, alpha);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        }
    } else {
        SkPMColor* span = fBuffer;
        SkBlitRow::Proc32 proc = (255 == alpha) ? fProc32 : fProc32Blend;
        do {
            fShaderContext->shadeSpan(x, y, span, 1);
            proc(device, span, 1, alpha);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    }
}
