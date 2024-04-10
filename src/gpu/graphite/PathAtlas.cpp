/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
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

}  // namespace skgpu::graphite
