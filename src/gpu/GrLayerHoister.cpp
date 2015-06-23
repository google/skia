/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrLayerCache.h"
#include "GrLayerHoister.h"
#include "GrRecordReplaceDraw.h"

#include "SkBigPicture.h"
#include "SkCanvas.h"
#include "SkGpuDevice.h"
#include "SkGrPixelRef.h"
#include "SkLayerInfo.h"
#include "SkRecordDraw.h"
#include "SkSurface.h"
#include "SkSurface_Gpu.h"

// Create the layer information for the hoisted layer and secure the
// required texture/render target resources.
static void prepare_for_hoisting(GrLayerCache* layerCache,
                                 const SkPicture* topLevelPicture,
                                 const SkMatrix& initialMat,
                                 const SkLayerInfo::BlockInfo& info,
                                 const SkIRect& srcIR,
                                 const SkIRect& dstIR,
                                 SkTDArray<GrHoistedLayer>* needRendering,
                                 SkTDArray<GrHoistedLayer>* recycled,
                                 bool attemptToAtlas,
                                 int numSamples) {
    const SkPicture* pict = info.fPicture ? info.fPicture : topLevelPicture;

    GrCachedLayer* layer = layerCache->findLayerOrCreate(topLevelPicture->uniqueID(),
                                                         SkToInt(info.fSaveLayerOpID),
                                                         SkToInt(info.fRestoreOpID),
                                                         srcIR,
                                                         dstIR,
                                                         initialMat,
                                                         info.fKey,
                                                         info.fKeySize,
                                                         info.fPaint);
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = srcIR.width();
    desc.fHeight = srcIR.height();
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fSampleCnt = numSamples;

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
    hl->fLocalMat = info.fLocalMat;
    hl->fInitialMat = initialMat;
    hl->fPreMat = initialMat;
    hl->fPreMat.preConcat(info.fPreMat);
}

// Compute the source rect and return false if it is empty.
static bool compute_source_rect(const SkLayerInfo::BlockInfo& info, const SkMatrix& initialMat,
                                const SkIRect& dstIR, SkIRect* srcIR) {
    SkIRect clipBounds = dstIR;

    SkMatrix totMat = initialMat;
    totMat.preConcat(info.fPreMat);
    totMat.preConcat(info.fLocalMat);

    if (info.fPaint && info.fPaint->getImageFilter()) {
        info.fPaint->getImageFilter()->filterBounds(clipBounds, totMat, &clipBounds);
    }

    if (!info.fSrcBounds.isEmpty()) {
        SkRect r;

        totMat.mapRect(&r, info.fSrcBounds);
        r.roundOut(srcIR);

        if (!srcIR->intersect(clipBounds)) {
            return false;
        }
    } else {
        *srcIR = clipBounds;
    }

    return true;
}

// Atlased layers must be small enough to fit in the atlas, not have a
// paint with an image filter and be neither nested nor nesting.
// TODO: allow leaf nested layers to appear in the atlas.
void GrLayerHoister::FindLayersToAtlas(GrContext* context,
                                       const SkPicture* topLevelPicture,
                                       const SkMatrix& initialMat,
                                       const SkRect& query,
                                       SkTDArray<GrHoistedLayer>* atlased,
                                       SkTDArray<GrHoistedLayer>* recycled,
                                       int numSamples) {
    if (0 != numSamples) {
        // MSAA layers are currently never atlased
        return;
    }

    GrLayerCache* layerCache = context->getLayerCache();
    layerCache->processDeletedPictures();

    const SkBigPicture::AccelData* topLevelData = NULL;
    if (const SkBigPicture* bp = topLevelPicture->asSkBigPicture()) {
        topLevelData = bp->accelData();
    }
    if (!topLevelData) {
        return;
    }

    const SkLayerInfo *topLevelGPUData = static_cast<const SkLayerInfo*>(topLevelData);
    if (0 == topLevelGPUData->numBlocks()) {
        return;
    }

    atlased->setReserve(atlased->count() + topLevelGPUData->numBlocks());

    for (int i = 0; i < topLevelGPUData->numBlocks(); ++i) {
        const SkLayerInfo::BlockInfo& info = topLevelGPUData->block(i);

        // TODO: ignore perspective projected layers here?
        bool disallowAtlasing = info.fHasNestedLayers || info.fIsNested ||
                                (info.fPaint && info.fPaint->getImageFilter());

        if (disallowAtlasing) {
            continue;
        }

        SkRect layerRect;
        initialMat.mapRect(&layerRect, info.fBounds);
        if (!layerRect.intersect(query)) {
            continue;
        }

        const SkIRect dstIR = layerRect.roundOut();

        SkIRect srcIR;

        if (!compute_source_rect(info, initialMat, dstIR, &srcIR) ||
            !GrLayerCache::PlausiblyAtlasable(srcIR.width(), srcIR.height())) {
            continue;
        }

        prepare_for_hoisting(layerCache, topLevelPicture, initialMat,
                             info, srcIR, dstIR, atlased, recycled, true, 0);
    }

}

