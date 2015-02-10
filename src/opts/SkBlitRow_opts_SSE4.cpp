#include "SkBlitRow_opts_SSE4.h"

// Some compilers can't compile SSSE3 or SSE4 intrinsics.  We give them stub methods.
// The stubs should never be called, so we make them crash just to confirm that.
#if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_SSE41
void S32A_Opaque_BlitRow32_SSE4(SkPMColor* SK_RESTRICT, const SkPMColor* SK_RESTRICT, int, U8CPU) {
    sk_throw();
}

void Color32A_D565_SSE4(uint16_t dst[], SkPMColor src, int count, int x, int y) {
    sk_throw();
}

#else

#include <smmintrin.h>  // SSE4.1 intrinsics

#include "SkColorPriv.h"
#include "SkColor_opts_SSE2.h"

void S32A_Opaque_BlitRow32_SSE4(SkPMColor* SK_RESTRICT dst,
                                const SkPMColor* SK_RESTRICT src,
                                int count,
                                U8CPU alpha) {
    SkASSERT(alpha == 255);
    // As long as we can, we'll work on 16 pixel pairs at once.
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
        if (_mm_testz_si128(ORed, alphaMask)) {
            // All 16 source pixels are fully transparent.  There's nothing to do!
            continue;
        }
        const __m128i ANDed = _mm_and_si128(s3, _mm_and_si128(s2, _mm_and_si128(s1, s0)));
        if (_mm_testc_si128(ANDed, alphaMask)) {
            // All 16 source pixels are fully opaque.  There's no need to read dst or blend it.
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
    for (int i = count16*16; i < count; i++) {
        // This check is not really necessarily, but it prevents pointless autovectorization.
        if (src[i] & 0xFF000000) {
            dst[i] = SkPMSrcOver(src[i], dst[i]);
        }
    }
}

static inline uint16_t Color32A_D565_1x(uint16_t dst, unsigned scale, uint32_t src_expand) {
    uint32_t dst_expand = SkExpand_rgb_16(dst) * scale;
    return SkCompact_rgb_16((src_expand + dst_expand) >> 5);
}

void Color32A_D565_SSE4(uint16_t dst[], SkPMColor src, int count, int x, int y) {
    SkASSERT(count > 0);

    uint32_t src_expand = (SkGetPackedG32(src) << 24) |
                          (SkGetPackedR32(src) << 13) |
                          (SkGetPackedB32(src) << 2);
    unsigned scale = SkAlpha255To256(0xFF - SkGetPackedA32(src)) >> 3;

    // Check if we have enough pixels to run SIMD
    if (count >= (int)(8 + (((16 - (size_t)dst) & 0x0F) >> 1))) {
        __m128i* dst_wide;
        const __m128i src_expand_wide = _mm_set1_epi32(src_expand);
        const __m128i scale_wide = _mm_set1_epi32(scale);
        const __m128i mask_green = _mm_set1_epi32(SK_R16_MASK_IN_PLACE |
                                                  SK_B16_MASK_IN_PLACE |
                                                 (SK_G16_MASK_IN_PLACE << 16));

        // Align dst to an even 16 byte address (0-7 pixels)
        while (((((size_t)dst) & 0x0F) != 0) && (count > 0)) {
            *dst = Color32A_D565_1x(*dst, scale, src_expand);
            dst += 1;
            count--;
        }

        dst_wide = reinterpret_cast<__m128i*>(dst);
        do {
            // Load 8 RGB565 pixels
            __m128i pixels = _mm_load_si128(dst_wide);

            // Duplicate and mask
            __m128i pixels_high = _mm_unpackhi_epi16(pixels, pixels);
            pixels_high = _mm_and_si128(mask_green, pixels_high);
            pixels = _mm_unpacklo_epi16(pixels, pixels);
            pixels = _mm_and_si128(mask_green, pixels);

            // Scale with alpha
            pixels_high = _mm_mullo_epi32(pixels_high, scale_wide);
            pixels = _mm_mullo_epi32(pixels, scale_wide);

            // Add src_expand_wide and shift down again
            pixels_high = _mm_add_epi32(pixels_high, src_expand_wide);
            pixels_high = _mm_srli_epi32(pixels_high, 5);
            pixels = _mm_add_epi32(pixels, src_expand_wide);
            pixels = _mm_srli_epi32(pixels, 5);

            // Mask
            pixels_high = _mm_and_si128(mask_green, pixels_high);
            pixels = _mm_and_si128(mask_green, pixels);

            // Combine into RGB565 and store
            pixels = _mm_hadd_epi16(pixels, pixels_high);
            _mm_store_si128(dst_wide, pixels);
            count -= 8;
            dst_wide++;
        } while (count >= 8);

        dst = reinterpret_cast<uint16_t*>(dst_wide);
    }

    // Small loop to handle remaining pixels.
    while (count > 0) {
        *dst = Color32A_D565_1x(*dst, scale, src_expand);
        dst += 1;
        count--;
    }
}

#endif
