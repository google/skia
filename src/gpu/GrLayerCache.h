/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrLayerCache_DEFINED
#define GrLayerCache_DEFINED

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
// For non-atlased layers, the lock field just corresponds to locking in
// the resource cache. For atlased layers, it implements an additional level
// of locking to allow atlased layers to be reused multiple times.
struct GrCachedLayer {
public:
    // For SkTDynamicHash
    struct Key {
        Key(uint32_t pictureID, int start, int stop, const SkIPoint& offset, const SkMatrix& ctm) 
        : fPictureID(pictureID)
        , fStart(start)
        , fStop(stop)
        , fOffset(offset)
        , fCTM(ctm) {
            fCTM.getType(); // force initialization of type so hashes match

            // Key needs to be tightly packed.
            GR_STATIC_ASSERT(sizeof(Key) == sizeof(uint32_t) + 2 * sizeof(int) + 
                                            2 * sizeof(int32_t) +
                                            9 * sizeof(SkScalar) + sizeof(uint32_t));
        }

        bool operator==(const Key& other) const {
            return fPictureID == other.fPictureID &&
                   fStart == other.fStart &&
                   fStop == other.fStop &&
                   fOffset == other.fOffset &&
                   fCTM.cheapEqualTo(other.fCTM);
        }

        uint32_t pictureID() const { return fPictureID; }
        int start() const { return fStart; }
        int stop() const { return fStop; }
        const SkIPoint& offset() const { return fOffset; }
        const SkMatrix& ctm() const { return fCTM; }

    private:
        // ID of the picture of which this layer is a part
        const uint32_t fPictureID;
        // The range of commands in the picture this layer represents
        const int      fStart;
        const int      fStop;
        // The offset of the layer in device space
        const SkIPoint fOffset;
        // The CTM applied to this layer in the picture
        SkMatrix       fCTM;
    };

    static const Key& GetKey(const GrCachedLayer& layer) { return layer.fKey; }
    static uint32_t Hash(const Key& key) { 
        return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(&key), sizeof(Key));
    }

    // GrCachedLayer proper
    GrCachedLayer(uint32_t pictureID, int start, int stop,
                  const SkIPoint& offset, const SkMatrix& ctm,
                  const SkPaint* paint)
        : fKey(pictureID, start, stop, offset, ctm)
        , fPaint(paint ? SkNEW_ARGS(SkPaint, (*paint)) : NULL)
        , fTexture(NULL)
        , fRect(GrIRect16::MakeEmpty())
        , fPlot(NULL)
        , fLocked(false) {
        SkASSERT(SK_InvalidGenID != pictureID && start >= 0 && stop >= 0);
    }

    ~GrCachedLayer() {
        SkSafeUnref(fTexture);
        SkDELETE(fPaint);
    }

    uint32_t pictureID() const { return fKey.pictureID(); }
    int start() const { return fKey.start(); }
    int stop() const { return fKey.stop(); }
    const SkIPoint& offset() const { return fKey.offset(); }
    const SkMatrix& ctm() const { return fKey.ctm(); }

    void setTexture(GrTexture* texture, const GrIRect16& rect) {
        SkRefCnt_SafeAssign(fTexture, texture);
        fRect = rect;
    }
    GrTexture* texture() { return fTexture; }
    const SkPaint* paint() const { return fPaint; }
    const GrIRect16& rect() const { return fRect; }

    void setPlot(GrPlot* plot) {
        SkASSERT(NULL == plot || NULL == fPlot);
        fPlot = plot;
    }
    GrPlot* plot() { return fPlot; }

    bool isAtlased() const { return SkToBool(fPlot); }

    void setLocked(bool locked) { fLocked = locked; }
    bool locked() const { return fLocked; }

    SkDEBUGCODE(const GrPlot* plot() const { return fPlot; })
    SkDEBUGCODE(void validate(const GrTexture* backingTexture) const;)

private:
    const Key       fKey;

    // The paint used when dropping the layer down into the owning canvas.
    // Can be NULL. This class makes a copy for itself.
    const SkPaint*  fPaint;

    // fTexture is a ref on the atlasing texture for atlased layers and a
    // ref on a GrTexture for non-atlased textures.
    GrTexture*      fTexture;

    // For both atlased and non-atlased layers 'fRect' contains the  bound of
    // the layer in whichever texture it resides. It is empty when 'fTexture'
    // is NULL.
    GrIRect16       fRect;

    // For atlased layers, fPlot stores the atlas plot in which the layer rests.
    // It is always NULL for non-atlased layers.
    GrPlot*         fPlot;

    // For non-atlased layers 'fLocked' should always match "fTexture".
    // (i.e., if there is a texture it is locked).
    // For atlased layers, 'fLocked' is true if the layer is in a plot and
    // actively required for rendering. If the layer is in a plot but not
    // actively required for rendering, then 'fLocked' is false. If the
    // layer isn't in a plot then is can never be locked.
    bool            fLocked;
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

    GrCachedLayer* findLayer(uint32_t pictureID, int start, int stop, 
                             const SkIPoint& offset, const SkMatrix& ctm);
    GrCachedLayer* findLayerOrCreate(uint32_t pictureID,
                                     int start, int stop, 
                                     const SkIPoint& offset,
                                     const SkMatrix& ctm,
                                     const SkPaint* paint);

    // Inform the cache that layer's cached image is now required. 
    // Return true if the layer must be re-rendered. Return false if the
    // layer was found in the cache and can be reused.
    bool lock(GrCachedLayer* layer, const GrTextureDesc& desc, bool dontAtlas);

    // Inform the cache that layer's cached image is not currently required
    void unlock(GrCachedLayer* layer);

    // Setup to be notified when 'picture' is deleted
    void trackPicture(const SkPicture* picture);

    // Cleanup after any SkPicture deletions
    void processDeletedPictures();

    SkDEBUGCODE(void validate() const;)

private:
    static const int kAtlasTextureWidth = 1024;
    static const int kAtlasTextureHeight = 1024;

    static const int kNumPlotsX = 2;
    static const int kNumPlotsY = 2;

    static const int kPlotWidth = kAtlasTextureWidth / kNumPlotsX;
    static const int kPlotHeight = kAtlasTextureHeight / kNumPlotsY;

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

    // This implements a plot-centric locking mechanism (since the atlas
    // backing texture is always locked). Each layer that is locked (i.e.,
    // needed for the current rendering) in a plot increments the plot lock
    // count for that plot. Similarly, once a rendering is complete all the
    // layers used in it decrement the lock count for the used plots.
    // Plots with a 0 lock count are open for recycling/purging.
    int fPlotLocks[kNumPlotsX * kNumPlotsY];

    void initAtlas();
    GrCachedLayer* createLayer(uint32_t pictureID, int start, int stop, 
                               const SkIPoint& offset, const SkMatrix& ctm,
                               const SkPaint* paint);

    void purgeAll();

    // Remove all the layers (and unlock any resources) associated with 'pictureID'
    void purge(uint32_t pictureID);

    static bool PlausiblyAtlasable(int width, int height) {
        return width <= kPlotWidth && height <= kPlotHeight;
    }

    void purgePlot(GrPlot* plot);

    // Try to find a purgeable plot and clear it out. Return true if a plot
    // was purged; false otherwise.
    bool purgePlot();

    // for testing
    friend class TestingAccess;
    int numLayers() const { return fLayerHash.count(); }
};

#endif
