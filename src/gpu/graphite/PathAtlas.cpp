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
#include "src/gpu/graphite/TextureUtils.h"
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
constexpr uint16_t kComputeAtlasDim = 4096;

}  // namespace

PathAtlas::PathAtlas(uint32_t width, uint32_t height) : fWidth(width), fHeight(height) {}

PathAtlas::~PathAtlas() = default;

std::optional<PathAtlas::MaskAndOrigin> PathAtlas::addShape(
        Recorder* recorder,
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
    const TextureProxy* atlasProxy = this->onAddShape(recorder,
                                                      shape,
                                                      atlasTransform,
                                                      style,
                                                      maskInfo.fMaskSize,
                                                      &maskInfo.fTextureOrigin);
    if (!atlasProxy) {
        return std::nullopt;
    }

    return std::make_pair(CoverageMaskShape(shape, atlasProxy, localToDevice.inverse(), maskInfo),
                          SkIPoint{(int) maskBounds.left(), (int) maskBounds.top()});
}

///////////////////////////////////////////////////////////////////////////////////////

ComputePathAtlas::ComputePathAtlas()
    : PathAtlas(kComputeAtlasDim, kComputeAtlasDim)
    , fRectanizer(fWidth, fHeight) {}

bool ComputePathAtlas::initializeTextureIfNeeded(Recorder* recorder) {
    if (!fTexture) {
        SkColorType targetCT = ComputeShaderCoverageMaskTargetFormat(recorder->priv().caps());
        fTexture = recorder->priv().atlasProvider()->getAtlasTexture(recorder,
                                                                     this->width(),
                                                                     this->height(),
                                                                     targetCT,
                                                                     /*identifier=*/0,
                                                                     /*requireStorageUsage=*/true);
    }
    return fTexture != nullptr;
}

bool ComputePathAtlas::isSuitableForAtlasing(const Rect& transformedShapeBounds) const {
    Rect maskBounds = transformedShapeBounds.makeRoundOut();
    skvx::float2 maskSize = maskBounds.size();
    float width = maskSize.x(), height = maskSize.y();

    if (width > kComputeAtlasDim || height > kComputeAtlasDim) {
        return false;
    }

    // For now we're allowing paths that are smaller than 1/32nd of the full 4096x4096 atlas size
    // to prevent the atlas texture from filling up too often. There are several approaches we
    // should explore to alleviate the cost of atlasing large paths.
    //
    // 1. Rendering multiple atlas textures requires an extra compute pass for each texture. This
    // impairs performance because there is a fixed cost to each dispatch and all dispatches get
    // serialized by pipeline barrier synchronization. We should explore ways to render to multiple
    // textures by issuing more workgroups in fewer dispatches as well as removing pipeline barriers
    // across dispatches that target different atlas pages.
    //
    // 2. Implement a compressed "sparse" mask rendering scheme to render paths with a large
    // bounding box using less texture space.
    return (width * height) <= (1024 * 512);
}

const TextureProxy* ComputePathAtlas::addRect(Recorder* recorder,
                                              skvx::half2 maskSize,
                                              SkIPoint16* outPos) {
    if (!this->initializeTextureIfNeeded(recorder)) {
        SKGPU_LOG_E("Failed to instantiate an atlas texture");
        return nullptr;
    }

    // An empty mask always fits, so just return the texture.
    // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(maskSize)) {
        *outPos = {0, 0};
        return fTexture.get();
    }

    if (!fRectanizer.addPaddedRect(maskSize.x(), maskSize.y(), kEntryPadding, outPos)) {
        return nullptr;
    }

    return fTexture.get();
}

void ComputePathAtlas::reset() {
    fRectanizer.reset();

    this->onReset();
}

#ifdef SK_ENABLE_VELLO_SHADERS

/**
 * ComputePathAtlas that uses a VelloRenderer.
 */
class VelloComputePathAtlas final : public ComputePathAtlas {
public:
    // Record the compute dispatches that will draw the atlas contents.
    std::unique_ptr<DispatchGroup> recordDispatches(Recorder*) const override;

private:
    const TextureProxy* onAddShape(Recorder* recorder,
                                   const Shape&,
                                   const Transform& transform,
                                   const SkStrokeRec&,
                                   skvx::half2 maskSize,
                                   skvx::half2* outPos) override;
    void onReset() override {
        fScene.reset();
        fOccuppiedWidth = fOccuppiedHeight = 0;
    }

    // Contains the encoded scene buffer data that serves as the input to a vello compute pass.
    VelloScene fScene;

    // Occuppied bounds of the atlas
    uint32_t fOccuppiedWidth = 0;
    uint32_t fOccuppiedHeight = 0;
};

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

const TextureProxy* VelloComputePathAtlas::onAddShape(
        Recorder* recorder,
        const Shape& shape,
        const Transform& transform,
        const SkStrokeRec& style,
        skvx::half2 maskSize,
        skvx::half2* outPos) {
    SkIPoint16 iPos;
    const TextureProxy* texProxy = this->addRect(recorder, maskSize, &iPos);
    if (!texProxy) {
        return nullptr;
    }
    *outPos = skvx::half2(iPos.x(), iPos.y());
    // If the mask is empty, just return.
    // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(maskSize)) {
        return texProxy;
    }

    // TODO: The compute renderer doesn't support perspective yet. We assume that the path has been
    // appropriately transformed in that case.
    SkASSERT(transform.type() != Transform::Type::kProjection);

    // Restrict the render to the occupied area of the atlas, including entry padding so that the
    // padded row/column is cleared when Vello renders.
    Rect atlasBounds = Rect::XYWH(skvx::float2(iPos.x(), iPos.y()), skvx::cast<float>(maskSize));
    fOccuppiedWidth = std::max(fOccuppiedWidth, (uint32_t)atlasBounds.right() + kEntryPadding);
    fOccuppiedHeight = std::max(fOccuppiedHeight, (uint32_t)atlasBounds.bot() + kEntryPadding);

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

    // Clip the mask to the bounds of the atlas slot, which are already inset by 1px relative to
    // the bounds that the Rectanizer assigned.
    SkPath clipRect = SkPath::Rect(atlasBounds.asSkRect());
    fScene.pushClipLayer(clipRect, Transform::Identity());

    // The atlas transform of the shape is the linear-components (scale, rotation, skew) of
    // `localToDevice` translated by the top-left offset of `atlasBounds`.
    Transform atlasTransform = transform.postTranslate(atlasBounds.x(), atlasBounds.y());
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

    return texProxy;
}

#endif  // SK_ENABLE_VELLO_SHADERS

std::unique_ptr<ComputePathAtlas> ComputePathAtlas::CreateDefault() {
#ifdef SK_ENABLE_VELLO_SHADERS
    return std::make_unique<VelloComputePathAtlas>();
#else
    return nullptr;
#endif
}

}  // namespace skgpu::graphite
