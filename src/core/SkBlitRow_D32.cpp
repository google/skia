/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkColorData.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkOpts.h"

// Everyone agrees memcpy() is the best way to do this.
static void blit_row_s32_opaque(SkPMColor* dst,
                                const SkPMColor* src,
                                int count,
                                U8CPU alpha) {
    SkASSERT(255 == alpha);
    memcpy(dst, src, count * sizeof(SkPMColor));
}

// We have SSE2, NEON, and portable implementations of
// blit_row_s32_blend() and blit_row_s32a_blend().

// TODO(mtklein): can we do better in NEON than 2 pixels at a time?

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <emmintrin.h>

    static inline __m128i SkPMLerp_SSE2(const __m128i& src,
                                        const __m128i& dst,
                                        const unsigned src_scale) {
        // Computes dst + (((src - dst)*src_scale)>>8)
        const __m128i mask = _mm_set1_epi32(0x00FF00FF);

        // Unpack the 16x8-bit source into 2 8x16-bit splayed halves.
        __m128i src_rb = _mm_and_si128(mask, src);
        __m128i src_ag = _mm_srli_epi16(src, 8);
        __m128i dst_rb = _mm_and_si128(mask, dst);
        __m128i dst_ag = _mm_srli_epi16(dst, 8);

        // Compute scaled differences.
        __m128i diff_rb = _mm_sub_epi16(src_rb, dst_rb);
        __m128i diff_ag = _mm_sub_epi16(src_ag, dst_ag);
        __m128i s = _mm_set1_epi16(src_scale);
        diff_rb = _mm_mullo_epi16(diff_rb, s);
        diff_ag = _mm_mullo_epi16(diff_ag, s);

        // Pack the differences back together.
        diff_rb = _mm_srli_epi16(diff_rb, 8);
        diff_ag = _mm_andnot_si128(mask, diff_ag);
        __m128i diff = _mm_or_si128(diff_rb, diff_ag);

        // Add difference to destination.
        return _mm_add_epi8(dst, diff);
    }


    static void blit_row_s32_blend(SkPMColor* dst, const SkPMColor* src, int count, U8CPU alpha) {
        SkASSERT(alpha <= 255);

        auto src4 = (const __m128i*)src;
        auto dst4 = (      __m128i*)dst;

        while (count >= 4) {
            _mm_storeu_si128(dst4, SkPMLerp_SSE2(_mm_loadu_si128(src4),
                                                 _mm_loadu_si128(dst4),
                                                 SkAlpha255To256(alpha)));
            src4++;
            dst4++;
            count -= 4;
        }

        src = (const SkPMColor*)src4;
        dst = (      SkPMColor*)dst4;

        while (count --> 0) {
            *dst = SkPMLerp(*src, *dst, SkAlpha255To256(alpha));
            src++;
            dst++;
        }
    }

    static inline __m128i SkBlendARGB32_SSE2(const __m128i& src,
                                             const __m128i& dst,
                                             const unsigned aa) {
        unsigned alpha = SkAlpha255To256(aa);
        __m128i src_scale = _mm_set1_epi16(alpha);
        // SkAlphaMulInv256(SkGetPackedA32(src), src_scale)
        __m128i dst_scale = _mm_srli_epi32(src, 24);
        // High words in dst_scale are 0, so it's safe to multiply with 16-bit src_scale.
        dst_scale = _mm_mullo_epi16(dst_scale, src_scale);
        dst_scale = _mm_sub_epi32(_mm_set1_epi32(0xFFFF), dst_scale);
        dst_scale = _mm_add_epi32(dst_scale, _mm_srli_epi32(dst_scale, 8));
        dst_scale = _mm_srli_epi32(dst_scale, 8);
        // Duplicate scales into 2x16-bit pattern per pixel.
        dst_scale = _mm_shufflelo_epi16(dst_scale, _MM_SHUFFLE(2, 2, 0, 0));
        dst_scale = _mm_shufflehi_epi16(dst_scale, _MM_SHUFFLE(2, 2, 0, 0));

        const __m128i mask = _mm_set1_epi32(0x00FF00FF);

        // Unpack the 16x8-bit source/destination into 2 8x16-bit splayed halves.
        __m128i src_rb = _mm_and_si128(mask, src);
        __m128i src_ag = _mm_srli_epi16(src, 8);
        __m128i dst_rb = _mm_and_si128(mask, dst);
        __m128i dst_ag = _mm_srli_epi16(dst, 8);

        // Scale them.
        src_rb = _mm_mullo_epi16(src_rb, src_scale);
        src_ag = _mm_mullo_epi16(src_ag, src_scale);
        dst_rb = _mm_mullo_epi16(dst_rb, dst_scale);
        dst_ag = _mm_mullo_epi16(dst_ag, dst_scale);

        // Add the scaled source and destination.
        dst_rb = _mm_add_epi16(src_rb, dst_rb);
        dst_ag = _mm_add_epi16(src_ag, dst_ag);

        // Unsplay the halves back together.
        dst_rb = _mm_srli_epi16(dst_rb, 8);
        dst_ag = _mm_andnot_si128(mask, dst_ag);
        return _mm_or_si128(dst_rb, dst_ag);
    }

    static void blit_row_s32a_blend(SkPMColor* dst, const SkPMColor* src, int count, U8CPU alpha) {
        SkASSERT(alpha <= 255);

        auto src4 = (const __m128i*)src;
        auto dst4 = (      __m128i*)dst;

        while (count >= 4) {
            _mm_storeu_si128(dst4, SkBlendARGB32_SSE2(_mm_loadu_si128(src4),
                                                      _mm_loadu_si128(dst4),
                                                      alpha));
            src4++;
            dst4++;
            count -= 4;
        }

        src = (const SkPMColor*)src4;
        dst = (      SkPMColor*)dst4;

        while (count --> 0) {
            *dst = SkBlendARGB32(*src, *dst, alpha);
            src++;
            dst++;
        }
    }

