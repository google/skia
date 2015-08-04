/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurImageFilter_opts_DEFINED
#define SkBlurImageFilter_opts_DEFINED

#include "SkColorPriv.h"
#include "SkTypes.h"

namespace SK_OPTS_NS {

enum class BlurDirection { kX, kY };

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
template<BlurDirection srcDirection, BlurDirection dstDirection>
void box_blur(const SkPMColor* src, int srcStride, SkPMColor* dst, int kernelSize,
              int leftOffset, int rightOffset, int width, int height) {
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    // ARGB -> 000A 000R 000G 000B
    auto expand = [](int p) {
        return _mm_cvtepu8_epi32(_mm_cvtsi32_si128(p));
    };
    // Axxx Rxxx Gxxx Bxxx -> ARGB
    auto repack = [](__m128i p) {
        const char _ = ~0;  // Don't care what ends up in these bytes.  This zeros them.
        p = _mm_shuffle_epi8(p, _mm_set_epi8(_,_,_,_, _,_,_,_, _,_,_,_, 15,11,7,3));
        return _mm_cvtsi128_si32(p);
    };

#else
    // ARGB -> 000A 000R 000G 000B
    auto expand = [](int p) {
        auto result = _mm_cvtsi32_si128(p);
        result = _mm_unpacklo_epi8(result, _mm_setzero_si128());
        result = _mm_unpacklo_epi16(result, _mm_setzero_si128());
        return result;
    };
    // Axxx Rxxx Gxxx Bxxx -> ARGB
    auto repack = [](__m128i p) {
        p = _mm_srli_epi32(p, 24);  // 000A 000R 000G 000B
        p = _mm_packs_epi32(p, p);  // xxxx xxxx 0A0R 0G0B
        p = _mm_packus_epi16(p, p); // xxxx xxxx xxxx ARGB
        return _mm_cvtsi128_si32(p);
    };

    // _mm_mullo_epi32 is not available, so use the standard trick to emulate it.
    auto _mm_mullo_epi32 = [](__m128i a, __m128i b) {
        __m128i p02 = _mm_mul_epu32(a, b),
                p13 = _mm_mul_epu32(_mm_srli_si128(a, 4),
                                    _mm_srli_si128(b, 4));
        return _mm_unpacklo_epi32(_mm_shuffle_epi32(p02, _MM_SHUFFLE(0,0,2,0)),
                                  _mm_shuffle_epi32(p13, _MM_SHUFFLE(0,0,2,0)));
    };
#endif
    const int rightBorder = SkMin32(rightOffset + 1, width);
    const int srcStrideX = srcDirection == BlurDirection::kX ? 1 : srcStride;
    const int dstStrideX = dstDirection == BlurDirection::kX ? 1 : height;
    const int srcStrideY = srcDirection == BlurDirection::kX ? srcStride : 1;
    const int dstStrideY = dstDirection == BlurDirection::kX ? width : 1;
    const __m128i scale = _mm_set1_epi32((1 << 24) / kernelSize);
    const __m128i half = _mm_set1_epi32(1 << 23);
    for (int y = 0; y < height; ++y) {
        __m128i sum = _mm_setzero_si128();
        const SkPMColor* p = src;
        for (int i = 0; i < rightBorder; ++i) {
            sum = _mm_add_epi32(sum, expand(*p));
            p += srcStrideX;
        }

        const SkPMColor* sptr = src;
        SkColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            // TODO(mtklein): We are working in 8.24 here. Drop to 8.8 when the kernel is narrow?
            // Multiply each component by scale (divide by kernel size) and add half to round.
            auto result = _mm_mullo_epi32(sum, scale);
            result = _mm_add_epi32(result, half);

            // Now pack the top byte of each 32-bit lane back down into one 32-bit color.
            // Axxx Rxxx Gxxx Bxxx -> xxxx xxxx xxxx ARGB
            *dptr = repack(result);

            // TODO(mtklein): experiment with breaking this loop into 3 parts
            if (x >= leftOffset) {
                SkColor l = *(sptr - leftOffset * srcStrideX);
                sum = _mm_sub_epi32(sum, expand(l));
            }
            if (x + rightOffset + 1 < width) {
                SkColor r = *(sptr + (rightOffset + 1) * srcStrideX);
                sum = _mm_add_epi32(sum, expand(r));
            }
            sptr += srcStrideX;
            if (srcDirection == BlurDirection::kY) {
                // TODO(mtklein): experiment with moving this prefetch forward
                _mm_prefetch(reinterpret_cast<const char*>(sptr + (rightOffset + 1) * srcStrideX),
                             _MM_HINT_T0);
            }
            dptr += dstStrideX;
        }
        src += srcStrideY;
        dst += dstStrideY;
    }
}

