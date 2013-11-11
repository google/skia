/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkColorPriv.h"
#include "SkMorphology_opts.h"
#include "SkMorphology_opts_neon.h"

#include <arm_neon.h>

/* neon version of dilateX, dilateY, erodeX, erodeY.
 * portable versions are in src/effects/SkMorphologyImageFilter.cpp.
 */

enum MorphType {
    kDilate, kErode
};

enum MorphDirection {
    kX, kY
};

template<MorphType type, MorphDirection direction>
static void SkMorph_neon(const SkPMColor* src, SkPMColor* dst, int radius,
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
            uint8x8_t max = vdup_n_u8(type == kDilate ? 0 : 255);
            for (const SkPMColor* p = lp; p <= up; p += srcStrideX) {
                uint8x8_t src_pixel = vreinterpret_u8_u32(vdup_n_u32(*p));
                max = type == kDilate ? vmax_u8(src_pixel, max) : vmin_u8(src_pixel, max);
            }
            *dptr = vget_lane_u32(vreinterpret_u32_u8(max), 0);
            dptr += dstStrideY;
            lp += srcStrideY;
            up += srcStrideY;
        }
        if (x >= radius) src += srcStrideX;
        if (x + radius < width - 1) upperSrc += srcStrideX;
        dst += dstStrideX;
    }
}

void SkDilateX_neon(const SkPMColor* src, SkPMColor* dst, int radius,
                    int width, int height, int srcStride, int dstStride)
{
    SkMorph_neon<kDilate, kX>(src, dst, radius, width, height, srcStride, dstStride);
}

void SkErodeX_neon(const SkPMColor* src, SkPMColor* dst, int radius,
                   int width, int height, int srcStride, int dstStride)
{
    SkMorph_neon<kErode, kX>(src, dst, radius, width, height, srcStride, dstStride);
}

void SkDilateY_neon(const SkPMColor* src, SkPMColor* dst, int radius,
                    int width, int height, int srcStride, int dstStride)
{
    SkMorph_neon<kDilate, kY>(src, dst, radius, width, height, srcStride, dstStride);
}

void SkErodeY_neon(const SkPMColor* src, SkPMColor* dst, int radius,
                   int width, int height, int srcStride, int dstStride)
{
    SkMorph_neon<kErode, kY>(src, dst, radius, width, height, srcStride, dstStride);
}
