/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/PathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
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
namespace {

// TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?
// TODO: This is the maximum target dimension that vello can handle today
constexpr uint32_t kComputeAtlasDim = 4096;

// TODO: for now
constexpr uint32_t kSoftwareAtlasDim = 4096;

}  // namespace

PathAtlas::PathAtlas(uint32_t width, uint32_t height) : fRectanizer(width, height) {}

PathAtlas::~PathAtlas() = default;

bool PathAtlas::addShape(Recorder* recorder,
                         const Rect& transformedShapeBounds,
                         const Shape& shape,
                         const Transform& localToDevice,
                         const SkStrokeRec& style,
                         AtlasShape::MaskInfo* out) {
    SkASSERT(out);
    SkASSERT(!transformedShapeBounds.isEmptyNegativeOrNaN());

    if (!fTexture) {
        fTexture = recorder->priv().atlasProvider()->getAtlasTexture(
                recorder, this->width(), this->height());
        if (!fTexture) {
            SKGPU_LOG_E("Failed to instantiate an atlas texture");
            return false;
        }
    }

    // Round out the shape bounds to preserve any fractional offset so that it is present in the
    // translation that we use when deriving the atlas-space transform later.
    Rect maskBounds = transformedShapeBounds.makeRoundOut();

    // Add an additional one pixel outset as buffer between atlas slots. This prevents sampling from
    // neighboring atlas slots; the AtlasShape renderer also uses the outset to sample zero coverage
    // on inverse fill pixels that fall outside the mask bounds.
    skvx::float2 maskSize = maskBounds.size();
    skvx::float2 atlasSize = maskSize + 2;
    SkIPoint16 pos;
    if (!fRectanizer.addRect(atlasSize.x(), atlasSize.y(), &pos)) {
        return false;
    }

    out->fDeviceOrigin = skvx::int2((int)maskBounds.x(), (int)maskBounds.y());
    out->fAtlasOrigin = skvx::half2(pos.x(), pos.y());
    out->fMaskSize = skvx::half2((uint16_t)maskSize.x(), (uint16_t)maskSize.y());

    this->onAddShape(shape,
                     localToDevice,
                     Rect::XYWH(skvx::float2(pos.x(), pos.y()), atlasSize),
                     out->fDeviceOrigin,
                     style);
    return true;
}

void PathAtlas::reset() {
    fRectanizer.reset();
    this->onReset();
}

ComputePathAtlas::ComputePathAtlas() : PathAtlas(kComputeAtlasDim, kComputeAtlasDim) {}

#ifdef SK_ENABLE_VELLO_SHADERS

std::unique_ptr<DispatchGroup> VelloComputePathAtlas::recordDispatches(Recorder* recorder) const {
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

void VelloComputePathAtlas::onAddShape(const Shape& shape,
                                       const Transform& localToDevice,
                                       const Rect& atlasBounds,
                                       skvx::int2 deviceOffset,
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
    // `localToDevice` translated by the top-left offset of `atlasBounds`, accounting for the 1
    // pixel-wide border we added earlier, so that the shape is correctly centered.
    SkM44 atlasMatrix = localToDevice.matrix();
    atlasMatrix.postTranslate(atlasBounds.x() + 1 - deviceOffset.x(),
                              atlasBounds.y() + 1 - deviceOffset.y());

    Transform atlasTransform(atlasMatrix);
    SkPath devicePath = shape.asPath();

    // For stroke-and-fill, draw two masks into the same atlas slot: one for the stroke and one for
    // the fill.
    SkStrokeRec::Style styleType = style.getStyle();
    if (styleType == SkStrokeRec::kStroke_Style ||
        styleType == SkStrokeRec::kHairline_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        // We need to special-case hairline strokes and strokes with sub-pixel width as Vello
        // draws these with aliasing and the results are barely visible. Draw the stroke with a
        // device-space width of 1 pixel and scale down the alpha by the true width to approximate
        // the sampled area.
        float width = style.getWidth();
        float deviceWidth = width * atlasTransform.maxScaleFactor();
        if (style.isHairlineStyle() || deviceWidth <= 1.0) {
            // Both strokes get 1/2 weight scaled by the theoretical area (1 for hairlines,
            // `deviceWidth` otherwise).
            SkColor4f color = SkColors::kRed;
            color.fR *= style.isHairlineStyle() ? 1.0 : deviceWidth;

            // Transform the stroke's width to its local coordinate space since it'll get drawn with
            // `atlasTransform`.
            float transformedWidth = 1.0f / atlasTransform.maxScaleFactor();
            SkStrokeRec adjustedStyle(style);
            adjustedStyle.setStrokeStyle(transformedWidth);
            fScene.solidStroke(devicePath, color, adjustedStyle, atlasTransform);
        } else {
            fScene.solidStroke(devicePath, SkColors::kRed, style, atlasTransform);
        }
    }
    if (styleType == SkStrokeRec::kFill_Style || styleType == SkStrokeRec::kStrokeAndFill_Style) {
        fScene.solidFill(devicePath, SkColors::kRed, shape.fillType(), atlasTransform);
    }

    fScene.popClipLayer();
}

#endif  // SK_ENABLE_VELLO_SHADERS

SoftwarePathAtlas::SoftwarePathAtlas() : PathAtlas(kSoftwareAtlasDim, kSoftwareAtlasDim) {}

}  // namespace skgpu::graphite
