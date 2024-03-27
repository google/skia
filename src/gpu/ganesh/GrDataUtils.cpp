/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrDataUtils.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkRectMemcpy.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPixmap.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>

using namespace skia_private;

#if defined(GR_TEST_UTILS)

// The following four helpers are copied from src/gpu/DataUtils.cpp to support the test only
// GrTwoColorBC1Compress function. Ideally we would copy the test function into DataUtils.cpp
// instead, but we're currently trying to avoid using the GR_TEST_UTILS define in src/gpu.

static int num_4x4_blocks(int size) {
    return ((size + 3) & ~3) >> 2;
}

struct BC1Block {
    uint16_t fColor0;
    uint16_t fColor1;
    uint32_t fIndices;
};

static uint16_t to565(SkColor col) {
    int r5 = SkMulDiv255Round(31, SkColorGetR(col));
    int g6 = SkMulDiv255Round(63, SkColorGetG(col));
    int b5 = SkMulDiv255Round(31, SkColorGetB(col));

    return (r5 << 11) | (g6 << 5) | b5;
}

// Create a BC1 compressed block that has two colors but is initialized to 'col0'
static void create_BC1_block(SkColor col0, SkColor col1, BC1Block* block) {
    block->fColor0 = to565(col0);
    block->fColor1 = to565(col1);
    SkASSERT(block->fColor0 <= block->fColor1); // we always assume transparent blocks

    if (col0 == SK_ColorTRANSPARENT) {
        // This sets all 16 pixels to just use color3 (under the assumption
        // that this is a kBC1_RGBA8_UNORM texture. Note that in this case
        // fColor0 will be opaque black.
        block->fIndices = 0xFFFFFFFF;
    } else {
        // This sets all 16 pixels to just use 'fColor0'
        block->fIndices = 0;
    }
}

// Fill in 'dstPixels' with BC1 blocks derived from the 'pixmap'.
void GrTwoColorBC1Compress(const SkPixmap& pixmap, SkColor otherColor, char* dstPixels) {
    BC1Block* dstBlocks = reinterpret_cast<BC1Block*>(dstPixels);
    SkASSERT(pixmap.colorType() == SkColorType::kRGBA_8888_SkColorType);

    BC1Block block;

    // black -> fColor0, otherColor -> fColor1
    create_BC1_block(SK_ColorBLACK, otherColor, &block);

    int numXBlocks = num_4x4_blocks(pixmap.width());
    int numYBlocks = num_4x4_blocks(pixmap.height());

    for (int y = 0; y < numYBlocks; ++y) {
        for (int x = 0; x < numXBlocks; ++x) {
            int shift = 0;
            int offsetX = 4 * x, offsetY = 4 * y;
            block.fIndices = 0;  // init all the pixels to color0 (i.e., opaque black)
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j, shift += 2) {
                    if (offsetX + j >= pixmap.width() || offsetY + i >= pixmap.height()) {
                        // This can happen for the topmost levels of a mipmap and for
                        // non-multiple of 4 textures
                        continue;
                    }

                    SkColor tmp = pixmap.getColor(offsetX + j, offsetY + i);
                    if (tmp == SK_ColorTRANSPARENT) {
                        // For RGBA BC1 images color3 is set to transparent black
                        block.fIndices |= 3 << shift;
                    } else if (tmp != SK_ColorBLACK) {
                        block.fIndices |= 1 << shift; // color1
                    }
                }
            }

            dstBlocks[y*numXBlocks + x] = block;
        }
    }
}

#endif

