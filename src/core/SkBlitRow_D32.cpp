/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitRow.h"
#include "SkBlitMask.h"
#include "SkColorPriv.h"
#include "SkUtils.h"

#define UNROLL

static void S32_Opaque_BlitRow32(SkPMColor* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src,
                                 int count, U8CPU alpha) {
    SkASSERT(255 == alpha);
    sk_memcpy32(dst, src, count);
}

static void S32_Blend_BlitRow32(SkPMColor* SK_RESTRICT dst,
                                const SkPMColor* SK_RESTRICT src,
                                int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count > 0) {
        unsigned src_scale = SkAlpha255To256(alpha);
        unsigned dst_scale = 256 - src_scale;

#ifdef UNROLL
        if (count & 1) {
            *dst = SkAlphaMulQ(*(src++), src_scale) + SkAlphaMulQ(*dst, dst_scale);
            dst += 1;
            count -= 1;
        }

        const SkPMColor* SK_RESTRICT srcEnd = src + count;
        while (src != srcEnd) {
            *dst = SkAlphaMulQ(*(src++), src_scale) + SkAlphaMulQ(*dst, dst_scale);
            dst += 1;
            *dst = SkAlphaMulQ(*(src++), src_scale) + SkAlphaMulQ(*dst, dst_scale);
            dst += 1;
        }
#else
        do {
            *dst = SkAlphaMulQ(*src, src_scale) + SkAlphaMulQ(*dst, dst_scale);
            src += 1;
            dst += 1;
        } while (--count > 0);
#endif
    }
}

static void S32A_Opaque_BlitRow32(SkPMColor* SK_RESTRICT dst,
                                  const SkPMColor* SK_RESTRICT src,
                                  int count, U8CPU alpha) {
    SkASSERT(255 == alpha);
    if (count > 0) {
#ifdef UNROLL
        if (count & 1) {
            *dst = SkPMSrcOver(*(src++), *dst);
            dst += 1;
            count -= 1;
        }

        const SkPMColor* SK_RESTRICT srcEnd = src + count;
        while (src != srcEnd) {
            *dst = SkPMSrcOver(*(src++), *dst);
            dst += 1;
            *dst = SkPMSrcOver(*(src++), *dst);
            dst += 1;
        }
#else
        do {
            *dst = SkPMSrcOver(*src, *dst);
            src += 1;
            dst += 1;
        } while (--count > 0);
#endif
    }
}

