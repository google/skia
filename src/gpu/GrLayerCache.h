/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrLayerCache_DEFINED
#define GrLayerCache_DEFINED

#include "GrAllocPool.h"
#include "GrTHashTable.h"
#include "GrPictureUtils.h"
#include "GrRect.h"

class GrAtlasMgr;
class GrGpu;
class GrPlot;
class SkPicture;

// GrAtlasLocation captures an atlased item's position in the atlas. This
// means the plot in which it resides and its bounds inside the plot.
// TODO: Make GrGlyph use one of these?
class GrAtlasLocation {
public:
    GrAtlasLocation() : fPlot(NULL) {}

    void set(GrPlot* plot, const GrIRect16& bounds) {
        fPlot = plot;
        fBounds = bounds;
    }

    const GrPlot* plot() const {
        return fPlot;
    }

    const GrIRect16& bounds() const {
        return fBounds;
    }

private:
    GrPlot*   fPlot;
    GrIRect16 fBounds;  // only valid is fPlot != NULL
};

// A GrAtlasedLayer encapsulates the atlasing information for a single saveLayer.
// It is roughly equivalent to a GrGlyph in the font caching system
class GrAtlasedLayer {
public:
    GrAtlasedLayer() : fPictureID(SkPicture::kInvalidGenID) { }

    uint32_t pictureID() const { return fPictureID; }
    int layerID() const { return fLayerID; }

    void init(uint32_t pictureID, int layerID) {
        fPictureID = pictureID;
        fLayerID   = layerID;
    }

private:
    uint32_t        fPictureID;
    int             fLayerID;        // only valid if fPicture != kInvalidGenID
    GrAtlasLocation fLocation;
};

// The GrLayerCache caches pre-computed saveLayers for later rendering.
// Unlike the GrFontCache, this cache only has one GrAtlasMgr (for 8888)
// and one GrPlot (for the entire atlas). As such, the GrLayerCache
// roughly combines the functionality of the GrFontCache and GrTextStrike
// classes.
class GrLayerCache {
public:
    GrLayerCache(GrGpu*);
    ~GrLayerCache();

    void freeAll();

    const GrAtlasedLayer* findLayerOrCreate(SkPicture* picture, int id);

private:
    SkAutoTUnref<GrGpu>       fGpu;
    SkAutoTDelete<GrAtlasMgr> fAtlasMgr; // TODO: could lazily allocate

    class PictureLayerKey;
    GrTHashTable<GrAtlasedLayer, PictureLayerKey, 7> fLayerHash;
    GrTAllocPool<GrAtlasedLayer> fLayerPool;

    void init();
    GrAtlasedLayer* createLayer(SkPicture* picture, int id);

};

#endif
