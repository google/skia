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
    // The implementation below performs alpha blending consistently with SkPMSrcOver, which at the
    // time of writing computes the color components (numbers in the interval [0, 255]) as:
    //
    //   result_i = src_i + trunc(f(src_alpha, dst_i))
    //
    // where, f(x, y) = ((256 - x) * y) / 256. A better approximation could be obtained using,
    // g(x, y) = ((255 - x) * y) / 255 and rounding to the nearest integer, rather than towards
    // zero (using libm names, this means rint rather than trunc). f is used as it is typically
    // quicker to compute than g. Some quick math shows that f = g + error, where
    // error = x * y / (255 * 256). In the worst case, error ~< 1.0, but it should be noted that
    // the truncation toward zero is working against the error (e.g. take the case x = y = 255).
    // In practice, the two formulas give different results in 28.6% of all the combinations of
    // (x, y). This difference is either +1 (2.25%) or -1 (26.37%), meaning that the formula using
    // f tends to give lower values than the one based on g.

    // Do 8-pixels at a time. This seems a good compromise as it shows performance improvements for
    // a variety of input data when compared to the 4-pixels code below. A 16-pixels at a time
    // version of this code was also tested, but it underperformed on some of the platforms under
    // test for inputs with frequent transitions of alpha (corresponding to changes of the
    // conditions [~]alpha_u64 == 0 below). It may be worth revisiting the situation in the future.
    while (len >= 8) {
        // Load 8 pixels in 4 NEON registers. src_col.val[i] will contain the same color component
        // for 8 consecutive pixels (e.g. src_col.val[3] will contain all alpha components of 8
        // pixels).
        uint8x8x4_t src_col = vld4_u8(reinterpret_cast<uint8_t const*>(src));
        src += 8;
        len -= 8;

        // We now detect 2 special cases: the first occurs when all alphas are zero (the 8 pixels
        // are all transparent), the second when all alphas are fully set (they are all opaque).
        uint8x8_t alphas = src_col.val[3];
        uint64_t alphas_u64 = vget_lane_u64(vreinterpret_u64_u8(alphas), 0);
        if (alphas_u64 == 0) {
            // All pixels transparent.
            dst += 8;
            continue;
        }

        if ((~alphas_u64) == 0) {
            // All pixels opaque.
            vst4_u8(reinterpret_cast<uint8_t*>(dst), src_col);
            dst += 8;
            continue;
        }

        uint8x8x4_t dst_col = vld4_u8(reinterpret_cast<uint8_t*>(dst));
        uint16x8_t nalphas = vsubw_u8(vdupq_n_u16(256), alphas);

        uint8x8x4_t result;
        result.val[0] = vadd_u8(src_col.val[0],
                                vshrn_n_u16(vmulq_u16(nalphas,  vmovl_u8(dst_col.val[0])), 8));
        result.val[1] = vadd_u8(src_col.val[1],
                                vshrn_n_u16(vmulq_u16(nalphas,  vmovl_u8(dst_col.val[1])), 8));
        result.val[2] = vadd_u8(src_col.val[2],
                                vshrn_n_u16(vmulq_u16(nalphas,  vmovl_u8(dst_col.val[2])), 8));
        result.val[3] = vadd_u8(src_col.val[3],
                                vshrn_n_u16(vmulq_u16(nalphas,  vmovl_u8(dst_col.val[3])), 8));
        vst4_u8(reinterpret_cast<uint8_t*>(dst), result);
        dst += 8;
    }
#endif

    while (len-- > 0) {
        // This 0xFF000000 is not semantically necessary, but for compatibility
        // with chromium:611002 we need to keep it until we figure out where
        // the non-premultiplied src values (like 0x00FFFFFF) are coming from.
        // TODO(mtklein): sort this out and assert *src is premul here.
        if (*src & 0xFF000000) {
            *dst = (*src >= 0xFF000000) ? *src : SkPMSrcOver(*src, *dst);
        }
        src++;
        dst++;
    }
}

}  // SK_OPTS_NS

#endif//SkBlitRow_opts_DEFINED
