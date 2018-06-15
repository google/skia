/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasManager.h"

#include "GrGlyph.h"
#include "GrGlyphCache.h"

void GrAtlasManager::ComputeAtlasLimits(int maxTextureSize, size_t maxTextureBytes, int* maxDim,
                                        int* minDim, int* maxPlot, int* minPlot) {
    SkASSERT(maxDim && minDim && maxPlot && minPlot);
    // Calculate RGBA size. Must be between 512 x 256 and MaxTextureSize x MaxTextureSize / 2
    int log2MaxTextureSize = SkPrevLog2(maxTextureSize);
    int log2MaxDim = 9;
    static const size_t kOne = 1u;
    for (; log2MaxDim <= log2MaxTextureSize; ++log2MaxDim) {
        size_t maxDimTmp = kOne << log2MaxDim;
        size_t minDimTmp = kOne << (log2MaxDim - 1);

        if (maxDimTmp * minDimTmp * 4 >= maxTextureBytes) {
            break;
        }
    }


    int log2MinDim = log2MaxDim - 1;
    *maxDim = 1 << log2MaxDim;
    *minDim = 1 << log2MinDim;
    // Plots are either 256 or 512.
    *maxPlot = SkTMin(512, SkTMax(256, 1 << (log2MaxDim - 2)));
    *minPlot = SkTMin(512, SkTMax(256, 1 << (log2MaxDim - 3)));
}

GrAtlasManager::GrAtlasManager(GrProxyProvider* proxyProvider, GrGlyphCache* glyphCache,
                               float maxTextureBytes,
                               GrDrawOpAtlas::AllowMultitexturing allowMultitexturing)
            : fAllowMultitexturing(allowMultitexturing)
            , fProxyProvider(proxyProvider)
            , fGlyphCache(glyphCache) {
    fCaps = fProxyProvider->refCaps();

    int maxDim, minDim, maxPlot, minPlot;
    ComputeAtlasLimits(fCaps->maxTextureSize(), maxTextureBytes, &maxDim, &minDim, &maxPlot,
                       &minPlot);

    // Setup default atlas configs. The A8 atlas uses maxDim for both width and height, as the A8
    // format is already very compact.
    fAtlasConfigs[kA8_GrMaskFormat].fWidth = maxDim;
    fAtlasConfigs[kA8_GrMaskFormat].fHeight = maxDim;
    fAtlasConfigs[kA8_GrMaskFormat].fPlotWidth = maxPlot;
    fAtlasConfigs[kA8_GrMaskFormat].fPlotHeight = minPlot;

    // A565 and ARGB use maxDim x minDim.
    fAtlasConfigs[kA565_GrMaskFormat].fWidth = minDim;
    fAtlasConfigs[kA565_GrMaskFormat].fHeight = maxDim;
    fAtlasConfigs[kA565_GrMaskFormat].fPlotWidth = minPlot;
    fAtlasConfigs[kA565_GrMaskFormat].fPlotHeight = minPlot;

    fAtlasConfigs[kARGB_GrMaskFormat].fWidth = minDim;
    fAtlasConfigs[kARGB_GrMaskFormat].fHeight = maxDim;
    fAtlasConfigs[kARGB_GrMaskFormat].fPlotWidth = minPlot;
    fAtlasConfigs[kARGB_GrMaskFormat].fPlotHeight = minPlot;

    fGlyphSizeLimit = minPlot;
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
