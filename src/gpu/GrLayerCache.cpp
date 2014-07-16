/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlas.h"
#include "GrGpu.h"
#include "GrLayerCache.h"

/**
 *  PictureLayerKey just wraps a saveLayer's id in a picture for GrTHashTable.
 */
class GrLayerCache::PictureLayerKey {
public:
    PictureLayerKey(uint32_t pictureID, int layerID)
        : fPictureID(pictureID)
        , fLayerID(layerID) {
    }

    uint32_t pictureID() const { return fPictureID; }
    int layerID() const { return fLayerID; }

    uint32_t getHash() const { return (fPictureID << 16) | fLayerID; }

    static bool LessThan(const GrCachedLayer& layer, const PictureLayerKey& key) {
        if (layer.pictureID() == key.pictureID()) {
            return layer.layerID() < key.layerID();
        }

        return layer.pictureID() < key.pictureID();
    }

    static bool Equals(const GrCachedLayer& layer, const PictureLayerKey& key) {
        return layer.pictureID() == key.pictureID() && layer.layerID() == key.layerID();
    }

private:
    uint32_t fPictureID;
    int      fLayerID;
};

#ifdef SK_DEBUG
void GrCachedLayer::validate(GrTexture* backingTexture) const {
    SkASSERT(SK_InvalidGenID != fPictureID);
    SkASSERT(-1 != fLayerID);

    if (NULL != fTexture) {
        // If the layer is in some texture then it must occupy some rectangle
        SkASSERT(!fRect.isEmpty());
        if (!this->isAtlased()) {
            // If it isn't atlased then the rectangle should start at the origin
            SkASSERT(0.0f == fRect.fLeft && 0.0f == fRect.fTop);
        }
    } else {
        SkASSERT(fRect.isEmpty());
    }
}

class GrAutoValidateLayer : ::SkNoncopyable {
public:
    GrAutoValidateLayer(GrTexture* backingTexture, const GrCachedLayer* layer) 
        : fBackingTexture(backingTexture)
        , fLayer(layer) {
        if (NULL != fLayer) {
            fLayer->validate(backingTexture);
        }
    }
    ~GrAutoValidateLayer() {
        if (NULL != fLayer) {
            fLayer->validate(fBackingTexture);
        }
    }

private:
    GrTexture* fBackingTexture;
    const GrCachedLayer* fLayer;
};
#endif

GrLayerCache::GrLayerCache(GrContext* context)
    : fContext(context) {
    this->initAtlas();
}

GrLayerCache::~GrLayerCache() {
    SkTDArray<GrCachedLayer*>& layerArray = fLayerHash.getArray();
    for (int i = 0; i < fLayerHash.count(); ++i) {
        this->unlock(layerArray[i]);
    }

    fLayerHash.deleteAll();

    // The atlas only lets go of its texture when the atlas is deleted. 
    fAtlas.free();    
}

void GrLayerCache::initAtlas() {
    static const int kAtlasTextureWidth = 1024;
    static const int kAtlasTextureHeight = 1024;

    SkASSERT(NULL == fAtlas.get());

    // The layer cache only gets 1 plot
    SkISize textureSize = SkISize::Make(kAtlasTextureWidth, kAtlasTextureHeight);
    fAtlas.reset(SkNEW_ARGS(GrAtlas, (fContext->getGpu(), kSkia8888_GrPixelConfig,
                                      kRenderTarget_GrTextureFlagBit,
                                      textureSize, 1, 1, false)));
}

void GrLayerCache::freeAll() {
    SkTDArray<GrCachedLayer*>& layerArray = fLayerHash.getArray();
    for (int i = 0; i < fLayerHash.count(); ++i) {
        this->unlock(layerArray[i]);
    }

    fLayerHash.deleteAll();

    // The atlas only lets go of its texture when the atlas is deleted. 
    fAtlas.free();
    // GrLayerCache always assumes an atlas exists so recreate it. The atlas 
    // lazily allocates a replacement texture so reallocating a new 
    // atlas here won't disrupt a GrContext::contextDestroyed or freeGpuResources.
    // TODO: Make GrLayerCache lazily allocate the atlas manager?
    this->initAtlas();
}

GrCachedLayer* GrLayerCache::createLayer(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID);

    GrCachedLayer* layer = SkNEW_ARGS(GrCachedLayer, (picture->uniqueID(), layerID));
    fLayerHash.insert(PictureLayerKey(picture->uniqueID(), layerID), layer);
    return layer;
}

