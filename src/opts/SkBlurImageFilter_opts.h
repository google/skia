/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurImageFilter_opts_DEFINED
#define SkBlurImageFilter_opts_DEFINED

#include "SkColorPriv.h"
#include "SkRect.h"

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include <immintrin.h>
#endif

namespace SK_OPTS_NS {

enum class BlurDirection { kX, kY };

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
// ARGB -> 000A 000R 000G 000B
static inline __m128i expand(SkPMColor p) {
    return _mm_cvtepu8_epi32(_mm_cvtsi32_si128(p));
};
// Axxx Rxxx Gxxx Bxxx -> ARGB
static inline SkPMColor repack(__m128i p) {
    const char _ = ~0;  // Don't care what ends up in these bytes.  This zeros them.
    p = _mm_shuffle_epi8(p, _mm_set_epi8(_,_,_,_, _,_,_,_, _,_,_,_, 15,11,7,3));
    return _mm_cvtsi128_si32(p);
};
#define mullo_epi32 _mm_mullo_epi32

#else
// ARGB -> 000A 000R 000G 000B
static inline __m128i expand(int p) {
    auto result = _mm_cvtsi32_si128(p);
    result = _mm_unpacklo_epi8(result, _mm_setzero_si128());
    result = _mm_unpacklo_epi16(result, _mm_setzero_si128());
    return result;
};
// Axxx Rxxx Gxxx Bxxx -> ARGB
static inline SkPMColor repack(__m128i p) {
    p = _mm_srli_epi32(p, 24);  // 000A 000R 000G 000B
    p = _mm_packs_epi32(p, p);  // xxxx xxxx 0A0R 0G0B
    p = _mm_packus_epi16(p, p); // xxxx xxxx xxxx ARGB
    return _mm_cvtsi128_si32(p);
};

// _mm_mullo_epi32 is not available, so use the standard trick to emulate it.
static inline __m128i mullo_epi32(__m128i a, __m128i b) {
    __m128i p02 = _mm_mul_epu32(a, b),
            p13 = _mm_mul_epu32(_mm_srli_si128(a, 4),
                                _mm_srli_si128(b, 4));
    return _mm_unpacklo_epi32(_mm_shuffle_epi32(p02, _MM_SHUFFLE(0,0,2,0)),
                              _mm_shuffle_epi32(p13, _MM_SHUFFLE(0,0,2,0)));
};
#endif
#define INIT_SCALE const __m128i scale = _mm_set1_epi32((1 << 24) / kernelSize);
#define INIT_HALF const __m128i half = _mm_set1_epi32(1 << 23);
#define INIT_SUMS __m128i sum = _mm_setzero_si128();
#define INCREMENT_SUMS(c) sum = _mm_add_epi32(sum, expand(c))
#define DECREMENT_SUMS(c) sum = _mm_sub_epi32(sum, expand(c))
#define STORE_SUMS \
    auto result = mullo_epi32(sum, scale); \
    result = _mm_add_epi32(result, half); \
    *dptr = repack(result);
#define DOUBLE_ROW_OPTIMIZATION

#elif defined(SK_ARM_HAS_NEON)

// val = (sum * scale * 2 + 0x8000) >> 16
#define STORE_SUMS_DOUBLE \
    uint16x8_t resultPixels = vreinterpretq_u16_s16(vqrdmulhq_s16( \
        vreinterpretq_s16_u16(sum), vreinterpretq_s16_u16(scale))); \
    if (dstDirection == BlurDirection::kX) { \
        uint32x2_t px2 = vreinterpret_u32_u8(vmovn_u16(resultPixels)); \
        vst1_lane_u32(dptr +     0, px2, 0); \
        vst1_lane_u32(dptr + width, px2, 1); \
    } else { \
        vst1_u8((uint8_t*)dptr, vmovn_u16(resultPixels)); \
    }

#define INCREMENT_SUMS_DOUBLE(p) sum = vaddw_u8(sum, load_2_pixels(p))
#define DECREMENT_SUMS_DOUBLE(p) sum = vsubw_u8(sum, load_2_pixels(p))

// Fast path for kernel sizes between 2 and 127, working on two rows at a time.
template<BlurDirection srcDirection, BlurDirection dstDirection>
static int box_blur_double(const SkPMColor** src, int srcStride, const SkIRect& srcBounds,
                           SkPMColor** dst, int kernelSize,
                           int leftOffset, int rightOffset, int width, int height) {
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
    int left = srcBounds.left();
    int right = srcBounds.right();
    int top = srcBounds.top();
    int bottom = srcBounds.bottom();
    int incrementStart = SkMax32(left - rightOffset - 1, left - right);
    int incrementEnd = SkMax32(right - rightOffset - 1, 0);
    int decrementStart = SkMin32(left + leftOffset, width);
    int decrementEnd = SkMin32(right + leftOffset, width);
    const int srcStrideX = srcDirection == BlurDirection::kX ? 1 : srcStride;
    const int dstStrideX = dstDirection == BlurDirection::kX ? 1 : height;
    const int srcStrideY = srcDirection == BlurDirection::kX ? srcStride : 1;
    const int dstStrideY = dstDirection == BlurDirection::kX ? width : 1;
    const uint16x8_t scale = vdupq_n_u16((1 << 15) / kernelSize);

    for (; bottom - top >= 2; top += 2) {
        uint16x8_t sum = vdupq_n_u16(0);
        const SkPMColor* lptr = *src;
        const SkPMColor* rptr = *src;
        SkPMColor* dptr = *dst;
        int x;
        for (x = incrementStart; x < 0; ++x) {
            INCREMENT_SUMS_DOUBLE(rptr);
            rptr += srcStrideX;
        }
        // Clear to zero when sampling to the left our domain. "sum" is zero here because we
        // initialized it above, and the preceeding loop has no effect in this case.
        for (x = 0; x < incrementStart; ++x) {
            STORE_SUMS_DOUBLE
            dptr += dstStrideX;
        }
        for (; x < decrementStart && x < incrementEnd; ++x) {
            STORE_SUMS_DOUBLE
            dptr += dstStrideX;
            INCREMENT_SUMS_DOUBLE(rptr);
            rptr += srcStrideX;
        }
        for (x = decrementStart; x < incrementEnd; ++x) {
            STORE_SUMS_DOUBLE
            dptr += dstStrideX;
            INCREMENT_SUMS_DOUBLE(rptr);
            rptr += srcStrideX;
            DECREMENT_SUMS_DOUBLE(lptr);
            lptr += srcStrideX;
        }
        for (x = incrementEnd; x < decrementStart; ++x) {
            STORE_SUMS_DOUBLE
            dptr += dstStrideX;
        }
        for (; x < decrementEnd; ++x) {
            STORE_SUMS_DOUBLE
            dptr += dstStrideX;
            DECREMENT_SUMS_DOUBLE(lptr);
            lptr += srcStrideX;
        }
        // Clear to zero when sampling to the right of our domain. "sum" is zero here because we
        // added on then subtracted off all of the pixels, leaving zero.
        for (; x < width; ++x) {
            STORE_SUMS_DOUBLE
            dptr += dstStrideX;
        }
        *src += srcStrideY * 2;
        *dst += dstStrideY * 2;
    }
    return top;
}

// ARGB -> 0A0R 0G0B
static inline uint16x4_t expand(SkPMColor p) {
    return vget_low_u16(vmovl_u8(vreinterpret_u8_u32(vdup_n_u32(p))));
};

#define INIT_SCALE const uint32x4_t scale = vdupq_n_u32((1 << 24) / kernelSize);
#define INIT_HALF const uint32x4_t half = vdupq_n_u32(1 << 23);
#define INIT_SUMS uint32x4_t sum = vdupq_n_u32(0);
#define INCREMENT_SUMS(c) sum = vaddw_u16(sum, expand(c));
#define DECREMENT_SUMS(c) sum = vsubw_u16(sum, expand(c));

#define STORE_SUMS \
    uint32x4_t result = vmlaq_u32(half, sum, scale); \
    uint16x4_t result16 = vqshrn_n_u32(result, 16); \
    uint8x8_t result8 = vqshrn_n_u16(vcombine_u16(result16, result16), 8); \
    vst1_lane_u32(dptr, vreinterpret_u32_u8(result8), 0);

#define DOUBLE_ROW_OPTIMIZATION \
    if (1 < kernelSize && kernelSize < 128) { \
        top = box_blur_double<srcDirection, dstDirection>(&src, srcStride, srcBounds, &dst, \
                                                          kernelSize, leftOffset, rightOffset, \
                                                          width, height); \
    }

#else  // Neither NEON nor >=SSE2.

#define INIT_SCALE uint32_t scale = (1 << 24) / kernelSize;
#define INIT_HALF  uint32_t half = 1 << 23;
#define INIT_SUMS int sumA = 0, sumR = 0, sumG = 0, sumB = 0;
#define INCREMENT_SUMS(c) \
    sumA += SkGetPackedA32(c); \
    sumR += SkGetPackedR32(c); \
    sumG += SkGetPackedG32(c); \
    sumB += SkGetPackedB32(c)
#define DECREMENT_SUMS(c) \
    sumA -= SkGetPackedA32(c); \
    sumR -= SkGetPackedR32(c); \
    sumG -= SkGetPackedG32(c); \
    sumB -= SkGetPackedB32(c)
#define STORE_SUMS \
    *dptr = SkPackARGB32((sumA * scale + half) >> 24, \
                         (sumR * scale + half) >> 24, \
                         (sumG * scale + half) >> 24, \
                         (sumB * scale + half) >> 24);
#define DOUBLE_ROW_OPTIMIZATION

#endif

#define PREFETCH_RPTR \
    if (srcDirection == BlurDirection::kY) { \
        SK_PREFETCH(rptr); \
    }

template<BlurDirection srcDirection, BlurDirection dstDirection>
static void box_blur(const SkPMColor* src, int srcStride, const SkIRect& srcBounds, SkPMColor* dst,
                     int kernelSize, int leftOffset, int rightOffset, int width, int height) {
    int left = srcBounds.left();
    int right = srcBounds.right();
    int top = srcBounds.top();
    int bottom = srcBounds.bottom();
    int incrementStart = SkMax32(left - rightOffset - 1, left - right);
    int incrementEnd = SkMax32(right - rightOffset - 1, 0);
    int decrementStart = SkMin32(left + leftOffset, width);
    int decrementEnd = SkMin32(right + leftOffset, width);
    int srcStrideX = srcDirection == BlurDirection::kX ? 1 : srcStride;
    int dstStrideX = dstDirection == BlurDirection::kX ? 1 : height;
    int srcStrideY = srcDirection == BlurDirection::kX ? srcStride : 1;
    int dstStrideY = dstDirection == BlurDirection::kX ? width : 1;
    INIT_SCALE
    INIT_HALF

    // Clear to zero when sampling above our domain.
    for (int y = 0; y < top; y++) {
        SkColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            *dptr = 0;
            dptr += dstStrideX;
        }
        dst += dstStrideY;
    }

