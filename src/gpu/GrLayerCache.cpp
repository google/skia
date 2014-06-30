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
 *  PictureLayerKey just wraps a saveLayer's id in the picture for GrTHashTable.
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

GrLayerCache::GrLayerCache(GrContext* context)
    : fContext(context)
    , fLayerPool(16) {      // TODO: may need to increase this later
}

GrLayerCache::~GrLayerCache() {
}

void GrLayerCache::init() {
    static const int kAtlasTextureWidth = 1024;
    static const int kAtlasTextureHeight = 1024;

    SkASSERT(NULL == fAtlas.get());

    // The layer cache only gets 1 plot
    SkISize textureSize = SkISize::Make(kAtlasTextureWidth, kAtlasTextureHeight);
    fAtlas.reset(SkNEW_ARGS(GrAtlas, (fContext->getGpu(), kSkia8888_GrPixelConfig,
                                      textureSize, 1, 1, false)));
}

void GrLayerCache::freeAll() {
    fLayerHash.deleteAll();
    fAtlas.free();
}

GrCachedLayer* GrLayerCache::createLayer(const SkPicture* picture, int layerID) {
    GrCachedLayer* layer = fLayerPool.alloc();

    SkASSERT(picture->uniqueID() != SK_InvalidGenID);
    layer->init(picture->uniqueID(), layerID);
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
    SkASSERT(NULL == layer->getTexture());

    // This just uses scratch textures and doesn't cache the texture.
    // This can yield a lot of re-rendering
    layer->setTexture(fContext->lockAndRefScratchTexture(desc, GrContext::kApprox_ScratchTexMatch));
    return false;
}

void GrLayerCache::unlock(GrCachedLayer* layer) {
    if (NULL == layer || NULL == layer->getTexture()) {
        return;
    }

    fContext->unlockScratchTexture(layer->getTexture());
    layer->setTexture(NULL);
}
