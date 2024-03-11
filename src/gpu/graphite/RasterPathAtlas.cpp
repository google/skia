/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathAtlas.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

static constexpr uint32_t kDefaultAtlasDim = 4096;

static constexpr uint32_t kSmallPathPlotWidth = 512;
static constexpr uint32_t kSmallPathPlotHeight = 256;

RasterPathAtlas::RasterPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim)
        , fCachedAtlasMgr(fWidth, fHeight, fWidth, fHeight, recorder->priv().caps())
        , fSmallPathAtlasMgr(std::max(fWidth/2, kSmallPathPlotWidth),
                             std::max(fHeight/2, kSmallPathPlotHeight),
                             kSmallPathPlotWidth, kSmallPathPlotHeight,
                             recorder->priv().caps())
        , fUncachedAtlasMgr(fWidth, fHeight, fWidth, fHeight, recorder->priv().caps()) {
    SkASSERT(recorder);
}

void RasterPathAtlas::recordUploads(DrawContext* dc) {
    fCachedAtlasMgr.recordUploads(dc, fRecorder);
    fSmallPathAtlasMgr.recordUploads(dc, fRecorder);
    fUncachedAtlasMgr.recordUploads(dc, fRecorder);
}

const TextureProxy* RasterPathAtlas::onAddShape(const Shape& shape,
                                                const Transform& transform,
                                                const SkStrokeRec& strokeRec,
                                                skvx::half2 maskSize,
                                                skvx::half2* outPos) {
    skgpu::UniqueKey maskKey;
    bool hasKey = shape.hasKey();
    if (hasKey) {
        constexpr int kMaxSmallPathSize = 162;
        // Try to locate or add to cached DrawAtlas
        const TextureProxy* proxy = nullptr;
        if (maskSize.x() <= kMaxSmallPathSize && maskSize.y() <= kMaxSmallPathSize) {
            proxy = fSmallPathAtlasMgr.findOrCreateEntry(fRecorder,
                                                         shape,
                                                         transform,
                                                         strokeRec,
                                                         maskSize,
                                                         outPos);
        }
        if (!proxy) {
            proxy = fCachedAtlasMgr.findOrCreateEntry(fRecorder,
                                                      shape,
                                                      transform,
                                                      strokeRec,
                                                      maskSize,
                                                      outPos);
        }
        if (proxy) {
            return proxy;
        }
    }

    // try to add to uncached DrawAtlas
    AtlasLocator loc;
    return fUncachedAtlasMgr.addToAtlas(fRecorder,
                                        shape,
                                        transform,
                                        strokeRec,
                                        maskSize,
                                        outPos,
                                        &loc);
}

/////////////////////////////////////////////////////////////////////////////////////////

RasterPathAtlas::DrawAtlasMgr::DrawAtlasMgr(size_t width, size_t height,
                                            size_t plotWidth, size_t plotHeight,
                                            const Caps* caps) {
    static constexpr SkColorType colorType = kAlpha_8_SkColorType;


    fDrawAtlas = DrawAtlas::Make(colorType,
                                 SkColorTypeBytesPerPixel(colorType),
                                 width, height,
                                 plotWidth, plotHeight,
                                 this,
                                 caps->allowMultipleAtlasTextures() ?
                                         DrawAtlas::AllowMultitexturing::kYes :
                                         DrawAtlas::AllowMultitexturing::kNo,
                                 this,
                                 /*label=*/"RasterPathAtlas");
    SkASSERT(fDrawAtlas);
    fKeyLists.resize(fDrawAtlas->numPlots() * fDrawAtlas->maxPages());
    for (int i = 0; i < fKeyLists.size(); ++i) {
        fKeyLists[i].reset();
    }
}

namespace {
uint32_t shape_key_list_index(const PlotLocator& locator, const DrawAtlas* drawAtlas) {
    return locator.pageIndex() * drawAtlas->numPlots() + locator.plotIndex();
}
}  // namespace

