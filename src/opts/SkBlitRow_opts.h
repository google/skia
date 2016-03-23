/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_DEFINED
#define SkBlitRow_opts_DEFINED

#include "Sk4px.h"
#include "SkColorPriv.h"
#include "SkMSAN.h"

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include "SkColor_opts_SSE2.h"
#endif

namespace SK_OPTS_NS {

// Color32 uses the blend_256_round_alt algorithm from tests/BlendTest.cpp.
// It's not quite perfect, but it's never wrong in the interesting edge cases,
// and it's quite a bit faster than blend_perfect.
//
// blend_256_round_alt is our currently blessed algorithm.  Please use it or an analogous one.
static inline
void blit_row_color32(SkPMColor* dst, const SkPMColor* src, int count, SkPMColor color) {
    unsigned invA = 255 - SkGetPackedA32(color);
    invA += invA >> 7;
    SkASSERT(invA < 256);  // We've should have already handled alpha == 0 externally.

    Sk16h colorHighAndRound = Sk4px::DupPMColor(color).widenHi() + Sk16h(128);
    Sk16b invA_16x(invA);

    Sk4px::MapSrc(count, dst, src, [&](const Sk4px& src4) -> Sk4px {
        return (src4 * invA_16x).addNarrowHi(colorHighAndRound);
    });
}

static inline
void blit_row_s32a_opaque(SkPMColor* dst, const SkPMColor* src, int len, U8CPU alpha) {
    SkASSERT(alpha == 0xFF);
    sk_msan_assert_initialized(src, src+len);

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    while (len >= 16) {
        // Load 16 source pixels.
        auto s0 = _mm_loadu_si128((const __m128i*)(src) + 0),
             s1 = _mm_loadu_si128((const __m128i*)(src) + 1),
             s2 = _mm_loadu_si128((const __m128i*)(src) + 2),
             s3 = _mm_loadu_si128((const __m128i*)(src) + 3);

        const auto alphaMask = _mm_set1_epi32(0xFF000000);

        auto ORed = _mm_or_si128(s3, _mm_or_si128(s2, _mm_or_si128(s1, s0)));
        if (_mm_testz_si128(ORed, alphaMask)) {
            // All 16 source pixels are transparent.  Nothing to do.
            src += 16;
            dst += 16;
            len -= 16;
            continue;
        }

        auto d0 = (__m128i*)(dst) + 0,
             d1 = (__m128i*)(dst) + 1,
             d2 = (__m128i*)(dst) + 2,
             d3 = (__m128i*)(dst) + 3;

        auto ANDed = _mm_and_si128(s3, _mm_and_si128(s2, _mm_and_si128(s1, s0)));
        if (_mm_testc_si128(ANDed, alphaMask)) {
            // All 16 source pixels are opaque.  SrcOver becomes Src.
            _mm_storeu_si128(d0, s0);
            _mm_storeu_si128(d1, s1);
            _mm_storeu_si128(d2, s2);
            _mm_storeu_si128(d3, s3);
            src += 16;
            dst += 16;
            len -= 16;
            continue;
        }

        // TODO: This math is wrong.
        // Do SrcOver.
        _mm_storeu_si128(d0, SkPMSrcOver_SSE2(s0, _mm_loadu_si128(d0)));
        _mm_storeu_si128(d1, SkPMSrcOver_SSE2(s1, _mm_loadu_si128(d1)));
        _mm_storeu_si128(d2, SkPMSrcOver_SSE2(s2, _mm_loadu_si128(d2)));
        _mm_storeu_si128(d3, SkPMSrcOver_SSE2(s3, _mm_loadu_si128(d3)));
        src += 16;
        dst += 16;
        len -= 16;
    }

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    while (len >= 16) {
        // Load 16 source pixels.
        auto s0 = _mm_loadu_si128((const __m128i*)(src) + 0),
             s1 = _mm_loadu_si128((const __m128i*)(src) + 1),
             s2 = _mm_loadu_si128((const __m128i*)(src) + 2),
             s3 = _mm_loadu_si128((const __m128i*)(src) + 3);

        const auto alphaMask = _mm_set1_epi32(0xFF000000);

        auto ORed = _mm_or_si128(s3, _mm_or_si128(s2, _mm_or_si128(s1, s0)));
        if (0xffff == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_and_si128(ORed, alphaMask),
                                                       _mm_setzero_si128()))) {
            // All 16 source pixels are transparent.  Nothing to do.
            src += 16;
            dst += 16;
            len -= 16;
            continue;
        }

