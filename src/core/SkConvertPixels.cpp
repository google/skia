/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformPriv.h"
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

// Fast Path 1: The memcpy() case.
static inline bool can_memcpy(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo) {
    if (dstInfo.colorType() != srcInfo.colorType()) {
        return false;
    }

    if (kAlpha_8_SkColorType == dstInfo.colorType()) {
        return true;
    }

    if (dstInfo.alphaType() != srcInfo.alphaType() &&
        kOpaque_SkAlphaType != dstInfo.alphaType() &&
        kOpaque_SkAlphaType != srcInfo.alphaType())
    {
        // We need to premultiply or unpremultiply.
        return false;
    }

    return !dstInfo.colorSpace() ||
           SkColorSpace::Equals(dstInfo.colorSpace(), srcInfo.colorSpace());
}

// Fast Path 2: Simple swizzles and premuls.
enum AlphaVerb {
    kNothing_AlphaVerb,
    kPremul_AlphaVerb,
    kUnpremul_AlphaVerb,
};

template <bool kSwapRB>
static void wrap_unpremultiply(uint32_t* dst, const void* src, int count) {
    SkUnpremultiplyRow<kSwapRB>(dst, (const uint32_t*) src, count);
}

void swizzle_and_multiply(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                          const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB) {
    void (*proc)(uint32_t* dst, const void* src, int count);
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

// Fast Path 3: Color space xform.
static inline bool optimized_color_xform(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo) {
    // Unpremultiplication is unsupported by SkColorSpaceXform.  Note that if |src| is non-linearly
    // premultiplied, we're always going to have to unpremultiply before doing anything.
    if (kPremul_SkAlphaType == srcInfo.alphaType()) {
        return false;
    }

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_F16_SkColorType:
            break;
        default:
            return false;
    }

    switch (srcInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            break;
        default:
            return false;
    }

    return true;
}

static inline bool apply_color_xform(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                     const SkImageInfo& srcInfo, const void* srcPixels,
                                     size_t srcRB) {
    SkColorSpaceXform::ColorFormat dstFormat = select_xform_format(dstInfo.colorType());
    SkColorSpaceXform::ColorFormat srcFormat = select_xform_format(srcInfo.colorType());
    SkAlphaType xformAlpha;
    switch (srcInfo.alphaType()) {
        case kOpaque_SkAlphaType:
            xformAlpha = kOpaque_SkAlphaType;
            break;
        case kPremul_SkAlphaType:
            SkASSERT(kPremul_SkAlphaType == dstInfo.alphaType());

            // This signal means: copy the src alpha to the dst, do not premultiply (in this
            // case because the pixels are already premultiplied).
            xformAlpha = kUnpremul_SkAlphaType;
            break;
        case kUnpremul_SkAlphaType:
            SkASSERT(kPremul_SkAlphaType == dstInfo.alphaType() ||
                     kUnpremul_SkAlphaType == dstInfo.alphaType());

            xformAlpha = dstInfo.alphaType();
            break;
        default:
            SkASSERT(false);
            xformAlpha = kUnpremul_SkAlphaType;
            break;
    }

    std::unique_ptr<SkColorSpaceXform> xform = SkMakeColorSpaceXform(srcInfo.colorSpace(),
                                                                     dstInfo.colorSpace());
    if (!xform) {
        return false;
    }

    for (int y = 0; y < dstInfo.height(); y++) {
        SkAssertResult(xform->apply(dstFormat, dstPixels, srcFormat, srcPixels, dstInfo.width(),
                       xformAlpha));
        dstPixels = SkTAddOffset<void>(dstPixels, dstRB);
        srcPixels = SkTAddOffset<const void>(srcPixels, srcRB);
    }
    return true;
}