size_t GrComputeTightCombinedBufferSize(size_t bytesPerPixel, SkISize baseDimensions,
                                        TArray<size_t>* individualMipOffsets, int mipLevelCount) {
    SkASSERT(individualMipOffsets && individualMipOffsets->empty());
    SkASSERT(mipLevelCount >= 1);

    individualMipOffsets->push_back(0);

    size_t combinedBufferSize = baseDimensions.width() * bytesPerPixel * baseDimensions.height();
    SkISize levelDimensions = baseDimensions;

    // The Vulkan spec for copying a buffer to an image requires that the alignment must be at
    // least 4 bytes and a multiple of the bytes per pixel of the image config.
    SkASSERT(bytesPerPixel == 1 || bytesPerPixel == 2 || bytesPerPixel == 3 ||
             bytesPerPixel == 4 || bytesPerPixel == 8 || bytesPerPixel == 16);
    int desiredAlignment = (bytesPerPixel == 3) ? 12 : (bytesPerPixel > 4 ? bytesPerPixel : 4);

    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        levelDimensions = {std::max(1, levelDimensions.width() /2),
                           std::max(1, levelDimensions.height()/2)};

        size_t trimmedSize = levelDimensions.area() * bytesPerPixel;
        const size_t alignmentDiff = combinedBufferSize % desiredAlignment;
        if (alignmentDiff != 0) {
            combinedBufferSize += desiredAlignment - alignmentDiff;
        }
        SkASSERT((0 == combinedBufferSize % 4) && (0 == combinedBufferSize % bytesPerPixel));

        individualMipOffsets->push_back(combinedBufferSize);
        combinedBufferSize += trimmedSize;
    }

    SkASSERT(individualMipOffsets->size() == mipLevelCount);
    return combinedBufferSize;
}

static skgpu::Swizzle get_load_and_src_swizzle(GrColorType ct, SkRasterPipelineOp* load,
                                               bool* isNormalized, bool* isSRGB) {
    skgpu::Swizzle swizzle("rgba");
    *isNormalized = true;
    *isSRGB = false;
    switch (ct) {
        case GrColorType::kAlpha_8:          *load = SkRasterPipelineOp::load_a8;       break;
        case GrColorType::kAlpha_16:         *load = SkRasterPipelineOp::load_a16;      break;
        case GrColorType::kBGR_565:          *load = SkRasterPipelineOp::load_565;      break;
        case GrColorType::kRGB_565:          swizzle = skgpu::Swizzle("bgr1");
                                             *load = SkRasterPipelineOp::load_565;      break;
        case GrColorType::kABGR_4444:        *load = SkRasterPipelineOp::load_4444;     break;
        case GrColorType::kARGB_4444:        swizzle = skgpu::Swizzle("bgra");
                                             *load = SkRasterPipelineOp::load_4444;     break;
        case GrColorType::kBGRA_4444:        swizzle = skgpu::Swizzle("gbar");
                                             *load = SkRasterPipelineOp::load_4444;     break;
        case GrColorType::kRGBA_8888:        *load = SkRasterPipelineOp::load_8888;     break;
        case GrColorType::kRG_88:            *load = SkRasterPipelineOp::load_rg88;     break;
        case GrColorType::kRGBA_1010102:     *load = SkRasterPipelineOp::load_1010102;  break;
        case GrColorType::kBGRA_1010102:     *load = SkRasterPipelineOp::load_1010102;
                                             swizzle = skgpu::Swizzle("bgra");
                                             break;
        case GrColorType::kRGBA_10x6:        *load = SkRasterPipelineOp::load_10x6;     break;
        case GrColorType::kAlpha_F16:        *load = SkRasterPipelineOp::load_af16;     break;
        case GrColorType::kRGBA_F16_Clamped: *load = SkRasterPipelineOp::load_f16;      break;
        case GrColorType::kRG_1616:          *load = SkRasterPipelineOp::load_rg1616;   break;
        case GrColorType::kRGBA_16161616:    *load = SkRasterPipelineOp::load_16161616; break;

        case GrColorType::kRGBA_8888_SRGB:   *load = SkRasterPipelineOp::load_8888;
                                             *isSRGB = true;
                                             break;
        case GrColorType::kRG_F16:           *load = SkRasterPipelineOp::load_rgf16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F16:         *load = SkRasterPipelineOp::load_f16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F32:         *load = SkRasterPipelineOp::load_f32;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_8xxx:       *load = SkRasterPipelineOp::load_8888;
                                             swizzle = skgpu::Swizzle("000r");
                                             break;
        case GrColorType::kAlpha_F32xxx:     *load = SkRasterPipelineOp::load_f32;
                                             swizzle = skgpu::Swizzle("000r");
                                             break;
        case GrColorType::kGray_8xxx:       *load = SkRasterPipelineOp::load_8888;
                                             swizzle = skgpu::Swizzle("rrr1");
                                             break;
        case GrColorType::kGray_8:           *load = SkRasterPipelineOp::load_a8;
                                             swizzle = skgpu::Swizzle("aaa1");
                                             break;
        case GrColorType::kR_8xxx:           *load = SkRasterPipelineOp::load_8888;
                                             swizzle = skgpu::Swizzle("r001");
                                             break;
        case GrColorType::kR_8:              *load = SkRasterPipelineOp::load_a8;
                                             swizzle = skgpu::Swizzle("a001");
                                             break;
        case GrColorType::kGrayAlpha_88:    *load = SkRasterPipelineOp::load_rg88;
                                             swizzle = skgpu::Swizzle("rrrg");
                                             break;
        case GrColorType::kBGRA_8888:        *load = SkRasterPipelineOp::load_8888;
                                             swizzle = skgpu::Swizzle("bgra");
                                             break;
        case GrColorType::kRGB_888x:         *load = SkRasterPipelineOp::load_8888;
                                             swizzle = skgpu::Swizzle("rgb1");
                                             break;

        // These are color types we don't expect to ever have to load.
        case GrColorType::kRGB_888:
        case GrColorType::kR_16:
        case GrColorType::kR_F16:
        case GrColorType::kGray_F16:
        case GrColorType::kUnknown:
            SK_ABORT("unexpected CT");
    }
    return swizzle;
}

