/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLayerAtlas.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrGpu.h"
#include "GrLayerCache.h"
#include "GrSurfacePriv.h"

#ifdef SK_DEBUG
void GrCachedLayer::validate(const GrTexture* backingTexture) const {
    SkASSERT(SK_InvalidGenID != fKey.pictureID());

    if (fTexture) {
        // If the layer is in some texture then it must occupy some rectangle
        SkASSERT(!fRect.isEmpty());
        if (!this->isAtlased()) {
            // If it isn't atlased then the rectangle should start at the origin
            SkASSERT(0.0f == fRect.fLeft && 0.0f == fRect.fTop);
        }
    } else {
        SkASSERT(fRect.isEmpty());
        SkASSERT(nullptr == fPlot);
        SkASSERT(!fLocked);     // layers without a texture cannot be locked
        SkASSERT(!fAtlased);    // can't be atlased if it doesn't have a texture
    }

    if (fPlot) {
        SkASSERT(fAtlased);
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

    // Unfortunately there is a brief time where a layer can be locked
    // but not used, so we can only check the "used implies locked"
    // invariant.
    if (fUses > 0) {
        SkASSERT(fLocked);
    } else {
        SkASSERT(0 == fUses);
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
        SkASSERT(nullptr == fBackingTexture || fBackingTexture == backingTexture);
        fBackingTexture = backingTexture;
    }

private:
    const GrTexture* fBackingTexture;
    const GrCachedLayer* fLayer;
};
#endif

GrLayerCache::GrLayerCache(GrContext* context)
    : fContext(context) {
    memset(fPlotLocks, 0, sizeof(fPlotLocks));
}

GrLayerCache::~GrLayerCache() {

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        GrCachedLayer* layer = &(*iter);
        SkASSERT(0 == layer->uses());
        this->unlock(layer);
        delete layer;
    }

    SkASSERT(0 == fPictureHash.count());

    // The atlas only lets go of its texture when the atlas is deleted.
    fAtlas.free();
}

void GrLayerCache::initAtlas() {
    SkASSERT(nullptr == fAtlas.get());
    GR_STATIC_ASSERT(kNumPlotsX*kNumPlotsX == GrPictureInfo::kNumPlots);

    SkISize textureSize = SkISize::Make(kAtlasTextureWidth, kAtlasTextureHeight);
    fAtlas.reset(new GrLayerAtlas(fContext->textureProvider(), kSkia8888_GrPixelConfig,
                                  kRenderTarget_GrSurfaceFlag, textureSize,
                                  kNumPlotsX, kNumPlotsY));
}

void GrLayerCache::freeAll() {

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        GrCachedLayer* layer = &(*iter);
        this->unlock(layer);
        delete layer;
    }
    fLayerHash.rewind();

    if (fAtlas) {
        fAtlas->resetPlots();
        fAtlas->detachBackingTexture();
    }
}

GrCachedLayer* GrLayerCache::createLayer(uint32_t pictureID,
                                         int start, int stop,
                                         const SkIRect& srcIR,
                                         const SkIRect& dstIR,
                                         const SkMatrix& initialMat,
                                         const int* key,
                                         int keySize,
                                         const SkPaint* paint) {
    SkASSERT(pictureID != SK_InvalidGenID && start >= 0 && stop > 0);

    GrCachedLayer* layer = new GrCachedLayer(pictureID, start, stop, srcIR, dstIR, initialMat, key,
                                             keySize, paint);
    fLayerHash.add(layer);
    return layer;
}

GrCachedLayer* GrLayerCache::findLayer(uint32_t pictureID, const SkMatrix& initialMat,
                                       const int* key, int keySize) {
    SkASSERT(pictureID != SK_InvalidGenID);
    return fLayerHash.find(GrCachedLayer::Key(pictureID, initialMat, key, keySize));
}

GrCachedLayer* GrLayerCache::findLayerOrCreate(uint32_t pictureID,
                                               int start, int stop,
                                               const SkIRect& srcIR,
                                               const SkIRect& dstIR,
                                               const SkMatrix& initialMat,
                                               const int* key,
                                               int keySize,
                                               const SkPaint* paint) {
    SkASSERT(pictureID != SK_InvalidGenID && start >= 0 && stop > 0);
    GrCachedLayer* layer = fLayerHash.find(GrCachedLayer::Key(pictureID, initialMat, key, keySize));
    if (nullptr == layer) {
        layer = this->createLayer(pictureID, start, stop,
                                  srcIR, dstIR, initialMat,
                                  key, keySize, paint);
    }

    return layer;
}

