/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emmintrin.h>
#include "SkColorPriv.h"
#include "SkMorphology_opts_SSE2.h"

/* SSE2 version of dilateX, dilateY, erodeX, erodeY.
 * portable versions are in src/effects/SkMorphologyImageFilter.cpp.
 */

enum MorphType {
    kDilate, kErode
};

enum MorphDirection {
    kX, kY
};

template<MorphType type, MorphDirection direction>
static void SkMorph_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                         int width, int height, int srcStride, int dstStride)
{
    const int srcStrideX = direction == kX ? 1 : srcStride;
    const int dstStrideX = direction == kX ? 1 : dstStride;
    const int srcStrideY = direction == kX ? srcStride : 1;
    const int dstStrideY = direction == kX ? dstStride : 1;
    radius = SkMin32(radius, width - 1);
    const SkPMColor* upperSrc = src + radius * srcStrideX;
    for (int x = 0; x < width; ++x) {
        const SkPMColor* lp = src;
        const SkPMColor* up = upperSrc;
        SkPMColor* dptr = dst;
        for (int y = 0; y < height; ++y) {
            __m128i max = type == kDilate ? _mm_setzero_si128() : _mm_set1_epi32(0xFFFFFFFF);
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                __m128i src_pixel = _mm_cvtsi32_si128(*p);
                max = type == kDilate ? _mm_max_epu8(src_pixel, max) : _mm_min_epu8(src_pixel, max);
            }
            *dptr = _mm_cvtsi128_si32(max);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) {
            src += srcStrideX;
        }
        if (x + radius < width - 1) {
            upperSrc += srcStrideX;
        }
        dst += dstStrideX;
    }
}

void SkDilateX_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                    int width, int height, int srcStride, int dstStride)
{
    SkMorph_SSE2<kDilate, kX>(src, dst, radius, width, height, srcStride, dstStride);
}

void SkErodeX_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                   int width, int height, int srcStride, int dstStride)
{
    SkMorph_SSE2<kErode, kX>(src, dst, radius, width, height, srcStride, dstStride);
}

void SkDilateY_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                    int width, int height, int srcStride, int dstStride)
{
    SkMorph_SSE2<kDilate, kY>(src, dst, radius, width, height, srcStride, dstStride);
}

void SkErodeY_SSE2(const SkPMColor* src, SkPMColor* dst, int radius,
                   int width, int height, int srcStride, int dstStride)
{
    SkMorph_SSE2<kErode, kY>(src, dst, radius, width, height, srcStride, dstStride);
}