GrCachedLayer* GrLayerCache::findLayer(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID);
    return fLayerHash.find(PictureLayerKey(picture->uniqueID(), layerID));
}

GrCachedLayer* GrLayerCache::findLayerOrCreate(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID);
    GrCachedLayer* layer = fLayerHash.find(PictureLayerKey(picture->uniqueID(), layerID));
    if (NULL == layer) {
        layer = this->createLayer(picture, layerID);
    }

    return layer;
}

bool GrLayerCache::lock(GrCachedLayer* layer, const GrTextureDesc& desc) {
    SkDEBUGCODE(GrAutoValidateLayer avl(fAtlas->getTexture(), layer);)

    if (NULL != layer->texture()) {
        // This layer is already locked
#ifdef SK_DEBUG
        if (layer->isAtlased()) {
            // It claims to be atlased
            SkASSERT(layer->rect().width() == desc.fWidth);
            SkASSERT(layer->rect().height() == desc.fHeight);
        }
#endif
        return true;
    }

#if USE_ATLAS
    SkIPoint16 loc;
    GrPlot* plot = fAtlas->addToAtlas(&fPlotUsage, desc.fWidth, desc.fHeight, NULL, &loc);
    if (NULL != plot) {
        GrIRect16 bounds = GrIRect16::MakeXYWH(loc.fX, loc.fY, 
                                               SkToS16(desc.fWidth), SkToS16(desc.fHeight));
        layer->setTexture(fAtlas->getTexture(), bounds);
        layer->setAtlased(true);
        return false;
    }
#endif

    // The texture wouldn't fit in the cache - give it it's own texture.
    // This path always uses a new scratch texture and (thus) doesn't cache anything.
    // This can yield a lot of re-rendering
    layer->setTexture(fContext->lockAndRefScratchTexture(desc, GrContext::kApprox_ScratchTexMatch),
                      GrIRect16::MakeWH(SkToS16(desc.fWidth), SkToS16(desc.fHeight)));
    return false;
}

void GrLayerCache::unlock(GrCachedLayer* layer) {
    SkDEBUGCODE(GrAutoValidateLayer avl(fAtlas->getTexture(), layer);)

    if (NULL == layer || NULL == layer->texture()) {
        return;
    }

    if (layer->isAtlased()) {
        // The atlas doesn't currently use a scratch texture (and we would have
        // to free up space differently anyways)
        // TODO: unlock atlas space when a recycling rectanizer is available
    } else {
        fContext->unlockScratchTexture(layer->texture());
        layer->setTexture(NULL, GrIRect16::MakeEmpty());
    }
}

#ifdef SK_DEBUG
void GrLayerCache::validate() const {
    const SkTDArray<GrCachedLayer*>& layerArray = fLayerHash.getArray();
    for (int i = 0; i < fLayerHash.count(); ++i) {
        layerArray[i]->validate(fAtlas->getTexture());
    }
}

class GrAutoValidateCache : ::SkNoncopyable {
public:
    explicit GrAutoValidateCache(GrLayerCache* cache)
        : fCache(cache) {
        fCache->validate();
    }
    ~GrAutoValidateCache() {
        fCache->validate();
    }
private:
    GrLayerCache* fCache;
};
#endif

void GrLayerCache::purge(const SkPicture* picture) {
    SkDEBUGCODE(GrAutoValidateCache avc(this);)

    // This is somewhat of an abuse of GrTHashTable. We need to find all the
    // layers associated with 'picture' but the usual hash calls only look for
    // exact key matches. This code peeks into the hash table's innards to
    // find all the 'picture'-related layers.
    // TODO: use a different data structure for the layer hash?
    SkTDArray<GrCachedLayer*> toBeRemoved;

    const SkTDArray<GrCachedLayer*>& layerArray = fLayerHash.getArray();
    for (int i = 0; i < fLayerHash.count(); ++i) {
        if (picture->uniqueID() == layerArray[i]->pictureID()) {
            *toBeRemoved.append() = layerArray[i];
        }
    }

    for (int i = 0; i < toBeRemoved.count(); ++i) {
        this->unlock(toBeRemoved[i]);

        PictureLayerKey key(picture->uniqueID(), toBeRemoved[i]->layerID());
        fLayerHash.remove(key, toBeRemoved[i]);
        SkDELETE(toBeRemoved[i]);
    }
}
