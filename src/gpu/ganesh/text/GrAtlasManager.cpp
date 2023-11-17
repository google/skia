/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/text/GrAtlasManager.h"

#include "include/core/SkColorSpace.h"
#include "include/encode/SkPngEncoder.h"
#include "src/base/SkAutoMalloc.h"
#include "src/codec/SkMasks.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"
#include "src/text/gpu/Glyph.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/StrikeCache.h"

using Glyph = sktext::gpu::Glyph;
using MaskFormat = skgpu::MaskFormat;

GrAtlasManager::GrAtlasManager(GrProxyProvider* proxyProvider,
                               size_t maxTextureBytes,
                               GrDrawOpAtlas::AllowMultitexturing allowMultitexturing,
                               bool supportBilerpAtlas)
            : fAllowMultitexturing{allowMultitexturing}
            , fSupportBilerpAtlas{supportBilerpAtlas}
            , fProxyProvider{proxyProvider}
            , fCaps{fProxyProvider->refCaps()}
            , fAtlasConfig{fCaps->maxTextureSize(), maxTextureBytes} { }

GrAtlasManager::~GrAtlasManager() = default;

void GrAtlasManager::freeAll() {
    for (int i = 0; i < skgpu::kMaskFormatCount; ++i) {
        fAtlases[i] = nullptr;
    }
}

bool GrAtlasManager::hasGlyph(MaskFormat format, Glyph* glyph) {
    SkASSERT(glyph);
    return this->getAtlas(format)->hasID(glyph->fAtlasLocator.plotLocator());
}

template <typename INT_TYPE>
static void expand_bits(INT_TYPE* dst,
                        const uint8_t* src,
                        int width,
                        int height,
                        int dstRowBytes,
                        int srcRowBytes) {
    for (int y = 0; y < height; ++y) {
        int rowWritesLeft = width;
        const uint8_t* s = src;
        INT_TYPE* d = dst;
        while (rowWritesLeft > 0) {
            unsigned mask = *s++;
            for (int x = 7; x >= 0 && rowWritesLeft; --x, --rowWritesLeft) {
                *d++ = (mask & (1 << x)) ? (INT_TYPE)(~0UL) : 0;
            }
        }
        dst = reinterpret_cast<INT_TYPE*>(reinterpret_cast<intptr_t>(dst) + dstRowBytes);
        src += srcRowBytes;
    }
}