bool GrLayerCache::tryToAtlas(GrCachedLayer* layer,
                              const GrSurfaceDesc& desc,
                              bool* needsRendering) {
    SkDEBUGCODE(GrAutoValidateLayer avl(fAtlas ? fAtlas->getTextureOrNull() : nullptr, layer);)

    SkASSERT(PlausiblyAtlasable(desc.fWidth, desc.fHeight));
    SkASSERT(0 == desc.fSampleCnt);

    if (layer->locked()) {
        // This layer is already locked
        SkASSERT(fAtlas);
        SkASSERT(layer->isAtlased());
        SkASSERT(layer->rect().width() == desc.fWidth);
        SkASSERT(layer->rect().height() == desc.fHeight);
        *needsRendering = false;
        return true;
    }

    if (layer->isAtlased()) {
        SkASSERT(fAtlas);
        // Hooray it is still in the atlas - make sure it stays there
        layer->setLocked(true);
        this->incPlotLock(layer->plot()->id());
        *needsRendering = false;
        return true;
    } else {
        if (!fAtlas) {
            this->initAtlas();
            if (!fAtlas) {
                return false;
            }
        }
        // Not in the atlas - will it fit?
        GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        if (nullptr == pictInfo) {
            pictInfo = new GrPictureInfo(layer->pictureID());
            fPictureHash.add(pictInfo);
        }

        SkIPoint16 loc;
        for (int i = 0; i < 2; ++i) { // extra pass in case we fail to add but are able to purge
            GrLayerAtlas::Plot* plot = fAtlas->addToAtlas(&pictInfo->fPlotUsage,
                                                          desc.fWidth, desc.fHeight,
                                                          &loc);
            // addToAtlas can allocate the backing texture
            SkDEBUGCODE(avl.setBackingTexture(fAtlas->getTexture()));
            if (plot) {
#if !GR_CACHE_HOISTED_LAYERS
                pictInfo->incPlotUsage(plot->id());
#endif
                // The layer was successfully added to the atlas
                const SkIRect bounds = SkIRect::MakeXYWH(loc.fX, loc.fY,
                                                         desc.fWidth, desc.fHeight);
                layer->setTexture(fAtlas->getTexture(), bounds, true);
                layer->setPlot(plot);
                layer->setLocked(true);
                this->incPlotLock(layer->plot()->id());
                *needsRendering = true;
                return true;
            }

            // The layer was rejected by the atlas (even though we know it is
            // plausibly atlas-able). See if a plot can be purged and try again.
            if (!this->purgePlots(true)) {
                break;  // We weren't able to purge any plots
            }
        }

        if (pictInfo->fPlotUsage.isEmpty()) {
            fPictureHash.remove(pictInfo->fPictureID);
            delete pictInfo;
        }
    }

    return false;
}

bool GrLayerCache::lock(GrCachedLayer* layer, const GrSurfaceDesc& desc, bool* needsRendering) {
    if (layer->locked()) {
        // This layer is already locked
        *needsRendering = false;
        return true;
    }

    // TODO: make the test for exact match depend on the image filters themselves
    SkAutoTUnref<GrTexture> tex;
    if (layer->fFilter) {
        tex.reset(fContext->textureProvider()->createTexture(desc, true));
    } else {
        tex.reset(fContext->textureProvider()->createApproxTexture(desc));
    }

    if (!tex) {
        return false;
    }

    layer->setTexture(tex, SkIRect::MakeWH(desc.fWidth, desc.fHeight), false);
    layer->setLocked(true);
    *needsRendering = true;
    return true;
}

