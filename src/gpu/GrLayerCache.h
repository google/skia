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

// GrCachedLayer encapsulates the caching information for a single saveLayer.
//
// Atlased layers get a ref to their atlas GrTexture and their GrAtlasLocation
// is filled in.
// In this case GrCachedLayer is roughly equivalent to a GrGlyph in the font
// caching system.
//
// Non-atlased layers get a ref to the GrTexture in which they reside.
// TODO: can we easily reuse the empty space in the non-atlased GrTexture's?
struct GrCachedLayer {
public:
    uint32_t pictureID() const { return fPictureID; }
    int layerID() const { return fLayerID; }

    void init(uint32_t pictureID, int layerID) {
        fPictureID = pictureID;
        fLayerID   = layerID;
        fTexture   = NULL;
        fLocation.set(NULL, GrIRect16::MakeEmpty());
    }

    // This call takes over the caller's ref
    void setTexture(GrTexture* texture) {
        if (NULL != fTexture) {
            fTexture->unref();
        }

        fTexture = texture; // just take over caller's ref
    }
    GrTexture* getTexture() { return fTexture; }

private:
    uint32_t        fPictureID;
    // fLayerID is only valid when fPicture != kInvalidGenID in which case it
    // is the index of this layer in the picture (one of 0 .. #layers).
    int             fLayerID;

    // fTexture is a ref on the atlasing texture for atlased layers and a
    // ref on a GrTexture for non-atlased textures. In both cases, if this is
    // non-NULL, that means that the texture is locked in the texture cache.
    GrTexture*      fTexture;

    GrAtlasLocation fLocation;       // only valid if the layer is atlased
};

// The GrLayerCache caches pre-computed saveLayers for later rendering.
// Non-atlased layers are stored in their own GrTexture while the atlased
// layers share a single GrTexture.
// Unlike the GrFontCache, the GrTexture atlas only has one GrAtlasMgr (for 8888)
// and one GrPlot (for the entire atlas). As such, the GrLayerCache
// roughly combines the functionality of the GrFontCache and GrTextStrike
// classes.
class GrLayerCache {
public:
    GrLayerCache(GrGpu*);
    ~GrLayerCache();

    void freeAll();

    GrCachedLayer* findLayerOrCreate(const SkPicture* picture, int id);

private:
    SkAutoTUnref<GrGpu>       fGpu;
    SkAutoTDelete<GrAtlasMgr> fAtlasMgr; // TODO: could lazily allocate

    class PictureLayerKey;
    GrTHashTable<GrCachedLayer, PictureLayerKey, 7> fLayerHash;
    GrTAllocPool<GrCachedLayer> fLayerPool;

    void init();
    GrCachedLayer* createLayer(const SkPicture* picture, int id);

};

#endif
