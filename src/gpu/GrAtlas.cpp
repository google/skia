
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

#define GR_ATLAS_WIDTH  256
#define GR_ATLAS_HEIGHT 256

#define GR_PLOT_WIDTH   (GR_ATLAS_TEXTURE_WIDTH / GR_ATLAS_WIDTH)
#define GR_PLOT_HEIGHT  (GR_ATLAS_TEXTURE_HEIGHT / GR_ATLAS_HEIGHT)

#endif

///////////////////////////////////////////////////////////////////////////////

// for testing
#define FONT_CACHE_STATS 0
#if FONT_CACHE_STATS
static int g_UploadCount = 0;
#endif

GrPlot::GrPlot() : fDrawToken(NULL, 0)
                 , fTexture(NULL)
                 , fAtlasMgr(NULL)
                 , fBytesPerPixel(1)
{
    fRects = GrRectanizer::Factory(GR_ATLAS_WIDTH,
                                   GR_ATLAS_HEIGHT);
    fOffset.set(0, 0);
}

GrPlot::~GrPlot() {
    delete fRects;
}

static inline void adjust_for_offset(GrIPoint16* loc, const GrIPoint16& offset) {
    loc->fX += offset.fX * GR_ATLAS_WIDTH;
    loc->fY += offset.fY * GR_ATLAS_HEIGHT;
}

bool GrPlot::addSubImage(int width, int height, const void* image,
                          GrIPoint16* loc) {
    if (!fRects->addRect(width, height, loc)) {
        return false;
    }

    SkAutoSMalloc<1024> storage;
    adjust_for_offset(loc, fOffset);
    GrContext* context = fTexture->getContext();
    // We pass the flag that does not force a flush. We assume our caller is
    // smart and hasn't referenced the part of the texture we're about to update
    // since the last flush.
    context->writeTexturePixels(fTexture,
                                loc->fX, loc->fY, width, height,
                                fTexture->config(), image, 0,
                                GrContext::kDontFlush_PixelOpsFlag);

#if FONT_CACHE_STATS
    ++g_UploadCount;
#endif

    return true;
}

void GrPlot::resetRects() {
    SkASSERT(NULL != fRects);
    fRects->reset();
}

///////////////////////////////////////////////////////////////////////////////

GrAtlasMgr::GrAtlasMgr(GrGpu* gpu, GrPixelConfig config) {
    fGpu = gpu;
    fPixelConfig = config;
    gpu->ref();
    fTexture = NULL;

    // set up allocated plots
    size_t bpp = GrBytesPerPixel(fPixelConfig);
    fPlotArray = SkNEW_ARRAY(GrPlot, (GR_PLOT_WIDTH*GR_PLOT_HEIGHT));

    GrPlot* currPlot = fPlotArray;
    for (int y = GR_PLOT_HEIGHT-1; y >= 0; --y) {
        for (int x = GR_PLOT_WIDTH-1; x >= 0; --x) {
            currPlot->fAtlasMgr = this;
            currPlot->fOffset.set(x, y);
            currPlot->fBytesPerPixel = bpp;

            // build LRU list
            fPlotList.addToHead(currPlot);
            ++currPlot;
        }
    }
}

GrAtlasMgr::~GrAtlasMgr() {
    SkSafeUnref(fTexture);
    SkDELETE_ARRAY(fPlotArray);

    fGpu->unref();
#if FONT_CACHE_STATS
      GrPrintf("Num uploads: %d\n", g_UploadCount);
#endif
}

void GrAtlasMgr::moveToHead(GrPlot* plot) {
    if (fPlotList.head() == plot) {
        return;
    }

    fPlotList.remove(plot);
    fPlotList.addToHead(plot);
};

GrPlot* GrAtlasMgr::addToAtlas(GrAtlas* atlas,
                               int width, int height, const void* image,
                               GrIPoint16* loc) {
    // iterate through entire plot list for this atlas, see if we can find a hole
    // last one was most recently added and probably most empty
    for (int i = atlas->fPlots.count()-1; i >= 0; --i) {
        GrPlot* plot = atlas->fPlots[i];
        if (plot->addSubImage(width, height, image, loc)) {
            this->moveToHead(plot);
            return plot;
        }
    }

    // before we get a new plot, make sure we have a backing texture
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

    // now look through all allocated plots for one we can share, in MRU order
    GrPlotList::Iter plotIter;
    plotIter.init(fPlotList, GrPlotList::Iter::kHead_IterStart);
    GrPlot* plot;
    while (NULL != (plot = plotIter.get())) {
        // make sure texture is set for quick lookup
        plot->fTexture = fTexture;
        if (plot->addSubImage(width, height, image, loc)) {
            this->moveToHead(plot);
            // new plot for atlas, put at end of array
            *(atlas->fPlots.append()) = plot;
            return plot;
        }
        plotIter.next();
    }

    // If the above fails, then the current plot list has no room
    return NULL;
}

bool GrAtlasMgr::removePlot(GrAtlas* atlas, const GrPlot* plot) {
    // iterate through plot list for this atlas
    int count = atlas->fPlots.count();
    for (int i = 0; i < count; ++i) {
        if (plot == atlas->fPlots[i]) {
            atlas->fPlots.remove(i);
            return true;
        }
    }

    return false;
}

// get a plot that's not being used by the current draw
GrPlot* GrAtlasMgr::getUnusedPlot() {
    GrPlotList::Iter plotIter;
    plotIter.init(fPlotList, GrPlotList::Iter::kTail_IterStart);
    GrPlot* plot;
    while (NULL != (plot = plotIter.get())) {
        if (plot->drawToken().isIssued()) {
            return plot;
        }
        plotIter.prev();
    }

    return NULL;
}

SkISize GrAtlas::getSize() const {
    return SkISize::Make(GR_ATLAS_TEXTURE_WIDTH, GR_ATLAS_TEXTURE_HEIGHT);
}
