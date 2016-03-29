/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuResourcePriv.h"
#include "GrLayerAtlas.h"
#include "GrRectanizer.h"
#include "GrTextureProvider.h"

///////////////////////////////////////////////////////////////////////////////
GrLayerAtlas::Plot::Plot()
    : fID(-1)
    , fRects(nullptr) {
    fOffset.set(0, 0);
}

GrLayerAtlas::Plot::~Plot() {
    delete fRects;
}

void GrLayerAtlas::Plot::init(int id, int offX, int offY, int width, int height) {
    fID = id;
    fRects = GrRectanizer::Factory(width, height);
    fOffset.set(offX * width, offY * height);
}

bool GrLayerAtlas::Plot::allocateRect(int width, int height, SkIPoint16* loc) {
    if (!fRects->addRect(width, height, loc)) {
        return false;
    }

    loc->fX += fOffset.fX;
    loc->fY += fOffset.fY;
    return true;
}

void GrLayerAtlas::Plot::reset() {
    SkASSERT(fRects);
    fRects->reset();
}

///////////////////////////////////////////////////////////////////////////////
GR_DECLARE_STATIC_UNIQUE_KEY(gLayerAtlasKey);
static const GrUniqueKey& get_layer_atlas_key() {
    GR_DEFINE_STATIC_UNIQUE_KEY(gLayerAtlasKey);
    return gLayerAtlasKey;
}

bool GrLayerAtlas::reattachBackingTexture() {
    SkASSERT(!fTexture);

    fTexture.reset(fTexProvider->findAndRefTextureByUniqueKey(get_layer_atlas_key()));
    return fTexture != nullptr;
}

void GrLayerAtlas::createBackingTexture() {
    SkASSERT(!fTexture);

    GrSurfaceDesc desc;
    desc.fFlags = fFlags;
    desc.fWidth = fBackingTextureSize.width();
    desc.fHeight = fBackingTextureSize.height();
    desc.fConfig = fPixelConfig;

    fTexture.reset(fTexProvider->createTexture(desc, SkBudgeted::kYes, nullptr, 0));

    fTexture->resourcePriv().setUniqueKey(get_layer_atlas_key());
}

GrLayerAtlas::GrLayerAtlas(GrTextureProvider* texProvider, GrPixelConfig config,
                           GrSurfaceFlags flags,
                           const SkISize& backingTextureSize,
                           int numPlotsX, int numPlotsY) {
    fTexProvider = texProvider;
    fPixelConfig = config;
    fFlags = flags;
    fBackingTextureSize = backingTextureSize;

    int textureWidth = fBackingTextureSize.width();
    int textureHeight = fBackingTextureSize.height();

    int plotWidth = textureWidth / numPlotsX;
    int plotHeight = textureHeight / numPlotsY;

    SkASSERT(plotWidth * numPlotsX == textureWidth);
    SkASSERT(plotHeight * numPlotsY == textureHeight);

    // We currently do not support compressed atlases...
    SkASSERT(!GrPixelConfigIsCompressed(config));

    // set up allocated plots
    fPlotArray = new Plot[numPlotsX * numPlotsY];

    Plot* currPlot = fPlotArray;
    for (int y = numPlotsY-1; y >= 0; --y) {
        for (int x = numPlotsX-1; x >= 0; --x) {
            currPlot->init(y*numPlotsX+x, x, y, plotWidth, plotHeight);

            // build LRU list
            fPlotList.addToHead(currPlot);
            ++currPlot;
        }
    }
}

void GrLayerAtlas::resetPlots() {
    PlotIter iter;
    for (Plot* plot = iter.init(fPlotList, PlotIter::kHead_IterStart); plot; plot = iter.next()) {
        plot->reset();
    }
}

GrLayerAtlas::~GrLayerAtlas() {
    delete[] fPlotArray;
}

void GrLayerAtlas::makeMRU(Plot* plot) {
    if (fPlotList.head() == plot) {
        return;
    }

    fPlotList.remove(plot);
    fPlotList.addToHead(plot);
};

GrLayerAtlas::Plot* GrLayerAtlas::addToAtlas(ClientPlotUsage* usage,
                                             int width, int height, SkIPoint16* loc) {
    // Iterate through the plots currently being used by this client and see if we can find a hole.
    // The last one was most recently added and probably most empty.
    // We want to consolidate the uses from individual clients to the same plot(s) so that
    // when a specific client goes away they are more likely to completely empty a plot.
    for (int i = usage->numPlots()-1; i >= 0; --i) {
        Plot* plot = usage->plot(i);
        if (plot->allocateRect(width, height, loc)) {
            this->makeMRU(plot);
            return plot;
        }
    }

    // before we get a new plot, make sure we have a backing texture
    if (nullptr == fTexture) {
        this->createBackingTexture();
        if (nullptr == fTexture) {
            return nullptr;
        }
    }

    // Now look through all allocated plots for one we can share, in MRU order
    // TODO: its seems like traversing from emptiest to fullest would make more sense
    PlotList::Iter plotIter;
    plotIter.init(fPlotList, PlotList::Iter::kHead_IterStart);
    Plot* plot;
    while ((plot = plotIter.get())) {
        if (plot->allocateRect(width, height, loc)) {
            this->makeMRU(plot);
            // new plot for atlas, put at end of array
            usage->appendPlot(plot);
            return plot;
        }
        plotIter.next();
    }

    // If the above fails, then the current plot list has no room
    return nullptr;
}