enum class LumMode {
    kNone,
    kToRGB,
    kToAlpha
};

static skgpu::Swizzle get_dst_swizzle_and_store(GrColorType ct, SkRasterPipelineOp* store,
                                                LumMode* lumMode, bool* isNormalized,
                                                bool* isSRGB) {
    skgpu::Swizzle swizzle("rgba");
    *isNormalized = true;
    *isSRGB = false;
    *lumMode = LumMode::kNone;
    switch (ct) {
        case GrColorType::kAlpha_8:          *store = SkRasterPipelineOp::store_a8;       break;
        case GrColorType::kAlpha_16:         *store = SkRasterPipelineOp::store_a16;      break;
        case GrColorType::kBGR_565:          *store = SkRasterPipelineOp::store_565;      break;
        case GrColorType::kRGB_565:          swizzle = skgpu::Swizzle("bgr1");
                                             *store = SkRasterPipelineOp::store_565;      break;
        case GrColorType::kABGR_4444:        *store = SkRasterPipelineOp::store_4444;     break;
        case GrColorType::kARGB_4444:        swizzle = skgpu::Swizzle("bgra");
                                             *store = SkRasterPipelineOp::store_4444;     break;
        case GrColorType::kBGRA_4444:        swizzle = skgpu::Swizzle("argb");
                                             *store = SkRasterPipelineOp::store_4444;     break;
        case GrColorType::kRGBA_8888:        *store = SkRasterPipelineOp::store_8888;     break;
        case GrColorType::kRG_88:            *store = SkRasterPipelineOp::store_rg88;     break;
        case GrColorType::kRGBA_1010102:     *store = SkRasterPipelineOp::store_1010102;  break;
        case GrColorType::kBGRA_1010102:     swizzle = skgpu::Swizzle("bgra");
                                             *store = SkRasterPipelineOp::store_1010102;
                                             break;
        case GrColorType::kRGBA_10x6:        *store = SkRasterPipelineOp::store_10x6;     break;
        case GrColorType::kRGBA_F16_Clamped: *store = SkRasterPipelineOp::store_f16;      break;
        case GrColorType::kRG_1616:          *store = SkRasterPipelineOp::store_rg1616;   break;
        case GrColorType::kRGBA_16161616:    *store = SkRasterPipelineOp::store_16161616; break;

        case GrColorType::kRGBA_8888_SRGB:   *store = SkRasterPipelineOp::store_8888;
                                             *isSRGB = true;
                                             break;
        case GrColorType::kRG_F16:           *store = SkRasterPipelineOp::store_rgf16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_F16:        *store = SkRasterPipelineOp::store_af16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F16:         *store = SkRasterPipelineOp::store_f16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F32:         *store = SkRasterPipelineOp::store_f32;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_8xxx:       *store = SkRasterPipelineOp::store_8888;
                                             swizzle = skgpu::Swizzle("a000");
                                             break;
        case GrColorType::kAlpha_F32xxx:     *store = SkRasterPipelineOp::store_f32;
                                             swizzle = skgpu::Swizzle("a000");
                                             break;
        case GrColorType::kBGRA_8888:        swizzle = skgpu::Swizzle("bgra");
                                             *store = SkRasterPipelineOp::store_8888;
                                             break;
        case GrColorType::kRGB_888x:         swizzle = skgpu::Swizzle("rgb1");
                                             *store = SkRasterPipelineOp::store_8888;
                                             break;
        case GrColorType::kR_8xxx:           swizzle = skgpu::Swizzle("r001");
                                             *store = SkRasterPipelineOp::store_8888;
                                             break;
        case GrColorType::kR_8:              swizzle = skgpu::Swizzle("agbr");
                                             *store = SkRasterPipelineOp::store_a8;
                                             break;
        case GrColorType::kR_16:             swizzle = skgpu::Swizzle("agbr");
                                             *store = SkRasterPipelineOp::store_a16;
                                             break;
        case GrColorType::kR_F16:            swizzle = skgpu::Swizzle("agbr");
                                             *store = SkRasterPipelineOp::store_af16;
                                             break;
        case GrColorType::kGray_F16:         *lumMode = LumMode::kToAlpha;
                                             *store = SkRasterPipelineOp::store_af16;
                                             break;
        case GrColorType::kGray_8:           *lumMode = LumMode::kToAlpha;
                                             *store = SkRasterPipelineOp::store_a8;
                                             break;
        case GrColorType::kGrayAlpha_88:     *lumMode = LumMode::kToRGB;
                                             swizzle = skgpu::Swizzle("ragb");
                                             *store = SkRasterPipelineOp::store_rg88;
                                             break;
        case GrColorType::kGray_8xxx:        *lumMode = LumMode::kToRGB;
                                             *store = SkRasterPipelineOp::store_8888;
                                             swizzle = skgpu::Swizzle("r000");
                                             break;

        // These are color types we don't expect to ever have to store.
        case GrColorType::kRGB_888:  // This is handled specially in GrConvertPixels.
        case GrColorType::kUnknown:
            SK_ABORT("unexpected CT");
    }
    return swizzle;
}

