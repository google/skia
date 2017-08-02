/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform_Base.h"
#include "SkColorSpaceXformPriv.h"
#include "SkColorTable.h"
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
static inline bool optimized_color_xform(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo,
                                         SkTransferFunctionBehavior behavior) {
    // Unpremultiplication is unsupported by SkColorSpaceXform.  Note that if |src| is non-linearly
    // premultiplied, we're always going to have to unpremultiply before doing anything.
    if (kPremul_SkAlphaType == srcInfo.alphaType() &&
            (kUnpremul_SkAlphaType == dstInfo.alphaType() ||
             SkTransferFunctionBehavior::kIgnore == behavior)) {
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

static inline void apply_color_xform(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                                     const SkImageInfo& srcInfo, const void* srcPixels,
                                     size_t srcRB, SkTransferFunctionBehavior behavior) {
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

    std::unique_ptr<SkColorSpaceXform> xform =
            SkColorSpaceXform_Base::New(srcInfo.colorSpace(), dstInfo.colorSpace(), behavior);
    SkASSERT(xform);

    for (int y = 0; y < dstInfo.height(); y++) {
        SkAssertResult(xform->apply(dstFormat, dstPixels, srcFormat, srcPixels, dstInfo.width(),
                       xformAlpha));
        dstPixels = SkTAddOffset<void>(dstPixels, dstRB);
        srcPixels = SkTAddOffset<const void>(srcPixels, srcRB);
    }
}

// Fast Path 4: Index 8 sources.
template <typename T>
void do_index8(const SkImageInfo& dstInfo, T* dstPixels, size_t dstRB,
               const SkImageInfo& srcInfo, const uint8_t* srcPixels, size_t srcRB,
               SkColorTable* ctable, SkTransferFunctionBehavior behavior) {
    T dstCTable[256];
    int count = ctable->count();
    SkImageInfo srcInfo8888 = srcInfo.makeColorType(kN32_SkColorType).makeWH(count, 1);
    SkImageInfo dstInfoCT = dstInfo.makeWH(count, 1);
    size_t rowBytes = count * sizeof(T);
    SkConvertPixels(dstInfoCT, dstCTable, rowBytes, srcInfo8888, ctable->readColors(), rowBytes,
                    nullptr, behavior);

    for (int y = 0; y < dstInfo.height(); y++) {
        for (int x = 0; x < dstInfo.width(); x++) {
            dstPixels[x] = dstCTable[srcPixels[x]];
        }
        dstPixels = SkTAddOffset<T>(dstPixels, dstRB);
        srcPixels = SkTAddOffset<const uint8_t>(srcPixels, srcRB);
    }
}

void convert_from_index8(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                         const SkImageInfo& srcInfo, const uint8_t* srcPixels, size_t srcRB,
                         SkColorTable* ctable, SkTransferFunctionBehavior behavior) {
    switch (dstInfo.colorType()) {
        case kAlpha_8_SkColorType:
            do_index8(dstInfo, (uint8_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable,
                      behavior);
            break;
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
            do_index8(dstInfo, (uint16_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable,
                      behavior);
            break;
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            do_index8(dstInfo, (uint32_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable,
                      behavior);
            break;
        case kRGBA_F16_SkColorType:
            do_index8(dstInfo, (uint64_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable,
                      behavior);
            break;
        default:
            SkASSERT(false);
    }
}

// Fast Path 5: Alpha 8 dsts.
static void convert_to_alpha8(uint8_t* dst, size_t dstRB, const SkImageInfo& srcInfo,
                              const void* src, size_t srcRB, SkColorTable* ctable) {
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
        default:
            SkASSERT(false);
            break;
    }
}

// Default: Use the pipeline.
static void convert_with_pipeline(const SkImageInfo& dstInfo, void* dstRow, size_t dstRB,
                                  const SkImageInfo& srcInfo, const void* srcRow, size_t srcRB,
                                  bool isColorAware, SkTransferFunctionBehavior behavior) {
    SkRasterPipeline_<256> pipeline;
    switch (srcInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::load_8888, &srcRow);
            break;
        case kBGRA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::load_bgra, &srcRow);
            break;
        case kRGB_565_SkColorType:
            pipeline.append(SkRasterPipeline::load_565, &srcRow);
            break;
        case kRGBA_F16_SkColorType:
            pipeline.append(SkRasterPipeline::load_f16, &srcRow);
            break;
        case kGray_8_SkColorType:
            pipeline.append(SkRasterPipeline::load_g8, &srcRow);
            break;
        case kARGB_4444_SkColorType:
            pipeline.append(SkRasterPipeline::load_4444, &srcRow);
            break;
        default:
            SkASSERT(false);
            break;
    }

    SkAlphaType premulState = srcInfo.alphaType();
    if (kPremul_SkAlphaType == premulState && SkTransferFunctionBehavior::kIgnore == behavior) {
        pipeline.append(SkRasterPipeline::unpremul);
        premulState = kUnpremul_SkAlphaType;
    }

    SkColorSpaceTransferFn srcFn;
    if (isColorAware && srcInfo.gammaCloseToSRGB()) {
        pipeline.append_from_srgb(premulState);
    } else if (isColorAware && !srcInfo.colorSpace()->gammaIsLinear()) {
        SkAssertResult(srcInfo.colorSpace()->isNumericalTransferFn(&srcFn));
        pipeline.append(SkRasterPipeline::parametric_r, &srcFn);
        pipeline.append(SkRasterPipeline::parametric_g, &srcFn);
        pipeline.append(SkRasterPipeline::parametric_b, &srcFn);
    }

    float matrix[12];
    if (isColorAware) {
        append_gamut_transform(&pipeline, matrix, srcInfo.colorSpace(), dstInfo.colorSpace(),
                               premulState);
    }

    SkAlphaType dat = dstInfo.alphaType();
    if (SkTransferFunctionBehavior::kRespect == behavior) {
        if (kPremul_SkAlphaType == premulState && kUnpremul_SkAlphaType == dat) {
            pipeline.append(SkRasterPipeline::unpremul);
            premulState = kUnpremul_SkAlphaType;
        } else if (kUnpremul_SkAlphaType == premulState && kPremul_SkAlphaType == dat) {
            pipeline.append(SkRasterPipeline::premul);
            premulState = kPremul_SkAlphaType;
        }
    }

    SkColorSpaceTransferFn dstFn;
    if (isColorAware && dstInfo.gammaCloseToSRGB()) {
        pipeline.append(SkRasterPipeline::to_srgb);
    } else if (isColorAware && !dstInfo.colorSpace()->gammaIsLinear()) {
        SkAssertResult(dstInfo.colorSpace()->isNumericalTransferFn(&dstFn));
        dstFn = dstFn.invert();
        pipeline.append(SkRasterPipeline::parametric_r, &dstFn);
        pipeline.append(SkRasterPipeline::parametric_g, &dstFn);
        pipeline.append(SkRasterPipeline::parametric_b, &dstFn);
    }

    if (kUnpremul_SkAlphaType == premulState && kPremul_SkAlphaType == dat &&
        SkTransferFunctionBehavior::kIgnore == behavior)
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
            pipeline.append(SkRasterPipeline::store_8888, &dstRow);
            break;
        case kBGRA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::store_bgra, &dstRow);
            break;
        case kRGB_565_SkColorType:
            pipeline.append(SkRasterPipeline::store_565, &dstRow);
            break;
        case kRGBA_F16_SkColorType:
            pipeline.append(SkRasterPipeline::store_f16, &dstRow);
            break;
        case kARGB_4444_SkColorType:
            pipeline.append(SkRasterPipeline::store_4444, &dstRow);
            break;
        default:
            SkASSERT(false);
            break;
    }

    auto run = pipeline.compile();
    for (int y = 0; y < srcInfo.height(); ++y) {
        run(0,y, srcInfo.width());
        // The pipeline has pointers to srcRow and dstRow, so we just need to update them in the
        // loop to move between rows of src/dst.
        dstRow = SkTAddOffset<void>(dstRow, dstRB);
        srcRow = SkTAddOffset<const void>(srcRow, srcRB);
    }
}

void SkConvertPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                     const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                     SkColorTable* ctable, SkTransferFunctionBehavior behavior) {
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
    if (4 == srcInfo.bytesPerPixel() && 4 == dstInfo.bytesPerPixel() && !isColorAware) {
        swizzle_and_multiply(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB);
        return;
    }

    // Fast Path 3: Color space xform.
    if (isColorAware && optimized_color_xform(dstInfo, srcInfo, behavior)) {
        apply_color_xform(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, behavior);
        return;
    }

    // Fast Path 5: Alpha 8 dsts.
    if (kAlpha_8_SkColorType == dstInfo.colorType()) {
        convert_to_alpha8((uint8_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable);
        return;
    }

    // Default: Use the pipeline.
    convert_with_pipeline(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, isColorAware,
                          behavior);
}
