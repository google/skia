/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Sk4px.h"
#include "SkBlitRow.h"
#include "SkColorData.h"
#include "SkOpts.h"
#include "SkUtils.h"

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
    #include "SkColor_opts_SSE2.h"

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

    #include "SkColor_opts_neon.h"
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

    unsigned invA = 255 - SkGetPackedA32(color);
    invA += invA >> 7;
    SkASSERT(invA < 256);  // We've should have already handled alpha == 0 externally.

    Sk16h colorHighAndRound = Sk4px::DupPMColor(color).widenHi() + Sk16h(128);
    Sk16b invA_16x(invA);

    Sk4px::MapSrc(count, dst, src, [&](const Sk4px& src4) -> Sk4px {
        return (src4 * invA_16x).addNarrowHi(colorHighAndRound);
    });
}
