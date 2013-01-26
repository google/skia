
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

class GrGpu;
class GrRectanizer;
class GrAtlasMgr;

class GrAtlas {
public:
    GrAtlas(GrAtlasMgr*, int plotX, int plotY, GrMaskFormat);

    int getPlotX() const { return fPlot.fX; }
    int getPlotY() const { return fPlot.fY; }
    GrMaskFormat getMaskFormat() const { return fMaskFormat; }

    GrTexture* texture() const { return fTexture; }

    bool addSubImage(int width, int height, const void*, GrIPoint16*);

    static void FreeLList(GrAtlas* atlas) {
        while (atlas) {
            GrAtlas* next = atlas->fNext;
            delete atlas;
            atlas = next;
        }
    }

    // testing
    GrAtlas* nextAtlas() const { return fNext; }

private:
    ~GrAtlas(); // does not try to delete the fNext field

    GrAtlas*        fNext;
    GrTexture*      fTexture;
    GrRectanizer*   fRects;
    GrAtlasMgr*     fAtlasMgr;
    GrIPoint16      fPlot;
    GrMaskFormat    fMaskFormat;

    friend class GrAtlasMgr;
};

class GrPlotMgr;

class GrAtlasMgr {
public:
    GrAtlasMgr(GrGpu*);
    ~GrAtlasMgr();

    GrAtlas* addToAtlas(GrAtlas*, int width, int height, const void*,
                        GrMaskFormat, GrIPoint16*);

    GrTexture* getTexture(GrMaskFormat format) const {
        GrAssert((unsigned)format < kCount_GrMaskFormats);
        return fTexture[format];
    }

    // to be called by ~GrAtlas()
    void freePlot(int x, int y);

private:
    GrGpu*      fGpu;
    GrTexture*  fTexture[kCount_GrMaskFormats];
    GrPlotMgr*  fPlotMgr;
};

#endif
