/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_DEFINED
#define SkBlitRow_opts_DEFINED

#include "include/private/SkColorData.h"
#include "include/private/SkVx.h"
#include "src/core/SkMSAN.h"
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    #include <immintrin.h>

    static inline __m256i SkPMSrcOver_AVX2(const __m256i& src, const __m256i& dst) {
        // Abstractly srcover is
        //     b = s + d*(1-srcA)
        //
        // In terms of unorm8 bytes, that works out to
        //     b = s + (d*(255-srcA) + 127) / 255
        //
        // But we approximate that to within a bit with
        //     b = s + (d*(255-srcA) + d) / 256
        // a.k.a
        //     b = s + (d*(256-srcA)) >> 8

        // The bottleneck of this math is the multiply, and we want to do it as
        // narrowly as possible, here getting inputs into 16-bit lanes and
        // using 16-bit multiplies.  We can do twice as many multiplies at once
        // as using naive 32-bit multiplies, and on top of that, the 16-bit multiplies
        // are themselves a couple cycles quicker.  Win-win.

        // We'll get everything in 16-bit lanes for two multiplies, one
        // handling dst red and blue, the other green and alpha.  (They're
        // conveniently 16-bits apart, you see.) We don't need the individual
        // src channels beyond alpha until the very end when we do the "s + "
        // add, and we don't even need to unpack them; the adds cannot overflow.

        // Shuffle each pixel's srcA to the low byte of each 16-bit half of the pixel.
        const int _ = -1;   // fills a literal 0 byte.
        __m256i srcA_x2 = _mm256_shuffle_epi8(src,
                _mm256_setr_epi8(3,_,3,_, 7,_,7,_, 11,_,11,_, 15,_,15,_,
                                 3,_,3,_, 7,_,7,_, 11,_,11,_, 15,_,15,_));
        __m256i scale_x2 = _mm256_sub_epi16(_mm256_set1_epi16(256),
                                            srcA_x2);

        // Scale red and blue, leaving results in the low byte of each 16-bit lane.
        __m256i rb = _mm256_and_si256(_mm256_set1_epi32(0x00ff00ff), dst);
        rb = _mm256_mullo_epi16(rb, scale_x2);
        rb = _mm256_srli_epi16 (rb, 8);

        // Scale green and alpha, leaving results in the high byte, masking off the low bits.
        __m256i ga = _mm256_srli_epi16(dst, 8);
        ga = _mm256_mullo_epi16(ga, scale_x2);
        ga = _mm256_andnot_si256(_mm256_set1_epi32(0x00ff00ff), ga);

        return _mm256_add_epi32(src, _mm256_or_si256(rb, ga));
    }

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <immintrin.h>

    static inline __m128i SkPMSrcOver_SSE2(const __m128i& src, const __m128i& dst) {
        auto SkAlphaMulQ_SSE2 = [](const __m128i& c, const __m128i& scale) {
            const __m128i mask = _mm_set1_epi32(0xFF00FF);
            __m128i s = _mm_or_si128(_mm_slli_epi32(scale, 16), scale);

            // uint32_t rb = ((c & mask) * scale) >> 8
            __m128i rb = _mm_and_si128(mask, c);
            rb = _mm_mullo_epi16(rb, s);
            rb = _mm_srli_epi16(rb, 8);

            // uint32_t ag = ((c >> 8) & mask) * scale
            __m128i ag = _mm_srli_epi16(c, 8);
            ag = _mm_mullo_epi16(ag, s);

            // (rb & mask) | (ag & ~mask)
            ag = _mm_andnot_si128(mask, ag);
            return _mm_or_si128(rb, ag);
        };
        return _mm_add_epi32(src,
                             SkAlphaMulQ_SSE2(dst, _mm_sub_epi32(_mm_set1_epi32(256),
                                                                 _mm_srli_epi32(src, 24))));
    }
#endif

