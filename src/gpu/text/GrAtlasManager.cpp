/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/text/GrAtlasManager.h"

#include "src/codec/SkMasks.h"
#include "src/core/SkAutoMalloc.h"
#include "src/gpu/GrGlyph.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/text/GrStrikeCache.h"

GrAtlasManager::GrAtlasManager(GrProxyProvider* proxyProvider,
                               size_t maxTextureBytes,
                               GrDrawOpAtlas::AllowMultitexturing allowMultitexturing)
            : fAllowMultitexturing{allowMultitexturing}
            , fProxyProvider{proxyProvider}
            , fCaps{fProxyProvider->refCaps()}
            , fAtlasConfig{fCaps->maxTextureSize(), maxTextureBytes} { }

GrAtlasManager::~GrAtlasManager() = default;

void GrAtlasManager::freeAll() {
    for (int i = 0; i < kMaskFormatCount; ++i) {
        fAtlases[i] = nullptr;
    }
}

bool GrAtlasManager::hasGlyph(GrMaskFormat format, GrGlyph* glyph) {
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
    for (int i = 0; i < height; ++i) {
        int rowWritesLeft = width;
        const uint8_t* s = src;
        INT_TYPE* d = dst;
        while (rowWritesLeft > 0) {
            unsigned mask = *s++;
            for (int i = 7; i >= 0 && rowWritesLeft; --i, --rowWritesLeft) {
                *d++ = (mask & (1 << i)) ? (INT_TYPE)(~0UL) : 0;
            }
        }
        dst = reinterpret_cast<INT_TYPE*>(reinterpret_cast<intptr_t>(dst) + dstRowBytes);
        src += srcRowBytes;
    }
}

static void get_packed_glyph_image(
        const SkGlyph& glyph, int dstRB, GrMaskFormat expectedMaskFormat, void* dst) {
    const int width = glyph.width();
    const int height = glyph.height();
    const void* src = glyph.image();
    SkASSERT(src != nullptr);

    GrMaskFormat grMaskFormat = GrGlyph::FormatFromSkGlyph(glyph.maskFormat());
    if (grMaskFormat == expectedMaskFormat) {
        int srcRB = glyph.rowBytes();
        // Notice this comparison is with the glyphs raw mask format, and not its GrMaskFormat.
        if (glyph.maskFormat() != SkMask::kBW_Format) {
            if (srcRB != dstRB) {
                const int bbp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
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
                case kA8_GrMaskFormat: {
                    uint8_t* bytes = reinterpret_cast<uint8_t*>(dst);
                    expand_bits(bytes, bits, width, height, dstRB, srcRB);
                    break;
                }
                case kA565_GrMaskFormat: {
                    uint16_t* rgb565 = reinterpret_cast<uint16_t*>(dst);
                    expand_bits(rgb565, bits, width, height, dstRB, srcRB);
                    break;
                }
                default:
                    SK_ABORT("Invalid GrMaskFormat");
            }
        }
    } else if (grMaskFormat == kA565_GrMaskFormat && expectedMaskFormat == kARGB_GrMaskFormat) {
        // Convert if the glyph uses a 565 mask format since it is using LCD text rendering
        // but the expected format is 8888 (will happen on macOS with Metal since that
        // combination does not support 565).
        static constexpr SkMasks masks{
                {0b1111'1000'0000'0000, 11, 5},  // Red
                {0b0000'0111'1110'0000,  5, 6},  // Green
                {0b0000'0000'0001'1111,  0, 5},  // Blue
                {0, 0, 0}                        // Alpha
        };
        constexpr int a565Bpp = GrMaskFormatBytesPerPixel(kA565_GrMaskFormat);
        constexpr int argbBpp = GrMaskFormatBytesPerPixel(kARGB_GrMaskFormat);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint16_t color565 = 0;
                memcpy(&color565, src, a565Bpp);
                uint32_t colorRGBA = GrColorPackRGBA(masks.getRed(color565),
                                                     masks.getGreen(color565),
                                                     masks.getBlue(color565),
                                                     0xFF);
                memcpy(dst, &colorRGBA, argbBpp);
                src = (char*)src + a565Bpp;
                dst = (char*)dst + argbBpp;
            }
        }
    } else {
        // crbug:510931
        // Retrieving the image from the cache can actually change the mask format. This case is
        // very uncommon so for now we just draw a clear box for these glyphs.
        const int bpp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
        for (int y = 0; y < height; y++) {
            sk_bzero(dst, width * bpp);
            dst = (char*)dst + dstRB;
        }
    }
}

// returns true if glyph successfully added to texture atlas, false otherwise.  If the glyph's
// mask format has changed, then addGlyphToAtlas will draw a clear box.  This will almost never
// happen.
// TODO we can handle some of these cases if we really want to, but the long term solution is to
// get the actual glyph image itself when we get the glyph metrics.
GrDrawOpAtlas::ErrorCode GrAtlasManager::addGlyphToAtlas(const SkGlyph& skGlyph,
                                                         GrGlyph* grGlyph,
                                                         int srcPadding,
                                                         GrResourceProvider* resourceProvider,
                                                         GrDeferredUploadTarget* uploadTarget,
                                                         bool bilerpPadding) {
    if (skGlyph.image() == nullptr) {
        return GrDrawOpAtlas::ErrorCode::kError;
    }
    SkASSERT(grGlyph != nullptr);

    GrMaskFormat glyphFormat = GrGlyph::FormatFromSkGlyph(skGlyph.maskFormat());
    GrMaskFormat expectedMaskFormat = this->resolveMaskFormat(glyphFormat);
    int bytesPerPixel = GrMaskFormatBytesPerPixel(expectedMaskFormat);

    // Add 1 pixel padding around grGlyph if needed.
    int padding = bilerpPadding ? 1 : 0;
    const int width = skGlyph.width() + 2*padding;
    const int height = skGlyph.height() + 2*padding;
    int rowBytes = width * bytesPerPixel;
    size_t size = height * rowBytes;

    // Temporary storage for normalizing grGlyph image.
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
                                      &grGlyph->fAtlasLocator);

    if (errorCode == GrDrawOpAtlas::ErrorCode::kSucceeded) {
        grGlyph->fAtlasLocator.insetSrc(srcPadding);
    }

    return errorCode;
}

