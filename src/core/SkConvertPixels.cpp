/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpacePriv.h"
#include "SkConvertPixels.h"
#include "SkHalf.h"
#include "SkImageInfoPriv.h"
#include "SkOpts.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkUnPreMultiply.h"
#include "SkUnPreMultiplyPriv.h"
#include "../jumper/SkJumper.h"

static bool rect_memcpy(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                        const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                        const SkColorSpaceXformSteps& steps) {
    // We can copy the pixels unchanged if no color type,
    // alpha type, or color space conversion is needed.
    if (dstInfo().colorType() != srcInfo.colorType()
            || steps.flags.mask() != 0b00000) {
        return false;
    }

    SkRectMemcpy(dstPixels, dstRB,
                 srcPixels, srcRB, dstInfo.minRowBytes(), dstInfo.height());
    return true;
}

template <bool kSwapRB>
static void wrap_unpremultiply(uint32_t* dst, const void* src, int count) {
    SkUnpremultiplyRow<kSwapRB>(dst, (const uint32_t*) src, count);
}

bool swizzle_and_multiply(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                          const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                          const SkColorSpaceXformSteps& steps) {

    auto is_8888 = [](SkColorType ct) {
        return ct == kRGBA_8888_SkColorType || ct == kBGRA_8888_SkColorType;
    };

    if (!is_8888(dstInfo) ||
        !is_8888(srcInfo) ||
        steps.flags.linearize ||
        steps.flags.gamut_transform ||
        steps.flags.encode) {
        return false;
    }

    SkASSERT(!(steps.premul && steps.unpremul));

    void (*proc)(uint32_t* dst, const void* src, int count) = nullptr;

    const bool swapRB = dstInfo.colorType() != srcInfo.colorType();

    AlphaVerb alphaVerb = kNothing_AlphaVerb;
    if (kPremul_SkAlphaType == dstInfo.alphaType() &&
        kUnpremul_SkAlphaType == srcInfo.alphaType())
    {
        alphaVerb = kPremul_AlphaVerb;
    } else if (kUnpremul_SkAlphaType == dstInfo.alphaType() &&
               kPremul_SkAlphaType == srcInfo.alphaType()) {
        alphaVerb = kUnpremul_AlphaVerb;
    }

    switch (alphaVerb) {
        case kNothing_AlphaVerb:
            // If we do not need to swap or multiply, we should hit the memcpy case.
            SkASSERT(swapRB);
            proc = SkOpts::RGBA_to_BGRA;
            break;
        case kPremul_AlphaVerb:
            proc = swapRB ? SkOpts::RGBA_to_bgrA : SkOpts::RGBA_to_rgbA;
            break;
        case kUnpremul_AlphaVerb:
            proc = swapRB ? wrap_unpremultiply<true> : wrap_unpremultiply<false>;
            break;
    }

    for (int y = 0; y < dstInfo.height(); y++) {
        proc((uint32_t*) dstPixels, srcPixels, dstInfo.width());
        dstPixels = SkTAddOffset<void>(dstPixels, dstRB);
        srcPixels = SkTAddOffset<const void>(srcPixels, srcRB);
    }
}

static void convert_to_alpha8(uint8_t*    dst, size_t dstRB, const SkImageInfo& srcInfo,
                              const void* src, size_t srcRB) {
    if (srcInfo.isOpaque()) {
        for (int y = 0; y < srcInfo.height(); ++y) {
           memset(dst, 0xFF, srcInfo.width());
           dst = SkTAddOffset<uint8_t>(dst, dstRB);
        }
        return;
    }

    switch (srcInfo.colorType()) {
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
            break;
        }
        case kRGBA_1010102_SkColorType: {
            auto src32 = (const uint32_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    switch (src32[x] >> 30) {
                        case 0:
                            dst[x] = 0;
                            break;
                        case 1:
                            dst[x] = 0x55;
                            break;
                        case 2:
                            dst[x] = 0xAA;
                            break;
                        case 3:
                            dst[x] = 0xFF;
                            break;
                    }
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src32 = SkTAddOffset<const uint32_t>(src32, srcRB);
            }
            break;
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
            break;
        }
        case kRGBA_F16_SkColorType: {
            auto src64 = (const uint64_t*) src;
            for (int y = 0; y < srcInfo.height(); y++) {
                for (int x = 0; x < srcInfo.width(); x++) {
                    dst[x] = (uint8_t) (255.0f * SkHalfToFloat(src64[x] >> 48));
                }
                dst = SkTAddOffset<uint8_t>(dst, dstRB);
                src64 = SkTAddOffset<const uint64_t>(src64, srcRB);
            }
            break;
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
        } break;
        default:
            SkASSERT(false);
            break;
    }
}

// Default: Use the pipeline.
static void convert_with_pipeline(const SkImageInfo& dstInfo, void* dstRow, size_t dstRB,
                                  const SkImageInfo& srcInfo, const void* srcRow, size_t srcRB,
                                  const SkColorSpaceXformSteps& steps) {

    SkJumper_MemoryCtx src = { (void*)srcRow, (int)(srcRB / srcInfo.bytesPerPixel()) },
                       dst = { (void*)dstRow, (int)(dstRB / dstInfo.bytesPerPixel()) };

    SkRasterPipeline_<256> pipeline;
    pipeline.append_load(srcInfo.colorType(), &src);

    steps.apply(&pipeline);

    // We'll dither if we're decreasing precision below 32-bit.
    float dither_rate = 0.0f;
    if (srcInfo.bytesPerPixel() > dstInfo.bytesPerPixel()) {
        switch (dstInfo.colorType()) {
            case   kRGB_565_SkColorType: dither_rate = 1/63.0f; break;
            case kARGB_4444_SkColorType: dither_rate = 1/15.0f; break;
            default:                     dither_rate =    0.0f; break;
        }
    }
    if (dither_rate > 0) {
        pipeline.append(SkRasterPipeline::dither, &dither_rate);
    }

    pipeline.append_store(dstInfo.colorType(), &dst);
    pipeline.run(0,0, srcInfo.width(), srcInfo.height());
}

void SkConvertPixels(const SkImageInfo& dstInfo,       void* dstPixels, size_t dstRB,
                     const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB) {
    SkASSERT(dstInfo.dimensions() == srcInfo.dimensions());
    SkASSERT(SkImageInfoValidConversion(dstInfo, srcInfo));

    SkColorSpaceXformSteps steps{srcInfo.colorSpace(), srcInfo.alphaType(),
                                 dstInfo.colorSpace(), dstInfo.alphaType()};

    if (srcInfo.colorType() == dstInfo.colorType()
            && steps.flags.mask() == 0b00000) {
        return;
    }

    if (rect_memcpy         (dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, steps)) {
        return;
    }
    if (swizzle_and_multiply(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, steps)) {
        return;
    }
    if (convert_to_alpha8   (dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, steps)) {
        return;
    }
    convert_with_pipeline   (dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, steps);
}
