/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkShader.h"
#include "include/private/SkColorData.h"
#include "include/private/SkVx.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkUtils.h"
#include "src/core/SkXfermodePriv.h"

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

    int dstR = SkGetPackedR32(dst);
    int dstG = SkGetPackedG32(dst);
    int dstB = SkGetPackedB32(dst);

    // LCD blitting is only supported if the dst is known/required
    // to be opaque
    return SkPackARGB32(0xFF,
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

    int dstR = SkGetPackedR32(dst);
    int dstG = SkGetPackedG32(dst);
    int dstB = SkGetPackedB32(dst);

    // LCD blitting is only supported if the dst is known/required
    // to be opaque
    return SkPackARGB32(0xFF,
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

        uint8x8_t vcolR = vdup_n_u8(colR);
        uint8x8_t vcolG = vdup_n_u8(colG);
        uint8x8_t vcolB = vdup_n_u8(colB);
        uint8x8_t vopqDstA = vdup_n_u8(SkGetPackedA32(opaqueDst));
        uint8x8_t vopqDstR = vdup_n_u8(SkGetPackedR32(opaqueDst));
        uint8x8_t vopqDstG = vdup_n_u8(SkGetPackedG32(opaqueDst));
        uint8x8_t vopqDstB = vdup_n_u8(SkGetPackedB32(opaqueDst));

        while (width >= 8) {
            uint8x8x4_t vdst;
            uint16x8_t vmask;
            uint16x8_t vmaskR, vmaskG, vmaskB;
            uint8x8_t vsel_trans, vsel_opq;

            vdst = vld4_u8((uint8_t*)dst);
            vmask = vld1q_u16(src);

            // Prepare compare masks
            vsel_trans = vmovn_u16(vceqq_u16(vmask, vdupq_n_u16(0)));
            vsel_opq = vmovn_u16(vceqq_u16(vmask, vdupq_n_u16(0xFFFF)));

            // Get all the color masks on 5 bits
            vmaskR = vshrq_n_u16(vmask, SK_R16_SHIFT);
            vmaskG = vshrq_n_u16(vshlq_n_u16(vmask, SK_R16_BITS),
                                 SK_B16_BITS + SK_R16_BITS + 1);
            vmaskB = vmask & vdupq_n_u16(SK_B16_MASK);

            // Upscale to 0..32
            vmaskR = vmaskR + vshrq_n_u16(vmaskR, 4);
            vmaskG = vmaskG + vshrq_n_u16(vmaskG, 4);
            vmaskB = vmaskB + vshrq_n_u16(vmaskB, 4);

            vdst.val[NEON_A] = vbsl_u8(vsel_trans, vdst.val[NEON_A], vdup_n_u8(0xFF));
            vdst.val[NEON_A] = vbsl_u8(vsel_opq, vopqDstA, vdst.val[NEON_A]);

            vdst.val[NEON_R] = blend_32_neon(vcolR, vdst.val[NEON_R], vmaskR);
            vdst.val[NEON_G] = blend_32_neon(vcolG, vdst.val[NEON_G], vmaskG);
            vdst.val[NEON_B] = blend_32_neon(vcolB, vdst.val[NEON_B], vmaskB);

            vdst.val[NEON_R] = vbsl_u8(vsel_opq, vopqDstR, vdst.val[NEON_R]);
            vdst.val[NEON_G] = vbsl_u8(vsel_opq, vopqDstG, vdst.val[NEON_G]);
            vdst.val[NEON_B] = vbsl_u8(vsel_opq, vopqDstB, vdst.val[NEON_B]);

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

        colA = SkAlpha255To256(colA);

        uint16x8_t vcolA = vdupq_n_u16(colA);
        uint8x8_t vcolR = vdup_n_u8(colR);
        uint8x8_t vcolG = vdup_n_u8(colG);
        uint8x8_t vcolB = vdup_n_u8(colB);

        while (width >= 8) {
            uint8x8x4_t vdst;
            uint16x8_t vmask;
            uint16x8_t vmaskR, vmaskG, vmaskB;

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

            vdst.val[NEON_A] = vdup_n_u8(0xFF);
            vdst.val[NEON_R] = blend_32_neon(vcolR, vdst.val[NEON_R], vmaskR);
            vdst.val[NEON_G] = blend_32_neon(vcolG, vdst.val[NEON_G], vmaskG);
            vdst.val[NEON_B] = blend_32_neon(vcolB, vdst.val[NEON_B], vmaskB);

            vst4_u8((uint8_t*)dst, vdst);

            dst += 8;
            src += 8;
            width -= 8;
        }

        for (int i = 0; i < width; i++) {
            dst[i] = blend_lcd16(colA, colR, colG, colB, dst[i], src[i]);
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

const SkPixmap* SkARGB32_Blitter::justAnOpaqueColor(uint32_t* value) {
    if (255 == fSrcA) {
        *value = fPMColor;
        return &fDevice;
    }
    return nullptr;
}

#if defined _WIN32  // disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

void SkARGB32_Blitter::blitH(int x, int y, int width) {
    SkASSERT(x >= 0 && y >= 0 && x + width <= fDevice.width());

    uint32_t* device = fDevice.writable_addr32(x, y);
    SkBlitRow::Color32(device, device, width, fPMColor);
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
                sk_memset32(device, color, count);
            } else {
                uint32_t sc = SkAlphaMulQ(color, SkAlpha255To256(aa));
                SkBlitRow::Color32(device, device, count, sc);
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
            SkBlitRow::Color32(device, device, width, color);
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
                sk_memset32(device, black, count);
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

// Special version of SkBlitRow::Factory32 that knows we're in kSrc_Mode,
// instead of kSrcOver_Mode
static void blend_srcmode(SkPMColor* SK_RESTRICT device,
                          const SkPMColor* SK_RESTRICT span,
                          int count, U8CPU aa) {
    int aa256 = SkAlpha255To256(aa);
    for (int i = 0; i < count; ++i) {
        device[i] = SkFourByteInterp256(span[i], device[i], aa256);
    }
}

SkARGB32_Shader_Blitter::SkARGB32_Shader_Blitter(const SkPixmap& device,
        const SkPaint& paint, SkShaderBase::Context* shaderContext)
    : INHERITED(device, paint, shaderContext)
{
    fBuffer = (SkPMColor*)sk_malloc_throw(device.width() * (sizeof(SkPMColor)));

    fXfermode = SkXfermode::Peek(paint.getBlendMode());

    int flags = 0;
    if (!(shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag)) {
        flags |= SkBlitRow::kSrcPixelAlpha_Flag32;
    }
    // we call this on the output from the shader
    fProc32 = SkBlitRow::Factory32(flags);
    // we call this on the output from the shader + alpha from the aa buffer
    fProc32Blend = SkBlitRow::Factory32(flags | SkBlitRow::kGlobalAlpha_Flag32);

    fShadeDirectlyIntoDevice = false;
    if (fXfermode == nullptr) {
        if (shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag) {
            fShadeDirectlyIntoDevice = true;
        }
    } else {
        if (SkBlendMode::kSrc == paint.getBlendMode()) {
            fShadeDirectlyIntoDevice = true;
            fProc32Blend = blend_srcmode;
        }
    }

    fConstInY = SkToBool(shaderContext->getFlags() & SkShaderBase::kConstInY32_Flag);
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
        if (fXfermode) {
            fXfermode->xfer32(device, span, width, nullptr);
        } else {
            fProc32(device, span, width, 255);
        }
    }
}

void SkARGB32_Shader_Blitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(x >= 0 && y >= 0 &&
             x + width <= fDevice.width() && y + height <= fDevice.height());

    uint32_t*  device = fDevice.writable_addr32(x, y);
    size_t     deviceRB = fDevice.rowBytes();
    auto*      shaderContext = fShaderContext;
    SkPMColor* span = fBuffer;

    if (fConstInY) {
        if (fShadeDirectlyIntoDevice) {
            // shade the first row directly into the device
            shaderContext->shadeSpan(x, y, device, width);
            span = device;
            while (--height > 0) {
                device = (uint32_t*)((char*)device + deviceRB);
                memcpy(device, span, width << 2);
            }
        } else {
            shaderContext->shadeSpan(x, y, span, width);
            SkXfermode* xfer = fXfermode;
            if (xfer) {
                do {
                    xfer->xfer32(device, span, width, nullptr);
                    y += 1;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            } else {
                SkBlitRow::Proc32 proc = fProc32;
                do {
                    proc(device, span, width, 255);
                    y += 1;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            }
        }
        return;
    }

    if (fShadeDirectlyIntoDevice) {
        do {
            shaderContext->shadeSpan(x, y, device, width);
            y += 1;
            device = (uint32_t*)((char*)device + deviceRB);
        } while (--height > 0);
    } else {
        SkXfermode* xfer = fXfermode;
        if (xfer) {
            do {
                shaderContext->shadeSpan(x, y, span, width);
                xfer->xfer32(device, span, width, nullptr);
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
}

void SkARGB32_Shader_Blitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                        const int16_t runs[]) {
    SkPMColor* span = fBuffer;
    uint32_t*  device = fDevice.writable_addr32(x, y);
    auto*      shaderContext = fShaderContext;

    if (fXfermode && !fShadeDirectlyIntoDevice) {
        for (;;) {
            SkXfermode* xfer = fXfermode;

            int count = *runs;
            if (count <= 0)
                break;
            int aa = *antialias;
            if (aa) {
                shaderContext->shadeSpan(x, y, span, count);
                if (aa == 255) {
                    xfer->xfer32(device, span, count, nullptr);
                } else {
                    // count is almost always 1
                    for (int i = count - 1; i >= 0; --i) {
                        xfer->xfer32(&device[i], &span[i], 1, antialias);
                    }
                }
            }
            device += count;
            runs += count;
            antialias += count;
            x += count;
        }
    } else if (fShadeDirectlyIntoDevice ||
               (shaderContext->getFlags() & SkShaderBase::kOpaqueAlpha_Flag)) {
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

#ifndef SK_SUPPORT_LEGACY_A8_MASKBLITTER
using U32  = skvx::Vec< 4, uint32_t>;
using U8x4 = skvx::Vec<16, uint8_t>;
using U8   = skvx::Vec< 4, uint8_t>;

static void drive(SkPMColor* dst, const SkPMColor* src, const uint8_t* cov, int n,
                  U8x4 (*kernel)(U8x4,U8x4,U8x4)) {

    auto apply = [kernel](U32 dst, U32 src, U8 cov) -> U32 {
        U8x4 cov_splat = skvx::shuffle<0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3>(cov);
        return skvx::bit_pun<U32>(kernel(skvx::bit_pun<U8x4>(dst),
                                         skvx::bit_pun<U8x4>(src),
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
#endif

static void blend_row_A8(SkPMColor* dst, const void* mask, const SkPMColor* src, int n) {
    auto cov = (const uint8_t*)mask;

#ifdef SK_SUPPORT_LEGACY_A8_MASKBLITTER
    for (int i = 0; i < n; ++i) {
        if (cov[i]) {
            dst[i] = SkBlendARGB32(src[i], dst[i], cov[i]);
        }
    }
#else
    drive(dst, src, cov, n, [](U8x4 d, U8x4 s, U8x4 c) {
        U8x4 s_aa  = skvx::approx_scale(s, c),
             alpha = skvx::shuffle<3,3,3,3, 7,7,7,7, 11,11,11,11, 15,15,15,15>(s_aa);
        return s_aa + skvx::approx_scale(d, 255 - alpha);
    });
#endif
}

static void blend_row_A8_opaque(SkPMColor* dst, const void* mask, const SkPMColor* src, int n) {
    auto cov = (const uint8_t*)mask;

#ifdef SK_SUPPORT_LEGACY_A8_MASKBLITTER
    for (int i = 0; i < n; ++i) {
        if (int c = cov[i]) {
            c += (c >> 7);
            dst[i] = SkAlphaMulQ(src[i], c) + SkAlphaMulQ(dst[i], 256 - c);
        }
    }
#else
    drive(dst, src, cov, n, [](U8x4 d, U8x4 s, U8x4 c) {
        return skvx::div255( skvx::cast<uint16_t>(s) * skvx::cast<uint16_t>(  c  )
                           + skvx::cast<uint16_t>(d) * skvx::cast<uint16_t>(255-c));
    });
#endif
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
    // we only handle kA8 with an xfermode
    if (fXfermode && (SkMask::kA8_Format != mask.fFormat)) {
        this->INHERITED::blitMask(mask, clip);
        return;
    }

    SkASSERT(mask.fBounds.contains(clip));

    void (*blend_row)(SkPMColor*, const void* mask, const SkPMColor*, int) = nullptr;

    if (!fXfermode) {
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

    if (fXfermode) {
        SkASSERT(SkMask::kA8_Format == mask.fFormat);
        SkXfermode* xfer = fXfermode;
        do {
            fShaderContext->shadeSpan(x, y, span, width);
            xfer->xfer32(reinterpret_cast<SkPMColor*>(dstRow), span, width, maskRow);
            dstRow += dstRB;
            maskRow += maskRB;
            y += 1;
        } while (--height > 0);
    } else {
        SkASSERT(blend_row);
        do {
            fShaderContext->shadeSpan(x, y, span, width);
            blend_row(reinterpret_cast<SkPMColor*>(dstRow), maskRow, span, width);
            dstRow += dstRB;
            maskRow += maskRB;
            y += 1;
        } while (--height > 0);
    }
}

void SkARGB32_Shader_Blitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(x >= 0 && y >= 0 && y + height <= fDevice.height());

    uint32_t* device = fDevice.writable_addr32(x, y);
    size_t    deviceRB = fDevice.rowBytes();

    if (fConstInY) {
        SkPMColor c;
        fShaderContext->shadeSpan(x, y, &c, 1);

        if (fShadeDirectlyIntoDevice) {
            if (255 == alpha) {
                do {
                    *device = c;
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            } else {
                do {
                    *device = SkFourByteInterp(c, *device, alpha);
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            }
        } else {
            SkXfermode* xfer = fXfermode;
            if (xfer) {
                do {
                    xfer->xfer32(device, &c, 1, &alpha);
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            } else {
                SkBlitRow::Proc32 proc = (255 == alpha) ? fProc32 : fProc32Blend;
                do {
                    proc(device, &c, 1, alpha);
                    device = (uint32_t*)((char*)device + deviceRB);
                } while (--height > 0);
            }
        }
        return;
    }

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
        SkXfermode* xfer = fXfermode;
        if (xfer) {
            do {
                fShaderContext->shadeSpan(x, y, span, 1);
                xfer->xfer32(device, span, 1, &alpha);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        } else {
            SkBlitRow::Proc32 proc = (255 == alpha) ? fProc32 : fProc32Blend;
            do {
                fShaderContext->shadeSpan(x, y, span, 1);
                proc(device, span, 1, alpha);
                y += 1;
                device = (uint32_t*)((char*)device + deviceRB);
            } while (--height > 0);
        }
    }
}
