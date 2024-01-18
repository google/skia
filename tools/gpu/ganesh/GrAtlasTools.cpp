/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/ganesh/GrAtlasTools.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SurfaceContext.h"

#include <cstdio>
#include <utility>

/**
 * Write the contents of the surface proxy to a PNG. Returns true if successful.
 * @param filename      Full path to desired file
 */
static bool save_pixels(GrDirectContext* dContext,
                        GrSurfaceProxyView view,
                        GrColorType colorType,
                        const char* filename) {
    if (!view.proxy()) {
        return false;
    }

    auto ii = SkImageInfo::Make(
            view.proxy()->dimensions(), kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bm;
    if (!bm.tryAllocPixels(ii)) {
        return false;
    }

    auto sContext =
            dContext->priv().makeSC(std::move(view), {colorType, kUnknown_SkAlphaType, nullptr});
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
        remove(filename);  // remove any partial file
        return false;
    }

    if (!SkPngEncoder::Encode(&file, bm.pixmap(), {})) {
        SkDebugf("------ failed to encode %s\n", filename);
        remove(filename);  // remove any partial file
        return false;
    }

    return true;
}

void GrAtlasManagerTools::Dump(const GrAtlasManager* am, GrDirectContext* context) {
    SkASSERT(am);
    static int gDumpCount = 0;
    for (int i = 0; i < skgpu::kMaskFormatCount; ++i) {
        if (am->fAtlases[i]) {
            const GrSurfaceProxyView* views = am->fAtlases[i]->getViews();
            for (uint32_t pageIdx = 0; pageIdx < am->fAtlases[i]->numActivePages(); ++pageIdx) {
                SkASSERT(views[pageIdx].proxy());
                SkString filename;
#ifdef SK_BUILD_FOR_ANDROID
                filename.printf("/sdcard/fontcache_%d%d%d.png", gDumpCount, i, pageIdx);
#else
                filename.printf("fontcache_%d%d%d.png", gDumpCount, i, pageIdx);
#endif
                SkColorType ct = MaskFormatToColorType(GrAtlasManager::AtlasIndexToMaskFormat(i));
                save_pixels(
                        context, views[pageIdx], SkColorTypeToGrColorType(ct), filename.c_str());
            }
        }
    }
    ++gDumpCount;
}

void GrAtlasManagerTools::SetAtlasDimensionsToMinimum(GrAtlasManager* am) {
    SkASSERT(am);
    // Delete any old atlases.
    // This should be safe to do as long as we are not in the middle of a flush.
    for (int i = 0; i < skgpu::kMaskFormatCount; i++) {
        am->fAtlases[i] = nullptr;
    }

    // Set all the atlas sizes to 1x1 plot each.
    new (&am->fAtlasConfig) GrDrawOpAtlasConfig{};
}

void GrAtlasManagerTools::SetMaxPages(GrAtlasManager* am, uint32_t maxPages) {
    SkASSERT(am);
    for (int i = 0; i < skgpu::kMaskFormatCount; i++) {
        if (am->fAtlases[i]) {
            GrDrawOpAtlasTools::SetMaxPages(am->fAtlases[i].get(), maxPages);
        }
    }
}

int GrDrawOpAtlasTools::NumAllocated(const GrDrawOpAtlas* doa) {
    SkASSERT(doa);
    int count = 0;
    for (uint32_t i = 0; i < doa->maxPages(); ++i) {
        if (doa->fViews[i].proxy()->isInstantiated()) {
            ++count;
        }
    }

    return count;
}

void GrDrawOpAtlasTools::SetMaxPages(GrDrawOpAtlas* doa, uint32_t maxPages) {
    SkASSERT(!doa->fNumActivePages);

    doa->fMaxPages = maxPages;
}