#elif defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>

    static void blit_row_s32_blend(SkPMColor* dst, const SkPMColor* src, int count, U8CPU alpha) {
        SkASSERT(alpha <= 255);

        uint16_t src_scale = SkAlpha255To256(alpha);
        uint16_t dst_scale = 256 - src_scale;

        while (count >= 2) {
            uint8x8_t vsrc, vdst, vres;
            uint16x8_t vsrc_wide, vdst_wide;

            vsrc = vreinterpret_u8_u32(vld1_u32(src));
            vdst = vreinterpret_u8_u32(vld1_u32(dst));

            vsrc_wide = vmovl_u8(vsrc);
            vsrc_wide = vmulq_u16(vsrc_wide, vdupq_n_u16(src_scale));

            vdst_wide = vmull_u8(vdst, vdup_n_u8(dst_scale));

            vdst_wide += vsrc_wide;
            vres = vshrn_n_u16(vdst_wide, 8);

            vst1_u32(dst, vreinterpret_u32_u8(vres));

            src += 2;
            dst += 2;
            count -= 2;
        }

        if (count == 1) {
            uint8x8_t vsrc = vdup_n_u8(0), vdst = vdup_n_u8(0), vres;
            uint16x8_t vsrc_wide, vdst_wide;

            vsrc = vreinterpret_u8_u32(vld1_lane_u32(src, vreinterpret_u32_u8(vsrc), 0));
            vdst = vreinterpret_u8_u32(vld1_lane_u32(dst, vreinterpret_u32_u8(vdst), 0));

            vsrc_wide = vmovl_u8(vsrc);
            vsrc_wide = vmulq_u16(vsrc_wide, vdupq_n_u16(src_scale));
            vdst_wide = vmull_u8(vdst, vdup_n_u8(dst_scale));
            vdst_wide += vsrc_wide;
            vres = vshrn_n_u16(vdst_wide, 8);

            vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);
        }
    }

    static void blit_row_s32a_blend(SkPMColor* dst, const SkPMColor* src, int count, U8CPU alpha) {
        SkASSERT(alpha < 255);

        unsigned alpha256 = SkAlpha255To256(alpha);

        if (count & 1) {
            uint8x8_t vsrc = vdup_n_u8(0), vdst = vdup_n_u8(0), vres;
            uint16x8_t vdst_wide, vsrc_wide;
            unsigned dst_scale;

            vsrc = vreinterpret_u8_u32(vld1_lane_u32(src, vreinterpret_u32_u8(vsrc), 0));
            vdst = vreinterpret_u8_u32(vld1_lane_u32(dst, vreinterpret_u32_u8(vdst), 0));

            dst_scale = vget_lane_u8(vsrc, 3);
            dst_scale = SkAlphaMulInv256(dst_scale, alpha256);

            vsrc_wide = vmovl_u8(vsrc);
            vsrc_wide = vmulq_n_u16(vsrc_wide, alpha256);

            vdst_wide = vmovl_u8(vdst);
            vdst_wide = vmulq_n_u16(vdst_wide, dst_scale);

            vdst_wide += vsrc_wide;
            vres = vshrn_n_u16(vdst_wide, 8);

            vst1_lane_u32(dst, vreinterpret_u32_u8(vres), 0);
            dst++;
            src++;
            count--;
        }

        uint8x8_t alpha_mask;
        static const uint8_t alpha_mask_setup[] = {3,3,3,3,7,7,7,7};
        alpha_mask = vld1_u8(alpha_mask_setup);

        while (count) {

            uint8x8_t vsrc, vdst, vres, vsrc_alphas;
            uint16x8_t vdst_wide, vsrc_wide, vsrc_scale, vdst_scale;

            __builtin_prefetch(src+32);
            __builtin_prefetch(dst+32);

            vsrc = vreinterpret_u8_u32(vld1_u32(src));
            vdst = vreinterpret_u8_u32(vld1_u32(dst));

            vsrc_scale = vdupq_n_u16(alpha256);

            vsrc_alphas = vtbl1_u8(vsrc, alpha_mask);
            vdst_scale = vmovl_u8(vsrc_alphas);
            // Calculate SkAlphaMulInv256(vdst_scale, vsrc_scale).
            // A 16-bit lane would overflow if we used 0xFFFF here,
            // so use an approximation with 0xFF00 that is off by 1,
            // and add back 1 after to get the correct value.
            // This is valid if alpha256 <= 255.
            vdst_scale = vmlsq_u16(vdupq_n_u16(0xFF00), vdst_scale, vsrc_scale);
            vdst_scale = vsraq_n_u16(vdst_scale, vdst_scale, 8);
            vdst_scale = vsraq_n_u16(vdupq_n_u16(1), vdst_scale, 8);

            vsrc_wide = vmovl_u8(vsrc);
            vsrc_wide *= vsrc_scale;

            vdst_wide = vmovl_u8(vdst);
            vdst_wide *= vdst_scale;

            vdst_wide += vsrc_wide;
            vres = vshrn_n_u16(vdst_wide, 8);

            vst1_u32(dst, vreinterpret_u32_u8(vres));

            src += 2;
            dst += 2;
            count -= 2;
        }
    }