void GrLayerHoister::FindLayersToHoist(GrContext* context,
                                       const SkPicture* topLevelPicture,
                                       const SkMatrix& initialMat,
                                       const SkRect& query,
                                       SkTDArray<GrHoistedLayer>* needRendering,
                                       SkTDArray<GrHoistedLayer>* recycled,
                                       int numSamples) {
    GrLayerCache* layerCache = context->getLayerCache();

    layerCache->processDeletedPictures();

    const SkBigPicture::AccelData* topLevelData = NULL;
    if (const SkBigPicture* bp = topLevelPicture->asSkBigPicture()) {
        topLevelData = bp->accelData();
    }
    if (!topLevelData) {
        return;
    }

    const SkLayerInfo *topLevelGPUData = static_cast<const SkLayerInfo*>(topLevelData);
    if (0 == topLevelGPUData->numBlocks()) {
        return;
    }

    // Find and prepare for hoisting all the layers that intersect the query rect
    for (int i = 0; i < topLevelGPUData->numBlocks(); ++i) {
        const SkLayerInfo::BlockInfo& info = topLevelGPUData->block(i);
        if (info.fIsNested) {
            // Parent layers are currently hoisted while nested layers are not.
            continue;
        }

        SkRect layerRect;
        initialMat.mapRect(&layerRect, info.fBounds);
        if (!layerRect.intersect(query)) {
            continue;
        }

        const SkIRect dstIR = layerRect.roundOut();

        SkIRect srcIR;
        if (!compute_source_rect(info, initialMat, dstIR, &srcIR)) {
            continue;
        }

        prepare_for_hoisting(layerCache, topLevelPicture, initialMat, info, srcIR, dstIR,
                             needRendering, recycled, false, numSamples);
    }
}

void GrLayerHoister::DrawLayersToAtlas(GrContext* context,
                                       const SkTDArray<GrHoistedLayer>& atlased) {
    if (atlased.count() > 0) {
        // All the atlased layers are rendered into the same GrTexture
        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                        atlased[0].fLayer->texture()->asRenderTarget(), &props));

        SkCanvas* atlasCanvas = surface->getCanvas();

        for (int i = 0; i < atlased.count(); ++i) {
            const GrCachedLayer* layer = atlased[i].fLayer;
            const SkBigPicture* pict = atlased[i].fPicture->asSkBigPicture();
            if (!pict) {
                // TODO: can we assume / assert this?
                continue;
            }
            const SkIPoint offset = SkIPoint::Make(layer->srcIR().fLeft, layer->srcIR().fTop);
            SkDEBUGCODE(const SkPaint* layerPaint = layer->paint();)

            SkASSERT(!layerPaint || !layerPaint->getImageFilter());
            SkASSERT(!layer->filter());

            atlasCanvas->save();

            // Add a rect clip to make sure the rendering doesn't
            // extend beyond the boundaries of the atlased sub-rect
            const SkRect bound = SkRect::Make(layer->rect());
            atlasCanvas->clipRect(bound);
            atlasCanvas->clear(0);

            // '-offset' maps the layer's top/left to the origin.
            // Since this layer is atlased, the top/left corner needs
            // to be offset to the correct location in the backing texture.
            SkMatrix initialCTM;
            initialCTM.setTranslate(SkIntToScalar(-offset.fX), SkIntToScalar(-offset.fY));
            initialCTM.preTranslate(bound.fLeft, bound.fTop);
            initialCTM.preConcat(atlased[i].fPreMat);

            atlasCanvas->setMatrix(initialCTM);
            atlasCanvas->concat(atlased[i].fLocalMat);

            pict->partialPlayback(atlasCanvas, layer->start() + 1, layer->stop(), initialCTM);
            atlasCanvas->restore();
        }

        atlasCanvas->flush();
    }
}

