/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitRow_opts_DEFINED
#define SkBlitRow_opts_DEFINED

#include "include/private/SkColorData.h"
#include "src/base/SkMSAN.h"
#include "src/base/SkVx.h"

// Helpers for blit_row_s32a_opaque(),
// then blit_row_s32a_opaque() itself,
// then unrelated blit_row_color32() at the bottom.
//
// To keep Skia resistant to timing attacks, it's important not to branch on pixel data.
// In particular, don't be tempted to [v]ptest, pmovmskb, etc. to branch on the source alpha.

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

        return _mm256_adds_epu8(src, _mm256_or_si256(rb, ga));
    }
#endif

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <immintrin.h>

    static inline __m128i SkPMSrcOver_SSE2(const __m128i& src, const __m128i& dst) {
        __m128i scale = _mm_sub_epi32(_mm_set1_epi32(256),
                                      _mm_srli_epi32(src, 24));
        __m128i scale_x2 = _mm_or_si128(_mm_slli_epi32(scale, 16), scale);

        __m128i rb = _mm_and_si128(_mm_set1_epi32(0x00ff00ff), dst);
        rb = _mm_mullo_epi16(rb, scale_x2);
        rb = _mm_srli_epi16(rb, 8);

        __m128i ga = _mm_srli_epi16(dst, 8);
        ga = _mm_mullo_epi16(ga, scale_x2);
        ga = _mm_andnot_si128(_mm_set1_epi32(0x00ff00ff), ga);

        return _mm_adds_epu8(src, _mm_or_si128(rb, ga));
    }
#endif

#if defined(SK_ARM_HAS_NEON)
    #include <arm_neon.h>

    // SkMulDiv255Round() applied to each lane.
    static inline uint8x8_t SkMulDiv255Round_neon8(uint8x8_t x, uint8x8_t y) {
        uint16x8_t prod = vmull_u8(x, y);
        return vraddhn_u16(prod, vrshrq_n_u16(prod, 8));
    }

    static inline uint8x8x4_t SkPMSrcOver_neon8(uint8x8x4_t dst, uint8x8x4_t src) {
        uint8x8_t nalphas = vmvn_u8(src.val[3]);  // 256 - alpha
        return {
            vqadd_u8(src.val[0], SkMulDiv255Round_neon8(nalphas,  dst.val[0])),
            vqadd_u8(src.val[1], SkMulDiv255Round_neon8(nalphas,  dst.val[1])),
            vqadd_u8(src.val[2], SkMulDiv255Round_neon8(nalphas,  dst.val[2])),
            vqadd_u8(src.val[3], SkMulDiv255Round_neon8(nalphas,  dst.val[3])),
        };
    }

    // Variant assuming dst and src contain the color components of two consecutive pixels.
    static inline uint8x8_t SkPMSrcOver_neon2(uint8x8_t dst, uint8x8_t src) {
        const uint8x8_t alpha_indices = vcreate_u8(0x0707070703030303);
        uint8x8_t nalphas = vmvn_u8(vtbl1_u8(src, alpha_indices));
        return vqadd_u8(src, SkMulDiv255Round_neon8(nalphas, dst));
    }

#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    #include <lasxintrin.h>

    static inline __m256i SkPMSrcOver_LASX(const __m256i& src, const __m256i& dst) {
        __m256i val = __lasx_xvreplgr2vr_w(256);
        __m256i scale = __lasx_xvsub_w(val, __lasx_xvsrli_w(src, 24));
        __m256i scale_x2 = __lasx_xvor_v(__lasx_xvslli_w(scale, 16), scale);

        val = __lasx_xvreplgr2vr_w(0x00ff00ff);
        __m256i rb = __lasx_xvand_v(val, dst);
        rb = __lasx_xvmul_h(rb, scale_x2);
        rb = __lasx_xvsrli_h(rb, 8);

        __m256i ga = __lasx_xvsrli_h(dst, 8);
        ga = __lasx_xvmul_h(ga, scale_x2);
        ga = __lasx_xvandn_v(val, ga);

        return __lasx_xvsadd_bu(src, __lasx_xvor_v(rb, ga));
    }
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    #include <lsxintrin.h>

    static inline __m128i SkPMSrcOver_LSX(const __m128i& src, const __m128i& dst) {
        __m128i val = __lsx_vreplgr2vr_w(256);
        __m128i scale = __lsx_vsub_w(val, __lsx_vsrli_w(src, 24));
        __m128i scale_x2 = __lsx_vor_v(__lsx_vslli_w(scale, 16), scale);

        val = __lsx_vreplgr2vr_w(0x00ff00ff);
        __m128i rb = __lsx_vand_v(val, dst);
        rb = __lsx_vmul_h(rb, scale_x2);
        rb = __lsx_vsrli_h(rb, 8);

        __m128i ga = __lsx_vsrli_h(dst, 8);
        ga = __lsx_vmul_h(ga, scale_x2);
        ga = __lsx_vandn_v(val, ga);

        return __lsx_vsadd_bu(src, __lsx_vor_v(rb, ga));
    }
