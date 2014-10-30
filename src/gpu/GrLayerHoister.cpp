/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLayerCache.h"
#include "GrLayerHoister.h"
#include "GrRecordReplaceDraw.h"

#include "SkCanvas.h"
#include "SkGrPixelRef.h"
#include "SkRecordDraw.h"
#include "SkSurface.h"

// Create the layer information for the hoisted layer and secure the
// required texture/render target resources.
static void prepare_for_hoisting(GrLayerCache* layerCache, 
                                 const SkPicture* topLevelPicture,
                                 const GrAccelData::SaveLayerInfo& info,
                                 const SkIRect& layerRect,
                                 SkTDArray<GrHoistedLayer>* needRendering,
                                 SkTDArray<GrHoistedLayer>* recycled,
                                 bool attemptToAtlas) {
    const SkPicture* pict = info.fPicture ? info.fPicture : topLevelPicture;

    SkMatrix combined = SkMatrix::Concat(info.fPreMat, info.fLocalMat);

    GrCachedLayer* layer = layerCache->findLayerOrCreate(pict->uniqueID(),
                                                         info.fSaveLayerOpID,
                                                         info.fRestoreOpID,
                                                         layerRect,
                                                         combined,
                                                         info.fPaint);
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = layerRect.width();
    desc.fHeight = layerRect.height();
    desc.fConfig = kSkia8888_GrPixelConfig;
    // TODO: need to deal with sample count

    bool locked, needsRendering;
    if (attemptToAtlas) {
        locked = layerCache->tryToAtlas(layer, desc, &needsRendering);
    } else {
        locked = layerCache->lock(layer, desc, &needsRendering);
    }
    if (!locked) {
        // GPU resources could not be secured for the hoisting of this layer
        return;
    }

    if (attemptToAtlas) {
        SkASSERT(layer->isAtlased());
    }

    GrHoistedLayer* hl;

    if (needsRendering) {
        if (!attemptToAtlas) {
            SkASSERT(!layer->isAtlased());
        }
        hl = needRendering->append();
    } else {
        hl = recycled->append();
    }
    
    layerCache->addUse(layer);
    hl->fLayer = layer;
    hl->fPicture = pict;
    hl->fOffset = SkIPoint::Make(layerRect.fLeft, layerRect.fTop);
    hl->fLocalMat = info.fLocalMat;
    hl->fPreMat = info.fPreMat;
}

// Atlased layers must be small enough to fit in the atlas, not have a
// paint with an image filter and be neither nested nor nesting.
// TODO: allow leaf nested layers to appear in the atlas.
void GrLayerHoister::FindLayersToAtlas(GrContext* context,
                                       const SkPicture* topLevelPicture,
                                       const SkRect& query,
                                       SkTDArray<GrHoistedLayer>* atlased,
                                       SkTDArray<GrHoistedLayer>* recycled) {
    GrLayerCache* layerCache = context->getLayerCache();

    layerCache->processDeletedPictures();

    SkPicture::AccelData::Key key = GrAccelData::ComputeAccelDataKey();

    const SkPicture::AccelData* topLevelData = topLevelPicture->EXPERIMENTAL_getAccelData(key);
    if (!topLevelData) {
        return;
    }

    const GrAccelData *topLevelGPUData = static_cast<const GrAccelData*>(topLevelData);
    if (0 == topLevelGPUData->numSaveLayers()) {
        return;
    }

    atlased->setReserve(atlased->count() + topLevelGPUData->numSaveLayers());

    for (int i = 0; i < topLevelGPUData->numSaveLayers(); ++i) {
        const GrAccelData::SaveLayerInfo& info = topLevelGPUData->saveLayerInfo(i);

        // TODO: ignore perspective projected layers here?
        bool disallowAtlasing = info.fHasNestedLayers || info.fIsNested ||
                                (info.fPaint && info.fPaint->getImageFilter());

        if (disallowAtlasing) {
            continue;
        }

        SkRect layerRect = info.fBounds;
        if (!layerRect.intersect(query)) {
            continue;
        }

        SkIRect ir;
        layerRect.roundOut(&ir);

        if (!GrLayerCache::PlausiblyAtlasable(ir.width(), ir.height())) {
            continue;
        }

        prepare_for_hoisting(layerCache, topLevelPicture, info, ir, atlased, recycled, true);
    }

}

