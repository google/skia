/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputePathAtlas.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

#ifdef SK_ENABLE_VELLO_SHADERS
#include "src/gpu/graphite/compute/DispatchGroup.h"
#endif

namespace skgpu::graphite {
namespace {

// TODO: This is the maximum target dimension that vello can handle today.
constexpr uint16_t kComputeAtlasDim = 4096;

// TODO: Currently we reject shapes that are smaller than a subset of a given atlas page to avoid
// creating too many flushes in a Recording containing many large path draws. These shapes often
// don't make efficient use of the available atlas texture space and the cost of sequential
// dispatches to render multiple atlas pages can be prohibitive.
constexpr size_t kBboxAreaThreshold = 1024 * 512;

// Coordinate size that is too large for vello to handle efficiently. See the discussion on
// https://github.com/linebender/vello/pull/542.
constexpr float kCoordinateThreshold = 1e10;

}  // namespace

ComputePathAtlas::ComputePathAtlas(Recorder* recorder)
    : PathAtlas(recorder, kComputeAtlasDim, kComputeAtlasDim)
    , fRectanizer(this->width(), this->height()) {}

bool ComputePathAtlas::initializeTextureIfNeeded() {
    if (!fTexture) {
        SkColorType targetCT = ComputeShaderCoverageMaskTargetFormat(fRecorder->priv().caps());
        fTexture = fRecorder->priv().atlasProvider()->getAtlasTexture(fRecorder,
                                                                      this->width(),
                                                                      this->height(),
                                                                      targetCT,
                                                                      /*identifier=*/0,
                                                                      /*requireStorageUsage=*/true);
    }
    return fTexture != nullptr;
}

bool ComputePathAtlas::isSuitableForAtlasing(const Rect& transformedShapeBounds,
                                             const Rect& clipBounds) const {
    Rect shapeBounds = transformedShapeBounds.makeRoundOut();
    Rect maskBounds = shapeBounds.makeIntersect(clipBounds);
    skvx::float2 maskSize = maskBounds.size();
    float width = maskSize.x(), height = maskSize.y();

    if (width > this->width() || height > this->height()) {
        return false;
    }

    // For now we're allowing paths that are smaller than 1/32nd of the full 4096x4096 atlas size
    // to prevent the atlas texture from filling up too often. There are several approaches we
    // should explore to alleviate the cost of atlasing large paths.
    if (width * height > kBboxAreaThreshold) {
        return false;
    }

    // Reject pathological shapes that vello can't handle efficiently yet.
    skvx::float2 unclippedSize = shapeBounds.size();
    if (std::fabs(unclippedSize.x()) > kCoordinateThreshold ||
        std::fabs(unclippedSize.y()) > kCoordinateThreshold) {
        return false;
    }

    return true;
}

const TextureProxy* ComputePathAtlas::addRect(skvx::half2 maskSize,
                                              SkIPoint16* outPos) {
    if (!this->initializeTextureIfNeeded()) {
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
    explicit VelloComputePathAtlas(Recorder* recorder)
        : ComputePathAtlas(recorder)
        , fCachedAtlasMgr(fWidth, fHeight, recorder->priv().caps()) {}
    // Record the compute dispatches that will draw the atlas contents.
    bool recordDispatches(Recorder*, ComputeTask::DispatchGroupList*) const override;

private:
    const TextureProxy* onAddShape(const Shape&,
                                   const Transform&,
                                   const SkStrokeRec&,
                                   skvx::half2 maskSize,
                                   skvx::half2* outPos) override;
    void onReset() override {
        fCachedAtlasMgr.onReset();

        fUncachedScene.reset();
        fUncachedOccupiedArea = { 0, 0 };
    }

    class VelloAtlasMgr : public PathAtlas::DrawAtlasMgr {
    public:
        VelloAtlasMgr(size_t width, size_t height, const Caps* caps)
            : PathAtlas::DrawAtlasMgr(width, height, width, height,
                                      DrawAtlas::UseStorageTextures::kYes,
                                      /*label=*/"VelloPathAtlas", caps) {}

        bool recordDispatches(Recorder* recorder, ComputeTask::DispatchGroupList* dispatches) const;

        void onReset() {
            fDrawAtlas->markUsedPlotsAsFull();
            for (int i = 0; i < PlotLocator::kMaxMultitexturePages; ++i) {
                fScenes[i].reset();
                fOccupiedAreas[i] = {0, 0};
            }
        }

    protected:
        bool onAddToAtlas(const Shape&,
                          const Transform& transform,
                          const SkStrokeRec&,
                          SkIRect shapeBounds,
                          const AtlasLocator&) override;

    private:
        VelloScene fScenes[PlotLocator::kMaxMultitexturePages];
        SkISize fOccupiedAreas[PlotLocator::kMaxMultitexturePages] = {
            {0, 0}, {0, 0}, {0, 0}, {0, 0}
        };
    };

    VelloAtlasMgr fCachedAtlasMgr;

    // Contains the encoded scene buffer data that serves as the input to a vello compute pass.
    // For the uncached atlas.
    VelloScene fUncachedScene;

    // Occupied bounds of the uncached atlas
    SkISize fUncachedOccupiedArea = { 0, 0 };
};

static VelloAaConfig get_vello_aa_config(Recorder* recorder) {
    // Use the analytic area AA mode unless caps say otherwise.
    VelloAaConfig config = VelloAaConfig::kAnalyticArea;
#if defined(GRAPHITE_TEST_UTILS)
    PathRendererStrategy strategy = recorder->priv().caps()->requestedPathRendererStrategy();
    if (strategy == PathRendererStrategy::kComputeMSAA16) {
        config = VelloAaConfig::kMSAA16;
    } else if (strategy == PathRendererStrategy::kComputeMSAA8) {
        config = VelloAaConfig::kMSAA8;
    }
#endif

    return config;
}

static std::unique_ptr<DispatchGroup> render_vello_scene(Recorder* recorder,
                                                         sk_sp<TextureProxy> texture,
                                                         const VelloScene& scene,
                                                         SkISize occupiedArea,
                                                         VelloAaConfig config) {
    return recorder->priv().rendererProvider()->velloRenderer()->renderScene(
                {(uint32_t)occupiedArea.width(),
                 (uint32_t)occupiedArea.height(),
                 SkColors::kBlack,
                 config},
                scene,
                std::move(texture),
                recorder);
}

static void add_shape_to_scene(const Shape& shape,
                               const Transform& transform,
                               const SkStrokeRec& style,
                               Rect atlasBounds,
                               VelloScene* scene,
                               SkISize* occupiedArea) {
    occupiedArea->fWidth = std::max(occupiedArea->fWidth,
                                    (int)atlasBounds.right() + PathAtlas::kEntryPadding);
    occupiedArea->fHeight = std::max(occupiedArea->fHeight,
                                     (int)atlasBounds.bot() + PathAtlas::kEntryPadding);

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
    scene->pushClipLayer(clipRect, Transform::Identity());

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
            scene->solidStroke(devicePath, color, adjustedStyle, atlasTransform);
        } else {
            scene->solidStroke(devicePath, SkColors::kRed, style, atlasTransform);
        }
    }
    if (styleType == SkStrokeRec::kFill_Style || styleType == SkStrokeRec::kStrokeAndFill_Style) {
        scene->solidFill(devicePath, SkColors::kRed, shape.fillType(), atlasTransform);
    }