namespace SK_OPTS_NS {

// Blend constant color over count src pixels, writing into dst.
inline void blit_row_color32(SkPMColor* dst, const SkPMColor* src, int count, SkPMColor color) {
    constexpr int N = 4;  // 8, 16 also reasonable choices
    using U32 = skvx::Vec<  N, uint32_t>;
    using U16 = skvx::Vec<4*N, uint16_t>;
    using U8  = skvx::Vec<4*N, uint8_t>;

    auto kernel = [color](U32 src) {
        unsigned invA = 255 - SkGetPackedA32(color);
        invA += invA >> 7;
        SkASSERT(0 < invA && invA < 256);  // We handle alpha == 0 or alpha == 255 specially.

        // (src * invA + (color << 8) + 128) >> 8
        // Should all fit in 16 bits.
        U8 s = skvx::bit_pun<U8>(src),
           a = U8(invA);
        U16 c = skvx::cast<uint16_t>(skvx::bit_pun<U8>(U32(color))),
            d = (mull(s,a) + (c << 8) + 128)>>8;
        return skvx::bit_pun<U32>(skvx::cast<uint8_t>(d));
    };

    while (count >= N) {
        kernel(U32::Load(src)).store(dst);
        src   += N;
        dst   += N;
        count -= N;
    }
    while (count --> 0) {
        *dst++ = kernel(U32{*src++})[0];
    }
}

#if defined(SK_ARM_HAS_NEON)

// Return a uint8x8_t value, r, computed as r[i] = SkMulDiv255Round(x[i], y[i]), where r[i], x[i],
// y[i] are the i-th lanes of the corresponding NEON vectors.
static inline uint8x8_t SkMulDiv255Round_neon8(uint8x8_t x, uint8x8_t y) {
    uint16x8_t prod = vmull_u8(x, y);
    return vraddhn_u16(prod, vrshrq_n_u16(prod, 8));
}

// The implementations of SkPMSrcOver below perform alpha blending consistently with
// SkMulDiv255Round. They compute the color components (numbers in the interval [0, 255]) as:
//
//   result_i = src_i + rint(g(src_alpha, dst_i))
//
// where g(x, y) = ((255.0 - x) * y) / 255.0 and rint rounds to the nearest integer.

// In this variant of SkPMSrcOver each NEON register, dst.val[i], src.val[i], contains the value
// of the same color component for 8 consecutive pixels. The result of this function follows the
// same convention.
static inline uint8x8x4_t SkPMSrcOver_neon8(uint8x8x4_t dst, uint8x8x4_t src) {
    uint8x8_t nalphas = vmvn_u8(src.val[3]);
    uint8x8x4_t result;
    result.val[0] = vadd_u8(src.val[0], SkMulDiv255Round_neon8(nalphas,  dst.val[0]));
    result.val[1] = vadd_u8(src.val[1], SkMulDiv255Round_neon8(nalphas,  dst.val[1]));
    result.val[2] = vadd_u8(src.val[2], SkMulDiv255Round_neon8(nalphas,  dst.val[2]));
    result.val[3] = vadd_u8(src.val[3], SkMulDiv255Round_neon8(nalphas,  dst.val[3]));
    return result;
}

// In this variant of SkPMSrcOver dst and src contain the color components of two consecutive
// pixels. The return value follows the same convention.
static inline uint8x8_t SkPMSrcOver_neon2(uint8x8_t dst, uint8x8_t src) {
    const uint8x8_t alpha_indices = vcreate_u8(0x0707070703030303);
    uint8x8_t nalphas = vmvn_u8(vtbl1_u8(src, alpha_indices));
    return vadd_u8(src, SkMulDiv255Round_neon8(nalphas, dst));
}

#endif

/*not static*/ inline
void blit_row_s32a_opaque(SkPMColor* dst, const SkPMColor* src, int len, U8CPU alpha) {
    SkASSERT(alpha == 0xFF);
    sk_msan_assert_initialized(src, src+len);
// Require AVX2 because of AVX2 integer calculation intrinsics in SrcOver
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    while (len >= 32) {
        // Load 32 source pixels.
        auto s0 = _mm256_loadu_si256((const __m256i*)(src) + 0),
             s1 = _mm256_loadu_si256((const __m256i*)(src) + 1),
             s2 = _mm256_loadu_si256((const __m256i*)(src) + 2),
             s3 = _mm256_loadu_si256((const __m256i*)(src) + 3);

        const auto alphaMask = _mm256_set1_epi32(0xFF000000);

        auto ORed = _mm256_or_si256(s3, _mm256_or_si256(s2, _mm256_or_si256(s1, s0)));
        if (_mm256_testz_si256(ORed, alphaMask)) {
            // All 32 source pixels are transparent.  Nothing to do.
            src += 32;
            dst += 32;
            len -= 32;
            continue;
        }

        auto d0 = (__m256i*)(dst) + 0,
             d1 = (__m256i*)(dst) + 1,
             d2 = (__m256i*)(dst) + 2,
             d3 = (__m256i*)(dst) + 3;

        auto ANDed = _mm256_and_si256(s3, _mm256_and_si256(s2, _mm256_and_si256(s1, s0)));
        if (_mm256_testc_si256(ANDed, alphaMask)) {
            // All 32 source pixels are opaque.  SrcOver becomes Src.
            _mm256_storeu_si256(d0, s0);
            _mm256_storeu_si256(d1, s1);
            _mm256_storeu_si256(d2, s2);
            _mm256_storeu_si256(d3, s3);
            src += 32;
            dst += 32;
            len -= 32;
            continue;
        }

        // TODO: This math is wrong.
        // Do SrcOver.
        _mm256_storeu_si256(d0, SkPMSrcOver_AVX2(s0, _mm256_loadu_si256(d0)));
        _mm256_storeu_si256(d1, SkPMSrcOver_AVX2(s1, _mm256_loadu_si256(d1)));
        _mm256_storeu_si256(d2, SkPMSrcOver_AVX2(s2, _mm256_loadu_si256(d2)));
        _mm256_storeu_si256(d3, SkPMSrcOver_AVX2(s3, _mm256_loadu_si256(d3)));
        src += 32;
        dst += 32;
        len -= 32;
    }

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
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
    // Do 8-pixels at a time. A 16-pixels at a time version of this code was also tested, but it
    // underperformed on some of the platforms under test for inputs with frequent transitions of
    // alpha (corresponding to changes of the conditions [~]alpha_u64 == 0 below). It may be worth
    // revisiting the situation in the future.
    while (len >= 8) {
        // Load 8 pixels in 4 NEON registers. src_col.val[i] will contain the same color component
        // for 8 consecutive pixels (e.g. src_col.val[3] will contain all alpha components of 8
        // pixels).
        uint8x8x4_t src_col = vld4_u8(reinterpret_cast<const uint8_t*>(src));
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

        if (~alphas_u64 == 0) {
            // All pixels opaque.
            vst4_u8(reinterpret_cast<uint8_t*>(dst), src_col);
            dst += 8;
            continue;
        }

        uint8x8x4_t dst_col = vld4_u8(reinterpret_cast<uint8_t*>(dst));
        vst4_u8(reinterpret_cast<uint8_t*>(dst), SkPMSrcOver_neon8(dst_col, src_col));
        dst += 8;
    }

    // Deal with leftover pixels.
    for (; len >= 2; len -= 2, src += 2, dst += 2) {
        uint8x8_t src2 = vld1_u8(reinterpret_cast<const uint8_t*>(src));
        uint8x8_t dst2 = vld1_u8(reinterpret_cast<const uint8_t*>(dst));
        vst1_u8(reinterpret_cast<uint8_t*>(dst), SkPMSrcOver_neon2(dst2, src2));
    }

    if (len != 0) {
        uint8x8_t result = SkPMSrcOver_neon2(vcreate_u8(*dst), vcreate_u8(*src));
        vst1_lane_u32(dst, vreinterpret_u32_u8(result), 0);
    }
    return;
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