static void S32A_Blend_BlitRow32(SkPMColor* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src,
                                 int count, U8CPU alpha) {
    SkASSERT(alpha <= 255);
    if (count > 0) {
#ifdef UNROLL
        if (count & 1) {
            *dst = SkBlendARGB32(*(src++), *dst, alpha);
            dst += 1;
            count -= 1;
        }

        const SkPMColor* SK_RESTRICT srcEnd = src + count;
        while (src != srcEnd) {
            *dst = SkBlendARGB32(*(src++), *dst, alpha);
            dst += 1;
            *dst = SkBlendARGB32(*(src++), *dst, alpha);
            dst += 1;
        }
#else
        do {
            *dst = SkBlendARGB32(*src, *dst, alpha);
            src += 1;
            dst += 1;
        } while (--count > 0);
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////

static const SkBlitRow::Proc32 gDefault_Procs32[] = {
    S32_Opaque_BlitRow32,
    S32_Blend_BlitRow32,
    S32A_Opaque_BlitRow32,
    S32A_Blend_BlitRow32
};

SkBlitRow::Proc32 SkBlitRow::Factory32(unsigned flags) {
    SkASSERT(flags < SK_ARRAY_COUNT(gDefault_Procs32));
    // just so we don't crash
    flags &= kFlags32_Mask;

    SkBlitRow::Proc32 proc = PlatformProcs32(flags);
    if (NULL == proc) {
        proc = gDefault_Procs32[flags];
    }
    SkASSERT(proc);
    return proc;
}

// Color32 uses the blend_256_round_alt algorithm from tests/BlendTest.cpp.
// It's not quite perfect, but it's never wrong in the interesting edge cases,
// and it's quite a bit faster than blend_perfect.
//
// blend_256_round_alt is our currently blessed algorithm.  Please use it or an analogous one.
void SkBlitRow::Color32(SkPMColor dst[], const SkPMColor src[], int count, SkPMColor color) {
    switch (SkGetPackedA32(color)) {
        case   0: memmove(dst, src, count * sizeof(SkPMColor)); return;
        case 255: sk_memset32(dst, color, count);               return;
    }

    unsigned invA = 255 - SkGetPackedA32(color);
    invA += invA >> 7;
    SkASSERT(invA < 256);  // We've already handled alpha == 0 above.

#if defined(SK_ARM_HAS_NEON)
    uint16x8_t colorHigh = vshll_n_u8((uint8x8_t)vdup_n_u32(color), 8);
    uint16x8_t colorAndRound = vaddq_u16(colorHigh, vdupq_n_u16(128));
    uint8x8_t invA8 = vdup_n_u8(invA);

    // Does the core work of blending color onto 4 pixels, returning the resulting 4 pixels.
    auto kernel = [&](const uint32x4_t& src4) -> uint32x4_t {
        uint16x8_t lo = vmull_u8(vget_low_u8( (uint8x16_t)src4), invA8),
                   hi = vmull_u8(vget_high_u8((uint8x16_t)src4), invA8);
        return (uint32x4_t)
            vcombine_u8(vaddhn_u16(colorAndRound, lo), vaddhn_u16(colorAndRound, hi));
    };

    while (count >= 8) {
        uint32x4_t dst0 = kernel(vld1q_u32(src+0)),
                   dst4 = kernel(vld1q_u32(src+4));
        vst1q_u32(dst+0, dst0);
        vst1q_u32(dst+4, dst4);
        src   += 8;
        dst   += 8;
        count -= 8;
    }
    if (count >= 4) {
        vst1q_u32(dst, kernel(vld1q_u32(src)));
        src   += 4;
        dst   += 4;
        count -= 4;
    }
    if (count >= 2) {
        uint32x2_t src2 = vld1_u32(src);
        vst1_u32(dst, vget_low_u32(kernel(vcombine_u32(src2, src2))));
        src   += 2;
        dst   += 2;
        count -= 2;
    }
    if (count >= 1) {
        vst1q_lane_u32(dst, kernel(vdupq_n_u32(*src)), 0);
    }

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    __m128i colorHigh = _mm_unpacklo_epi8(_mm_setzero_si128(), _mm_set1_epi32(color));
    __m128i colorAndRound = _mm_add_epi16(colorHigh, _mm_set1_epi16(128));
    __m128i invA16 = _mm_set1_epi16(invA);

    // Does the core work of blending color onto 4 pixels, returning the resulting 4 pixels.
    auto kernel = [&](const __m128i& src4) -> __m128i {
        __m128i lo = _mm_mullo_epi16(invA16, _mm_unpacklo_epi8(src4, _mm_setzero_si128())),
                hi = _mm_mullo_epi16(invA16, _mm_unpackhi_epi8(src4, _mm_setzero_si128()));
        return _mm_packus_epi16(_mm_srli_epi16(_mm_add_epi16(colorAndRound, lo), 8),
                                _mm_srli_epi16(_mm_add_epi16(colorAndRound, hi), 8));
    };

    while (count >= 8) {
        __m128i dst0 = kernel(_mm_loadu_si128((const __m128i*)(src+0))),
                dst4 = kernel(_mm_loadu_si128((const __m128i*)(src+4)));
        _mm_storeu_si128((__m128i*)(dst+0), dst0);
        _mm_storeu_si128((__m128i*)(dst+4), dst4);
        src   += 8;
        dst   += 8;
        count -= 8;
    }
    if (count >= 4) {
        _mm_storeu_si128((__m128i*)dst, kernel(_mm_loadu_si128((const __m128i*)src)));
        src   += 4;
        dst   += 4;
        count -= 4;
    }
    if (count >= 2) {
        _mm_storel_epi64((__m128i*)dst, kernel(_mm_loadl_epi64((const __m128i*)src)));
        src   += 2;
        dst   += 2;
        count -= 2;
    }
    if (count >= 1) {
        *dst = _mm_cvtsi128_si32(kernel(_mm_cvtsi32_si128(*src)));
    }
#else  // Neither NEON nor SSE2.
    unsigned round = (128 << 16) + (128 << 0);

    while (count --> 0) {
        // Our math is 16-bit, so we can do a little bit of SIMD in 32-bit registers.
        const uint32_t mask = 0x00FF00FF;
        uint32_t rb = (((*src >> 0) & mask) * invA + round) >> 8,  // _r_b
                 ag = (((*src >> 8) & mask) * invA + round) >> 0;  // a_g_
        *dst = color + ((rb & mask) | (ag & ~mask));
        src++;
        dst++;
    }
#endif
}