#else
    static void blit_row_s32_blend(SkPMColor* dst, const SkPMColor* src, int count, U8CPU alpha) {
        SkASSERT(alpha <= 255);
        while (count --> 0) {
            *dst = SkPMLerp(*src, *dst, SkAlpha255To256(alpha));
            src++;
            dst++;
        }
    }

    static void blit_row_s32a_blend(SkPMColor* dst, const SkPMColor* src, int count, U8CPU alpha) {
        SkASSERT(alpha <= 255);
        while (count --> 0) {
            *dst = SkBlendARGB32(*src, *dst, alpha);
            src++;
            dst++;
        }
    }
#endif

SkBlitRow::Proc32 SkBlitRow::Factory32(unsigned flags) {
    static const SkBlitRow::Proc32 kProcs[] = {
        blit_row_s32_opaque,
        blit_row_s32_blend,
        nullptr,  // blit_row_s32a_opaque is in SkOpts
        blit_row_s32a_blend
    };

    SkASSERT(flags < SK_ARRAY_COUNT(kProcs));
    flags &= SK_ARRAY_COUNT(kProcs) - 1;  // just to be safe

    return flags == 2 ? SkOpts::blit_row_s32a_opaque
                      : kProcs[flags];
}

void SkBlitRow::Color32(SkPMColor dst[], const SkPMColor src[], int count, SkPMColor color) {
    switch (SkGetPackedA32(color)) {
        case   0: memmove(dst, src, count * sizeof(SkPMColor)); return;
        case 255: sk_memset32(dst, color, count);               return;
    }
    return SkOpts::blit_row_color32(dst, src, count, color);
}
