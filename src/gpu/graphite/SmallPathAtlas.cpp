/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SmallPathAtlas.h"

#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

SmallPathAtlas::SmallPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim) {
}

bool SmallPathAtlas::initAtlas() {
    if (fDrawAtlas) {
        return true;
    }

    static constexpr size_t kPlotWidth = 512;
    static constexpr size_t kPlotHeight = 256;
    static constexpr SkColorType colorType = kAlpha_8_SkColorType;

    // TODO: Set atlas size based on maxTextureSize()
    fDrawAtlas = DrawAtlas::Make(colorType,
                                 SkColorTypeBytesPerPixel(colorType),
                                 kDefaultAtlasDim, kDefaultAtlasDim,
                                 kPlotWidth, kPlotHeight,
                                 this,
                                 DrawAtlas::AllowMultitexturing::kYes,
                                 this,
                                 /*label=*/"SmallPathAtlas");
    fKeyLists.resize(fDrawAtlas->numPlots() * fDrawAtlas->maxPages());

    return SkToBool(fDrawAtlas);
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
        *outPos = skvx::half2(topLeft.x(), topLeft.y());
        return fDrawAtlas->getProxies()[cachedLocator->pageIndex()].get();
    }

    // Render mask.
    // TODO: Render directly into the atlas backing store, rather than doing a copy.
    //       This will require some refactoring of DrawAtlas.
    SkAutoPixmapStorage dst;
    // Rasterize path to backing pixmap
    RasterMaskHelper helper(&dst);
    if (!helper.init({maskSize.x(), maskSize.y()})) {
        return nullptr;
    }
    SkIRect iAtlasBounds = SkIRect::MakeXYWH(0, 0, maskSize.x(), maskSize.y());
    helper.drawShape(shape, transform, strokeRec, iAtlasBounds);
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
    *outPos = skvx::half2(topLeft.x(), topLeft.y());

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
