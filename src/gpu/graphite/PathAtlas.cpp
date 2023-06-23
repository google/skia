/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/DispatchGroup.h"
#endif

namespace skgpu::graphite {

PathAtlas::PathAtlas(uint32_t width, uint32_t height) : fRectanizer(width, height) {}

PathAtlas::~PathAtlas() = default;

bool PathAtlas::addShape(Recorder* recorder,
                         const Rect& maskBounds,
                         const Shape& shape,
                         const Transform& localToDevice,
                         const SkStrokeRec& style,
                         Rect* out) {
    SkASSERT(out);
    SkASSERT(!maskBounds.isEmptyNegativeOrNaN());

    if (!fTexture) {
        // TODO(chromium:1856): Dawn does not support the "storage binding" usage for the R8Unorm
        // texture format. This means that we will have to use RGBA8 until Dawn provides an optional
        // feature.
        fTexture = TextureProxy::MakeStorage(
                recorder->priv().caps(),
                SkISize::Make(int32_t(this->width()), int32_t(this->height())),
                kAlpha_8_SkColorType,
                skgpu::Budgeted::kYes);
        if (!fTexture) {
            return false;
        }
    }
    // Add a 2 pixel-wide border around the shape bounds when allocating the atlas slot. The outer
    // border acts as a buffer between atlas entries and the pixels contain 0. The inner border is
    // included in the mask and provides additional coverage pixels for analytic AA.
    // TODO(b/273924867) Should the inner outset get applied in drawGeometry/applyClipToDraw  and
    // included implicitly?
    Rect bounds = maskBounds.makeOutset(2);
    skvx::float2 size = bounds.size();
    SkIPoint16 pos;
    if (!fRectanizer.addRect(size.x(), size.y(), &pos)) {
        return false;
    }

    *out = Rect::XYWH(skvx::float2(pos.x(), pos.y()), size);
    this->onAddShape(shape, localToDevice, *out, maskBounds.x(), maskBounds.y(), style);

    return true;
}

void PathAtlas::reset() {
    fRectanizer.reset();
    this->onReset();
}

#ifdef SK_ENABLE_VELLO_SHADERS
namespace {

// TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?
// TODO: This is the maximum target dimension that vello can handle today
constexpr uint32_t kComputeAtlasDim = 4096;

}  // namespace

ComputePathAtlas::ComputePathAtlas() : PathAtlas(kComputeAtlasDim, kComputeAtlasDim) {}

std::unique_ptr<DispatchGroup> ComputePathAtlas::recordDispatches(Recorder* recorder) const {
    if (!this->texture()) {
        return nullptr;
    }

    SkASSERT(recorder);
    return recorder->priv().rendererProvider()->velloRenderer()->renderScene(
            {fOccuppiedWidth, fOccuppiedHeight, SkColors::kBlack},
            fScene,
            sk_ref_sp(this->texture()),
            recorder);
}

void ComputePathAtlas::onAddShape(const Shape& shape,
                                  const Transform& localToDevice,
                                  const Rect& atlasBounds,
                                  float deviceOffsetX,
                                  float deviceOffsetY,
                                  const SkStrokeRec& style) {
    // TODO: The compute renderer doesn't support perspective yet. We assume that the path has been
    // appropriately transformed in that case.
    SkASSERT(localToDevice.type() != Transform::Type::kProjection);

    // Restrict the render to the occupied area of the atlas.
    fOccuppiedWidth = std::max(fOccuppiedWidth, (uint32_t)atlasBounds.right());
    fOccuppiedHeight = std::max(fOccuppiedHeight, (uint32_t)atlasBounds.bot());

    // TODO(b/283876964): Apply clips here. Initially we'll need to encode the clip stack repeatedly
    // for each shape since the full vello renderer treats clips and their affected draws as a
    // single shape hierarchy in the same scene coordinate space. For coverage masks we want each
    // mask to be transformed to its atlas allocation coordinates and for the clip to be applied
    // with a translation relative to the atlas slot.
    //
    // Repeatedly encoding the clip stack should be relatively cheap (depending on how deep the
    // clips get) however it is wasteful both in terms of time and memory. If this proves to hurt
    // performance, future work will explore building an atlas-oriented element processing stage
    // that applies the atlas-relative translation while evaluating the stack monoid on the GPU.

    // Clip the mask to the bounds of the atlas slot. When the rectangle gets turned into a path,
    // its bottom and right edges are included in the clip, however semantically those pixels are
    // outside the atlas region (the implementation of Rect::size() implies that the bottom-right
    // bounds are exclusive). For the clip shape we inset the bottom and right edges by one pixel to
    // avoid filling into neighboring regions.
    Rect clipBounds(atlasBounds.topLeft() + 1, atlasBounds.botRight() - 1);
    SkPath clipRect = SkPath::Rect(clipBounds.asSkRect());
    fScene.pushClipLayer(clipRect, Transform::Identity());

    // The atlas transform of the shape is the linear-components (scale, rotation, skew) of
    // `localToDevice` translated by the top-left offset of `atlasBounds`, accounting for the 2
    // pixel-wide border we added earlier, so that the shape is correctly centered.
    SkM44 atlasMatrix = localToDevice.matrix();
    atlasMatrix.postTranslate(atlasBounds.x() + 2 - deviceOffsetX,
                              atlasBounds.y() + 2 - deviceOffsetY);
    Transform atlasTransform(atlasMatrix);
    SkPath devicePath = shape.asPath();

    // For stroke-and-fill, draw two masks into the same atlas slot: one for the stroke and one for
    // the fill.
    SkStrokeRec::Style styleType = style.getStyle();
    if (styleType == SkStrokeRec::kStroke_Style ||
        styleType == SkStrokeRec::kHairline_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        fScene.solidStroke(devicePath, SkColors::kRed, style.getWidth(), atlasTransform);
    }
    if (styleType == SkStrokeRec::kFill_Style || styleType == SkStrokeRec::kStrokeAndFill_Style) {
        fScene.solidFill(devicePath, SkColors::kRed, shape.fillType(), atlasTransform);
    }

    fScene.popClipLayer();
}

#endif  // SK_ENABLE_VELLO_SHADERS

}  // namespace skgpu::graphite