#elif defined(SK_ARM_HAS_NEON)

// Fast path for kernel sizes between 2 and 127, working on two rows at a time.
template<BlurDirection srcDirection, BlurDirection dstDirection>
void box_blur_double(const SkPMColor** src, int srcStride, SkPMColor** dst, int kernelSize,
                     int leftOffset, int rightOffset, int width, int* height) {
    // Load 2 pixels from adjacent rows.
    auto load_2_pixels = [&](const SkPMColor* s) {
        if (srcDirection == BlurDirection::kX) {
            // 10% faster by adding these 2 prefetches
            SK_PREFETCH(s + 16);
            SK_PREFETCH(s + 16 + srcStride);
            auto one = vld1_lane_u32(s +         0, vdup_n_u32(0), 0),
                 two = vld1_lane_u32(s + srcStride,           one, 1);
            return vreinterpret_u8_u32(two);
        } else {
            return vld1_u8((uint8_t*)s);
        }
    };
    const int rightBorder = SkMin32(rightOffset + 1, width);
    const int srcStrideX = srcDirection == BlurDirection::kX ? 1 : srcStride;
    const int dstStrideX = dstDirection == BlurDirection::kX ? 1 : *height;
    const int srcStrideY = srcDirection == BlurDirection::kX ? srcStride : 1;
    const int dstStrideY = dstDirection == BlurDirection::kX ? width : 1;
    const uint16x8_t scale = vdupq_n_u16((1 << 15) / kernelSize);

    for (; *height >= 2; *height -= 2) {
        uint16x8_t sum = vdupq_n_u16(0);
        const SkPMColor* p = *src;
        for (int i = 0; i < rightBorder; i++) {
            sum = vaddw_u8(sum, load_2_pixels(p));
            p += srcStrideX;
        }

        const SkPMColor* sptr = *src;
        SkPMColor* dptr = *dst;
        for (int x = 0; x < width; x++) {
            // val = (sum * scale * 2 + 0x8000) >> 16
            uint16x8_t resultPixels = vreinterpretq_u16_s16(vqrdmulhq_s16(
                vreinterpretq_s16_u16(sum), vreinterpretq_s16_u16(scale)));
            if (dstDirection == BlurDirection::kX) {
                uint32x2_t px2 = vreinterpret_u32_u8(vmovn_u16(resultPixels));
                vst1_lane_u32(dptr +     0, px2, 0);
                vst1_lane_u32(dptr + width, px2, 1);
            } else {
                vst1_u8((uint8_t*)dptr, vmovn_u16(resultPixels));
            }

            if (x >= leftOffset) {
                sum = vsubw_u8(sum, load_2_pixels(sptr - leftOffset * srcStrideX));
            }
            if (x + rightOffset + 1 < width) {
                sum = vaddw_u8(sum, load_2_pixels(sptr + (rightOffset + 1) * srcStrideX));
            }
            sptr += srcStrideX;
            dptr += dstStrideX;
        }
        *src += srcStrideY * 2;
        *dst += dstStrideY * 2;
    }
}

