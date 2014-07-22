/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlas.h"
#include "GrGpu.h"
#include "GrLayerCache.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrPictureDeletedMessage);

#ifdef SK_DEBUG
void GrCachedLayer::validate(const GrTexture* backingTexture) const {
    SkASSERT(SK_InvalidGenID != fKey.getPictureID());
    SkASSERT(-1 != fKey.getLayerID());


    if (NULL != fTexture) {
        // If the layer is in some texture then it must occupy some rectangle
        SkASSERT(!fRect.isEmpty());
        if (!this->isAtlased()) {
            // If it isn't atlased then the rectangle should start at the origin
            SkASSERT(0.0f == fRect.fLeft && 0.0f == fRect.fTop);
        }
    } else {
        SkASSERT(fRect.isEmpty());
        SkASSERT(NULL == fPlot);
    }

    if (NULL != fPlot) {
        // If a layer has a plot (i.e., is atlased) then it must point to
        // the backing texture. Additionally, its rect should be non-empty.
        SkASSERT(NULL != fTexture && backingTexture == fTexture);
        SkASSERT(!fRect.isEmpty());
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
    void setBackingTexture(GrTexture* backingTexture) {
        SkASSERT(NULL == fBackingTexture || fBackingTexture == backingTexture);
        fBackingTexture = backingTexture;
    }

private:
    const GrTexture* fBackingTexture;
    const GrCachedLayer* fLayer;
};
#endif

GrLayerCache::GrLayerCache(GrContext* context)
    : fContext(context) {
    this->initAtlas();
}

GrLayerCache::~GrLayerCache() {

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        GrCachedLayer* layer = &(*iter);
        this->unlock(layer);
        SkDELETE(layer);
    }

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
                                      textureSize, kNumPlotsX, kNumPlotsY, false)));
}

void GrLayerCache::freeAll() {

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        GrCachedLayer* layer = &(*iter);
        this->unlock(layer);
        SkDELETE(layer);
    }
    fLayerHash.rewind();

    // The atlas only lets go of its texture when the atlas is deleted. 
    fAtlas.free();
    // GrLayerCache always assumes an atlas exists so recreate it. The atlas 
    // lazily allocates a replacement texture so reallocating a new 
    // atlas here won't disrupt a GrContext::contextDestroyed or freeGpuResources.
    // TODO: Make GrLayerCache lazily allocate the atlas manager?
    this->initAtlas();
}

GrCachedLayer* GrLayerCache::createLayer(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID && layerID >= 0);

    GrCachedLayer* layer = SkNEW_ARGS(GrCachedLayer, (picture->uniqueID(), layerID));
    fLayerHash.add(layer);
    return layer;
}

GrCachedLayer* GrLayerCache::findLayer(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID && layerID >= 0);
    return fLayerHash.find(GrCachedLayer::Key(picture->uniqueID(), layerID));
}

GrCachedLayer* GrLayerCache::findLayerOrCreate(const SkPicture* picture, int layerID) {
    SkASSERT(picture->uniqueID() != SK_InvalidGenID && layerID >= 0);
    GrCachedLayer* layer = fLayerHash.find(GrCachedLayer::Key(picture->uniqueID(), layerID));
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
    {
        GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        if (NULL == pictInfo) {
            pictInfo = SkNEW_ARGS(GrPictureInfo, (layer->pictureID()));
            fPictureHash.add(pictInfo);
        }

        SkIPoint16 loc;
        GrPlot* plot = fAtlas->addToAtlas(&pictInfo->fPlotUsage, 
                                          desc.fWidth, desc.fHeight, 
                                          NULL, &loc);
        // addToAtlas can allocate the backing texture
        SkDEBUGCODE(avl.setBackingTexture(fAtlas->getTexture()));
        if (NULL != plot) {
            GrIRect16 bounds = GrIRect16::MakeXYWH(loc.fX, loc.fY,
                                                   SkToS16(desc.fWidth), SkToS16(desc.fHeight));
            layer->setTexture(fAtlas->getTexture(), bounds);
            layer->setPlot(plot);
            return false;
        }
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
        SkASSERT(layer->texture() == fAtlas->getTexture());

        GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        SkASSERT(NULL != pictInfo);
        pictInfo->fPlotUsage.isEmpty(); // just to silence compiler warnings for the time being

        // TODO: purging from atlas goes here
    } else {
        fContext->unlockScratchTexture(layer->texture());
        layer->setTexture(NULL, GrIRect16::MakeEmpty());
    }
}

#ifdef SK_DEBUG
void GrLayerCache::validate() const {
    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::ConstIter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        (*iter).validate(fAtlas->getTexture());
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

void GrLayerCache::purge(uint32_t pictureID) {

    SkDEBUGCODE(GrAutoValidateCache avc(this);)

    // We need to find all the layers associated with 'picture' and remove them.
    SkTDArray<GrCachedLayer*> toBeRemoved;

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        if (pictureID == (*iter).pictureID()) {
            *toBeRemoved.append() = &(*iter);
        }
    }

    for (int i = 0; i < toBeRemoved.count(); ++i) {
        this->unlock(toBeRemoved[i]);
        fLayerHash.remove(GrCachedLayer::GetKey(*toBeRemoved[i]));
        SkDELETE(toBeRemoved[i]);
    }

    GrPictureInfo* pictInfo = fPictureHash.find(pictureID);
    if (NULL != pictInfo) {
        fPictureHash.remove(pictureID);
        SkDELETE(pictInfo);
    }
}

class GrPictureDeletionListener : public SkPicture::DeletionListener {
    virtual void onDeletion(uint32_t pictureID) SK_OVERRIDE{
        const GrPictureDeletedMessage message = { pictureID };
        SkMessageBus<GrPictureDeletedMessage>::Post(message);
    }
};

void GrLayerCache::trackPicture(const SkPicture* picture) {
    if (NULL == fDeletionListener) {
        fDeletionListener.reset(SkNEW(GrPictureDeletionListener));
    }

    picture->addDeletionListener(fDeletionListener);
}

void GrLayerCache::processDeletedPictures() {
    SkTDArray<GrPictureDeletedMessage> deletedPictures;
    fPictDeletionInbox.poll(&deletedPictures);

    for (int i = 0; i < deletedPictures.count(); i++) {
        this->purge(deletedPictures[i].pictureID);
    }
}