void GrLayerHoister::FindLayersToHoist(GrContext* context,
                                       const SkPicture* topLevelPicture,
                                       const SkRect& query,
                                       SkTDArray<GrHoistedLayer>* needRendering,
                                       SkTDArray<GrHoistedLayer>* recycled) {
    GrLayerCache* layerCache = context->getLayerCache();

    layerCache->processDeletedPictures();

    SkPicture::AccelData::Key key = GrAccelData::ComputeAccelDataKey();

    const SkPicture::AccelData* topLevelData = topLevelPicture->EXPERIMENTAL_getAccelData(key);
    if (!topLevelData) {
        return;
    }

    const GrAccelData *topLevelGPUData = static_cast<const GrAccelData*>(topLevelData);
    if (0 == topLevelGPUData->numSaveLayers()) {
        return;
    }

    // Find and prepare for hoisting all the layers that intersect the query rect
    for (int i = 0; i < topLevelGPUData->numSaveLayers(); ++i) {
        const GrAccelData::SaveLayerInfo& info = topLevelGPUData->saveLayerInfo(i);
        if (info.fIsNested) {
            // Parent layers are currently hoisted while nested layers are not.
            continue;
        }

        SkRect layerRect = info.fBounds;
        if (!layerRect.intersect(query)) {
            continue;
        }

        SkIRect ir;
        layerRect.roundOut(&ir);

        prepare_for_hoisting(layerCache, topLevelPicture, info, ir, 
                             needRendering, recycled, false);
    }
}

static void wrap_texture(GrTexture* texture, int width, int height, SkBitmap* result) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    result->setInfo(info);
    result->setPixelRef(SkNEW_ARGS(SkGrPixelRef, (info, texture)))->unref();
}

void GrLayerHoister::ConvertLayersToReplacements(const SkTDArray<GrHoistedLayer>& layers,
                                                 GrReplacements* replacements) {
    // TODO: just replace GrReplacements::ReplacementInfo with GrCachedLayer?
    for (int i = 0; i < layers.count(); ++i) {
        GrCachedLayer* layer = layers[i].fLayer;
        const SkPicture* picture = layers[i].fPicture;

        SkMatrix combined = SkMatrix::Concat(layers[i].fPreMat, layers[i].fLocalMat);

        GrReplacements::ReplacementInfo* layerInfo =
                    replacements->newReplacement(picture->uniqueID(),
                                                 layer->start(),
                                                 combined);
        layerInfo->fStop = layer->stop();
        layerInfo->fPos = layers[i].fOffset;

        SkBitmap bm;
        wrap_texture(layers[i].fLayer->texture(),
                     !layers[i].fLayer->isAtlased() ? layers[i].fLayer->rect().width()
                                                    : layers[i].fLayer->texture()->width(),
                     !layers[i].fLayer->isAtlased() ? layers[i].fLayer->rect().height()
                                                    : layers[i].fLayer->texture()->height(),
                     &bm);
        layerInfo->fImage = SkImage::NewTexture(bm);

        layerInfo->fPaint = layers[i].fLayer->paint()
                                ? SkNEW_ARGS(SkPaint, (*layers[i].fLayer->paint()))
                                : NULL;

        layerInfo->fSrcRect = SkIRect::MakeXYWH(layers[i].fLayer->rect().fLeft,
                                                layers[i].fLayer->rect().fTop,
                                                layers[i].fLayer->rect().width(),
                                                layers[i].fLayer->rect().height());
    }
}

