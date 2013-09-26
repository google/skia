
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrAtlas.h"
#include "GrContext.h"
#include "GrGpu.h"
#include "GrRectanizer.h"
#include "GrPlotMgr.h"

#if 0
#define GR_PLOT_WIDTH   8
#define GR_PLOT_HEIGHT  4
#define GR_ATLAS_WIDTH  256
#define GR_ATLAS_HEIGHT 256

#define GR_ATLAS_TEXTURE_WIDTH  (GR_PLOT_WIDTH * GR_ATLAS_WIDTH)
#define GR_ATLAS_TEXTURE_HEIGHT (GR_PLOT_HEIGHT * GR_ATLAS_HEIGHT)

#else

#define GR_ATLAS_TEXTURE_WIDTH  1024
#define GR_ATLAS_TEXTURE_HEIGHT 2048

#define GR_ATLAS_WIDTH  341
#define GR_ATLAS_HEIGHT 341

#define GR_PLOT_WIDTH   (GR_ATLAS_TEXTURE_WIDTH / GR_ATLAS_WIDTH)
#define GR_PLOT_HEIGHT  (GR_ATLAS_TEXTURE_HEIGHT / GR_ATLAS_HEIGHT)

#endif

///////////////////////////////////////////////////////////////////////////////

#define BORDER      1

#ifdef SK_DEBUG
    static int gCounter;
#endif

// for testing
#define FONT_CACHE_STATS 0
#if FONT_CACHE_STATS
static int g_UploadCount = 0;
#endif

GrAtlas::GrAtlas(GrAtlasMgr* mgr, int plotX, int plotY, int bpp) :
                 fDrawToken(NULL, 0) {
    fAtlasMgr = mgr;    // just a pointer, not an owner
    fNext = NULL;

    fTexture = mgr->getTexture(); // we're not an owner, just a pointer
    fPlot.set(plotX, plotY);

    fRects = GrRectanizer::Factory(GR_ATLAS_WIDTH - BORDER,
                                   GR_ATLAS_HEIGHT - BORDER);

    fBytesPerPixel = bpp;

#ifdef SK_DEBUG
//    GrPrintf(" GrAtlas %p [%d %d] %d\n", this, plotX, plotY, gCounter);
    gCounter += 1;
#endif
}

GrAtlas::~GrAtlas() {
    fAtlasMgr->freePlot(fPlot.fX, fPlot.fY);

    delete fRects;

#ifdef SK_DEBUG
    --gCounter;
//    GrPrintf("~GrAtlas %p [%d %d] %d\n", this, fPlot.fX, fPlot.fY, gCounter);
#endif
}

bool GrAtlas::RemoveUnusedAtlases(GrAtlasMgr* atlasMgr, GrAtlas** startAtlas) {
    // GrAtlas** is used so that a pointer to the head element can be passed in and
    // modified when the first element is deleted
    GrAtlas** atlasRef = startAtlas;
    GrAtlas* atlas = *startAtlas;
    bool removed = false;
    while (NULL != atlas) {
        if (atlas->drawToken().isIssued()) {
            *atlasRef = atlas->fNext;
            atlasMgr->deleteAtlas(atlas);
            atlas = *atlasRef;
            removed = true;
        } else {
            atlasRef = &atlas->fNext;
            atlas = atlas->fNext;
        }
    }

    return removed;
}

static void adjustForPlot(GrIPoint16* loc, const GrIPoint16& plot) {
    loc->fX += plot.fX * GR_ATLAS_WIDTH;
    loc->fY += plot.fY * GR_ATLAS_HEIGHT;
}

static uint8_t* zerofill(uint8_t* ptr, int count) {
    while (--count >= 0) {
        *ptr++ = 0;
    }
    return ptr;
}

