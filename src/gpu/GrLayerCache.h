/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrLayerCache_DEFINED
#define GrLayerCache_DEFINED

#define USE_ATLAS 0

#include "GrAllocPool.h"
#include "GrAtlas.h"
#include "GrTHashTable.h"
#include "GrPictureUtils.h"
#include "GrRect.h"

class GrGpu;
class SkPicture;

// GrCachedLayer encapsulates the caching information for a single saveLayer.
//
// Atlased layers get a ref to their atlas GrTexture and 'fRect' contains
// their absolute location in the backing texture.
//
// Non-atlased layers get a ref to the GrTexture in which they reside. Their
// 'fRect' will be empty.
//
// TODO: can we easily reuse the empty space in the non-atlased GrTexture's?
struct GrCachedLayer {
public:
    GrCachedLayer(uint32_t pictureID, int layerID) {
        fPictureID = pictureID;
        fLayerID = layerID;
        fTexture = NULL;
        fRect = GrIRect16::MakeEmpty();
    }

    uint32_t pictureID() const { return fPictureID; }
    int layerID() const { return fLayerID; }

    // This call takes over the caller's ref
    void setTexture(GrTexture* texture, const GrIRect16& rect) {
        if (NULL != fTexture) {
            fTexture->unref();
        }

        fTexture = texture; // just take over caller's ref
        fRect = rect;
    }
    GrTexture* texture() { return fTexture; }
    const GrIRect16& rect() const { return fRect; }

private:
    uint32_t        fPictureID;
    // fLayerID is only valid when fPicture != kInvalidGenID in which case it
    // is the index of this layer in the picture (one of 0 .. #layers).
    int             fLayerID;

    // fTexture is a ref on the atlasing texture for atlased layers and a
    // ref on a GrTexture for non-atlased textures. In both cases, if this is
    // non-NULL, that means that the texture is locked in the texture cache.
    GrTexture*      fTexture;

    // For non-atlased layers 'fRect' is empty otherwise it is the bound of
    // the layer in the atlas.
    GrIRect16       fRect;
};

// The GrLayerCache caches pre-computed saveLayers for later rendering.
// Non-atlased layers are stored in their own GrTexture while the atlased
// layers share a single GrTexture.
// Unlike the GrFontCache, the GrTexture atlas only has one GrAtlas (for 8888)
// and one GrPlot (for the entire atlas). As such, the GrLayerCache
// roughly combines the functionality of the GrFontCache and GrTextStrike
// classes.
class GrLayerCache {
public:
    GrLayerCache(GrContext*);
    ~GrLayerCache();

    // As a cache, the GrLayerCache can be ordered to free up all its cached
    // elements by the GrContext
    void freeAll();

    GrCachedLayer* findLayer(const SkPicture* picture, int layerID);
    GrCachedLayer* findLayerOrCreate(const SkPicture* picture, int layerID);
    
    // Inform the cache that layer's cached image is now required. Return true
    // if it was found in the ResourceCache and doesn't need to be regenerated.
    // If false is returned the caller should (re)render the layer into the
    // newly acquired texture.
    bool lock(GrCachedLayer* layer, const GrTextureDesc& desc);

    // Inform the cache that layer's cached image is not currently required
    void unlock(GrCachedLayer* layer);

    // Remove all the layers (and unlock any resources) associated with 'picture'
    void purge(const SkPicture* picture);

private:
    GrContext*                fContext;  // pointer back to owning context
    SkAutoTDelete<GrAtlas>    fAtlas;    // TODO: could lazily allocate
    GrAtlas::ClientPlotUsage  fPlotUsage;

    class PictureLayerKey;
    GrTHashTable<GrCachedLayer, PictureLayerKey, 7> fLayerHash;

    void initAtlas();
    GrCachedLayer* createLayer(const SkPicture* picture, int layerID);

    // for testing
    friend class GetNumLayers;
    int numLayers() const { return fLayerHash.count(); }
};

#endif
