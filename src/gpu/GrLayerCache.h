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
#include "GrPictureUtils.h"
#include "GrRect.h"
#include "SkChecksum.h"
#include "SkTDynamicHash.h"
#include "SkMessageBus.h"

class SkPicture;

// The layer cache listens for these messages to purge picture-related resources.
struct GrPictureDeletedMessage {
    uint32_t pictureID;
};

// GrPictureInfo stores the atlas plots used by a single picture. A single 
// plot may be used to store layers from multiple pictures.
struct GrPictureInfo {
public:
    // for SkTDynamicHash - just use the pictureID as the hash key
    static const uint32_t& GetKey(const GrPictureInfo& pictInfo) { return pictInfo.fPictureID; }
    static uint32_t Hash(const uint32_t& key) { return SkChecksum::Mix(key); }

    // GrPictureInfo proper
    GrPictureInfo(uint32_t pictureID) : fPictureID(pictureID) { }

    const uint32_t fPictureID;

    GrAtlas::ClientPlotUsage  fPlotUsage;
};

// GrCachedLayer encapsulates the caching information for a single saveLayer.
//
// Atlased layers get a ref to the backing GrTexture while non-atlased layers
// get a ref to the GrTexture in which they reside. In both cases 'fRect' 
// contains the layer's extent in its texture.
// Atlased layers also get a pointer to the plot in which they reside.
struct GrCachedLayer {
public:
    // For SkTDynamicHash
    struct Key {
        Key(uint32_t pictureID, int layerID) : fPictureID(pictureID) , fLayerID(layerID) {}

        bool operator==(const Key& other) const {
            return fPictureID == other.fPictureID && fLayerID == other.fLayerID;
        }

        uint32_t getPictureID() const { return fPictureID; }
        int      getLayerID() const { return fLayerID; }

    private:
        // ID of the picture of which this layer is a part
        const uint32_t fPictureID;
        // fLayerID is the index of this layer in the picture (one of 0 .. #layers).
        const int      fLayerID;
    };

    static const Key& GetKey(const GrCachedLayer& layer) { return layer.fKey; }
    static uint32_t Hash(const Key& key) { 
        return SkChecksum::Mix((key.getPictureID() << 16) | key.getLayerID());
    }

    // GrCachedLayer proper
    GrCachedLayer(uint32_t pictureID, int layerID) 
        : fKey(pictureID, layerID)
        , fTexture(NULL)
        , fRect(GrIRect16::MakeEmpty())
        , fPlot(NULL) {
        SkASSERT(SK_InvalidGenID != pictureID && layerID >= 0);
    }

    uint32_t pictureID() const { return fKey.getPictureID(); }
    int layerID() const { return fKey.getLayerID(); }

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

    void setPlot(GrPlot* plot) {
        SkASSERT(NULL == fPlot);
        fPlot = plot;
    }
    GrPlot* plot() { return fPlot; }

    bool isAtlased() const { return NULL != fPlot; }

    SkDEBUGCODE(void validate(const GrTexture* backingTexture) const;)

private:
    const Key       fKey;

    // fTexture is a ref on the atlasing texture for atlased layers and a
    // ref on a GrTexture for non-atlased textures. In both cases, if this is
    // non-NULL, that means that the texture is locked in the texture cache.
    GrTexture*      fTexture;

    // For both atlased and non-atlased layers 'fRect' contains the  bound of
    // the layer in whichever texture it resides. It is empty when 'fTexture'
    // is NULL.
    GrIRect16       fRect;

    // For atlased layers, fPlot stores the atlas plot in which the layer rests.
    // It is always NULL for non-atlased layers.
    GrPlot*         fPlot;
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

    // Setup to be notified when 'picture' is deleted
    void trackPicture(const SkPicture* picture);

    // Cleanup after any SkPicture deletions
    void processDeletedPictures();

    SkDEBUGCODE(void validate() const;)

private:
    static const int kNumPlotsX = 2;
    static const int kNumPlotsY = 2;

    GrContext*                fContext;  // pointer back to owning context
    SkAutoTDelete<GrAtlas>    fAtlas;    // TODO: could lazily allocate

    // We cache this information here (rather then, say, on the owning picture)
    // because we want to be able to clean it up as needed (e.g., if a picture
    // is leaked and never cleans itself up we still want to be able to 
    // remove the GrPictureInfo once its layers are purged from all the atlas
    // plots).
    SkTDynamicHash<GrPictureInfo, uint32_t> fPictureHash;

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key> fLayerHash;

    SkMessageBus<GrPictureDeletedMessage>::Inbox fPictDeletionInbox;

    SkAutoTUnref<SkPicture::DeletionListener> fDeletionListener;

    void initAtlas();
    GrCachedLayer* createLayer(const SkPicture* picture, int layerID);

    // Remove all the layers (and unlock any resources) associated with 'pictureID'
    void purge(uint32_t pictureID);

    // for testing
    friend class TestingAccess;
    int numLayers() const { return fLayerHash.count(); }
};

#endif