const TextureProxy* RasterPathAtlas::DrawAtlasMgr::findOrCreateEntry(Recorder* recorder,
                                                                    const Shape& shape,
                                                                    const Transform& transform,
                                                                    const SkStrokeRec& strokeRec,
                                                                    skvx::half2 maskSize,
                                                                    skvx::half2* outPos) {
    // Shapes must have a key to use this method
    skgpu::UniqueKey maskKey = GeneratePathMaskKey(shape, transform, strokeRec, maskSize);
    AtlasLocator* cachedLocator = fShapeCache.find(maskKey);
    if (cachedLocator) {
        SkIPoint topLeft = cachedLocator->topLeft();
        *outPos = skvx::half2(topLeft.x() + kEntryPadding, topLeft.y() + kEntryPadding);
        fDrawAtlas->setLastUseToken(*cachedLocator,
                                    recorder->priv().tokenTracker()->nextFlushToken());
        return fDrawAtlas->getProxies()[cachedLocator->pageIndex()].get();
    }

    AtlasLocator locator;
    const TextureProxy* proxy = this->addToAtlas(recorder, shape, transform, strokeRec,
                                                 maskSize, outPos, &locator);
    if (!proxy) {
        return nullptr;
    }

    // Add locator to ShapeCache.
    fShapeCache.set(maskKey, locator);
    // Add key to Plot's ShapeKeyList.
    uint32_t index = shape_key_list_index(locator.plotLocator(), fDrawAtlas.get());
    ShapeKeyEntry* keyEntry = new ShapeKeyEntry();
    keyEntry->fKey = maskKey;
    fKeyLists[index].addToTail(keyEntry);

    return proxy;
}

const TextureProxy* RasterPathAtlas::DrawAtlasMgr::addToAtlas(Recorder* recorder,
                                                              const Shape& shape,
                                                              const Transform& transform,
                                                              const SkStrokeRec& strokeRec,
                                                              skvx::half2 maskSize,
                                                              skvx::half2* outPos,
                                                              AtlasLocator* locator) {
    // Render mask.
    SkIRect iShapeBounds = SkIRect::MakeXYWH(0, 0, maskSize.x(), maskSize.y());
    // Outset to take padding into account
    SkIRect iAtlasBounds = iShapeBounds.makeOutset(kEntryPadding, kEntryPadding);

    // Request space in DrawAtlas.
    DrawAtlas::ErrorCode errorCode = fDrawAtlas->addRect(recorder,
                                                         iAtlasBounds.width(),
                                                         iAtlasBounds.height(),
                                                         locator);
    if (errorCode != DrawAtlas::ErrorCode::kSucceeded) {
        return nullptr;
    }
    SkIPoint topLeft = locator->topLeft();
    *outPos = skvx::half2(topLeft.x()+kEntryPadding, topLeft.y()+kEntryPadding);

    // If the mask is empty, just return.
    // TODO: this may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(maskSize)) {
        fDrawAtlas->setLastUseToken(*locator,
                                    recorder->priv().tokenTracker()->nextFlushToken());
        return fDrawAtlas->getProxies()[locator->pageIndex()].get();
    }

    // Rasterize path to backing pixmap.
    // This pixmap will be the size of the Plot that contains the given rect, not the entire atlas,
    // and hence the position we render at will be relative to that Plot.
    // The value of outPos is relative to the entire texture, to be used for texture coords.
    SkAutoPixmapStorage dst;
    SkIPoint renderPos = fDrawAtlas->prepForRender(*locator, &dst);

    RasterMaskHelper helper(&dst);
    if (!helper.init(fDrawAtlas->plotSize())) {
        return nullptr;
    }
    // Offset to plot location and draw
    iShapeBounds.offset(renderPos.x()+kEntryPadding, renderPos.y()+kEntryPadding);
    helper.drawShape(shape, transform, strokeRec, iShapeBounds);

    fDrawAtlas->setLastUseToken(*locator,
                                recorder->priv().tokenTracker()->nextFlushToken());

    return fDrawAtlas->getProxies()[locator->pageIndex()].get();
}

bool RasterPathAtlas::DrawAtlasMgr::recordUploads(DrawContext* dc, Recorder* recorder) {
    return fDrawAtlas->recordUploads(dc, recorder);
}

void RasterPathAtlas::DrawAtlasMgr::evict(PlotLocator plotLocator) {
    // Remove all entries for this Plot from the ShapeCache
    uint32_t index = shape_key_list_index(plotLocator, fDrawAtlas.get());
    ShapeKeyList::Iter iter;
    iter.init(fKeyLists[index], ShapeKeyList::Iter::kHead_IterStart);
    ShapeKeyEntry* currEntry;
    while ((currEntry = iter.get())) {
        iter.next();
        fShapeCache.remove(currEntry->fKey);
        fKeyLists[index].remove(currEntry);
        delete currEntry;
    }
}

void RasterPathAtlas::DrawAtlasMgr::postFlush(Recorder* recorder) {
    fDrawAtlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
}

}  // namespace skgpu::graphite