    DOUBLE_ROW_OPTIMIZATION

    for (int y = top; y < bottom; ++y) {
        INIT_SUMS
        const SkPMColor* lptr = src;
        const SkPMColor* rptr = src;
        SkColor* dptr = dst;
        int x;
        for (x = incrementStart; x < 0; ++x) {
            INCREMENT_SUMS(*rptr);
            rptr += srcStrideX;
            PREFETCH_RPTR
        }
        // Clear to zero when sampling to the left of our domain.
        for (x = 0; x < incrementStart; ++x) {
            *dptr = 0;
            dptr += dstStrideX;
        }
        for (; x < decrementStart && x < incrementEnd; ++x) {
            STORE_SUMS
            dptr += dstStrideX;
            INCREMENT_SUMS(*rptr);
            rptr += srcStrideX;
            PREFETCH_RPTR
        }
        for (x = decrementStart; x < incrementEnd; ++x) {
            STORE_SUMS
            dptr += dstStrideX;
            INCREMENT_SUMS(*rptr);
            rptr += srcStrideX;
            PREFETCH_RPTR
            DECREMENT_SUMS(*lptr);
            lptr += srcStrideX;
        }
        for (x = incrementEnd; x < decrementStart; ++x) {
            STORE_SUMS
            dptr += dstStrideX;
        }
        for (; x < decrementEnd; ++x) {
            STORE_SUMS
            dptr += dstStrideX;
            DECREMENT_SUMS(*lptr);
            lptr += srcStrideX;
        }
        // Clear to zero when sampling to the right of our domain.
        for (; x < width; ++x) {
            *dptr = 0;
            dptr += dstStrideX;
        }
        src += srcStrideY;
        dst += dstStrideY;
    }
    // Clear to zero when sampling below our domain.
    for (int y = bottom; y < height; ++y) {
        SkColor* dptr = dst;
        for (int x = 0; x < width; ++x) {
            *dptr = 0;
            dptr += dstStrideX;
        }
        dst += dstStrideY;
    }
}

static auto box_blur_xx = &box_blur<BlurDirection::kX, BlurDirection::kX>,
            box_blur_xy = &box_blur<BlurDirection::kX, BlurDirection::kY>,
            box_blur_yx = &box_blur<BlurDirection::kY, BlurDirection::kX>;

}  // namespace SK_OPTS_NS

#endif
