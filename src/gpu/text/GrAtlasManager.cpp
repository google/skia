/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasManager.h"

#include <algorithm>
#include <cmath>

#include "GrGlyph.h"
#include "GrGlyphCache.h"
#include "SkGlyphRun.h"

GrDrawOpAtlasConfig GrAtlasManager::ComputeAtlasLimits(
        int maxTextureSize, size_t maxTextureBytes, int pixelSize) {
    // Find the largest area of pixels in a width:height with a proportion of 1:2 that fits in
    // maxTextureBytes. In the following P is pixel size, H is height, and W is width.
    // P*H*W = maxTextureSize => P*H*(H/2) = maxTextureSize => H = sqrt(2*maxTextureSize/P)
    double fitsHeight = std::sqrt(2.0 * maxTextureBytes / pixelSize);

    // The minimum atlas height.
    static constexpr double kMinimumAtlasHeight = 512;

    // Limit height to the maximum texture dimension and the minimum atlas size.
    double height = std::max(std::min(fitsHeight, (double)maxTextureSize), kMinimumAtlasHeight);

    // Find the greatest power of 2 that is less than height.
    double alignedHeight = std::exp2(std::floor(std::log2(height)));

    // Calculate the atlas dimensions.
    int atlasHeight = (int)alignedHeight;
    int atlasWidth = atlasHeight / 2;

    // Constant copied from BulkUseTokenUpdater
    // TODO: unify kMaxPlots.
    static constexpr size_t kMaxPlots = 32;

    // Start with a plot that is large enough to satisfy the maximum glyph dimension limit.
    int plotWidth = SkGlyphCacheCommon::kSkGlyphSideTooBigForAtlas;
    int plotHeight = SkGlyphCacheCommon::kSkGlyphSideTooBigForAtlas;

    auto countPlots = [&plotWidth, &plotHeight, atlasWidth, atlasHeight] () -> size_t {
        return ((size_t)atlasWidth / plotWidth) * ((size_t)atlasHeight / plotHeight);
    };

    // If there are too many plots increase from 256x256 to 256x512.
    if (countPlots() > kMaxPlots) {
        plotWidth = 2 * SkGlyphCacheCommon::kSkGlyphSideTooBigForAtlas;
    }

    // If there are still too many plots increase from 256x512 to 512x512.
    if (countPlots() > kMaxPlots) {
        plotHeight = 2 * SkGlyphCacheCommon::kSkGlyphSideTooBigForAtlas;
    }

    return {atlasWidth, atlasHeight, plotWidth, plotHeight};
}

GrAtlasManager::GrAtlasManager(GrProxyProvider* proxyProvider,
                               GrGlyphCache* glyphCache,
                               size_t maxTextureBytes,
                               GrDrawOpAtlas::AllowMultitexturing allowMultitexturing)
            : fAllowMultitexturing(allowMultitexturing)
            , fProxyProvider(proxyProvider)
            , fGlyphCache(glyphCache) {
    fCaps = fProxyProvider->refCaps();

    SkASSERT(maxTextureBytes > 0);

    int maxTexSize = fCaps->maxTextureSize();
    fAtlasConfigs[kA8_GrMaskFormat] =
            ComputeAtlasLimits(maxTexSize, maxTextureBytes,
                               GrMaskFormatBytesPerPixel(kA8_GrMaskFormat));
    fAtlasConfigs[kA565_GrMaskFormat] =
            ComputeAtlasLimits(maxTexSize, maxTextureBytes,
                               GrMaskFormatBytesPerPixel(kA565_GrMaskFormat));
    fAtlasConfigs[kARGB_GrMaskFormat] =
            ComputeAtlasLimits(maxTexSize, maxTextureBytes,
                               GrMaskFormatBytesPerPixel(kARGB_GrMaskFormat));

    fGlyphSizeLimit = fAtlasConfigs[kARGB_GrMaskFormat].fPlotWidth;
}

GrAtlasManager::~GrAtlasManager() {
}

static GrPixelConfig mask_format_to_pixel_config(GrMaskFormat format) {
    switch (format) {
        case kA8_GrMaskFormat:
            return kAlpha_8_GrPixelConfig;
        case kA565_GrMaskFormat:
            return kRGB_565_GrPixelConfig;
        case kARGB_GrMaskFormat:
            return kRGBA_8888_GrPixelConfig;
        default:
            SkDEBUGFAIL("unsupported GrMaskFormat");
            return kAlpha_8_GrPixelConfig;
    }
}

