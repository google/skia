/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

namespace skgpu::graphite {
namespace {

constexpr int kMinAtlasTextureSize = 512;  // the smallest we want the PathAtlas textures to be
                                           // unless the device requires smaller

}  // namespace

PathAtlas::PathAtlas(Recorder* recorder, uint32_t requestedWidth, uint32_t requestedHeight)
        : fRecorder(recorder) {
    const Caps* caps = recorder->priv().caps();
    int maxTextureSize = std::max(caps->maxPathAtlasTextureSize(), kMinAtlasTextureSize);
    maxTextureSize = std::min(maxTextureSize, caps->maxTextureSize());

    fWidth = SkPrevPow2(std::min<uint32_t>(requestedWidth, maxTextureSize));
    fHeight = SkPrevPow2(std::min<uint32_t>(requestedHeight, maxTextureSize));
}

PathAtlas::~PathAtlas() = default;

std::pair<const Renderer*, std::optional<PathAtlas::MaskAndOrigin>> PathAtlas::addShape(
        const Rect& transformedShapeBounds,
        const Shape& shape,
        const Transform& localToDevice,
        const SkStrokeRec& style) {
    // It is possible for the transformed shape bounds to be fully clipped out while the draw still
    // produces coverage due to an inverse fill. In this case, don't render any mask;
    // CoverageMaskShapeRenderStep will automatically handle the simple fill. We'll handle this
    // by adding an empty mask.
    // TODO: We could have addShape() handle this fully except we need a valid TextureProxy still.
    const bool emptyMask = transformedShapeBounds.isEmptyNegativeOrNaN();

    // Round out the shape bounds to preserve any fractional offset so that it is present in the
    // translation that we use when deriving the atlas-space transform later.
    Rect maskBounds = transformedShapeBounds.makeRoundOut();

    CoverageMaskShape::MaskInfo maskInfo;
    // This size does *not* include any padding that the atlas may place around the mask. This size
    // represents the area the shape can actually modify.
    maskInfo.fMaskSize = emptyMask ? skvx::half2(0) : skvx::cast<uint16_t>(maskBounds.size());
    Transform atlasTransform = localToDevice.postTranslate(-maskBounds.left(), -maskBounds.top());
    const TextureProxy* atlasProxy = this->onAddShape(shape,
                                                      atlasTransform,
                                                      style,
                                                      maskInfo.fMaskSize,
                                                      &maskInfo.fTextureOrigin);
    if (!atlasProxy) {
        return std::make_pair(nullptr, std::nullopt);
    }

    std::optional<PathAtlas::MaskAndOrigin> atlasMask =
            std::make_pair(CoverageMaskShape(shape, atlasProxy, localToDevice.inverse(), maskInfo),
                           SkIPoint{(int) maskBounds.left(), (int) maskBounds.top()});
    return std::make_pair(fRecorder->priv().rendererProvider()->coverageMask(), atlasMask);
}

/////////////////////////////////////////////////////////////////////////////////////////

PathAtlas::DrawAtlasMgr::DrawAtlasMgr(size_t width, size_t height,
                                      size_t plotWidth, size_t plotHeight,
                                      DrawAtlas::UseStorageTextures useStorageTextures,
                                      std::string_view label,
                                      const Caps* caps) {
    static constexpr SkColorType colorType = kAlpha_8_SkColorType;

    fDrawAtlas = DrawAtlas::Make(colorType,
                                 SkColorTypeBytesPerPixel(colorType),
                                 width, height,
                                 plotWidth, plotHeight,
                                 /*generationCounter=*/this,
                                 caps->allowMultipleAtlasTextures() ?
                                         DrawAtlas::AllowMultitexturing::kYes :
                                         DrawAtlas::AllowMultitexturing::kNo,
                                 useStorageTextures,
                                 /*evictor=*/this,
                                 label);
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

const TextureProxy* PathAtlas::DrawAtlasMgr::findOrCreateEntry(Recorder* recorder,
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

const TextureProxy* PathAtlas::DrawAtlasMgr::addToAtlas(Recorder* recorder,
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

    if (!this->onAddToAtlas(shape, transform, strokeRec, iShapeBounds, *locator)) {
        return nullptr;
    }

    fDrawAtlas->setLastUseToken(*locator,
                                recorder->priv().tokenTracker()->nextFlushToken());

    return fDrawAtlas->getProxies()[locator->pageIndex()].get();
}

bool PathAtlas::DrawAtlasMgr::recordUploads(DrawContext* dc, Recorder* recorder) {
    return fDrawAtlas->recordUploads(dc, recorder);
}

void PathAtlas::DrawAtlasMgr::evict(PlotLocator plotLocator) {
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

void PathAtlas::DrawAtlasMgr::postFlush(Recorder* recorder) {
    fDrawAtlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
}

}  // namespace skgpu::graphite
