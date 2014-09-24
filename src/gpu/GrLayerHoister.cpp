/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLayerCache.h"
#include "GrLayerHoister.h"
#include "SkCanvas.h"
#include "SkRecordDraw.h"
#include "GrRecordReplaceDraw.h"
#include "SkGrPixelRef.h"
#include "SkSurface.h"

// Return true if any layers are suitable for hoisting
bool GrLayerHoister::FindLayersToHoist(const SkPicture* topLevelPicture,
                                       const SkRect& query,
                                       SkTDArray<HoistedLayer>* atlased,
                                       SkTDArray<HoistedLayer>* nonAtlased,
                                       GrLayerCache* layerCache) {
    bool anyHoisted = false;

    SkPicture::AccelData::Key key = GrAccelData::ComputeAccelDataKey();

    const SkPicture::AccelData* topLevelData = topLevelPicture->EXPERIMENTAL_getAccelData(key);
    if (NULL == topLevelData) {
        return false;
    }

    const GrAccelData *topLevelGPUData = static_cast<const GrAccelData*>(topLevelData);
    if (0 == topLevelGPUData->numSaveLayers()) {
        return false;
    }

    // Layer hoisting pre-renders the entire layer since it will be cached and potentially
    // reused with different clips (e.g., in different tiles). Because of this the
    // clip will not be limiting the size of the pre-rendered layer. kSaveLayerMaxSize
    // is used to limit which clips are pre-rendered.
    static const int kSaveLayerMaxSize = 256;

    SkAutoTArray<bool> pullForward(topLevelGPUData->numSaveLayers());

    // Pre-render all the layers that intersect the query rect
    for (int i = 0; i < topLevelGPUData->numSaveLayers(); ++i) {
        pullForward[i] = false;

        const GrAccelData::SaveLayerInfo& info = topLevelGPUData->saveLayerInfo(i);

        SkRect layerRect = SkRect::MakeXYWH(SkIntToScalar(info.fOffset.fX),
                                            SkIntToScalar(info.fOffset.fY),
                                            SkIntToScalar(info.fSize.fWidth),
                                            SkIntToScalar(info.fSize.fHeight));

        if (!SkRect::Intersects(query, layerRect)) {
            continue;
        }

        // TODO: once this code is more stable unsuitable layers can
        // just be omitted during the optimization stage
        if (!info.fValid ||
            kSaveLayerMaxSize < info.fSize.fWidth ||
            kSaveLayerMaxSize < info.fSize.fHeight ||
            info.fIsNested) {
            continue;
        }

        pullForward[i] = true;
        anyHoisted = true;
    }

    if (!anyHoisted) {
        return false;
    }

    atlased->setReserve(atlased->reserved() + topLevelGPUData->numSaveLayers());

    // Generate the layer and/or ensure it is locked
    for (int i = 0; i < topLevelGPUData->numSaveLayers(); ++i) {
        if (pullForward[i]) {
            const GrAccelData::SaveLayerInfo& info = topLevelGPUData->saveLayerInfo(i);
            const SkPicture* pict = info.fPicture ? info.fPicture : topLevelPicture;

            GrCachedLayer* layer = layerCache->findLayerOrCreate(pict->uniqueID(),
                                                                 info.fSaveLayerOpID,
                                                                 info.fRestoreOpID,
                                                                 info.fOffset,
                                                                 info.fOriginXform,
                                                                 info.fPaint);

            GrTextureDesc desc;
            desc.fFlags = kRenderTarget_GrTextureFlagBit;
            desc.fWidth = info.fSize.fWidth;
            desc.fHeight = info.fSize.fHeight;
            desc.fConfig = kSkia8888_GrPixelConfig;
            // TODO: need to deal with sample count

            bool needsRendering = layerCache->lock(layer, desc,
                                                   info.fHasNestedLayers || info.fIsNested);
            if (NULL == layer->texture()) {
                continue;
            }

            if (needsRendering) {
                HoistedLayer* info;

                if (layer->isAtlased()) {
                    info = atlased->append();
                } else {
                    info = nonAtlased->append();
                }

                info->fLayer = layer;
                info->fPicture = pict;
            }
        }
    }

    return anyHoisted;
}

static void wrap_texture(GrTexture* texture, int width, int height, SkBitmap* result) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    result->setInfo(info);
    result->setPixelRef(SkNEW_ARGS(SkGrPixelRef, (info, texture)))->unref();
}