template<BlurDirection srcDirection, BlurDirection dstDirection>
void box_blur(const SkPMColor* src, int srcStride, SkPMColor* dst, int kernelSize,
              int leftOffset, int rightOffset, int width, int height) {
    // ARGB -> 0A0R 0G0B
    auto expand = [](uint32_t p) {
        return vget_low_u16(vmovl_u8(vreinterpret_u8_u32(vdup_n_u32(p))));
    };
    const int rightBorder = SkMin32(rightOffset + 1, width);
    const int srcStrideX = srcDirection == BlurDirection::kX ? 1 : srcStride;
    const int dstStrideX = dstDirection == BlurDirection::kX ? 1 : height;
    const int srcStrideY = srcDirection == BlurDirection::kX ? srcStride : 1;
    const int dstStrideY = dstDirection == BlurDirection::kX ? width : 1;
    const uint32x4_t scale = vdupq_n_u32((1 << 24) / kernelSize);
    const uint32x4_t half = vdupq_n_u32(1 << 23);

    if (1 < kernelSize && kernelSize < 128) {
        box_blur_double<srcDirection, dstDirection>(&src, srcStride, &dst, kernelSize,
                                                    leftOffset, rightOffset, width, &height);
    }

    for (; height > 0; height--) {
        uint32x4_t sum = vdupq_n_u32(0);
        const SkPMColor* p = src;
        for (int i = 0; i < rightBorder; ++i) {
            sum = vaddw_u16(sum, expand(*p));
            p += srcStrideX;
        }

        const SkPMColor* sptr = src;
        SkPMColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            // ( half+sumA*scale half+sumR*scale half+sumG*scale half+sumB*scale )
            uint32x4_t result = vmlaq_u32(half, sum, scale);

            // Saturated conversion to 16-bit.
            // ( AAAA RRRR GGGG BBBB ) -> ( 0A 0R 0G 0B )
            uint16x4_t result16 = vqshrn_n_u32(result, 16);

            // Saturated conversion to 8-bit.
            // ( 0A 0R 0G 0B ) -> ( 0A 0R 0G 0B 0A 0R 0G 0B ) -> ( A R G B A R G B )
            uint8x8_t result8 = vqshrn_n_u16(vcombine_u16(result16, result16), 8);

            // ( A R G B A R G B ) -> ( ARGB ARGB ) -> ( ARGB )
            // Store low 32 bits to destination.
            vst1_lane_u32(dptr, vreinterpret_u32_u8(result8), 0);

            if (x >= leftOffset) {
                const SkPMColor* l = sptr - leftOffset * srcStrideX;
                sum = vsubw_u16(sum, expand(*l));
            }
            if (x + rightOffset + 1 < width) {
                const SkPMColor* r = sptr + (rightOffset + 1) * srcStrideX;
                sum = vaddw_u16(sum, expand(*r));
            }
            sptr += srcStrideX;
            if (srcDirection == BlurDirection::kX) {
                SK_PREFETCH(sptr + (rightOffset + 16) * srcStrideX);
            }
            dptr += dstStrideX;
        }
        src += srcStrideY;
        dst += dstStrideY;
    }
}

#else  // Neither NEON nor >=SSE2.

template<BlurDirection srcDirection, BlurDirection dstDirection>
static void box_blur(const SkPMColor* src, int srcStride, SkPMColor* dst, int kernelSize,
                     int leftOffset, int rightOffset, int width, int height) {
    int rightBorder = SkMin32(rightOffset + 1, width);
    int srcStrideX = srcDirection == BlurDirection::kX ? 1 : srcStride;
    int dstStrideX = dstDirection == BlurDirection::kX ? 1 : height;
    int srcStrideY = srcDirection == BlurDirection::kX ? srcStride : 1;
    int dstStrideY = dstDirection == BlurDirection::kX ? width : 1;
    uint32_t scale = (1 << 24) / kernelSize;
    uint32_t half = 1 << 23;
    for (int y = 0; y < height; ++y) {
        int sumA = 0, sumR = 0, sumG = 0, sumB = 0;
        const SkPMColor* p = src;
        for (int i = 0; i < rightBorder; ++i) {
            sumA += SkGetPackedA32(*p);
            sumR += SkGetPackedR32(*p);
            sumG += SkGetPackedG32(*p);
            sumB += SkGetPackedB32(*p);
            p += srcStrideX;
        }

        const SkPMColor* sptr = src;
        SkColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            *dptr = SkPackARGB32((sumA * scale + half) >> 24,
                                 (sumR * scale + half) >> 24,
                                 (sumG * scale + half) >> 24,
                                 (sumB * scale + half) >> 24);
            if (x >= leftOffset) {
                SkColor l = *(sptr - leftOffset * srcStrideX);
                sumA -= SkGetPackedA32(l);
                sumR -= SkGetPackedR32(l);
                sumG -= SkGetPackedG32(l);
                sumB -= SkGetPackedB32(l);
            }
            if (x + rightOffset + 1 < width) {
                SkColor r = *(sptr + (rightOffset + 1) * srcStrideX);
                sumA += SkGetPackedA32(r);
                sumR += SkGetPackedR32(r);
                sumG += SkGetPackedG32(r);
                sumB += SkGetPackedB32(r);
            }
            sptr += srcStrideX;
            if (srcDirection == BlurDirection::kY) {
                SK_PREFETCH(sptr + (rightOffset + 1) * srcStrideX);
            }
            dptr += dstStrideX;
        }
        src += srcStrideY;
        dst += dstStrideY;
    }
}

#endif

static auto box_blur_xx = &box_blur<BlurDirection::kX, BlurDirection::kX>,
            box_blur_xy = &box_blur<BlurDirection::kX, BlurDirection::kY>,
            box_blur_yx = &box_blur<BlurDirection::kY, BlurDirection::kX>;

}  // namespace SK_OPTS_NS

#endif