#endif

namespace SK_OPTS_NS {

/*not static*/
inline void blit_row_s32a_opaque(SkPMColor* dst, const SkPMColor* src, int len, U8CPU alpha) {
    SkASSERT(alpha == 0xFF);
    sk_msan_assert_initialized(src, src+len);

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    while (len >= 8) {
        _mm256_storeu_si256((__m256i*)dst,
                            SkPMSrcOver_AVX2(_mm256_loadu_si256((const __m256i*)src),
                                             _mm256_loadu_si256((const __m256i*)dst)));
        src += 8;
        dst += 8;
        len -= 8;
    }
#endif

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    while (len >= 4) {
        _mm_storeu_si128((__m128i*)dst, SkPMSrcOver_SSE2(_mm_loadu_si128((const __m128i*)src),
                                                         _mm_loadu_si128((const __m128i*)dst)));
        src += 4;
        dst += 4;
        len -= 4;
    }
#endif

#if defined(SK_ARM_HAS_NEON)
    while (len >= 8) {
        vst4_u8((uint8_t*)dst, SkPMSrcOver_neon8(vld4_u8((const uint8_t*)dst),
                                                 vld4_u8((const uint8_t*)src)));
        src += 8;
        dst += 8;
        len -= 8;
    }

    while (len >= 2) {
        vst1_u8((uint8_t*)dst, SkPMSrcOver_neon2(vld1_u8((const uint8_t*)dst),
                                                 vld1_u8((const uint8_t*)src)));
        src += 2;
        dst += 2;
        len -= 2;
    }

    if (len != 0) {
        uint8x8_t result = SkPMSrcOver_neon2(vcreate_u8((uint64_t)*dst),
                                             vcreate_u8((uint64_t)*src));
        vst1_lane_u32(dst, vreinterpret_u32_u8(result), 0);
    }
    return;
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
    while (len >= 8) {
        __lasx_xvst(SkPMSrcOver_LASX(__lasx_xvld(src, 0),
                                     __lasx_xvld(dst, 0)), (__m256i*)dst, 0);
        src += 8;
        dst += 8;
        len -= 8;
    }
#endif

#if SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
    while (len >= 4) {
        __lsx_vst(SkPMSrcOver_LSX(__lsx_vld(src, 0),
                                  __lsx_vld(dst, 0)), (__m128i*)dst, 0);
        src += 4;
        dst += 4;
        len -= 4;
    }
#endif

    while (len --> 0) {
        *dst = SkPMSrcOver(*src, *dst);
        src++;
        dst++;
    }
}

// Blend constant color over count dst pixels
/*not static*/
inline void blit_row_color32(SkPMColor* dst, int count, SkPMColor color) {
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
        U8 s = sk_bit_cast<U8>(src),
           a = U8(invA);
        U16 c = skvx::cast<uint16_t>(sk_bit_cast<U8>(U32(color))),
            d = (mull(s,a) + (c << 8) + 128)>>8;
        return sk_bit_cast<U32>(skvx::cast<uint8_t>(d));
    };

    while (count >= N) {
        kernel(U32::Load(dst)).store(dst);
        dst   += N;
        count -= N;
    }
    while (count --> 0) {
        *dst = kernel(U32{*dst})[0];
        dst++;
    }
}

}  // namespace SK_OPTS_NS

#endif//SkBlitRow_opts_DEFINED
