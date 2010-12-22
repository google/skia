/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrAtlas.h"
#include "GrGpu.h"
#include "GrMemory.h"
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

GrAtlas::GrAtlas(GrAtlasMgr* mgr, int plotX, int plotY) {
    fAtlasMgr = mgr;    // just a pointer, not an owner
    fNext = NULL;
    fTexture = mgr->getTexture(); // we're not an owner, just a pointer
    fPlot.set(plotX, plotY);

    fRects = GrRectanizer::Factory(GR_ATLAS_WIDTH - BORDER,
                                   GR_ATLAS_HEIGHT - BORDER);

#if GR_DEBUG
    GrPrintf(" GrAtlas %p [%d %d] %d\n", this, plotX, plotY, gCounter);
    gCounter += 1;
#endif
}

GrAtlas::~GrAtlas() {
    fAtlasMgr->freePlot(fPlot.fX, fPlot.fY);

    delete fRects;

#if GR_DEBUG
    --gCounter;
    GrPrintf("~GrAtlas %p [%d %d] %d\n", this, fPlot.fX, fPlot.fY, gCounter);
#endif
}

static void adjustForPlot(GrIPoint16* loc, const GrIPoint16& plot) {
    loc->fX += plot.fX * GR_ATLAS_WIDTH;
    loc->fY += plot.fY * GR_ATLAS_HEIGHT;
}

bool GrAtlas::addSubImage(int width, int height, const void* image,
                          GrIPoint16* loc) {
    if (!fRects->addRect(width + BORDER, height + BORDER, loc)) {
        return false;
    }

    GrAutoSMalloc<1024> storage;
    int srcW = width + 2*BORDER;
    int srcH = height + 2*BORDER;
    if (BORDER) {
        uint8_t* ptr = (uint8_t*)storage.realloc(srcW * srcH);
        Gr_bzero(ptr, srcW);                // zero top row
        ptr += srcW;
        for (int y = 0; y < height; y++) {
            *ptr++ = 0;                     // zero left edge
            memcpy(ptr, image, width); ptr += width;
            *ptr++ = 0;                     // zero right edge
            image = (const void*)((const char*)image + width);
        }
        Gr_bzero(ptr, srcW);                // zero bottom row
        image = storage.get();
    }
    adjustForPlot(loc, fPlot);
    fTexture->uploadTextureData(loc->fX, loc->fY, srcW, srcH, image);

    // now tell the caller to skip the top/left BORDER
    loc->fX += BORDER;
    loc->fY += BORDER;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

GrAtlasMgr::GrAtlasMgr(GrGpu* gpu) {
    fGpu = gpu;
    gpu->ref();
    fTexture = NULL;
    fPlotMgr = new GrPlotMgr(GR_PLOT_WIDTH, GR_PLOT_HEIGHT);
}

GrAtlasMgr::~GrAtlasMgr() {
    GrSafeUnref(fTexture);
    delete fPlotMgr;
    fGpu->unref();
}

GrAtlas* GrAtlasMgr::addToAtlas(GrAtlas* atlas,
                                int width, int height, const void* image,
                                GrIPoint16* loc) {
    if (atlas && atlas->addSubImage(width, height, image, loc)) {
        return atlas;
    }

    // If the above fails, then either we have no starting atlas, or the current
    // one is full. Either way we need to allocate a new atlas

    GrIPoint16 plot;
    if (!fPlotMgr->newPlot(&plot)) {
        return NULL;
    }

    if (NULL == fTexture) {
        GrGpu::TextureDesc desc = {
            GrGpu::kDynamicUpdate_TextureFlag, 
            GrGpu::kNone_AALevel,
            GR_ATLAS_TEXTURE_WIDTH, 
            GR_ATLAS_TEXTURE_HEIGHT,
            GrTexture::kAlpha_8_PixelConfig
        };
        fTexture = fGpu->createTexture(desc, NULL, 0);
        if (NULL == fTexture) {
            return NULL;
        }
    }

    GrAtlas* newAtlas = new GrAtlas(this, plot.fX, plot.fY);
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

void GrAtlasMgr::abandonAll() {
#if 0
    GrAtlas** curr = fList.begin();
    GrAtlas** stop = fList.end();
    for (; curr < stop; curr++) {
        (*curr)->texture()->abandon();
        delete *curr;
    }
    fList.reset();
#endif
}