static void convert_layers_to_replacements(const SkTDArray<GrLayerHoister::HoistedLayer>& layers,
                                           GrReplacements* replacements) {
    // TODO: just replace GrReplacements::ReplacementInfo with GrCachedLayer?
    for (int i = 0; i < layers.count(); ++i) {
        GrReplacements::ReplacementInfo* layerInfo = replacements->push();
        layerInfo->fStart = layers[i].fLayer->start();
        layerInfo->fStop = layers[i].fLayer->stop();
        layerInfo->fPos = layers[i].fLayer->offset();;

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

void GrLayerHoister::DrawLayers(const SkTDArray<HoistedLayer>& atlased,
                                const SkTDArray<HoistedLayer>& nonAtlased,
                                GrReplacements* replacements) {
    // Render the atlased layers that require it
    if (atlased.count() > 0) {
        // All the atlased layers are rendered into the same GrTexture
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                        atlased[0].fLayer->texture()->asRenderTarget(), NULL));

        SkCanvas* atlasCanvas = surface->getCanvas();

        SkPaint paint;
        paint.setColor(SK_ColorTRANSPARENT);
        paint.setXfermode(SkXfermode::Create(SkXfermode::kSrc_Mode))->unref();

        for (int i = 0; i < atlased.count(); ++i) {
            GrCachedLayer* layer = atlased[i].fLayer;
            const SkPicture* pict = atlased[i].fPicture;

            atlasCanvas->save();

            // Add a rect clip to make sure the rendering doesn't
            // extend beyond the boundaries of the atlased sub-rect
            SkRect bound = SkRect::MakeXYWH(SkIntToScalar(layer->rect().fLeft),
                                            SkIntToScalar(layer->rect().fTop),
                                            SkIntToScalar(layer->rect().width()),
                                            SkIntToScalar(layer->rect().height()));
            atlasCanvas->clipRect(bound);

            // Since 'clear' doesn't respect the clip we need to draw a rect
            // TODO: ensure none of the atlased layers contain a clear call!
            atlasCanvas->drawRect(bound, paint);

            // info.fCTM maps the layer's top/left to the origin.
            // Since this layer is atlased, the top/left corner needs
            // to be offset to the correct location in the backing texture.
            SkMatrix initialCTM;
            initialCTM.setTranslate(SkIntToScalar(-layer->offset().fX), 
                                    SkIntToScalar(-layer->offset().fY));
            initialCTM.postTranslate(bound.fLeft, bound.fTop);
            
            atlasCanvas->translate(SkIntToScalar(-layer->offset().fX), 
                                   SkIntToScalar(-layer->offset().fY));
            atlasCanvas->translate(bound.fLeft, bound.fTop);
            atlasCanvas->concat(layer->ctm());

            SkRecordPartialDraw(*pict->fRecord.get(), atlasCanvas, bound,
                                layer->start()+1, layer->stop(), initialCTM);

            atlasCanvas->restore();
        }

        atlasCanvas->flush();
    }

    // Render the non-atlased layers that require it
    for (int i = 0; i < nonAtlased.count(); ++i) {
        GrCachedLayer* layer = nonAtlased[i].fLayer;
        const SkPicture* pict = nonAtlased[i].fPicture;

        // Each non-atlased layer has its own GrTexture
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                        layer->texture()->asRenderTarget(), NULL));

        SkCanvas* layerCanvas = surface->getCanvas();

        // Add a rect clip to make sure the rendering doesn't
        // extend beyond the boundaries of the atlased sub-rect
        SkRect bound = SkRect::MakeXYWH(SkIntToScalar(layer->rect().fLeft),
                                        SkIntToScalar(layer->rect().fTop),
                                        SkIntToScalar(layer->rect().width()),
                                        SkIntToScalar(layer->rect().height()));

        layerCanvas->clipRect(bound); // TODO: still useful?

        layerCanvas->clear(SK_ColorTRANSPARENT);

        SkMatrix initialCTM;
        initialCTM.setTranslate(SkIntToScalar(-layer->offset().fX), 
                                SkIntToScalar(-layer->offset().fY));

        layerCanvas->translate(SkIntToScalar(-layer->offset().fX), 
                               SkIntToScalar(-layer->offset().fY));
        layerCanvas->concat(layer->ctm());

        SkRecordPartialDraw(*pict->fRecord.get(), layerCanvas, bound,
                            layer->start()+1, layer->stop(), initialCTM);

        layerCanvas->flush();
    }

    convert_layers_to_replacements(atlased, replacements);
    convert_layers_to_replacements(nonAtlased, replacements);
}

static void unlock_layer_in_cache(GrLayerCache* layerCache,
                                  const SkPicture* picture,
                                  GrCachedLayer* layer) {
    layerCache->unlock(layer);

#if DISABLE_CACHING
    // This code completely clears out the atlas. It is required when
    // caching is disabled so the atlas doesn't fill up and force more
    // free floating layers
    layerCache->purge(picture->uniqueID());
#endif
}

void GrLayerHoister::UnlockLayers(GrLayerCache* layerCache, 
                                  const SkTDArray<HoistedLayer>& atlased,
                                  const SkTDArray<HoistedLayer>& nonAtlased) {

    for (int i = 0; i < atlased.count(); ++i) {
        unlock_layer_in_cache(layerCache, atlased[i].fPicture, atlased[i].fLayer);
    }

    for (int i = 0; i < nonAtlased.count(); ++i) {
        unlock_layer_in_cache(layerCache, nonAtlased[i].fPicture, nonAtlased[i].fLayer);
    }

#if DISABLE_CACHING
    // This code completely clears out the atlas. It is required when
    // caching is disabled so the atlas doesn't fill up and force more
    // free floating layers
    layerCache->purgeAll();
#endif
}

