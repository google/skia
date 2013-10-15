
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

#define BORDER      1

#ifdef SK_DEBUG
    static int gCounter;
#endif

// for testing
#define FONT_CACHE_STATS 0
#if FONT_CACHE_STATS
static int g_UploadCount = 0;
#endif

GrPlot::GrPlot() : fDrawToken(NULL, 0)
                 , fNext(NULL)
                 , fTexture(NULL)
                 , fAtlasMgr(NULL)
                 , fBytesPerPixel(1)
{
    fRects = GrRectanizer::Factory(GR_ATLAS_WIDTH - BORDER,
                                   GR_ATLAS_HEIGHT - BORDER);
    fOffset.set(0, 0);
}

GrPlot::~GrPlot() {
    delete fRects;
}

static inline void adjust_for_offset(GrIPoint16* loc, const GrIPoint16& offset) {
    loc->fX += offset.fX * GR_ATLAS_WIDTH;
    loc->fY += offset.fY * GR_ATLAS_HEIGHT;
}

static inline uint8_t* zero_fill(uint8_t* ptr, size_t count) {
    sk_bzero(ptr, count);
    return ptr + count;
}

bool GrPlot::addSubImage(int width, int height, const void* image,
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
            dst = zero_fill(dst, fBytesPerPixel);   // zero left edge
            memcpy(dst, image, width * fBytesPerPixel);
            dst += width * fBytesPerPixel;
            dst = zero_fill(dst, fBytesPerPixel);   // zero right edge
            image = (const void*)((const char*)image + width * fBytesPerPixel);
        }
        sk_bzero(dst, dstRB);                // zero bottom row
        image = storage.get();
    }
    adjust_for_offset(loc, fOffset);
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

    // set up allocated plots
    size_t bpp = GrBytesPerPixel(fPixelConfig);
    fPlots = SkNEW_ARRAY(GrPlot, (GR_PLOT_WIDTH*GR_PLOT_HEIGHT));
    fFreePlots = NULL;
    GrPlot* currPlot = fPlots;
    for (int y = GR_PLOT_HEIGHT-1; y >= 0; --y) {
        for (int x = GR_PLOT_WIDTH-1; x >= 0; --x) {
            currPlot->fAtlasMgr = this;
            currPlot->fOffset.set(x, y);
            currPlot->fBytesPerPixel = bpp;

            // add to free list
            currPlot->fNext = fFreePlots;
            fFreePlots = currPlot;

            ++currPlot;
        }
    }
}

GrAtlasMgr::~GrAtlasMgr() {
    SkSafeUnref(fTexture);
    SkDELETE_ARRAY(fPlots);

    fGpu->unref();
#if FONT_CACHE_STATS
      GrPrintf("Num uploads: %d\n", g_UploadCount);
#endif
}

GrPlot* GrAtlasMgr::addToAtlas(GrAtlas* atlas,
                               int width, int height, const void* image,
                               GrIPoint16* loc) {
    // iterate through entire plot list, see if we can find a hole
    GrPlot* plotIter = atlas->fPlots;
    while (plotIter) {
        if (plotIter->addSubImage(width, height, image, loc)) {
            return plotIter;
        }
        plotIter = plotIter->fNext;
    }

    // If the above fails, then either we have no starting plot, or the current
    // plot list is full. Either way we need to allocate a new plot
    GrPlot* newPlot = this->allocPlot();
    if (NULL == newPlot) {
        return NULL;
    }

    if (NULL == fTexture) {
        // TODO: Update this to use the cache rather than directly creating a texture.
        GrTextureDesc desc;
#ifdef SK_DEVELOPER
        // RenderTarget so we can read the pixels to dump them
        // TODO: Fix to support RT
        desc.fFlags = kDynamicUpdate_GrTextureFlagBit;//|kRenderTarget_GrTextureFlagBit;
                                                      //|kNoStencil_GrTextureFlagBit;
#else
        desc.fFlags = kDynamicUpdate_GrTextureFlagBit;
#endif
        desc.fWidth = GR_ATLAS_TEXTURE_WIDTH;
        desc.fHeight = GR_ATLAS_TEXTURE_HEIGHT;
        desc.fConfig = fPixelConfig;

        fTexture = fGpu->createTexture(desc, NULL, 0);
        if (NULL == fTexture) {
            return NULL;
        }
    }
    // be sure to set texture for fast lookup
    newPlot->fTexture = fTexture;

    if (!newPlot->addSubImage(width, height, image, loc)) {
        this->freePlot(newPlot);
        return NULL;
    }

    // new plot, put at head
    newPlot->fNext = atlas->fPlots;
    atlas->fPlots = newPlot;

    return newPlot;
}

bool GrAtlasMgr::removeUnusedPlots(GrAtlas* atlas) {

    // GrPlot** is used so that the head element can be easily
    // modified when the first element is deleted
    GrPlot** plotRef = &atlas->fPlots;
    GrPlot* plot = atlas->fPlots;
    bool removed = false;
    while (NULL != plot) {
        if (plot->drawToken().isIssued()) {
            *plotRef = plot->fNext;
            this->freePlot(plot);
            plot = *plotRef;
            removed = true;
        } else {
            plotRef = &plot->fNext;
            plot = plot->fNext;
        }
    }

    return removed;
}

void GrAtlasMgr::deletePlotList(GrPlot* plot) {
    while (NULL != plot) {
        GrPlot* next = plot->fNext;
        this->freePlot(plot);
        plot = next;
    }
}

GrPlot* GrAtlasMgr::allocPlot() {
    if (NULL == fFreePlots) {
        return NULL;
    } else {
        GrPlot* alloc = fFreePlots;
        fFreePlots = alloc->fNext;
#ifdef SK_DEBUG
//        GrPrintf(" GrPlot %p [%d %d] %d\n", this, alloc->fOffset.fX, alloc->fOffset.fY, gCounter);
        gCounter += 1;
#endif
        return alloc;
    }

}

void GrAtlasMgr::freePlot(GrPlot* plot) {
    SkASSERT(this == plot->fAtlasMgr);

    plot->fRects->reset();
    plot->fNext = fFreePlots;
    fFreePlots = plot;

#ifdef SK_DEBUG
    --gCounter;
//    GrPrintf("~GrPlot %p [%d %d] %d\n", this, plot->fOffset.fX, plot->fOffset.fY, gCounter);
#endif
}
