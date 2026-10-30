/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkAttributes.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/draw/DrawParams.h"
#include "src/gpu/graphite/geom/Transform.h"

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

std::pair<const Renderer*, std::optional<CoverageMaskShape>> PathAtlas::addShape(
        const Clip& clip,
        const Shape& shape,
        const Transform& localToDevice,
        const SkStrokeRec& style) {
    SkASSERT(!clip.isClippedOut());

    // Start with the transformed shape bounds clipped to the scissor (matches drawBounds for
    // regular fills, but is a tighter bound for inverse fills).
    Rect maskBounds = clip.transformedShapeBounds().makeIntersect(clip.scissor());

    // Inverse fills that don't intersect the scissor are drawn as flood fills, so there is always a
    // non-empty region of the shape to rasterize here.
    SkASSERT(!maskBounds.isEmptyNegativeOrNaN());
    if (maskBounds.isEmptyNegativeOrNaN()) {
        return std::make_pair(nullptr, std::nullopt);
    }

    // If the clip isn't excluding very much of the original shape, store the entire path in the
    // atlas to avoid redundant entries with slightly different clips.
    if (maskBounds.area() >= 0.8f * clip.transformedShapeBounds().area()) {
        maskBounds = clip.transformedShapeBounds();
    }

    // Round out the shape bounds to preserve any fractional offset so that it is present in the
    // translation that we use when deriving the atlas-space transform later. Since `maskBounds`
    // is roughly bounded by the output device size, the float -> int conversion is safe.
    SkIRect iMaskBounds = maskBounds.makeRoundOut().asSkIRect();

    CoverageMaskShape::MaskInfo maskInfo;
    // This size does *not* include any padding that the atlas may place around the mask. This size
    // represents the area the shape can actually modify.
    maskInfo.fMaskSize = iMaskBounds.size();
    // We use the origin of the clipped mask bounds relative to the full mask to distinguish
    // between clips of the same size.
    SkIRect iShapeBounds = clip.transformedShapeBounds().makeRoundOut().asSkIRect();
    SkIVector clippedMaskOrigin = iMaskBounds.topLeft() - iShapeBounds.topLeft();
    sk_sp<TextureProxy> atlasProxy = this->onAddShape(shape,
                                                      localToDevice,
                                                      style,
                                                      iMaskBounds,
                                                      clippedMaskOrigin,
                                                      &maskInfo.fTextureOrigin);
    if (!atlasProxy) {
        return std::make_pair(nullptr, std::nullopt);
    }
    return std::make_pair(fRecorder->priv().rendererProvider()->coverageMask(),
                          CoverageMaskShape(shape,
                                            std::move(atlasProxy),
                                            SkM44::Translate(iMaskBounds.left(), iMaskBounds.top()),
                                            maskInfo));
}

/////////////////////////////////////////////////////////////////////////////////////////

PathAtlas::DrawAtlasMgr::DrawAtlasMgr(size_t width, size_t height,
                                      size_t plotWidth, size_t plotHeight,
                                      DrawAtlas::UseStorageTextures useStorageTextures,
                                      std::string_view label,
                                      const Caps* caps) {
    static constexpr auto kMaskFormat = MaskFormat::kA8;

    fDrawAtlas = DrawAtlas::Make(kMaskFormat,
                                 width, height,
                                 plotWidth, plotHeight,
                                 /*generationCounter=*/this,
                                 caps->allowMultipleAtlasTextures()
                                         ? DrawAtlas::AllowMultitexturing::kYes
                                         : DrawAtlas::AllowMultitexturing::kNo,
                                 useStorageTextures,
                                 /*evictor=*/this,
                                 label);
    SkASSERT(fDrawAtlas);
    fKeyLists.resize(fDrawAtlas->numPlots() * fDrawAtlas->maxPages());
    for (int i = 0; i < fKeyLists.size(); ++i) {
        fKeyLists[i].reset();
    }
}

