/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SmallPathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

SmallPathAtlas::SmallPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim) {
    static constexpr size_t kPlotWidth = 512;
    static constexpr size_t kPlotHeight = 256;
    static constexpr SkColorType colorType = kAlpha_8_SkColorType;

    fDrawAtlas = DrawAtlas::Make(colorType,
                                 SkColorTypeBytesPerPixel(colorType),
                                 this->width(), this->height(),
                                 kPlotWidth, kPlotHeight,
                                 this,
                                 DrawAtlas::AllowMultitexturing::kYes,
                                 this,
                                 /*label=*/"SmallPathAtlas");
    fKeyLists.resize(fDrawAtlas->numPlots() * fDrawAtlas->maxPages());
}

bool SmallPathAtlas::recordUploads(DrawContext* dc) {
    return fDrawAtlas->recordUploads(dc, fRecorder);
}

namespace {
uint32_t shape_key_list_index(const PlotLocator& locator, const DrawAtlas* drawAtlas) {
    return locator.pageIndex() * drawAtlas->numPlots() + locator.plotIndex();
}
}  // namespace

const TextureProxy* SmallPathAtlas::onAddShape(const Shape& shape,
                                               const Transform& transform,
                                               const SkStrokeRec& strokeRec,
                                               skvx::half2 maskSize,
                                               skvx::half2* outPos) {
    // Shapes must have a key to be stored in the SmallPathAtlas
    SkASSERT(shape.hasKey());
    skgpu::UniqueKey maskKey = GeneratePathMaskKey(shape, transform, strokeRec, maskSize);
    AtlasLocator* cachedLocator = fShapeCache.find(maskKey);
    if (cachedLocator) {
        SkIPoint topLeft = cachedLocator->topLeft();
        *outPos = skvx::half2(topLeft.x() + kEntryPadding, topLeft.y() + kEntryPadding);
        fDrawAtlas->setLastUseToken(*cachedLocator,
                                    fRecorder->priv().tokenTracker()->nextFlushToken());
        return fDrawAtlas->getProxies()[cachedLocator->pageIndex()].get();
    }

    // Render mask.
    // TODO: Render directly into the atlas backing store, rather than doing a copy.
    //       This will require some refactoring of DrawAtlas.
    SkIRect iShapeBounds = SkIRect::MakeXYWH(0, 0, maskSize.x(), maskSize.y());
    // Outset to take padding into account
    SkIRect iAtlasBounds = iShapeBounds.makeOutset(kEntryPadding, kEntryPadding);
    SkAutoPixmapStorage dst;
    // Rasterize path to backing pixmap
    RasterMaskHelper helper(&dst);
    if (!helper.init(iAtlasBounds.size())) {
        return nullptr;
    }
    // Offset to padded location
    iShapeBounds.offset(kEntryPadding, kEntryPadding);
    helper.drawShape(shape, transform, strokeRec, iShapeBounds);
    sk_sp<SkData> pixelData = dst.detachPixelsAsData();

    // Add to DrawAtlas.
    AtlasLocator locator;
    DrawAtlas::ErrorCode errorCode = fDrawAtlas->addToAtlas(fRecorder,
                                                            iAtlasBounds.width(),
                                                            iAtlasBounds.height(),
                                                            pixelData->data(),
                                                            &locator);
    if (errorCode != DrawAtlas::ErrorCode::kSucceeded) {
        return nullptr;
    }
    SkIPoint topLeft = locator.topLeft();
    *outPos = skvx::half2(topLeft.x()+kEntryPadding, topLeft.y()+kEntryPadding);

    // Add locator to ShapeCache.
    fShapeCache.set(maskKey, locator);
    // Add key to Plot's ShapeKeyList.
    uint32_t index = shape_key_list_index(locator.plotLocator(), fDrawAtlas.get());
    ShapeKeyEntry* keyEntry = new ShapeKeyEntry();
    keyEntry->fKey = maskKey;
    fKeyLists[index].addToTail(keyEntry);

    return fDrawAtlas->getProxies()[locator.pageIndex()].get();
}

void SmallPathAtlas::evict(PlotLocator plotLocator) {
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

}  // namespace skgpu::graphite