bool GrAtlas::addSubImage(int width, int height, const void* image,
                          GrIPoint16* loc) {
    if (!fRects->addRect(width + BORDER, height + BORDER, loc)) {
        return false;
    }

    SkAutoSMalloc<1024> storage;
    int dstW = width + 2*BORDER;
    int dstH = height + 2*BORDER;
    if (BORDER) {
        const size_t dstRB = dstW * fBytesPerPixel;
        uint8_t* dst = (uint8_t*)storage.reset(dstH * dstRB);
        sk_bzero(dst, dstRB);                // zero top row
        dst += dstRB;
        for (int y = 0; y < height; y++) {
            dst = zerofill(dst, fBytesPerPixel);   // zero left edge
            memcpy(dst, image, width * fBytesPerPixel);
            dst += width * fBytesPerPixel;
            dst = zerofill(dst, fBytesPerPixel);   // zero right edge
            image = (const void*)((const char*)image + width * fBytesPerPixel);
        }
        sk_bzero(dst, dstRB);                // zero bottom row
        image = storage.get();
    }
    adjustForPlot(loc, fPlot);
    GrContext* context = fTexture->getContext();
    // We pass the flag that does not force a flush. We assume our caller is
    // smart and hasn't referenced the part of the texture we're about to update
    // since the last flush.
    context->writeTexturePixels(fTexture,
                                loc->fX, loc->fY, dstW, dstH,
                                fTexture->config(), image, 0,
                                GrContext::kDontFlush_PixelOpsFlag);

    // now tell the caller to skip the top/left BORDER
    loc->fX += BORDER;
    loc->fY += BORDER;

#if FONT_CACHE_STATS
    ++g_UploadCount;
#endif

    return true;
}

///////////////////////////////////////////////////////////////////////////////

GrAtlasMgr::GrAtlasMgr(GrGpu* gpu, GrPixelConfig config) {
    fGpu = gpu;
    fPixelConfig = config;
    gpu->ref();
    fTexture = NULL;
    fPlotMgr = SkNEW_ARGS(GrPlotMgr, (GR_PLOT_WIDTH, GR_PLOT_HEIGHT));
}

GrAtlasMgr::~GrAtlasMgr() {
    SkSafeUnref(fTexture);
    delete fPlotMgr;

    fGpu->unref();
#if FONT_CACHE_STATS
      GrPrintf("Num uploads: %d\n", g_UploadCount);
#endif
}

GrAtlas* GrAtlasMgr::addToAtlas(GrAtlas** atlas,
                                int width, int height, const void* image,
                                GrIPoint16* loc) {
    // iterate through entire atlas list, see if we can find a hole
    GrAtlas* atlasIter = *atlas;
    while (atlasIter) {
        if (atlasIter->addSubImage(width, height, image, loc)) {
            return atlasIter;
        }
        atlasIter = atlasIter->fNext;
    }

    // If the above fails, then either we have no starting atlas, or the current
    // atlas list is full. Either way we need to allocate a new atlas

    GrIPoint16 plot;
    if (!fPlotMgr->newPlot(&plot)) {
        return NULL;
    }

    if (NULL == fTexture) {
        // TODO: Update this to use the cache rather than directly creating a texture.
        GrTextureDesc desc;
        desc.fFlags = kDynamicUpdate_GrTextureFlagBit;
        desc.fWidth = GR_ATLAS_TEXTURE_WIDTH;
        desc.fHeight = GR_ATLAS_TEXTURE_HEIGHT;
        desc.fConfig = fPixelConfig;

        fTexture = fGpu->createTexture(desc, NULL, 0);
        if (NULL == fTexture) {
            return NULL;
        }
    }

    int bpp = GrBytesPerPixel(fPixelConfig);
    GrAtlas* newAtlas = SkNEW_ARGS(GrAtlas, (this, plot.fX, plot.fY, bpp));
    if (!newAtlas->addSubImage(width, height, image, loc)) {
        delete newAtlas;
        return NULL;
    }

    // new atlas, put at head
    newAtlas->fNext = *atlas;
    *atlas = newAtlas;

    return newAtlas;
}

void GrAtlasMgr::freePlot(int x, int y) {
    SkASSERT(fPlotMgr->isBusy(x, y));
    fPlotMgr->freePlot(x, y);
}
