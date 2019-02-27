/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorData.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXformSteps.h"
#include "SkConvertPixels.h"
#include "SkHalf.h"
#include "SkImageInfoPriv.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"

static bool rect_memcpy(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                        const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                        const SkColorSpaceXformSteps& steps) {
    // We can copy the pixels when no color type, alpha type, or color space changes.
    if (dstInfo.colorType() != srcInfo.colorType()) {
        return false;
    }
    if (dstInfo.colorType() != kAlpha_8_SkColorType
            && steps.flags.mask() != 0b00000) {
        return false;
    }

    SkRectMemcpy(dstPixels, dstRB,
                 srcPixels, srcRB, dstInfo.minRowBytes(), dstInfo.height());
    return true;
}

static bool swizzle_or_premul(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                              const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                              const SkColorSpaceXformSteps& steps) {
    auto is_8888 = [](SkColorType ct) {
        return ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType;
    };
    if (!is_8888(dstInfo.colorType()) ||
        !is_8888(srcInfo.colorType()) ||
        steps.flags.linearize         ||
        steps.flags.gamut_transform   ||
        steps.flags.unpremul          ||
        steps.flags.encode) {
        return false;
    }

    const bool swapRB = dstInfo.colorType() != srcInfo.colorType();

    void (*fn)(uint32_t*, const uint32_t*, int) = nullptr;

    if (steps.flags.premul) {
        fn = swapRB ? SkOpts::RGBA_to_bgrA
                    : SkOpts::RGBA_to_rgbA;
    } else {
        // If we're not swizzling, we ought to have used rect_memcpy().
        SkASSERT(swapRB);
        fn = SkOpts::RGBA_to_BGRA;
    }

    for (int y = 0; y < dstInfo.height(); y++) {
        fn((uint32_t*)dstPixels, (const uint32_t*)srcPixels, dstInfo.width());
        dstPixels = SkTAddOffset<void>(dstPixels, dstRB);
        srcPixels = SkTAddOffset<const void>(srcPixels, srcRB);
    }
    return true;
}

static bool convert_to_alpha8(const SkImageInfo& dstInfo,       void* vdst, size_t dstRB,
                              const SkImageInfo& srcInfo, const void*  src, size_t srcRB,
                              const SkColorSpaceXformSteps&) {
    if (dstInfo.colorType() != kAlpha_8_SkColorType) {
        return false;
    }
    auto dst = (uint8_t*)vdst;

    switch (srcInfo.colorType()) {
        case kUnknown_SkColorType:
        case kAlpha_8_SkColorType: {
            // Unknown should never happen.
            // Alpha8 should have been handled by rect_memcpy().
            SkASSERT(false);
            return false;
        }

        case kGray_8_SkColorType:
        case kRGB_565_SkColorType:
        case kRGB_888x_SkColorType:
        case kRGB_101010x_SkColorType: {
            for (int y = 0; y < srcInfo.height(); ++y) {
               memset(dst, 0xFF, srcInfo.width());
               dst = SkTAddOffset<uint8_t>(dst, dstRB);
            }
            return true;
        }

        case kARGB_4444_SkColorType: {
            auto src16 = (const uint16_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = SkPacked4444ToA32(src16[x]);
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src16 = SkTAddOffset<const uint16_t>(src16, srcRB);
            }
            return true;
        }

        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType: {
            auto src32 = (const uint32_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = src32[x] >> 24;
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src32 = SkTAddOffset<const uint32_t>(src32, srcRB);
            }
            return true;
        }

        case kRGBA_1010102_SkColorType: {
            auto src32 = (const uint32_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (src32[x] >> 30) * 0x55;
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src32 = SkTAddOffset<const uint32_t>(src32, srcRB);
            }
            return true;
        }

        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType: {
            auto src64 = (const uint64_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (uint8_t) (255.0f * SkHalfToFloat(src64[x] >> 48));
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src64 = SkTAddOffset<const uint64_t>(src64, srcRB);
            }
            return true;
        }

        case kRGBA_F32_SkColorType: {
            auto rgba = (const float*)src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (uint8_t)(255.0f * rgba[4*x+3]);
                }
                dst  = SkTAddOffset<uint8_t>(dst, dstRB);
                rgba = SkTAddOffset<const float>(rgba, srcRB);
            }
            return true;
        }
    }
    return false;
}

// Default: Use the pipeline.
static void convert_with_pipeline(const SkImageInfo& dstInfo, void* dstRow, size_t dstRB,
                                  const SkImageInfo& srcInfo, const void* srcRow, size_t srcRB,
                                  const SkColorSpaceXformSteps& steps) {

    SkRasterPipeline_MemoryCtx src = { (void*)srcRow, (int)(srcRB / srcInfo.bytesPerPixel()) },
                               dst = { (void*)dstRow, (int)(dstRB / dstInfo.bytesPerPixel()) };

    SkRasterPipeline_<256> pipeline;
    pipeline.append_load(srcInfo.colorType(), &src);
    steps.apply(&pipeline, srcInfo.colorType());

    pipeline.append_gamut_clamp_if_normalized(dstInfo);

    pipeline.append_store(dstInfo.colorType(), &dst);
    pipeline.run(0,0, srcInfo.width(), srcInfo.height());
}

void SkConvertPixels(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                     const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB) {
    SkASSERT(dstInfo.dimensions() == srcInfo.dimensions());
    SkASSERT(SkImageInfoValidConversion(dstInfo, srcInfo));

    SkColorSpaceXformSteps steps{srcInfo.colorSpace(), srcInfo.alphaType(),
                                 dstInfo.colorSpace(), dstInfo.alphaType()};

    for (auto fn : {rect_memcpy, swizzle_or_premul, convert_to_alpha8}) {
        if (fn(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, steps)) {
            return;
        }
    }
    convert_with_pipeline(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, steps);
}