void GrLayerHoister::DrawLayersToAtlas(GrContext* context,
                                       const SkTDArray<GrHoistedLayer>& atlased) {
    if (atlased.count() > 0) {
        // All the atlased layers are rendered into the same GrTexture
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                        atlased[0].fLayer->texture()->asRenderTarget(), NULL));

        SkCanvas* atlasCanvas = surface->getCanvas();

        SkPaint clearPaint;
        clearPaint.setColor(SK_ColorTRANSPARENT);
        clearPaint.setXfermode(SkXfermode::Create(SkXfermode::kSrc_Mode))->unref();

        for (int i = 0; i < atlased.count(); ++i) {
            const GrCachedLayer* layer = atlased[i].fLayer;
            const SkPicture* pict = atlased[i].fPicture;
            const SkIPoint offset = atlased[i].fOffset;
            SkDEBUGCODE(const SkPaint* layerPaint = layer->paint();)

            SkASSERT(!layerPaint || !layerPaint->getImageFilter());

            atlasCanvas->save();

            // Add a rect clip to make sure the rendering doesn't
            // extend beyond the boundaries of the atlased sub-rect
            SkRect bound = SkRect::MakeXYWH(SkIntToScalar(layer->rect().fLeft),
                                            SkIntToScalar(layer->rect().fTop),
                                            SkIntToScalar(layer->rect().width()),
                                            SkIntToScalar(layer->rect().height()));
            atlasCanvas->clipRect(bound);

            // Since 'clear' doesn't respect the clip we need to draw a rect
            atlasCanvas->drawRect(bound, clearPaint);

            // '-offset' maps the layer's top/left to the origin.
            // Since this layer is atlased, the top/left corner needs
            // to be offset to the correct location in the backing texture.
            SkMatrix initialCTM;
            initialCTM.setTranslate(SkIntToScalar(-offset.fX), SkIntToScalar(-offset.fY));
            initialCTM.preTranslate(bound.fLeft, bound.fTop);
            initialCTM.preConcat(atlased[i].fPreMat);

            atlasCanvas->setMatrix(initialCTM);
            atlasCanvas->concat(atlased[i].fLocalMat);

            SkRecordPartialDraw(*pict->fRecord.get(), atlasCanvas, bound,
                                layer->start() + 1, layer->stop(), initialCTM);

            atlasCanvas->restore();
        }

        atlasCanvas->flush();
    }
}

void GrLayerHoister::DrawLayers(GrContext* context, const SkTDArray<GrHoistedLayer>& layers) {
    for (int i = 0; i < layers.count(); ++i) {
        GrCachedLayer* layer = layers[i].fLayer;
        const SkPicture* pict = layers[i].fPicture;
        const SkIPoint& offset = layers[i].fOffset;

        // Each non-atlased layer has its own GrTexture
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                        layer->texture()->asRenderTarget(), NULL));

        SkCanvas* layerCanvas = surface->getCanvas();

        SkASSERT(0 == layer->rect().fLeft && 0 == layer->rect().fTop);

        // Add a rect clip to make sure the rendering doesn't
        // extend beyond the boundaries of the layer
        SkRect bound = SkRect::MakeXYWH(SkIntToScalar(layer->rect().fLeft),
                                        SkIntToScalar(layer->rect().fTop),
                                        SkIntToScalar(layer->rect().width()),
                                        SkIntToScalar(layer->rect().height()));

        layerCanvas->clipRect(bound);

        layerCanvas->clear(SK_ColorTRANSPARENT);

        SkMatrix initialCTM;
        initialCTM.setTranslate(SkIntToScalar(-offset.fX), SkIntToScalar(-offset.fY));
        initialCTM.preConcat(layers[i].fPreMat);

        layerCanvas->setMatrix(initialCTM);
        layerCanvas->concat(layers[i].fLocalMat);

        SkRecordPartialDraw(*pict->fRecord.get(), layerCanvas, bound,
                            layer->start()+1, layer->stop(), initialCTM);

        layerCanvas->flush();
    }
}

void GrLayerHoister::UnlockLayers(GrContext* context,
                                  const SkTDArray<GrHoistedLayer>& layers) {
    GrLayerCache* layerCache = context->getLayerCache();

    for (int i = 0; i < layers.count(); ++i) {
        layerCache->removeUse(layers[i].fLayer);
    }

    SkDEBUGCODE(layerCache->validate();)
}

void GrLayerHoister::PurgeCache(GrContext* context) {
#if !GR_CACHE_HOISTED_LAYERS
    GrLayerCache* layerCache = context->getLayerCache();

    // This code completely clears out the atlas. It is required when
    // caching is disabled so the atlas doesn't fill up and force more
    // free floating layers
    layerCache->purgeAll();
#endif
}