static void get_packed_glyph_image(
        const SkGlyph& glyph, int dstRB, MaskFormat expectedMaskFormat, void* dst) {
    const int width = glyph.width();
    const int height = glyph.height();
    const void* src = glyph.image();
    SkASSERT(src != nullptr);

    MaskFormat maskFormat = Glyph::FormatFromSkGlyph(glyph.maskFormat());
    if (maskFormat == expectedMaskFormat) {
        int srcRB = glyph.rowBytes();
        // Notice this comparison is with the glyphs raw mask format, and not its MaskFormat.
        if (glyph.maskFormat() != SkMask::kBW_Format) {
            if (srcRB != dstRB) {
                const int bbp = MaskFormatBytesPerPixel(expectedMaskFormat);
                for (int y = 0; y < height; y++) {
                    memcpy(dst, src, width * bbp);
                    src = (const char*) src + srcRB;
                    dst = (char*) dst + dstRB;
                }
            } else {
                memcpy(dst, src, dstRB * height);
            }
        } else {
            // Handle 8-bit format by expanding the mask to the expected format.
            const uint8_t* bits = reinterpret_cast<const uint8_t*>(src);
            switch (expectedMaskFormat) {
                case MaskFormat::kA8: {
                    uint8_t* bytes = reinterpret_cast<uint8_t*>(dst);
                    expand_bits(bytes, bits, width, height, dstRB, srcRB);
                    break;
                }
                case MaskFormat::kA565: {
                    uint16_t* rgb565 = reinterpret_cast<uint16_t*>(dst);
                    expand_bits(rgb565, bits, width, height, dstRB, srcRB);
                    break;
                }
                default:
                    SK_ABORT("Invalid MaskFormat");
            }
        }
    } else if (maskFormat == MaskFormat::kA565 &&
               expectedMaskFormat == MaskFormat::kARGB) {
        // Convert if the glyph uses a 565 mask format since it is using LCD text rendering
        // but the expected format is 8888 (will happen on macOS with Metal since that
        // combination does not support 565).
        static constexpr SkMasks masks{
                {0b1111'1000'0000'0000, 11, 5},  // Red
                {0b0000'0111'1110'0000,  5, 6},  // Green
                {0b0000'0000'0001'1111,  0, 5},  // Blue
                {0, 0, 0}                        // Alpha
        };
        constexpr int a565Bpp = MaskFormatBytesPerPixel(MaskFormat::kA565);
        constexpr int argbBpp = MaskFormatBytesPerPixel(MaskFormat::kARGB);
        char* dstRow = (char*)dst;
        for (int y = 0; y < height; y++) {
            dst = dstRow;
            for (int x = 0; x < width; x++) {
                uint16_t color565 = 0;
                memcpy(&color565, src, a565Bpp);
                uint32_t colorRGBA = GrColorPackRGBA(masks.getRed(color565),
                                                     masks.getGreen(color565),
                                                     masks.getBlue(color565),
                                                     0xFF);
                memcpy(dst, &colorRGBA, argbBpp);
                src = (const char*)src + a565Bpp;
                dst = (char*)dst + argbBpp;
            }
            dstRow += dstRB;
        }
    } else {
        SkUNREACHABLE;
    }
}

// returns true if glyph successfully added to texture atlas, false otherwise.
GrDrawOpAtlas::ErrorCode GrAtlasManager::addGlyphToAtlas(const SkGlyph& skGlyph,
                                                         Glyph* glyph,
                                                         int srcPadding,
                                                         GrResourceProvider* resourceProvider,
                                                         GrDeferredUploadTarget* uploadTarget) {
#if !defined(SK_DISABLE_SDF_TEXT)
    SkASSERT(0 <= srcPadding && srcPadding <= SK_DistanceFieldInset);
#else
    SkASSERT(0 <= srcPadding);
#endif

    if (skGlyph.image() == nullptr) {
        return GrDrawOpAtlas::ErrorCode::kError;
    }
    SkASSERT(glyph != nullptr);

    MaskFormat glyphFormat = Glyph::FormatFromSkGlyph(skGlyph.maskFormat());
    MaskFormat expectedMaskFormat = this->resolveMaskFormat(glyphFormat);
    int bytesPerPixel = MaskFormatBytesPerPixel(expectedMaskFormat);

    int padding;
    switch (srcPadding) {
        case 0:
            // The direct mask/image case.
            padding = 0;
            if (fSupportBilerpAtlas) {
                // Force direct masks (glyph with no padding) to have padding.
                padding = 1;
                srcPadding = 1;
            }
            break;
        case 1:
            // The transformed mask/image case.
            padding = 1;
            break;
#if !defined(SK_DISABLE_SDF_TEXT)
        case SK_DistanceFieldInset:
            // The SDFT case.
            // If the srcPadding == SK_DistanceFieldInset (SDFT case) then the padding is built
            // into the image on the glyph; no extra padding needed.
            // TODO: can the SDFT glyph image in the cache be reduced by the padding?
            padding = 0;
            break;
#endif
        default:
            // The padding is not one of the know forms.
            return GrDrawOpAtlas::ErrorCode::kError;
    }

    const int width = skGlyph.width() + 2*padding;
    const int height = skGlyph.height() + 2*padding;
    int rowBytes = width * bytesPerPixel;
    size_t size = height * rowBytes;

    // Temporary storage for normalizing glyph image.
    SkAutoSMalloc<1024> storage(size);
    void* dataPtr = storage.get();
    if (padding > 0) {
        sk_bzero(dataPtr, size);
        // Advance in one row and one column.
        dataPtr = (char*)(dataPtr) + rowBytes + bytesPerPixel;
    }

    get_packed_glyph_image(skGlyph, rowBytes, expectedMaskFormat, dataPtr);

    auto errorCode = this->addToAtlas(resourceProvider,
                                      uploadTarget,
                                      expectedMaskFormat,
                                      width,
                                      height,
                                      storage.get(),
                                      &glyph->fAtlasLocator);

    if (errorCode == GrDrawOpAtlas::ErrorCode::kSucceeded) {
        glyph->fAtlasLocator.insetSrc(srcPadding);
    }

    return errorCode;
}

// add to texture atlas that matches this format
GrDrawOpAtlas::ErrorCode GrAtlasManager::addToAtlas(GrResourceProvider* resourceProvider,
                                                    GrDeferredUploadTarget* target,
                                                    MaskFormat format,
                                                    int width, int height, const void* image,
                                                    skgpu::AtlasLocator* atlasLocator) {
    return this->getAtlas(format)->addToAtlas(resourceProvider, target, width, height, image,
                                              atlasLocator);
}

void GrAtlasManager::addGlyphToBulkAndSetUseToken(skgpu::BulkUsePlotUpdater* updater,
                                                  MaskFormat format, Glyph* glyph,
                                                  skgpu::AtlasToken token) {
    SkASSERT(glyph);
    if (updater->add(glyph->fAtlasLocator)) {
        this->getAtlas(format)->setLastUseToken(glyph->fAtlasLocator, token);
    }
}

#ifdef SK_DEBUG
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SurfaceContext.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include <stdio.h>

/**
  * Write the contents of the surface proxy to a PNG. Returns true if successful.
  * @param filename      Full path to desired file
  */
static bool save_pixels(GrDirectContext* dContext, GrSurfaceProxyView view, GrColorType colorType,
                        const char* filename) {
    if (!view.proxy()) {
        return false;
    }

    auto ii = SkImageInfo::Make(view.proxy()->dimensions(), kRGBA_8888_SkColorType,
                                kPremul_SkAlphaType);
    SkBitmap bm;
    if (!bm.tryAllocPixels(ii)) {
        return false;
    }

    auto sContext = dContext->priv().makeSC(std::move(view),
                                            {colorType, kUnknown_SkAlphaType, nullptr});
    if (!sContext || !sContext->asTextureProxy()) {
        return false;
    }

    bool result = sContext->readPixels(dContext, bm.pixmap(), {0, 0});
    if (!result) {
        SkDebugf("------ failed to read pixels for %s\n", filename);
        return false;
    }

    // remove any previous version of this file
    remove(filename);

    SkFILEWStream file(filename);
    if (!file.isValid()) {
        SkDebugf("------ failed to create file: %s\n", filename);
        remove(filename);   // remove any partial file
        return false;
    }

    if (!SkPngEncoder::Encode(&file, bm.pixmap(), {})) {
        SkDebugf("------ failed to encode %s\n", filename);
        remove(filename);   // remove any partial file
        return false;
    }

    return true;
}

void GrAtlasManager::dump(GrDirectContext* context) const {
    static int gDumpCount = 0;
    for (int i = 0; i < skgpu::kMaskFormatCount; ++i) {
        if (fAtlases[i]) {
            const GrSurfaceProxyView* views = fAtlases[i]->getViews();
            for (uint32_t pageIdx = 0; pageIdx < fAtlases[i]->numActivePages(); ++pageIdx) {
                SkASSERT(views[pageIdx].proxy());
                SkString filename;
#ifdef SK_BUILD_FOR_ANDROID
                filename.printf("/sdcard/fontcache_%d%d%d.png", gDumpCount, i, pageIdx);
#else
                filename.printf("fontcache_%d%d%d.png", gDumpCount, i, pageIdx);
#endif
                SkColorType ct = MaskFormatToColorType(AtlasIndexToMaskFormat(i));
                save_pixels(context, views[pageIdx], SkColorTypeToGrColorType(ct),
                            filename.c_str());
            }
        }
    }
    ++gDumpCount;
}
#endif

void GrAtlasManager::setAtlasDimensionsToMinimum_ForTesting() {
    // Delete any old atlases.
    // This should be safe to do as long as we are not in the middle of a flush.
    for (int i = 0; i < skgpu::kMaskFormatCount; i++) {
        fAtlases[i] = nullptr;
    }

    // Set all the atlas sizes to 1x1 plot each.
    new (&fAtlasConfig) GrDrawOpAtlasConfig{};
}

bool GrAtlasManager::initAtlas(MaskFormat format) {
    int index = MaskFormatToAtlasIndex(format);
    if (fAtlases[index] == nullptr) {
        SkColorType colorType = MaskFormatToColorType(format);
        GrColorType grColorType = SkColorTypeToGrColorType(colorType);
        SkISize atlasDimensions = fAtlasConfig.atlasDimensions(format);
        SkISize plotDimensions = fAtlasConfig.plotDimensions(format);

        const GrBackendFormat backendFormat =
                fCaps->getDefaultBackendFormat(grColorType, GrRenderable::kNo);

        fAtlases[index] = GrDrawOpAtlas::Make(fProxyProvider, backendFormat,
                                              GrColorTypeToSkColorType(grColorType),
                                              GrColorTypeBytesPerPixel(grColorType),
                                              atlasDimensions.width(), atlasDimensions.height(),
                                              plotDimensions.width(), plotDimensions.height(),
                                              this,
                                              fAllowMultitexturing,
                                              nullptr,
                                              /*label=*/"TextAtlas");
        if (!fAtlases[index]) {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////

namespace sktext::gpu {

std::tuple<bool, int> GlyphVector::regenerateAtlasForGanesh(
        int begin, int end, MaskFormat maskFormat, int srcPadding, GrMeshDrawTarget* target) {
    GrAtlasManager* atlasManager = target->atlasManager();
    GrDeferredUploadTarget* uploadTarget = target->deferredUploadTarget();

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(maskFormat);

    this->packedGlyphIDToGlyph(target->strikeCache());

    if (fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls..
        fBulkUseUpdater.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fTextStrike->strikeSpec()};

        // Update the atlas information in the GrStrike.
        auto tokenTracker = uploadTarget->tokenTracker();
        auto glyphs = fGlyphs.subspan(begin, end - begin);
        int glyphsPlacedInAtlas = 0;
        bool success = true;
        for (const Variant& variant : glyphs) {
            Glyph* gpuGlyph = variant.glyph;
            SkASSERT(gpuGlyph != nullptr);

            if (!atlasManager->hasGlyph(maskFormat, gpuGlyph)) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(gpuGlyph->fPackedID);
                auto code = atlasManager->addGlyphToAtlas(
                        skGlyph, gpuGlyph, srcPadding, target->resourceProvider(), uploadTarget);
                if (code != GrDrawOpAtlas::ErrorCode::kSucceeded) {
                    success = code != GrDrawOpAtlas::ErrorCode::kError;
                    break;
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(
                    &fBulkUseUpdater, maskFormat, gpuGlyph,
                    tokenTracker->nextDrawToken());
            glyphsPlacedInAtlas++;
        }

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == SkCount(fGlyphs)) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(maskFormat);
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == SkCount(fGlyphs)) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            atlasManager->setUseTokenBulk(fBulkUseUpdater,
                                          uploadTarget->tokenTracker()->nextDrawToken(),
                                          maskFormat);
        }
        return {true, end - begin};
    }
}

}  // namespace sktext::gpu
