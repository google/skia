/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMorphologyImageFilter_opts_DEFINED
#define SkMorphologyImageFilter_opts_DEFINED

#include "SkColor.h"

namespace SK_OPTS_NS {

enum MorphType { kDilate, kErode };
enum class MorphDirection { kX, kY };

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
template<MorphType type, MorphDirection direction>
static void morph(const SkPMColor* src, SkPMColor* dst,
                  int radius, int width, int height, int srcStride, int dstStride) {
    const int srcStrideX = direction == MorphDirection::kX ? 1 : srcStride;
    const int dstStrideX = direction == MorphDirection::kX ? 1 : dstStride;
    const int srcStrideY = direction == MorphDirection::kX ? srcStride : 1;
    const int dstStrideY = direction == MorphDirection::kX ? dstStride : 1;
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            __m128i extreme = (type == kDilate) ? _mm_setzero_si128()
                                                : _mm_set1_epi32(0xFFFFFFFF);
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                __m128i src_pixel = _mm_cvtsi32_si128(*p);
                extreme = (type == kDilate) ? _mm_max_epu8(src_pixel, extreme)
                                            : _mm_min_epu8(src_pixel, extreme);
            }
            *dptr = _mm_cvtsi128_si32(extreme);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) { src += srcStrideX; }
        if (x + radius < width - 1) { upperSrc += srcStrideX; }
        dst += dstStrideX;
    }
}

#elif defined(SK_ARM_HAS_NEON)
template<MorphType type, MorphDirection direction>
static void morph(const SkPMColor* src, SkPMColor* dst,
                  int radius, int width, int height, int srcStride, int dstStride) {
    const int srcStrideX = direction == MorphDirection::kX ? 1 : srcStride;
    const int dstStrideX = direction == MorphDirection::kX ? 1 : dstStride;
    const int srcStrideY = direction == MorphDirection::kX ? srcStride : 1;
    const int dstStrideY = direction == MorphDirection::kX ? dstStride : 1;
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            uint8x8_t extreme = vdup_n_u8(type == kDilate ? 0 : 255);
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                uint8x8_t src_pixel = vreinterpret_u8_u32(vdup_n_u32(*p));
                extreme = (type == kDilate) ? vmax_u8(src_pixel, extreme)
                                            : vmin_u8(src_pixel, extreme);
            }
            *dptr = vget_lane_u32(vreinterpret_u32_u8(extreme), 0);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) src += srcStrideX;
        if (x + radius < width - 1) upperSrc += srcStrideX;
        dst += dstStrideX;
    }
}

#else
template<MorphType type, MorphDirection direction>
static void morph(const SkPMColor* src, SkPMColor* dst,
                  int radius, int width, int height, int srcStride, int dstStride) {
    const int srcStrideX = direction == MorphDirection::kX ? 1 : srcStride;
    const int dstStrideX = direction == MorphDirection::kX ? 1 : dstStride;
    const int srcStrideY = direction == MorphDirection::kX ? srcStride : 1;
    const int dstStrideY = direction == MorphDirection::kX ? dstStride : 1;
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            // If we're maxing (dilate), start from 0; if minning (erode), start from 255.
            const int start = (type == kDilate) ? 0 : 255;
            int B = start, G = start, R = start, A = start;
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                int b = SkGetPackedB32(*p),
                    g = SkGetPackedG32(*p),
                    r = SkGetPackedR32(*p),
                    a = SkGetPackedA32(*p);
                if (type == kDilate) {
                    B = SkTMax(b, B);
                    G = SkTMax(g, G);
                    R = SkTMax(r, R);
                    A = SkTMax(a, A);
                } else {
                    B = SkTMin(b, B);
                    G = SkTMin(g, G);
                    R = SkTMin(r, R);
                    A = SkTMin(a, A);
                }
            }
            *dptr = SkPackARGB32(A, R, G, B);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) { src += srcStrideX; }
        if (x + radius < width - 1) { upperSrc += srcStrideX; }
        dst += dstStrideX;
    }
}

#endif

static auto dilate_x = &morph<kDilate, MorphDirection::kX>,
            dilate_y = &morph<kDilate, MorphDirection::kY>,
             erode_x = &morph<kErode,  MorphDirection::kX>,
             erode_y = &morph<kErode,  MorphDirection::kY>;

}  // namespace SK_OPTS_NS

#endif//SkMorphologyImageFilter_opts_DEFINED