sk_sp<TextureProxy> PathAtlas::DrawAtlasMgr::findOrCreateEntry(Recorder* recorder,
                                                               const Shape& shape,
                                                               const Transform& localToDevice,
                                                               const SkStrokeRec& strokeRec,
                                                               SkIRect maskBounds,
                                                               SkIPoint cacheMaskOrigin,
                                                               SkIPoint* outPos) {
    // TODO: pull this out so we don't have to recalculate it for each atlas?
    skgpu::UniqueKey maskKey = GeneratePathMaskKey(shape, localToDevice, strokeRec,
                                                   cacheMaskOrigin, maskBounds.size());
    DrawAtlas::AtlasLocator* cachedLocator = fShapeCache.find(maskKey);
    if (cachedLocator) {
        SkIPoint topLeft = cachedLocator->topLeft();
        *outPos = {topLeft.x() + kEntryPadding, topLeft.y() + kEntryPadding};
        fDrawAtlas->setLastUseToken(*cachedLocator,
                                    recorder->priv().tokenTracker()->nextFlushToken());
        return SK_UNSAFE_TODO(fDrawAtlas->getProxies()[cachedLocator->pageIndex()]);
    }

    DrawAtlas::AtlasLocator locator;
    sk_sp<TextureProxy> proxy = this->addToAtlas(recorder, shape, localToDevice, strokeRec,
                                                 maskBounds, outPos, &locator);
    if (!proxy) {
        return nullptr;
    }

    // Add locator to ShapeCache.
    fShapeCache.set(maskKey, locator);
    // Add key to Plot's ShapeKeyList.
    uint32_t index = fDrawAtlas->getListIndex(locator.plotLocator());
    ShapeKeyEntry* keyEntry = new ShapeKeyEntry();
    keyEntry->fKey = maskKey;
    fKeyLists[index].addToTail(keyEntry);

    return proxy;
}

sk_sp<TextureProxy> PathAtlas::DrawAtlasMgr::addToAtlas(Recorder* recorder,
                                                        const Shape& shape,
                                                        const Transform& localToDevice,
                                                        const SkStrokeRec& strokeRec,
                                                        SkIRect maskBounds,
                                                        SkIPoint* outPos,
                                                        DrawAtlas::AtlasLocator* locator) {
    // Should have been validated by `PathAtlas::addShape()` before getting this far.
    SkASSERT(!maskBounds.isEmpty());

    // Render mask with a logical (0,0) origin
    SkIRect iShapeBounds = SkIRect::MakeSize(maskBounds.size());
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
    *outPos = topLeft + SkIPoint{kEntryPadding, kEntryPadding};
    if (!this->onAddToAtlas(shape, localToDevice, strokeRec, maskBounds.topLeft(), *locator)) {
        return nullptr;
    }

    fDrawAtlas->setLastUseToken(*locator,
                                recorder->priv().tokenTracker()->nextFlushToken());

    return SK_UNSAFE_TODO(fDrawAtlas->getProxies()[locator->pageIndex()]);
}

bool PathAtlas::DrawAtlasMgr::recordUploads(DrawContext* dc, Recorder* recorder) {
    return fDrawAtlas->recordUploads(dc, recorder);
}

void PathAtlas::DrawAtlasMgr::evict(DrawAtlas::PlotLocator plotLocator) {
    // Remove all entries for this Plot from the ShapeCache
    uint32_t index = fDrawAtlas->getListIndex(plotLocator);
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

void PathAtlas::DrawAtlasMgr::evictAll() {
    fDrawAtlas->evictAllPlots();
    SkASSERT(fShapeCache.empty());
}

void PathAtlas::DrawAtlasMgr::compact(Recorder* recorder) {
    fDrawAtlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
}

void PathAtlas::DrawAtlasMgr::freeGpuResources(Recorder* recorder) {
    fDrawAtlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
}

}  // namespace skgpu::graphite
