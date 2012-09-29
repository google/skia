
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

#if GR_DEBUG
    static int gCounter;
#endif

GrAtlas::GrAtlas(GrAtlasMgr* mgr, int plotX, int plotY, GrMaskFormat format) {
    fAtlasMgr = mgr;    // just a pointer, not an owner
    fNext = NULL;
    fTexture = mgr->getTexture(format); // we're not an owner, just a pointer
    fPlot.set(plotX, plotY);

    fRects = GrRectanizer::Factory(GR_ATLAS_WIDTH - BORDER,
                                   GR_ATLAS_HEIGHT - BORDER);

    fMaskFormat = format;

#if GR_DEBUG
//    GrPrintf(" GrAtlas %p [%d %d] %d\n", this, plotX, plotY, gCounter);
    gCounter += 1;
#endif
}

GrAtlas::~GrAtlas() {
    fAtlasMgr->freePlot(fPlot.fX, fPlot.fY);

    delete fRects;

#if GR_DEBUG
    --gCounter;
//    GrPrintf("~GrAtlas %p [%d %d] %d\n", this, fPlot.fX, fPlot.fY, gCounter);
#endif
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
        const int bpp = GrMaskFormatBytesPerPixel(fMaskFormat);
        const size_t dstRB = dstW * bpp;
        uint8_t* dst = (uint8_t*)storage.reset(dstH * dstRB);
        Gr_bzero(dst, dstRB);                // zero top row
        dst += dstRB;
        for (int y = 0; y < height; y++) {
            dst = zerofill(dst, bpp);   // zero left edge
            memcpy(dst, image, width * bpp);
            dst += width * bpp;
            dst = zerofill(dst, bpp);   // zero right edge
            image = (const void*)((const char*)image + width * bpp);
        }
        Gr_bzero(dst, dstRB);                // zero bottom row
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
    return true;
}

///////////////////////////////////////////////////////////////////////////////

GrAtlasMgr::GrAtlasMgr(GrGpu* gpu) {
    fGpu = gpu;
    gpu->ref();
    Gr_bzero(fTexture, sizeof(fTexture));
    fPlotMgr = SkNEW_ARGS(GrPlotMgr, (GR_PLOT_WIDTH, GR_PLOT_HEIGHT));
}

GrAtlasMgr::~GrAtlasMgr() {
    for (size_t i = 0; i < GR_ARRAY_COUNT(fTexture); i++) {
        GrSafeUnref(fTexture[i]);
    }
    delete fPlotMgr;
    fGpu->unref();
}

static GrPixelConfig maskformat2pixelconfig(GrMaskFormat format) {
    switch (format) {
        case kA8_GrMaskFormat:
            return kAlpha_8_GrPixelConfig;
        case kA565_GrMaskFormat:
            return kRGB_565_GrPixelConfig;
        case kA888_GrMaskFormat:
            return kSkia8888_PM_GrPixelConfig;
        default:
            GrAssert(!"unknown maskformat");
    }
    return kUnknown_GrPixelConfig;
}

GrAtlas* GrAtlasMgr::addToAtlas(GrAtlas* atlas,
                                int width, int height, const void* image,
                                GrMaskFormat format,
                                GrIPoint16* loc) {
    GrAssert(NULL == atlas || atlas->getMaskFormat() == format);

    if (atlas && atlas->addSubImage(width, height, image, loc)) {
        return atlas;
    }

    // If the above fails, then either we have no starting atlas, or the current
    // one is full. Either way we need to allocate a new atlas

    GrIPoint16 plot;
    if (!fPlotMgr->newPlot(&plot)) {
        return NULL;
    }

    GrAssert(0 == kA8_GrMaskFormat);
    GrAssert(1 == kA565_GrMaskFormat);
    if (NULL == fTexture[format]) {
        GrTextureDesc desc;
        desc.fFlags = kDynamicUpdate_GrTextureFlagBit;
        desc.fWidth = GR_ATLAS_TEXTURE_WIDTH;
        desc.fHeight = GR_ATLAS_TEXTURE_HEIGHT;
        desc.fConfig = maskformat2pixelconfig(format);

        fTexture[format] = fGpu->createTexture(desc, NULL, 0);
        if (NULL == fTexture[format]) {
            return NULL;
        }
    }

    GrAtlas* newAtlas = SkNEW_ARGS(GrAtlas, (this, plot.fX, plot.fY, format));
    if (!newAtlas->addSubImage(width, height, image, loc)) {
        delete newAtlas;
        return NULL;
    }

    newAtlas->fNext = atlas;
    return newAtlas;
}

void GrAtlasMgr::freePlot(int x, int y) {
    GrAssert(fPlotMgr->isBusy(x, y));
    fPlotMgr->freePlot(x, y);
}