// Fast Path 4: Alpha 8 dsts.
static void convert_to_alpha8(uint8_t* dst, size_t dstRB, const SkImageInfo& srcInfo,
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
                                  bool isColorAware) {

    SkJumper_MemoryCtx src = { (void*)srcRow, (int)(srcRB / srcInfo.bytesPerPixel()) },
                       dst = { (void*)dstRow, (int)(dstRB / dstInfo.bytesPerPixel()) };

    SkRasterPipeline_<256> pipeline;
    switch (srcInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::load_8888, &src);
            break;
        case kRGB_888x_SkColorType:
            pipeline.append(SkRasterPipeline::load_8888, &src);
            pipeline.append(SkRasterPipeline::force_opaque);
            break;
        case kBGRA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::load_bgra, &src);
            break;
        case kRGBA_1010102_SkColorType:
            pipeline.append(SkRasterPipeline::load_1010102, &src);
            break;
        case kRGB_101010x_SkColorType:
            pipeline.append(SkRasterPipeline::load_1010102, &src);
            pipeline.append(SkRasterPipeline::force_opaque);
            break;
        case kRGB_565_SkColorType:
            pipeline.append(SkRasterPipeline::load_565, &src);
            break;
        case kRGBA_F16_SkColorType:
            pipeline.append(SkRasterPipeline::load_f16, &src);
            break;
        case kRGBA_F32_SkColorType:
            pipeline.append(SkRasterPipeline::load_f32, &src);
            break;
        case kGray_8_SkColorType:
            pipeline.append(SkRasterPipeline::load_g8, &src);
            break;
        case kARGB_4444_SkColorType:
            pipeline.append(SkRasterPipeline::load_4444, &src);
            break;
        default:
            SkASSERT(false);
            break;
    }

    SkAlphaType premulState = srcInfo.alphaType();
    if (kPremul_SkAlphaType == premulState) {
        pipeline.append(SkRasterPipeline::unpremul);
        premulState = kUnpremul_SkAlphaType;
    }

    SkColorSpaceTransferFn srcFn;
    if (isColorAware && srcInfo.gammaCloseToSRGB()) {
        pipeline.append(SkRasterPipeline::from_srgb);
    } else if (isColorAware && !srcInfo.colorSpace()->gammaIsLinear()) {
        SkAssertResult(srcInfo.colorSpace()->isNumericalTransferFn(&srcFn));
        if (is_just_gamma(srcFn)) {
            pipeline.append(SkRasterPipeline::gamma, &srcFn.fG);
        } else {
            pipeline.append(SkRasterPipeline::parametric, &srcFn);
        }
    }

    SkSTArenaAlloc<12*sizeof(float)> alloc;
    if (isColorAware) {
        append_gamut_transform(&pipeline, &alloc,
                               srcInfo.colorSpace(), dstInfo.colorSpace(), premulState);
    }

    SkColorSpaceTransferFn dstFn;
    if (isColorAware && dstInfo.gammaCloseToSRGB()) {
        pipeline.append(SkRasterPipeline::to_srgb);
    } else if (isColorAware && !dstInfo.colorSpace()->gammaIsLinear()) {
        SkAssertResult(dstInfo.colorSpace()->isNumericalTransferFn(&dstFn));
        dstFn = dstFn.invert();
        if (is_just_gamma(dstFn)) {
            pipeline.append(SkRasterPipeline::gamma, &dstFn.fG);
        } else {
            pipeline.append(SkRasterPipeline::parametric, &dstFn);
        }
    }

    SkAlphaType dat = dstInfo.alphaType();
    if (kUnpremul_SkAlphaType == premulState && kPremul_SkAlphaType == dat)
    {
        pipeline.append(SkRasterPipeline::premul);
        premulState = kPremul_SkAlphaType;
    }

    // The final premul state must equal the dst alpha type.  Note that if we are "converting"
    // opaque to another alpha type, there's no need to worry about multiplication.
    SkASSERT(premulState == dat || kOpaque_SkAlphaType == srcInfo.alphaType());

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

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::store_8888, &dst);
            break;
        case kRGB_888x_SkColorType:
            pipeline.append(SkRasterPipeline::force_opaque);
            pipeline.append(SkRasterPipeline::store_8888, &dst);
            break;
        case kBGRA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::store_bgra, &dst);
            break;
        case kRGBA_1010102_SkColorType:
            pipeline.append(SkRasterPipeline::store_1010102, &dst);
            break;
        case kRGB_101010x_SkColorType:
            pipeline.append(SkRasterPipeline::force_opaque);
            pipeline.append(SkRasterPipeline::store_1010102, &dst);
            break;
        case kRGB_565_SkColorType:
            pipeline.append(SkRasterPipeline::store_565, &dst);
            break;
        case kRGBA_F16_SkColorType:
            pipeline.append(SkRasterPipeline::store_f16, &dst);
            break;
        case kRGBA_F32_SkColorType:
            pipeline.append(SkRasterPipeline::store_f32, &dst);
            break;
        case kARGB_4444_SkColorType:
            pipeline.append(SkRasterPipeline::store_4444, &dst);
            break;
        default:
            SkASSERT(false);
            break;
    }

    pipeline.run(0,0, srcInfo.width(), srcInfo.height());
}

static bool swizzle_and_multiply_color_type(SkColorType ct) {
    switch (ct) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return true;
        default:
            return false;
    }
}

void SkConvertPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                     const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB) {
    SkASSERT(dstInfo.dimensions() == srcInfo.dimensions());
    SkASSERT(SkImageInfoValidConversion(dstInfo, srcInfo));

    // Fast Path 1: The memcpy() case.
    if (can_memcpy(dstInfo, srcInfo)) {
        SkRectMemcpy(dstPixels, dstRB, srcPixels, srcRB, dstInfo.minRowBytes(), dstInfo.height());
        return;
    }

    const bool isColorAware = dstInfo.colorSpace();
    SkASSERT(srcInfo.colorSpace() || !isColorAware);

    // Fast Path 2: Simple swizzles and premuls.
    if (swizzle_and_multiply_color_type(srcInfo.colorType()) &&
        swizzle_and_multiply_color_type(dstInfo.colorType()) && !isColorAware) {
        swizzle_and_multiply(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB);
        return;
    }

    // Fast Path 3: Color space xform.
    if (isColorAware && optimized_color_xform(dstInfo, srcInfo)) {
        if (apply_color_xform(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB)) {
            return;
        }
    }

    // Fast Path 4: Alpha 8 dsts.
    if (kAlpha_8_SkColorType == dstInfo.colorType()) {
        convert_to_alpha8((uint8_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB);
        return;
    }

    // Default: Use the pipeline.
    convert_with_pipeline(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, isColorAware);
}