        auto d0 = (__m128i*)(dst) + 0,
             d1 = (__m128i*)(dst) + 1,
             d2 = (__m128i*)(dst) + 2,
             d3 = (__m128i*)(dst) + 3;

        auto ANDed = _mm_and_si128(s3, _mm_and_si128(s2, _mm_and_si128(s1, s0)));
        if (0xffff == _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_and_si128(ANDed, alphaMask),
                                                       alphaMask))) {
            // All 16 source pixels are opaque.  SrcOver becomes Src.
            _mm_storeu_si128(d0, s0);
            _mm_storeu_si128(d1, s1);
            _mm_storeu_si128(d2, s2);
            _mm_storeu_si128(d3, s3);
            src += 16;
            dst += 16;
            len -= 16;
            continue;
        }

        // TODO: This math is wrong.
        // Do SrcOver.
        _mm_storeu_si128(d0, SkPMSrcOver_SSE2(s0, _mm_loadu_si128(d0)));
        _mm_storeu_si128(d1, SkPMSrcOver_SSE2(s1, _mm_loadu_si128(d1)));
        _mm_storeu_si128(d2, SkPMSrcOver_SSE2(s2, _mm_loadu_si128(d2)));
        _mm_storeu_si128(d3, SkPMSrcOver_SSE2(s3, _mm_loadu_si128(d3)));

        src += 16;
        dst += 16;
        len -= 16;
    }

#elif defined(SK_ARM_HAS_NEON)
    while (len >= 4) {
        if ((src[0] | src[1] | src[2] | src[3]) == 0x00000000) {
            // All 16 source pixels are transparent.  Nothing to do.
            src += 4;
            dst += 4;
            len -= 4;
            continue;
        }

        if ((src[0] & src[1] & src[2] & src[3]) >= 0xFF000000) {
            // All 16 source pixels are opaque.  SrcOver becomes Src.
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = src[3];
            src += 4;
            dst += 4;
            len -= 4;
            continue;
        }

        // Load 4 source and destination pixels.
        auto src0 = vreinterpret_u8_u32(vld1_u32(src+0)),
             src2 = vreinterpret_u8_u32(vld1_u32(src+2)),
             dst0 = vreinterpret_u8_u32(vld1_u32(dst+0)),
             dst2 = vreinterpret_u8_u32(vld1_u32(dst+2));

        // TODO: This math is wrong.
        const uint8x8_t alphas = vcreate_u8(0x0707070703030303);
        auto invSA0_w = vsubw_u8(vdupq_n_u16(256), vtbl1_u8(src0, alphas)),
             invSA2_w = vsubw_u8(vdupq_n_u16(256), vtbl1_u8(src2, alphas));

        auto dstInvSA0 = vmulq_u16(invSA0_w, vmovl_u8(dst0)),
             dstInvSA2 = vmulq_u16(invSA2_w, vmovl_u8(dst2));

        dst0 = vadd_u8(src0, vshrn_n_u16(dstInvSA0, 8));
        dst2 = vadd_u8(src2, vshrn_n_u16(dstInvSA2, 8));

        vst1_u32(dst+0, vreinterpret_u32_u8(dst0));
        vst1_u32(dst+2, vreinterpret_u32_u8(dst2));

        src += 4;
        dst += 4;
        len -= 4;
    }
#endif

    while (len-- > 0) {
        if (*src) {
            *dst = (*src >= 0xFF000000) ? *src : SkPMSrcOver(*src, *dst);
        }
        src++;
        dst++;
    }
}

}  // SK_OPTS_NS

#endif//SkBlitRow_opts_DEFINED