// add to texture atlas that matches this format
GrDrawOpAtlas::ErrorCode GrAtlasManager::addToAtlas(GrResourceProvider* resourceProvider,
                                                    GrDeferredUploadTarget* target,
                                                    GrMaskFormat format,
                                                    int width, int height, const void* image,
                                                    GrDrawOpAtlas::AtlasLocator* atlasLocator) {
    return this->getAtlas(format)->addToAtlas(resourceProvider, target, width, height, image,
                                              atlasLocator);
}

void GrAtlasManager::addGlyphToBulkAndSetUseToken(GrDrawOpAtlas::BulkUseTokenUpdater* updater,
                                                  GrMaskFormat format, GrGlyph* glyph,
                                                  GrDeferredUploadToken token) {
    SkASSERT(glyph);
    if (updater->add(glyph->fAtlasLocator)) {
        this->getAtlas(format)->setLastUseToken(glyph->fAtlasLocator, token);
    }
}

#ifdef SK_DEBUG
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrTextureProxy.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImageEncoder.h"
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

    auto sContext = GrSurfaceContext::Make(dContext,
                                           std::move(view),
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

    if (!SkEncodeImage(&file, bm, SkEncodedImageFormat::kPNG, 100)) {
        SkDebugf("------ failed to encode %s\n", filename);
        remove(filename);   // remove any partial file
        return false;
    }

    return true;
}

void GrAtlasManager::dump(GrDirectContext* context) const {
    static int gDumpCount = 0;
    for (int i = 0; i < kMaskFormatCount; ++i) {
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
                auto ct = GrMaskFormatToColorType(AtlasIndexToMaskFormat(i));
                save_pixels(context, views[pageIdx], ct, filename.c_str());
            }
        }
    }
    ++gDumpCount;
}
#endif

void GrAtlasManager::setAtlasDimensionsToMinimum_ForTesting() {
    // Delete any old atlases.
    // This should be safe to do as long as we are not in the middle of a flush.
    for (int i = 0; i < kMaskFormatCount; i++) {
        fAtlases[i] = nullptr;
    }

    // Set all the atlas sizes to 1x1 plot each.
    new (&fAtlasConfig) GrDrawOpAtlasConfig{};
}

bool GrAtlasManager::initAtlas(GrMaskFormat format) {
    int index = MaskFormatToAtlasIndex(format);
    if (fAtlases[index] == nullptr) {
        GrColorType grColorType = GrMaskFormatToColorType(format);
        SkISize atlasDimensions = fAtlasConfig.atlasDimensions(format);
        SkISize plotDimensions = fAtlasConfig.plotDimensions(format);

        const GrBackendFormat format = fCaps->getDefaultBackendFormat(grColorType,
                                                                      GrRenderable::kNo);

        fAtlases[index] = GrDrawOpAtlas::Make(fProxyProvider, format, grColorType,
                                              atlasDimensions.width(), atlasDimensions.height(),
                                              plotDimensions.width(), plotDimensions.height(),
                                              this, fAllowMultitexturing, nullptr);
        if (!fAtlases[index]) {
            return false;
        }
    }
    return true;
}