bool GrConvertPixels(const GrPixmap& dst, const GrCPixmap& src, bool flipY) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (src.dimensions().isEmpty() || dst.dimensions().isEmpty()) {
        return false;
    }
    if (src.colorType() == GrColorType::kUnknown || dst.colorType() == GrColorType::kUnknown) {
        return false;
    }
    if (!src.hasPixels() || !dst.hasPixels()) {
        return false;
    }
    if (dst.dimensions() != src.dimensions()) {
        return false;
    }
    if (dst.colorType() == GrColorType::kRGB_888) {
        // SkRasterPipeline doesn't handle writing to RGB_888. So we have it write to RGB_888x and
        // then do another conversion that does the 24bit packing. We could be cleverer and skip the
        // temp pixmap if this is the only conversion but this is rare so keeping it simple.
        GrPixmap temp = GrPixmap::Allocate(dst.info().makeColorType(GrColorType::kRGB_888x));
        if (!GrConvertPixels(temp, src, flipY)) {
            return false;
        }
        auto* tRow = reinterpret_cast<const char*>(temp.addr());
        auto* dRow = reinterpret_cast<char*>(dst.addr());
        for (int y = 0; y < dst.height(); ++y, tRow += temp.rowBytes(), dRow += dst.rowBytes()) {
            for (int x = 0; x < dst.width(); ++x) {
                auto t = tRow + x*sizeof(uint32_t);
                auto d = dRow + x*3;
                memcpy(d, t, 3);
            }
        }
        return true;
    } else if (src.colorType() == GrColorType::kRGB_888) {
        // SkRasterPipeline doesn't handle reading from RGB_888. So convert it to RGB_888x and then
        // do a recursive call if there is any remaining conversion.
        GrPixmap temp = GrPixmap::Allocate(src.info().makeColorType(GrColorType::kRGB_888x));
        auto* sRow = reinterpret_cast<const char*>(src.addr());
        auto* tRow = reinterpret_cast<char*>(temp.addr());
        for (int y = 0; y < src.height(); ++y, sRow += src.rowBytes(), tRow += temp.rowBytes()) {
            for (int x = 0; x < src.width(); ++x) {
                auto s = sRow + x*3;
                auto t = tRow + x*sizeof(uint32_t);
                memcpy(t, s, 3);
                t[3] = static_cast<char>(0xFF);
            }
        }
        return GrConvertPixels(dst, temp, flipY);
    }

    size_t srcBpp = src.info().bpp();
    size_t dstBpp = dst.info().bpp();

    // SkRasterPipeline operates on row-pixels not row-bytes.
    SkASSERT(dst.rowBytes() % dstBpp == 0);
    SkASSERT(src.rowBytes() % srcBpp == 0);

    bool premul   = src.alphaType() == kUnpremul_SkAlphaType &&
                    dst.alphaType() == kPremul_SkAlphaType;
    bool unpremul = src.alphaType() == kPremul_SkAlphaType &&
                    dst.alphaType() == kUnpremul_SkAlphaType;
    bool alphaOrCSConversion =
            premul || unpremul || !SkColorSpace::Equals(src.colorSpace(), dst.colorSpace());

    if (src.colorType() == dst.colorType() && !alphaOrCSConversion) {
        size_t tightRB = dstBpp * dst.width();
        if (flipY) {
            auto s = static_cast<const char*>(src.addr());
            auto d = SkTAddOffset<char>(dst.addr(), dst.rowBytes()*(dst.height() - 1));
            for (int y = 0; y < dst.height(); ++y, d -= dst.rowBytes(), s += src.rowBytes()) {
                memcpy(d, s, tightRB);
            }
        } else {
            SkRectMemcpy(dst.addr(), dst.rowBytes(),
                         src.addr(), src.rowBytes(),
                         tightRB, src.height());
        }
        return true;
    }

    SkRasterPipelineOp load;
    bool srcIsNormalized;
    bool srcIsSRGB;
    auto loadSwizzle = get_load_and_src_swizzle(src.colorType(),
                                                &load,
                                                &srcIsNormalized,
                                                &srcIsSRGB);

    SkRasterPipelineOp store;
    LumMode lumMode;
    bool dstIsNormalized;
    bool dstIsSRGB;
    auto storeSwizzle = get_dst_swizzle_and_store(dst.colorType(),
                                                  &store,
                                                  &lumMode,
                                                  &dstIsNormalized,
                                                  &dstIsSRGB);

    SkTLazy<SkColorSpaceXformSteps> steps;
    skgpu::Swizzle loadStoreSwizzle;
    if (alphaOrCSConversion) {
        steps.init(src.colorSpace(), src.alphaType(), dst.colorSpace(), dst.alphaType());
    } else {
        loadStoreSwizzle = skgpu::Swizzle::Concat(loadSwizzle, storeSwizzle);
    }
    int cnt = 1;
    int height = src.height();
    SkRasterPipeline_MemoryCtx
            srcCtx{const_cast<void*>(src.addr()), SkToInt(src.rowBytes()/srcBpp)},
            dstCtx{                   dst.addr(), SkToInt(dst.rowBytes()/dstBpp)};

    if (flipY) {
        // It *almost* works to point the src at the last row and negate the stride and run the
        // whole rectangle. However, SkRasterPipeline::run()'s control loop uses size_t loop
        // variables so it winds up relying on unsigned overflow math. It works out in practice
        // but UBSAN says "no!" as it's technically undefined and in theory a compiler could emit
        // code that didn't do what is intended. So we go one row at a time. :(
        srcCtx.pixels = static_cast<char*>(srcCtx.pixels) + src.rowBytes()*(height - 1);
        std::swap(cnt, height);
    }

    bool hasConversion = alphaOrCSConversion || lumMode != LumMode::kNone;

    if (srcIsSRGB && dstIsSRGB && !hasConversion) {
        // No need to convert from srgb if we are just going to immediately convert it back.
        srcIsSRGB = dstIsSRGB = false;
    }

    hasConversion = hasConversion || srcIsSRGB || dstIsSRGB;

    SkRasterPipeline_<256> pipeline;
    pipeline.append(load, &srcCtx);
    if (hasConversion) {
        loadSwizzle.apply(&pipeline);
        if (srcIsSRGB) {
            pipeline.appendTransferFunction(*skcms_sRGB_TransferFunction());
        }
        if (alphaOrCSConversion) {
            steps->apply(&pipeline);
        }
        switch (lumMode) {
            case LumMode::kNone:
                break;
            case LumMode::kToRGB:
                pipeline.append(SkRasterPipelineOp::bt709_luminance_or_luma_to_rgb);
                break;
            case LumMode::kToAlpha:
                pipeline.append(SkRasterPipelineOp::bt709_luminance_or_luma_to_alpha);
                // If we ever need to store srgb-encoded gray (e.g. GL_SLUMINANCE8) then we
                // should use ToRGB and then a swizzle stage rather than ToAlpha. The subsequent
                // transfer function stage ignores the alpha channel (where we just stashed the
                // gray).
                SkASSERT(!dstIsSRGB);
                break;
        }
        if (dstIsSRGB) {
            pipeline.appendTransferFunction(*skcms_sRGB_Inverse_TransferFunction());
        }
        storeSwizzle.apply(&pipeline);
    } else {
        loadStoreSwizzle.apply(&pipeline);
    }
    pipeline.append(store, &dstCtx);
    auto pipelineFn = pipeline.compile();
    for (int i = 0; i < cnt; ++i) {
        pipelineFn(0, 0, src.width(), height);
        srcCtx.pixels = static_cast<char*>(srcCtx.pixels) - src.rowBytes();
        dstCtx.pixels = static_cast<char*>(dstCtx.pixels) + dst.rowBytes();
    }

    return true;
}

