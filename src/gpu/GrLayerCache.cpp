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
    SkASSERT(SK_InvalidGenID != fKey.pictureID());
    SkASSERT(fKey.start() > 0 && fKey.stop() > 0);


    if (fTexture) {
        // If the layer is in some texture then it must occupy some rectangle
        SkASSERT(!fRect.isEmpty());
        if (!this->isAtlased()) {
            // If it isn't atlased then the rectangle should start at the origin
            SkASSERT(0.0f == fRect.fLeft && 0.0f == fRect.fTop);
        }
    } else {
        SkASSERT(fRect.isEmpty());
        SkASSERT(NULL == fPlot);
        SkASSERT(!fLocked);     // layers without a texture cannot be locked
    }

    if (fPlot) {
        // If a layer has a plot (i.e., is atlased) then it must point to
        // the backing texture. Additionally, its rect should be non-empty.
        SkASSERT(fTexture && backingTexture == fTexture);
        SkASSERT(!fRect.isEmpty());
    }

    if (fLocked) {
        // If a layer is locked it must have a texture (though it need not be
        // the atlas-backing texture) and occupy some space.
        SkASSERT(fTexture);
        SkASSERT(!fRect.isEmpty());
    }
}

class GrAutoValidateLayer : ::SkNoncopyable {
public:
    GrAutoValidateLayer(GrTexture* backingTexture, const GrCachedLayer* layer) 
        : fBackingTexture(backingTexture)
        , fLayer(layer) {
        if (fLayer) {
            fLayer->validate(backingTexture);
        }
    }
    ~GrAutoValidateLayer() {
        if (fLayer) {
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
    memset(fPlotLocks, 0, sizeof(fPlotLocks));
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
    SkASSERT(NULL == fAtlas.get());

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
    // atlas here won't disrupt a GrContext::abandonContext or freeGpuResources.
    // TODO: Make GrLayerCache lazily allocate the atlas manager?
    this->initAtlas();
}

GrCachedLayer* GrLayerCache::createLayer(uint32_t pictureID, 
                                         int start, int stop, 
                                         const SkIPoint& offset,
                                         const SkMatrix& ctm,
                                         const SkPaint* paint) {
    SkASSERT(pictureID != SK_InvalidGenID && start > 0 && stop > 0);

    GrCachedLayer* layer = SkNEW_ARGS(GrCachedLayer, (pictureID, start, stop, offset, ctm, paint));
    fLayerHash.add(layer);
    return layer;
}

GrCachedLayer* GrLayerCache::findLayer(uint32_t pictureID,
                                       int start, int stop, 
                                       const SkIPoint& offset,
                                       const SkMatrix& ctm) {
    SkASSERT(pictureID != SK_InvalidGenID && start > 0 && stop > 0);
    return fLayerHash.find(GrCachedLayer::Key(pictureID, start, stop, offset, ctm));
}

GrCachedLayer* GrLayerCache::findLayerOrCreate(uint32_t pictureID,
                                               int start, int stop,
                                               const SkIPoint& offset,
                                               const SkMatrix& ctm,
                                               const SkPaint* paint) {
    SkASSERT(pictureID != SK_InvalidGenID && start > 0 && stop > 0);
    GrCachedLayer* layer = fLayerHash.find(GrCachedLayer::Key(pictureID, start, stop, offset, ctm));
    if (NULL == layer) {
        layer = this->createLayer(pictureID, start, stop, offset, ctm, paint);
    }

    return layer;
}

bool GrLayerCache::lock(GrCachedLayer* layer, const GrTextureDesc& desc, bool dontAtlas) {
    SkDEBUGCODE(GrAutoValidateLayer avl(fAtlas->getTexture(), layer);)

    if (layer->locked()) {
        // This layer is already locked
#ifdef SK_DEBUG
        if (layer->isAtlased()) {
            // It claims to be atlased
            SkASSERT(!dontAtlas);
            SkASSERT(layer->rect().width() == desc.fWidth);
            SkASSERT(layer->rect().height() == desc.fHeight);
        }
#endif
        return false;
    }

    if (layer->isAtlased()) {
        // Hooray it is still in the atlas - make sure it stays there
        SkASSERT(!dontAtlas);
        layer->setLocked(true);
        fPlotLocks[layer->plot()->id()]++;
        return false;
    } else if (!dontAtlas && PlausiblyAtlasable(desc.fWidth, desc.fHeight)) {
        // Not in the atlas - will it fit?
        GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        if (NULL == pictInfo) {
            pictInfo = SkNEW_ARGS(GrPictureInfo, (layer->pictureID()));
            fPictureHash.add(pictInfo);
        }

        SkIPoint16 loc;
        for (int i = 0; i < 2; ++i) { // extra pass in case we fail to add but are able to purge
            GrPlot* plot = fAtlas->addToAtlas(&pictInfo->fPlotUsage,
                                              desc.fWidth, desc.fHeight,
                                              NULL, &loc);
            // addToAtlas can allocate the backing texture
            SkDEBUGCODE(avl.setBackingTexture(fAtlas->getTexture()));
            if (plot) {
                // The layer was successfully added to the atlas
                GrIRect16 bounds = GrIRect16::MakeXYWH(loc.fX, loc.fY,
                                                       SkToS16(desc.fWidth),
                                                       SkToS16(desc.fHeight));
                layer->setTexture(fAtlas->getTexture(), bounds);
                layer->setPlot(plot);
                layer->setLocked(true);
                fPlotLocks[layer->plot()->id()]++;
                return true;
            }

            // The layer was rejected by the atlas (even though we know it is 
            // plausibly atlas-able). See if a plot can be purged and try again.
            if (!this->purgePlot()) {
                break;  // We weren't able to purge any plots
            }
        }
    }

    // The texture wouldn't fit in the cache - give it it's own texture.
    // This path always uses a new scratch texture and (thus) doesn't cache anything.
    // This can yield a lot of re-rendering
    SkAutoTUnref<GrTexture> tex(fContext->lockAndRefScratchTexture(desc,
                                                        GrContext::kApprox_ScratchTexMatch));

    layer->setTexture(tex, GrIRect16::MakeWH(SkToS16(desc.fWidth), SkToS16(desc.fHeight)));
    layer->setLocked(true);
    return true;
}

void GrLayerCache::unlock(GrCachedLayer* layer) {
    SkDEBUGCODE(GrAutoValidateLayer avl(fAtlas->getTexture(), layer);)

    if (NULL == layer || !layer->locked()) {
        // invalid or not locked
        return;
    }

    if (layer->isAtlased()) {
        const int plotID = layer->plot()->id();

        SkASSERT(fPlotLocks[plotID] > 0);
        fPlotLocks[plotID]--;
        // At this point we could aggressively clear out un-locked plots but
        // by delaying we may be able to reuse some of the atlased layers later.
#if DISABLE_CACHING
        // This testing code aggressively removes the atlased layers. This
        // can be used to separate the performance contribution of less
        // render target pingponging from that due to the re-use of cached layers
        GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        SkASSERT(pictInfo);
        
        GrAtlas::RemovePlot(&pictInfo->fPlotUsage, layer->plot());
        
        layer->setPlot(NULL);
        layer->setTexture(NULL, GrIRect16::MakeEmpty());
#endif

    } else {
        fContext->unlockScratchTexture(layer->texture());
        layer->setTexture(NULL, GrIRect16::MakeEmpty());
    }

    layer->setLocked(false);
}

#ifdef SK_DEBUG
void GrLayerCache::validate() const {
    int plotLocks[kNumPlotsX * kNumPlotsY];
    memset(plotLocks, 0, sizeof(plotLocks));

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::ConstIter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        const GrCachedLayer* layer = &(*iter);

        layer->validate(fAtlas->getTexture());

        const GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        if (pictInfo) {
            // In aggressive cleanup mode a picture info should only exist if
            // it has some atlased layers
#if !DISABLE_CACHING
            SkASSERT(!pictInfo->fPlotUsage.isEmpty());
#endif
        } else {
            // If there is no picture info for this layer then all of its
            // layers should be non-atlased.
            SkASSERT(!layer->isAtlased());
        }

        if (layer->plot()) {
            SkASSERT(pictInfo);
            SkASSERT(pictInfo->fPictureID == layer->pictureID());

            SkASSERT(pictInfo->fPlotUsage.contains(layer->plot()));

            if (layer->locked()) {
                plotLocks[layer->plot()->id()]++;
            }
        } 
    }

    for (int i = 0; i < kNumPlotsX*kNumPlotsY; ++i) {
        SkASSERT(plotLocks[i] == fPlotLocks[i]);
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
    if (pictInfo) {
        fPictureHash.remove(pictureID);
        SkDELETE(pictInfo);
    }
}

bool GrLayerCache::purgePlot() {
    SkDEBUGCODE(GrAutoValidateCache avc(this);)

    GrAtlas::PlotIter iter;
    GrPlot* plot;
    for (plot = fAtlas->iterInit(&iter, GrAtlas::kLRUFirst_IterOrder);
         plot;
         plot = iter.prev()) {
        if (fPlotLocks[plot->id()] > 0) {
            continue;
        }

        this->purgePlot(plot);
        return true;
    }

    return false;
}

void GrLayerCache::purgePlot(GrPlot* plot) {
    SkASSERT(0 == fPlotLocks[plot->id()]);

    // We need to find all the layers in 'plot' and remove them.
    SkTDArray<GrCachedLayer*> toBeRemoved;

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        if (plot == (*iter).plot()) {
            *toBeRemoved.append() = &(*iter);
        }
    }

    for (int i = 0; i < toBeRemoved.count(); ++i) {
        SkASSERT(!toBeRemoved[i]->locked());

        GrPictureInfo* pictInfo = fPictureHash.find(toBeRemoved[i]->pictureID());
        SkASSERT(pictInfo);

        GrAtlas::RemovePlot(&pictInfo->fPlotUsage, plot);

        // Aggressively remove layers and, if now totally uncached, picture info
        fLayerHash.remove(GrCachedLayer::GetKey(*toBeRemoved[i]));
        SkDELETE(toBeRemoved[i]);

        if (pictInfo->fPlotUsage.isEmpty()) {
            fPictureHash.remove(pictInfo->fPictureID);
            SkDELETE(pictInfo);
        }
    }

    plot->resetRects();
}

void GrLayerCache::purgeAll() {
    GrAtlas::PlotIter iter;
    GrPlot* plot;
    for (plot = fAtlas->iterInit(&iter, GrAtlas::kLRUFirst_IterOrder);
         plot;
         plot = iter.prev()) {
        SkASSERT(0 == fPlotLocks[plot->id()]);

        this->purgePlot(plot);
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

