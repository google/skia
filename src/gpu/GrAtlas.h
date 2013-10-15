
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlas_DEFINED
#define GrAtlas_DEFINED


#include "GrPoint.h"
#include "GrTexture.h"
#include "GrDrawTarget.h"

class GrGpu;
class GrRectanizer;
class GrAtlasMgr;
class GrAtlas;

// The backing GrTexture for a set of GrAtlases is broken into a spatial grid of GrPlots. When
// a GrAtlas needs space on the texture, it requests a GrPlot. Each GrAtlas can claim one
// or more GrPlots. The GrPlots keep track of subimage placement via their GrRectanizer. Once a
// GrPlot is "full" (i.e. there is no room for the new subimage according to the GrRectanizer), the
// GrAtlas can request a new GrPlot via GrAtlasMgr::addToAtlas().
//
// If all GrPlots are allocated, the replacement strategy is up to the client. The drawToken is
// available to ensure that all draw calls are finished for that particular GrPlot.
// GrAtlasMgr::removeUnusedPlots() will free up any finished plots for a given GrAtlas.

class GrPlot {
public:
    int getOffsetX() const { return fOffset.fX; }
    int getOffsetY() const { return fOffset.fY; }

    GrTexture* texture() const { return fTexture; }

    bool addSubImage(int width, int height, const void*, GrIPoint16*);

    GrDrawTarget::DrawToken drawToken() const { return fDrawToken; }
    void setDrawToken(GrDrawTarget::DrawToken draw) { fDrawToken = draw; }

private:
    GrPlot();
    ~GrPlot(); // does not try to delete the fNext field

    // for recycling
    GrDrawTarget::DrawToken fDrawToken;

    GrPlot*                 fNext;

    GrTexture*              fTexture;
    GrRectanizer*           fRects;
    GrAtlasMgr*             fAtlasMgr;
    GrIPoint16              fOffset;
    size_t                  fBytesPerPixel;

    friend class GrAtlasMgr;
};

class GrAtlasMgr {
public:
    GrAtlasMgr(GrGpu*, GrPixelConfig);
    ~GrAtlasMgr();

    // add subimage of width, height dimensions to atlas
    // returns the containing GrPlot and location relative to the backing texture
    GrPlot* addToAtlas(GrAtlas*, int width, int height, const void*, GrIPoint16*);

    // free up any plots that are not waiting on a draw call
    bool removeUnusedPlots(GrAtlas* atlas);

    // to be called by ~GrAtlas()
    void deletePlotList(GrPlot* plot);

    GrTexture* getTexture() const {
        return fTexture;
    }

private:
    GrPlot* allocPlot();
    void freePlot(GrPlot* plot);

    GrGpu*        fGpu;
    GrPixelConfig fPixelConfig;
    GrTexture*    fTexture;

    // allocated array of GrPlots
    GrPlot*       fPlots;
    // linked list of free GrPlots
    GrPlot*       fFreePlots;
};

class GrAtlas {
public:
    GrAtlas(GrAtlasMgr* mgr) : fPlots(NULL), fAtlasMgr(mgr) { }
    ~GrAtlas() { fAtlasMgr->deletePlotList(fPlots); }

    bool isEmpty() { return NULL == fPlots; }

private:
    GrPlot*     fPlots;
    GrAtlasMgr* fAtlasMgr;

    friend class GrAtlasMgr;
};

#endif