bool GrClearImage(const GrImageInfo& dstInfo, void* dst, size_t dstRB, std::array<float, 4> color) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    if (!dstInfo.isValid()) {
        return false;
    }
    if (!dst) {
        return false;
    }
    if (dstRB < dstInfo.minRowBytes()) {
        return false;
    }
    if (dstInfo.colorType() == GrColorType::kRGB_888) {
        // SkRasterPipeline doesn't handle writing to RGB_888. So we handle that specially here.
        uint32_t rgba = SkColor4f{color[0], color[1], color[2], color[3]}.toBytes_RGBA();
        for (int y = 0; y < dstInfo.height(); ++y) {
            char* d = static_cast<char*>(dst) + y * dstRB;
            for (int x = 0; x < dstInfo.width(); ++x, d += 3) {
                memcpy(d, &rgba, 3);
            }
        }
        return true;
    }

    LumMode lumMode;
    bool isNormalized;
    bool dstIsSRGB;
    SkRasterPipelineOp store;
    skgpu::Swizzle storeSwizzle = get_dst_swizzle_and_store(dstInfo.colorType(), &store, &lumMode,
                                                            &isNormalized, &dstIsSRGB);
    char block[64];
    SkArenaAlloc alloc(block, sizeof(block), 1024);
    SkRasterPipeline_<256> pipeline;
    pipeline.appendConstantColor(&alloc, color.data());
    switch (lumMode) {
        case LumMode::kNone:
            break;
        case LumMode::kToRGB:
            pipeline.append(SkRasterPipelineOp::bt709_luminance_or_luma_to_rgb);
            break;
        case LumMode::kToAlpha:
            pipeline.append(SkRasterPipelineOp::bt709_luminance_or_luma_to_alpha);
            // If we ever need to store srgb-encoded gray (e.g. GL_SLUMINANCE8) then we should use
            // ToRGB and then a swizzle stage rather than ToAlpha. The subsequent transfer function
            // stage ignores the alpha channel (where we just stashed the gray).
            SkASSERT(!dstIsSRGB);
            break;
    }
    if (dstIsSRGB) {
        pipeline.appendTransferFunction(*skcms_sRGB_Inverse_TransferFunction());
    }
    storeSwizzle.apply(&pipeline);
    SkRasterPipeline_MemoryCtx dstCtx{dst, SkToInt(dstRB/dstInfo.bpp())};
    pipeline.append(store, &dstCtx);
    pipeline.run(0, 0, dstInfo.width(), dstInfo.height());

    return true;
}