    scene->popClipLayer();
}

bool VelloComputePathAtlas::recordDispatches(Recorder* recorder,
                                             ComputeTask::DispatchGroupList* dispatches) const {
    bool addedDispatches = fCachedAtlasMgr.recordDispatches(recorder, dispatches);

    if (this->texture() && !fUncachedOccupiedArea.isEmpty()) {
        SkASSERT(recorder && recorder == fRecorder);

        VelloAaConfig config = get_vello_aa_config(recorder);
        std::unique_ptr<DispatchGroup> dispatchGroup =
                render_vello_scene(recorder,
                                   sk_ref_sp(this->texture()),
                                   fUncachedScene,
                                   fUncachedOccupiedArea,
                                   config);
        if (dispatchGroup) {
            TRACE_EVENT_INSTANT1("skia.gpu", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD,
                                 "# dispatches", dispatchGroup->dispatches().size());
            dispatches->emplace_back(std::move(dispatchGroup));
            return true;
        } else {
            SKGPU_LOG_E("VelloComputePathAtlas:: Failed to create dispatch group.");
        }
    }

    return addedDispatches;
}

const TextureProxy* VelloComputePathAtlas::onAddShape(
        const Shape& shape,
        const Transform& transform,
        const SkStrokeRec& style,
        skvx::half2 maskSize,
        skvx::half2* outPos) {

    skgpu::UniqueKey maskKey;
    bool hasKey = shape.hasKey();
    if (hasKey) {
        // Try to locate or add to cached DrawAtlas
        const TextureProxy* proxy = fCachedAtlasMgr.findOrCreateEntry(fRecorder,
                                                                      shape,
                                                                      transform,
                                                                      style,
                                                                      maskSize,
                                                                      outPos);
        if (proxy) {
            return proxy;
        }
    }

    // Try to add to uncached texture
    SkIPoint16 iPos;
    const TextureProxy* texProxy = this->addRect(maskSize, &iPos);
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
    SkASSERT(transform.type() != Transform::Type::kPerspective);

    // Restrict the render to the occupied area of the atlas, including entry padding so that the
    // padded row/column is cleared when Vello renders.
    Rect atlasBounds = Rect::XYWH(skvx::float2(iPos.x(), iPos.y()), skvx::cast<float>(maskSize));

    add_shape_to_scene(shape, transform, style, atlasBounds,
                       &fUncachedScene, &fUncachedOccupiedArea);

    return texProxy;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool VelloComputePathAtlas::VelloAtlasMgr::onAddToAtlas(const Shape& shape,
                                                        const Transform& transform,
                                                        const SkStrokeRec& style,
                                                        SkIRect shapeBounds,
                                                        const AtlasLocator& locator) {
    uint32_t index = locator.pageIndex();
    const TextureProxy* texProxy = fDrawAtlas->getProxies()[index].get();
    if (!texProxy) {
        return false;
    }

    // TODO: The compute renderer doesn't support perspective yet. We assume that the path has been
    // appropriately transformed in that case.
    SkASSERT(transform.type() != Transform::Type::kPerspective);

    // Restrict the render to the occupied area of the atlas, including entry padding so that the
    // padded row/column is cleared when Vello renders.
    SkIPoint iPos = locator.topLeft();
    Rect atlasBounds = Rect::XYWH(skvx::float2(iPos.x() + kEntryPadding, iPos.y() + kEntryPadding),
                                  skvx::float2(shapeBounds.width(), shapeBounds.height()));

    add_shape_to_scene(shape, transform, style, atlasBounds,
                       &fScenes[index], &fOccupiedAreas[index]);

    return true;
}

bool VelloComputePathAtlas::VelloAtlasMgr::recordDispatches(
        Recorder* recorder, ComputeTask::DispatchGroupList* dispatches) const {
    SkASSERT(recorder);
    VelloAaConfig config = get_vello_aa_config(recorder);

    bool addedDispatches = false;
    for (int i = 0; i < 4; ++i) {
        if (!fOccupiedAreas[i].isEmpty()) {
            std::unique_ptr<DispatchGroup> dispatchGroup =
                    render_vello_scene(recorder,
                                       fDrawAtlas->getProxies()[i],
                                       fScenes[i],
                                       fOccupiedAreas[i],
                                       config);
            if (dispatchGroup) {
                TRACE_EVENT_INSTANT1("skia.gpu", TRACE_FUNC, TRACE_EVENT_SCOPE_THREAD,
                                     "# dispatches", dispatchGroup->dispatches().size());
                dispatches->emplace_back(std::move(dispatchGroup));
                addedDispatches = true;
            } else {
                SKGPU_LOG_E("VelloComputePathAtlas:: Failed to create dispatch group.");
            }
        }
    }
    return addedDispatches;
}


#endif  // SK_ENABLE_VELLO_SHADERS

std::unique_ptr<ComputePathAtlas> ComputePathAtlas::CreateDefault(Recorder* recorder) {
#ifdef SK_ENABLE_VELLO_SHADERS
    return std::make_unique<VelloComputePathAtlas>(recorder);
#else
    return nullptr;
#endif
}

}  // namespace skgpu::graphite
