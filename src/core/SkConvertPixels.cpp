/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformPriv.h"
#include "SkColorTable.h"
#include "SkConvertPixels.h"
#include "SkImageInfoPriv.h"
#include "SkOpts.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkUnPreMultiply.h"
#include "SkUnPreMultiplyPriv.h"

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
    if (kUnpremul_SkAlphaType == dstInfo.alphaType() && kPremul_SkAlphaType == srcInfo.alphaType())
    {
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

    std::unique_ptr<SkColorSpaceXform> xform = SkColorSpaceXform::New(srcInfo.colorSpace(),
                                                                      dstInfo.colorSpace());
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
               SkColorTable* ctable) {
    T dstCTable[256];
    int count = ctable->count();
    SkImageInfo srcInfo8888 = srcInfo.makeColorType(kN32_SkColorType).makeWH(count, 1);
    SkImageInfo dstInfoCT = dstInfo.makeWH(count, 1);
    size_t rowBytes = count * sizeof(T);
    SkConvertPixels(dstInfoCT, dstCTable, rowBytes, srcInfo8888, ctable->readColors(), rowBytes,
                    nullptr);

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
                         SkColorTable* ctable) {
    switch (dstInfo.colorType()) {
        case kAlpha_8_SkColorType:
            do_index8(dstInfo, (uint8_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable);
            break;
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
            do_index8(dstInfo, (uint16_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable);
            break;
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            do_index8(dstInfo, (uint32_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable);
            break;
        case kRGBA_F16_SkColorType:
            do_index8(dstInfo, (uint64_t*) dstPixels, dstRB, srcInfo, srcPixels, srcRB, ctable);
            break;
        default:
            SkASSERT(false);
    }
}

// Default: Use the pipeline.
static void convert_with_pipeline(const SkImageInfo& dstInfo, void* dstRow, size_t dstRB,
                                  const SkImageInfo& srcInfo, const void* srcRow, size_t srcRB,
                                  bool isColorAware) {
    SkRasterPipeline pipeline;
    switch (srcInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::load_8888, &srcRow);
            break;
        case kBGRA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::load_8888, &srcRow);
            pipeline.append(SkRasterPipeline::swap_rb);
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

    if (isColorAware && srcInfo.gammaCloseToSRGB()) {
        pipeline.append_from_srgb(srcInfo.alphaType());
    }

    float matrix[12];
    if (isColorAware) {
        SkAssertResult(append_gamut_transform(&pipeline, matrix, srcInfo.colorSpace(),
                                              dstInfo.colorSpace()));
    }

    SkAlphaType sat = srcInfo.alphaType();
    SkAlphaType dat = dstInfo.alphaType();
    if (sat == kPremul_SkAlphaType && dat == kUnpremul_SkAlphaType) {
        pipeline.append(SkRasterPipeline::unpremul);
    } else if (sat == kUnpremul_SkAlphaType && dat == kPremul_SkAlphaType) {
        pipeline.append(SkRasterPipeline::premul);
    }

    if (isColorAware && dstInfo.gammaCloseToSRGB()) {
        pipeline.append(SkRasterPipeline::to_srgb);
    }

    switch (dstInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::store_8888, &dstRow);
            break;
        case kBGRA_8888_SkColorType:
            pipeline.append(SkRasterPipeline::swap_rb);
            pipeline.append(SkRasterPipeline::store_8888, &dstRow);
            break;
        case kRGB_565_SkColorType:
            pipeline.append(SkRasterPipeline::store_565, &dstRow);
            break;
        case kRGBA_F16_SkColorType:
            pipeline.append(SkRasterPipeline::store_f16, &dstRow);
            break;
        case kAlpha_8_SkColorType:
            pipeline.append(SkRasterPipeline::store_a8, &dstRow);
            break;
        case kARGB_4444_SkColorType:
            pipeline.append(SkRasterPipeline::store_4444, &dstRow);
            break;
        default:
            SkASSERT(false);
            break;
    }

    for (int y = 0; y < srcInfo.height(); ++y) {
        pipeline.run(0,srcInfo.width());
        // The pipeline has pointers to srcRow and dstRow, so we just need to update them in the
        // loop to move between rows of src/dst.
        dstRow = SkTAddOffset<void>(dstRow, dstRB);
        srcRow = SkTAddOffset<const void>(srcRow, srcRB);
    }
}

void SkConvertPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                     const SkImageInfo& srcInfo, const void* srcPixels, size_t srcRB,
                     SkColorTable* ctable) {
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
    if (isColorAware && optimized_color_xform(dstInfo, srcInfo)) {
        apply_color_xform(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB);
        return;
    }

    // Fast Path 4: Index 8 sources.
    if (kIndex_8_SkColorType == srcInfo.colorType()) {
        SkASSERT(ctable);
        convert_from_index8(dstInfo, dstPixels, dstRB, srcInfo, (const uint8_t*) srcPixels, srcRB,
                            ctable);
        return;
    }

    // Default: Use the pipeline.
    convert_with_pipeline(dstInfo, dstPixels, dstRB, srcInfo, srcPixels, srcRB, isColorAware);
}