SkBitmap wrap_texture(GrTexture* texture) {
    SkASSERT(texture);

    SkBitmap result;
    result.setInfo(texture->surfacePriv().info(kPremul_SkAlphaType));
    result.setPixelRef(SkNEW_ARGS(SkGrPixelRef, (result.info(), texture)))->unref();
    return result;
}

void GrLayerHoister::FilterLayer(GrContext* context,
                                 SkGpuDevice* device,
                                 const GrHoistedLayer& info) {
    GrCachedLayer* layer = info.fLayer;

    SkASSERT(layer->filter());

    static const int kDefaultCacheSize = 32 * 1024 * 1024;

    SkBitmap filteredBitmap;
    SkIPoint offset = SkIPoint::Make(0, 0);

    const SkIPoint filterOffset = SkIPoint::Make(layer->srcIR().fLeft, layer->srcIR().fTop);

    SkMatrix totMat = SkMatrix::I();
    totMat.preConcat(info.fPreMat);
    totMat.preConcat(info.fLocalMat);
    totMat.postTranslate(-SkIntToScalar(filterOffset.fX), -SkIntToScalar(filterOffset.fY));

    SkASSERT(0 == layer->rect().fLeft && 0 == layer->rect().fTop);
    SkIRect clipBounds = layer->rect();

    // This cache is transient, and is freed (along with all its contained
    // textures) when it goes out of scope.
    SkAutoTUnref<SkImageFilter::Cache> cache(SkImageFilter::Cache::Create(kDefaultCacheSize));
    SkImageFilter::Context filterContext(totMat, clipBounds, cache);

    SkImageFilter::Proxy proxy(device);
    const SkBitmap src = wrap_texture(layer->texture());

    if (!layer->filter()->filterImage(&proxy, src, filterContext, &filteredBitmap, &offset)) {
        // Filtering failed. Press on with the unfiltered version.
        return;
    }

    SkIRect newRect = SkIRect::MakeWH(filteredBitmap.width(), filteredBitmap.height());
    layer->setTexture(filteredBitmap.getTexture(), newRect);
    layer->setOffset(offset);
}

void GrLayerHoister::DrawLayers(GrContext* context, const SkTDArray<GrHoistedLayer>& layers) {
    for (int i = 0; i < layers.count(); ++i) {
        GrCachedLayer* layer = layers[i].fLayer;
        const SkBigPicture* pict = layers[i].fPicture->asSkBigPicture();
        if (!pict) {
            // TODO: can we assume / assert this?
            continue;
        }
        const SkIPoint offset = SkIPoint::Make(layer->srcIR().fLeft, layer->srcIR().fTop);

        // Each non-atlased layer has its own GrTexture
        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
        SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTargetDirect(
                                        layer->texture()->asRenderTarget(), &props));

        SkCanvas* layerCanvas = surface->getCanvas();

        SkASSERT(0 == layer->rect().fLeft && 0 == layer->rect().fTop);

        // Add a rect clip to make sure the rendering doesn't
        // extend beyond the boundaries of the layer
        const SkRect bound = SkRect::Make(layer->rect());
        layerCanvas->clipRect(bound);
        layerCanvas->clear(SK_ColorTRANSPARENT);

        SkMatrix initialCTM;
        initialCTM.setTranslate(SkIntToScalar(-offset.fX), SkIntToScalar(-offset.fY));
        initialCTM.preConcat(layers[i].fPreMat);

        layerCanvas->setMatrix(initialCTM);
        layerCanvas->concat(layers[i].fLocalMat);

        pict->partialPlayback(layerCanvas, layer->start()+1, layer->stop(), initialCTM);
        layerCanvas->flush();

        if (layer->filter()) {
            SkSurface_Gpu* gpuSurf = static_cast<SkSurface_Gpu*>(surface.get());

            FilterLayer(context, gpuSurf->getDevice(), layers[i]);
        }
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