void GrLayerCache::unlock(GrCachedLayer* layer) {
    SkDEBUGCODE(GrAutoValidateLayer avl(fAtlas ? fAtlas->getTextureOrNull() : nullptr, layer);)

    if (nullptr == layer || !layer->locked()) {
        // invalid or not locked
        return;
    }

    if (layer->isAtlased()) {
        const int plotID = layer->plot()->id();

        this->decPlotLock(plotID);
        // At this point we could aggressively clear out un-locked plots but
        // by delaying we may be able to reuse some of the atlased layers later.
#if !GR_CACHE_HOISTED_LAYERS
        // This testing code aggressively removes the atlased layers. This
        // can be used to separate the performance contribution of less
        // render target pingponging from that due to the re-use of cached layers
        GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        SkASSERT(pictInfo);

        pictInfo->decPlotUsage(plotID);

        if (0 == pictInfo->plotUsage(plotID)) {
            pictInfo->fPlotUsage.removePlot(layer->plot());

            if (pictInfo->fPlotUsage.isEmpty()) {
                fPictureHash.remove(pictInfo->fPictureID);
                delete pictInfo;
            }
        }

        layer->setPlot(nullptr);
        layer->setTexture(nullptr, SkIRect::MakeEmpty(), false);
#endif

    } else {
        layer->setTexture(nullptr, SkIRect::MakeEmpty(), false);
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

        layer->validate(fAtlas.get() ? fAtlas->getTextureOrNull() : nullptr);

        const GrPictureInfo* pictInfo = fPictureHash.find(layer->pictureID());
        if (!pictInfo) {
            // If there is no picture info for this picture then all of its
            // layers should be non-atlased.
            SkASSERT(!layer->isAtlased());
        }

        if (layer->plot()) {
            SkASSERT(pictInfo);
            SkASSERT(pictInfo->fPictureID == layer->pictureID());

            SkASSERT(pictInfo->fPlotUsage.contains(layer->plot()));
#if !GR_CACHE_HOISTED_LAYERS
            SkASSERT(pictInfo->plotUsage(layer->plot()->id()) > 0);
#endif

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
        SkASSERT(0 == toBeRemoved[i]->uses());
        this->unlock(toBeRemoved[i]);
        fLayerHash.remove(GrCachedLayer::GetKey(*toBeRemoved[i]));
        delete toBeRemoved[i];
    }

    GrPictureInfo* pictInfo = fPictureHash.find(pictureID);
    if (pictInfo) {
        fPictureHash.remove(pictureID);
        delete pictInfo;
    }
}

bool GrLayerCache::purgePlots(bool justOne) {
    SkDEBUGCODE(GrAutoValidateCache avc(this);)
    SkASSERT(fAtlas);

    bool anyPurged = false;
    GrLayerAtlas::PlotIter iter;
    GrLayerAtlas::Plot* plot;
    for (plot = fAtlas->iterInit(&iter, GrLayerAtlas::kLRUFirst_IterOrder);
         plot;
         plot = iter.prev()) {
        if (fPlotLocks[plot->id()] > 0) {
            continue;
        }

        anyPurged = true;
        this->purgePlot(plot);
        if (justOne) {
            break;
        }
    }

    return anyPurged;
}

void GrLayerCache::purgePlot(GrLayerAtlas::Plot* plot) {
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
        SkASSERT(0 == toBeRemoved[i]->uses());
        SkASSERT(!toBeRemoved[i]->locked());

        uint32_t pictureIDToRemove = toBeRemoved[i]->pictureID();

        // Aggressively remove layers and, if it becomes totally uncached, delete the picture info
        fLayerHash.remove(GrCachedLayer::GetKey(*toBeRemoved[i]));
        delete toBeRemoved[i];

        GrPictureInfo* pictInfo = fPictureHash.find(pictureIDToRemove);
        if (pictInfo) {
#if !GR_CACHE_HOISTED_LAYERS
            SkASSERT(0 == pictInfo->plotUsage(plot->id()));
#endif
            pictInfo->fPlotUsage.removePlot(plot);

            if (pictInfo->fPlotUsage.isEmpty()) {
                fPictureHash.remove(pictInfo->fPictureID);
                delete pictInfo;
            }
        }
    }

    plot->reset();
}

#if !GR_CACHE_HOISTED_LAYERS
void GrLayerCache::purgeAll() {
    if (!fAtlas) {
        return;
    }

    this->purgePlots(false); // clear them all out

    SkASSERT(0 == fPictureHash.count());

    if (fAtlas->getTextureOrNull()) {
        SkAutoTUnref<GrDrawContext> drawContext(
                                    fContext->drawContext(fAtlas->getTexture()->asRenderTarget()));

        if (drawContext) {
            drawContext->discard();
        }
    }
}
#endif

void GrLayerCache::begin() {
    if (!fAtlas) {
        return;
    }

    if (!fAtlas->reattachBackingTexture()) {
        // We weren't able to re-attach. Clear out all the atlased layers.
        this->purgePlots(false);
        SkASSERT(0 == fPictureHash.count());
    }
#ifdef SK_DEBUG
    else {
        // we've reattached - everything had better make sense
        SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
        for (; !iter.done(); ++iter) {
            GrCachedLayer* layer = &(*iter);

            if (layer->isAtlased()) {
                SkASSERT(fAtlas->getTexture() == layer->texture());
            }
        }
    }
#endif
}

void GrLayerCache::end() {
    if (!fAtlas) {
        return;
    }

    // Adding this call will clear out all the layers in the atlas
    //this->purgePlots(false);

    fAtlas->detachBackingTexture();
}

void GrLayerCache::processDeletedPictures() {
    SkTArray<SkPicture::DeletionMessage> deletedPictures;
    fPictDeletionInbox.poll(&deletedPictures);

    for (int i = 0; i < deletedPictures.count(); i++) {
        this->purge(deletedPictures[i].fUniqueID);
    }
}

#ifdef SK_DEVELOPER
void GrLayerCache::writeLayersToDisk(const SkString& dirName) {

    if (fAtlas) {
        GrTexture* atlasTexture = fAtlas->getTextureOrNull();
        if (nullptr != atlasTexture) {
            SkString fileName(dirName);
            fileName.append("\\atlas.png");

            atlasTexture->surfacePriv().savePixels(fileName.c_str());
        }
    }

    SkTDynamicHash<GrCachedLayer, GrCachedLayer::Key>::Iter iter(&fLayerHash);
    for (; !iter.done(); ++iter) {
        GrCachedLayer* layer = &(*iter);

        if (layer->isAtlased() || !layer->texture()) {
            continue;
        }

        SkString fileName(dirName);
        fileName.appendf("\\%d", layer->fKey.pictureID());
        for (int i = 0; i < layer->fKey.keySize(); ++i) {
            fileName.appendf("-%d", layer->fKey.key()[i]);
        }
        fileName.appendf(".png");

        layer->texture()->surfacePriv().savePixels(fileName.c_str());
    }
}
#endif