void GrAtlasManager::freeAll() {
    for (int i = 0; i < kMaskFormatCount; ++i) {
        fAtlases[i] = nullptr;
    }
}

bool GrAtlasManager::hasGlyph(GrGlyph* glyph) {
    SkASSERT(glyph);
    return this->getAtlas(glyph->fMaskFormat)->hasID(glyph->fID);
}

// add to texture atlas that matches this format
GrDrawOpAtlas::ErrorCode GrAtlasManager::addToAtlas(
                                GrResourceProvider* resourceProvider,
                                GrGlyphCache* glyphCache,
                                GrTextStrike* strike, GrDrawOpAtlas::AtlasID* id,
                                GrDeferredUploadTarget* target, GrMaskFormat format,
                                int width, int height, const void* image, SkIPoint16* loc) {
    glyphCache->setStrikeToPreserve(strike);
    return this->getAtlas(format)->addToAtlas(resourceProvider, id, target, width, height,
                                              image, loc);
}

void GrAtlasManager::addGlyphToBulkAndSetUseToken(GrDrawOpAtlas::BulkUseTokenUpdater* updater,
                                                  GrGlyph* glyph,
                                                  GrDeferredUploadToken token) {
    SkASSERT(glyph);
    updater->add(glyph->fID);
    this->getAtlas(glyph->fMaskFormat)->setLastUseToken(glyph->fID, token);
}

#ifdef SK_DEBUG
#include "GrContextPriv.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceContext.h"
#include "GrTextureProxy.h"

#include "SkBitmap.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include <stdio.h>

/**
  * Write the contents of the surface proxy to a PNG. Returns true if successful.
  * @param filename      Full path to desired file
  */
static bool save_pixels(GrContext* context, GrSurfaceProxy* sProxy, const char* filename) {
    if (!sProxy) {
        return false;
    }

    SkImageInfo ii = SkImageInfo::Make(sProxy->width(), sProxy->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bm;
    if (!bm.tryAllocPixels(ii)) {
        return false;
    }

    sk_sp<GrSurfaceContext> sContext(context->contextPriv().makeWrappedSurfaceContext(
                                                                                sk_ref_sp(sProxy)));
    if (!sContext || !sContext->asTextureProxy()) {
        return false;
    }

    bool result = sContext->readPixels(ii, bm.getPixels(), bm.rowBytes(), 0, 0);
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

void GrAtlasManager::dump(GrContext* context) const {
    static int gDumpCount = 0;
    for (int i = 0; i < kMaskFormatCount; ++i) {
        if (fAtlases[i]) {
            const sk_sp<GrTextureProxy>* proxies = fAtlases[i]->getProxies();
            for (uint32_t pageIdx = 0; pageIdx < fAtlases[i]->numActivePages(); ++pageIdx) {
                SkASSERT(proxies[pageIdx]);
                SkString filename;
#ifdef SK_BUILD_FOR_ANDROID
                filename.printf("/sdcard/fontcache_%d%d%d.png", gDumpCount, i, pageIdx);
#else
                filename.printf("fontcache_%d%d%d.png", gDumpCount, i, pageIdx);
#endif

                save_pixels(context, proxies[pageIdx].get(), filename.c_str());
            }
        }
    }
    ++gDumpCount;
}
#endif

void GrAtlasManager::setAtlasSizes_ForTesting(const GrDrawOpAtlasConfig configs[3]) {
    // Delete any old atlases.
    // This should be safe to do as long as we are not in the middle of a flush.
    for (int i = 0; i < kMaskFormatCount; i++) {
        fAtlases[i] = nullptr;
    }
    memcpy(fAtlasConfigs, configs, sizeof(fAtlasConfigs));
}

bool GrAtlasManager::initAtlas(GrMaskFormat format) {
    int index = MaskFormatToAtlasIndex(format);
    if (!fAtlases[index]) {
        GrPixelConfig config = mask_format_to_pixel_config(format);
        int width = fAtlasConfigs[index].fWidth;
        int height = fAtlasConfigs[index].fHeight;
        int numPlotsX = fAtlasConfigs[index].numPlotsX();
        int numPlotsY = fAtlasConfigs[index].numPlotsY();

        fAtlases[index] = GrDrawOpAtlas::Make(fProxyProvider, config, width, height,
                                              numPlotsX, numPlotsY, fAllowMultitexturing,
                                              &GrGlyphCache::HandleEviction,
                                              fGlyphCache);
        if (!fAtlases[index]) {
            return false;
        }
    }
    return true;
}
