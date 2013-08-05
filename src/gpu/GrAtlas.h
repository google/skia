
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
    int getPlotX() const { return fPlot.fX; }
    int getPlotY() const { return fPlot.fY; }
    GrMaskFormat getMaskFormat() const { return fMaskFormat; }

    GrTexture* texture() const { return fTexture; }

    bool addSubImage(int width, int height, const void*, GrIPoint16*);

    static void FreeLList(GrAtlas* atlas) {
        while (NULL != atlas) {
            GrAtlas* next = atlas->fNext;
            delete atlas;
            atlas = next;
        }
    }

    static void MarkAllUnused(GrAtlas* atlas) {
        while (NULL != atlas) {
            atlas->fUsed = false;
            atlas = atlas->fNext;
        }
    }

    static bool RemoveUnusedAtlases(GrAtlasMgr* atlasMgr, GrAtlas** startAtlas);

    bool used() const { return fUsed; }
    void setUsed(bool used) { fUsed = used; }

private:
    GrAtlas(GrAtlasMgr*, int plotX, int plotY, GrMaskFormat format);
    ~GrAtlas(); // does not try to delete the fNext field

    GrAtlas*        fNext;

    // for recycling
    bool            fUsed;

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

    GrAtlas* addToAtlas(GrAtlas**, int width, int height, const void*,
                        GrMaskFormat, GrIPoint16*);
    void deleteAtlas(GrAtlas* atlas) { delete atlas; }

    GrTexture* getTexture(GrMaskFormat format) const {
        GrAssert((unsigned)format < kCount_GrMaskFormats);
        return fTexture[format];
    }

    // to be called by ~GrAtlas()
    void freePlot(GrMaskFormat format, int x, int y);

private:
    GrGpu*      fGpu;
    GrTexture*  fTexture[kCount_GrMaskFormats];
    GrPlotMgr*  fPlotMgr;
};

#endif
