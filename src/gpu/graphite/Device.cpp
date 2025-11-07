/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/Device.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkArc.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkVertices.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkVx.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkImageFilterTypes.h"  // IWYU pragma: keep
#include "src/core/SkImagePriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/TiledTextureUtils.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/Image_Base_Graphite.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/RasterPathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/geom/AnalyticBlurMask.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/CoverageMaskShape.h"
#include "src/gpu/graphite/geom/EdgeAAQuad.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/IntersectionTree.h"
#include "src/gpu/graphite/geom/NonMSAAClip.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/SubRunData.h"
#include "src/gpu/graphite/geom/Transform.h"
#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/UploadTask.h"
#include "src/image/SkImage_Base.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"
#include "src/text/gpu/VertexFiller.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using RescaleGamma       = SkImage::RescaleGamma;
using RescaleMode        = SkImage::RescaleMode;
using ReadPixelsCallback = SkImage::ReadPixelsCallback;
using ReadPixelsContext  = SkImage::ReadPixelsContext;

#if defined(GPU_TEST_UTILS)
int gOverrideMaxTextureSizeGraphite = 0;
// Allows tests to check how many tiles were drawn on the most recent call to
// Device::drawAsTiledImageRect. This is an atomic because we can write to it from
// multiple threads during "normal" operations. However, the tests that actually
// read from it are done single-threaded.
std::atomic<int> gNumTilesDrawnGraphite{0};
#endif

namespace skgpu::graphite {

class Recording;
enum class TextureFormat : uint8_t;

#define ASSERT_SINGLE_OWNER SkASSERT(fRecorder); SKGPU_ASSERT_SINGLE_OWNER(fRecorder->singleOwner())

namespace {

const SkStrokeRec& DefaultFillStyle() {
    static const SkStrokeRec kFillStyle(SkStrokeRec::kFill_InitStyle);
    return kFillStyle;
}

/** If the paint can be reduced to a solid flood-fill, determine the correct color to fill with. */
std::optional<SkColor4f> extract_paint_color(const PaintParams& paint,
                                             const SkColorInfo& dstColorInfo) {
    SkBlendMode bm = paint.finalBlendMode();
    // Since we don't depend on the dst, a dst-out blend mode implies source is
    // opaque, which causes dst-out to behave like clear.
    if (bm == SkBlendMode::kClear || bm == SkBlendMode::kDstOut) {
        return SkColors::kTransparent;
    }

    // PaintParams has already consolidated constant shaders or images and applied color filters to
    // constant input colors. If the paint still has any of those fields, then we can't extract it.
    if (paint.shader() || paint.imageShader() || paint.colorFilter()) {
        return std::nullopt;
    }

    // However, PaintParams converted the color in sRGB and we need to return this in the
    // destination color space.
    return PaintParams::Color4fPrepForDst(paint.color(), dstColorInfo);
}

// Returns a local rect that has been adjusted such that when it's rasterized with `localToDevice`
// it will be pixel aligned. If this adjustment is not possible (due to transform type or precision)
// then this returns the original local rect unmodified.
//
// If `strokeWidth` is null, it's assumed to be a filled rectangle. If it's not null, on input it
// should hold the stroke width (or 0 for a hairline). After this returns, the stroke width may
// have been adjusted so that outer and inner stroked edges are pixel aligned (in which case the
// underlying rectangle geometry probably won't be pixel aligned).
//
// A best effort is made to align the stroke edges when there's a non-uniform scale factor that
// prevents exactly aligning both X and Y axes.
Rect snap_rect_to_pixels(const Transform& localToDevice,
                         const Rect& rect,
                         float* strokeWidth=nullptr) {
    if (localToDevice.type() > Transform::Type::kRectStaysRect) {
        return rect;
    }

    Rect snappedDeviceRect;
    if (!strokeWidth) {
        // Just a fill, use round() to emulate non-AA rasterization (vs. roundOut() to get the
        // covering bounds). This matches how ClipStack treats clipRects with PixelSnapping::kYes.
        snappedDeviceRect = localToDevice.mapRect(rect).round();
    } else if (strokeWidth) {
        if (*strokeWidth == 0.f) {
            // Hairline case needs to be outset by 1/2 device pixels *before* rounding, and then
            // inset by 1/2px to get the base shape while leaving the stroke width as 0.
            snappedDeviceRect = localToDevice.mapRect(rect);
            snappedDeviceRect.outset(0.5f).round().inset(0.5f);
        } else {
            // For regular strokes, outset by the stroke radius *before* mapping to device space,
            // and then round.
            snappedDeviceRect = localToDevice.mapRect(rect.makeOutset(0.5f*(*strokeWidth))).round();

            // devScales.x() holds scale factor affecting device-space X axis (so max of |m00| or
            // |m01|) and y() holds the device Y axis scale (max of |m10| or |m11|).
            skvx::float2 devScales = max(abs(skvx::float2(localToDevice.matrix().rc(0,0),
                                                          localToDevice.matrix().rc(1,0))),
                                         abs(skvx::float2(localToDevice.matrix().rc(0,1),
                                                          localToDevice.matrix().rc(1,1))));
            skvx::float2 devStrokeWidth = max(round(*strokeWidth * devScales), 1.f);

            // Prioritize the axis that has the largest device-space radius (any error from a
            // non-uniform scale factor will go into the inner edge of the opposite axis).
            // During animating scale factors, preserving the large axis leads to better behavior.
            if (devStrokeWidth.x() > devStrokeWidth.y()) {
                *strokeWidth = devStrokeWidth.x() / devScales.x();
            } else {
                *strokeWidth = devStrokeWidth.y() / devScales.y();
            }

            snappedDeviceRect.inset(0.5f * devScales * (*strokeWidth));
        }
    }

    // Map back to local space so that it can be drawn with appropriate coord interpolation.
    Rect snappedLocalRect = localToDevice.inverseMapRect(snappedDeviceRect);
    // If the transform has an extreme scale factor or large translation, it's possible for floating
    // point precision to round `snappedLocalRect` in such a way that re-transforming it by the
    // local-to-device matrix no longer matches the expected device bounds.
    if (snappedDeviceRect.nearlyEquals(localToDevice.mapRect(snappedLocalRect))) {
        return snappedLocalRect;
    } else {
        // In this case we will just return the original geometry and the pixels will show
        // fractional coverage.
        return rect;
    }
}

// If possible, snaps `dstRect` such that its device-space transformation lands on pixel bounds,
// and then updates `srcRect` to match the original src-to-dst coordinate mapping.
void snap_src_and_dst_rect_to_pixels(const Transform& localToDevice,
                                     SkRect* srcRect,
                                     SkRect* dstRect) {
    if (localToDevice.type() > Transform::Type::kRectStaysRect) {
        return;
    }

    // Assume snapping will succeed and always update 'src' to match; in the event snapping
    // returns the original dst rect, then the recalculated src rect is a no-op.
    SkMatrix dstToSrc = SkMatrix::RectToRectOrIdentity(*dstRect, *srcRect);
    *dstRect = snap_rect_to_pixels(localToDevice, *dstRect).asSkRect();
    *srcRect = dstToSrc.mapRect(*dstRect);
}

// Returns the inner bounds of `geometry` that is known to have full coverage. This does not worry
// about identifying draws that are equivalent pixel aligned and thus entirely full coverage, as
// that should have been caught earlier and used a coverage-less renderer from the beginning.
//
// An empty Rect is returned if there is no available inner bounds, or if it's not worth performing.
Rect get_inner_bounds(const Geometry& geometry, const Transform& localToDevice) {
    auto applyAAInset = [&](Rect rect) {
        // If the aa inset is too large, rect becomes empty and the inner bounds draw is
        // automatically skipped
        float aaInset = localToDevice.localAARadius(rect);
        rect.inset(aaInset);
        // Only add a second draw if it will have a reasonable number of covered pixels; otherwise
        // we are just adding draws to sort and pipelines to switch around.
        static constexpr float kInnerFillArea = 64*64;
        // Approximate the device-space area based on the minimum scale factor of the transform.
        float scaleFactor = sk_ieee_float_divide(1.f, aaInset);
        return scaleFactor*rect.area() >= kInnerFillArea ? rect : Rect::InfiniteInverted();
    };

    if (geometry.isEdgeAAQuad()) {
        const EdgeAAQuad& quad = geometry.edgeAAQuad();
        if (quad.isRect()) {
            return applyAAInset(quad.bounds());
        }
        // else currently we don't have a function to calculate the largest interior axis aligned
        // bounding box of a quadrilateral so skip the inner fill draw.
    } else if (geometry.isShape()) {
        const Shape& shape = geometry.shape();
        if (shape.isRect()) {
            return applyAAInset(shape.rect());
        } else if (shape.isRRect()) {
            return applyAAInset(SkRRectPriv::InnerBounds(shape.rrect()));
        }
    }

    return Rect::InfiniteInverted();
}

SkIRect rect_to_pixelbounds(const Rect& r) {
    return r.makeRoundOut().asSkIRect();
}

bool is_pixel_aligned(const Rect& r, const Transform& t) {
    if (t.type() <= Transform::Type::kRectStaysRect) {
        Rect devRect = t.mapRect(r);
        return devRect.nearlyEquals(devRect.makeRound(), Shape::kDefaultPixelTolerance);
    }

    return false;
}

bool is_simple_shape(const Shape& shape, const Transform& localToDevice, SkStrokeRec::Style type) {
    if (shape.isFloodFill()) {
        return true; // Always supported
    } else if (!shape.inverted() && type != SkStrokeRec::kStrokeAndFill_Style) {
        // A filled line renders nothing but that should be caught earlier, so the actual branches
        // in this function can be simplified.
        SkASSERT(!shape.isLine() || type != SkStrokeRec::kFill_Style);

        if (shape.isRRect() && type == SkStrokeRec::kStroke_Style) {
            // Non-hairline stroked round rects require the corner radii to be circular to be
            // compatible with the shared Renderer.
            const float tol =
                    localToDevice.localAARadius(shape.bounds()) * Shape::kDefaultPixelTolerance;
            return SkRRectPriv::AllCornersRelativelyCircular(shape.rrect(), tol);
        } else if (shape.isRRect() || shape.isRect() || shape.isLine()) {
            // There are no restrictions on filled or hairline [r]rects and lines.
            return true;
        } // Fallthrough
    }

    // Requires path rendering
    return false;
}

bool use_compute_atlas_when_available(std::optional<PathRendererStrategy> strategy) {
    return !strategy.has_value() ||
           strategy == PathRendererStrategy::kComputeAnalyticAA ||
           strategy == PathRendererStrategy::kComputeMSAA16 ||
           strategy == PathRendererStrategy::kComputeMSAA8;
}

class ScopedDrawBuilder {
public:
    explicit ScopedDrawBuilder(Recorder* recorder)
            : fRecorder(recorder),
              fKeyAndDataBuilder(fRecorder->priv().popOrCreateKeyAndDataBuilder()) {
        SkASSERT(fKeyAndDataBuilder);
        SkDEBUGCODE(this->gatherer()->checkReset());
        SkDEBUGCODE(this->builder()->checkReset());
    }

    ~ScopedDrawBuilder() {
        SkASSERT(fKeyAndDataBuilder && fRecorder);
        // The PipelineDataGatherer must be reset before being returned to the pool for reuse.
        this->gatherer()->resetForDraw();
        fRecorder->priv().pushKeyAndDataBuilder(std::move(fKeyAndDataBuilder));
    }

    PipelineDataGatherer* gatherer() { return &fKeyAndDataBuilder->first; }
    PaintParamsKeyBuilder* builder() { return &fKeyAndDataBuilder->second; }

    ScopedDrawBuilder(const ScopedDrawBuilder&) = delete;
    ScopedDrawBuilder& operator=(const ScopedDrawBuilder&) = delete;

private:
    Recorder* fRecorder;
    std::unique_ptr<KeyAndDataBuilder> fKeyAndDataBuilder;
};

} // anonymous namespace

/**
 * IntersectionTreeSet controls multiple IntersectionTrees to organize all add rectangles into
 * disjoint sets. For a given CompressedPaintersOrder and bounds, it returns the smallest
 * DisjointStencilIndex that guarantees the bounds are disjoint from all other draws that use the
 * same painters order and stencil index.
 */
class Device::IntersectionTreeSet {
public:
    IntersectionTreeSet() = default;

    DisjointStencilIndex add(CompressedPaintersOrder drawOrder, Rect rect) {
        auto& trees = fTrees[drawOrder];
        DisjointStencilIndex stencil = DrawOrder::kUnassigned.next();
        for (auto&& tree : trees) {
            if (tree->add(rect)) {
                return stencil;
            }
            stencil = stencil.next(); // advance to the next tree's index
        }

        // If here, no existing intersection tree can hold the rect so add a new one
        IntersectionTree* newTree = this->makeTree();
        SkAssertResult(newTree->add(rect));
        trees.push_back(newTree);
        return stencil;
    }

    void reset() {
        fTrees.clear();
        fTreeStore.reset();
    }

private:
    struct Hash {
        size_t operator()(const CompressedPaintersOrder& o) const noexcept { return o.bits(); }
    };

    IntersectionTree* makeTree() {
        return fTreeStore.make<IntersectionTree>();
    }

    // Each compressed painters order defines a barrier around draws so each order's set of draws
    // are independent, even if they may intersect. Within each order, the list of trees holds the
    // IntersectionTrees representing each disjoint set.
    // TODO: This organization of trees is logically convenient but may need to be optimized based
    // on real world data (e.g. how sparse is the map, how long is each vector of trees,...)
    std::unordered_map<CompressedPaintersOrder, std::vector<IntersectionTree*>, Hash> fTrees;
    SkSTArenaAllocWithReset<4 * sizeof(IntersectionTree)> fTreeStore;
};

sk_sp<Device> Device::Make(Recorder* recorder,
                           const SkImageInfo& ii,
                           skgpu::Budgeted budgeted,
                           Mipmapped mipmapped,
                           SkBackingFit backingFit,
                           const SkSurfaceProps& props,
                           LoadOp initialLoadOp,
                           std::string_view label,
                           bool registerWithRecorder) {
    SkASSERT(!(mipmapped == Mipmapped::kYes && backingFit == SkBackingFit::kApprox));
    if (!recorder) {
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();
    SkISize backingDimensions = backingFit == SkBackingFit::kApprox ? GetApproxSize(ii.dimensions())
                                                                    : ii.dimensions();
    auto textureInfo = caps->getDefaultSampledTextureInfo(ii.colorType(),
                                                          mipmapped,
                                                          recorder->priv().isProtected(),
                                                          Renderable::kYes);

    return Make(recorder,
                TextureProxy::Make(caps, recorder->priv().resourceProvider(),
                                   backingDimensions, textureInfo, std::move(label), budgeted),
                ii.dimensions(),
                ii.colorInfo(),
                props,
                initialLoadOp,
                registerWithRecorder);
}

sk_sp<Device> Device::Make(Recorder* recorder,
                           sk_sp<TextureProxy> target,
                           SkISize deviceSize,
                           const SkColorInfo& colorInfo,
                           const SkSurfaceProps& props,
                           LoadOp initialLoadOp,
                           bool registerWithRecorder) {
    if (!recorder) {
        return nullptr;
    }

    sk_sp<DrawContext> dc = DrawContext::Make(recorder->priv().caps(),
                                              std::move(target),
                                              deviceSize,
                                              colorInfo,
                                              props);
    if (!dc) {
        return nullptr;
    } else if (initialLoadOp == LoadOp::kClear) {
        dc->clear(SkColors::kTransparent);
    } else if (initialLoadOp == LoadOp::kDiscard) {
        dc->discard();
    } // else kLoad is the default initial op for a DrawContext

    sk_sp<Device> device{new Device(recorder, std::move(dc))};
    if (registerWithRecorder) {
        // We don't register the device with the recorder until after the constructor has returned.
        recorder->registerDevice(device);
    } else {
        // Since it's not registered, it should go out of scope before nextRecordingID() changes
        // from what is saved to fScopedRecordingID.
        SkDEBUGCODE(device->fScopedRecordingID = recorder->priv().nextRecordingID();)
    }
    return device;
}

// These default tuning numbers for the HybridBoundsManager were chosen from looking at performance
// and accuracy curves produced by the BoundsManagerBench for random draw bounding boxes. This
// config will use brute force for the first 64 draw calls to the Device and then switch to a grid
// that is dynamically sized to produce cells that are 16x16, up to a grid that's 32x32 cells.
// This seemed like a sweet spot balancing accuracy for low-draw count surfaces and overhead for
// high-draw count and high-resolution surfaces. With the 32x32 grid limit, cell size will increase
// above 16px when the surface dimension goes above 512px.
// TODO: These could be exposed as context options or surface options, and we may want to have
// different strategies in place for a base device vs. a layer's device.
static constexpr int kGridCellSize = 16;
static constexpr int kMaxBruteForceN = 64;
static constexpr int kMaxGridSize = 32;

Device::Device(Recorder* recorder, sk_sp<DrawContext> dc)
        : SkDevice(dc->imageInfo(), dc->surfaceProps())
        , fRecorder(recorder)
        , fDC(std::move(dc))
        , fClip(this)
        , fColorDepthBoundsManager(std::make_unique<HybridBoundsManager>(
                  fDC->imageInfo().dimensions(), kGridCellSize, kMaxBruteForceN, kMaxGridSize))
        , fDisjointStencilSet(std::make_unique<IntersectionTreeSet>())
        , fCachedLocalToDevice(SkM44())
        , fCurrentDepth(DrawOrder::kClearDepth)
        , fSubRunControl(recorder->priv().caps()->getSubRunControl(
                fDC->surfaceProps().isUseDeviceIndependentFonts())) {
    SkASSERT(SkToBool(fDC) && SkToBool(fRecorder));
    if (fDC->target()->textureInfo().numSamples() > 1) {
        // Target is inherently multisampled
        fMSAASupported = true;
    } else if (fRecorder->priv().caps()->defaultMSAASamplesCount() > 1) {
        if (fRecorder->priv().caps()->msaaRenderToSingleSampledSupport()) {
            // Backend-managed MSAA is supported
            fMSAASupported = true;
        } else {
            // Graphite-managed MSAA is supported
            fMSAASupported = fRecorder->priv().caps()->isSampleCountSupported(
                    TextureInfoPriv::ViewFormat(fDC->target()->textureInfo()),
                    fRecorder->priv().caps()->defaultMSAASamplesCount());
        }
    }
}

Device::~Device() {
    // The Device should have been marked immutable before it's destroyed, or the Recorder was the
    // last holder of a reference to it and de-registered the device as part of its cleanup.
    // However, if the Device was not registered with the recorder (i.e. a scratch device) we don't
    // require that its recorder be adandoned. Scratch devices must either have been marked
    // immutable or be destroyed before the recorder has been snapped.
    SkASSERT(!fRecorder || fScopedRecordingID != 0);
#if defined(SK_DEBUG)
    if (fScopedRecordingID != 0 && fRecorder) {
        SkASSERT(fScopedRecordingID == fRecorder->priv().nextRecordingID());
    }
    // else it wasn't a scratch device, or it was a scratch device that was marked immutable so its
    // lifetime was validated when setImmutable() was called.
#endif
}

void Device::setImmutable() {
    if (fRecorder) {
        // Push any pending work to the Recorder now. setImmutable() is only called by the
        // destructor of a client-owned Surface, or explicitly in layer/filtering workflows. In
        // both cases this is restricted to the Recorder's thread. This is in contrast to ~Device(),
        // which might be called from another thread if it was linked to an Image used in multiple
        // recorders.
        this->flushPendingWork(/*drawContext=*/nullptr);
        fRecorder->deregisterDevice(this);
        // Abandoning the recorder ensures that there are no further operations that can be recorded
        // and is relied on by Image::notifyInUse() to detect when it can unlink from a Device.
        this->abandonRecorder();
    }
}

bool Device::notifyInUse(Recorder* recorder, DrawContext* drawContext) {
    if (this->isScratchDevice()) {
        if (fLastTask) {
            // Increment the pending read count for the device's target
            recorder->priv().addPendingRead(this->target());
            if (drawContext) {
                // Add a reference to the device's drawTask to `drawContext` if that's provided.
                drawContext->recordDependency(fLastTask);
            } else {
                // If there's no `drawContext` this notify represents a copy, so for now append the
                // task to the root task list since that is where the subsequent copy task will go
                // as well.
                recorder->priv().add(fLastTask);
            }
        } else {
            // If there's no draw task yet, there are two possible scenarios:
            //
            // 1) the device is being drawn into a child scratch device (backdrop filter or
            //    init-from-prev layer), and the child will later on be drawn back into the device's
            //    `drawContext`. In this case `device` should already have performed an internal
            //    flush and have no pending work, and not yet be marked immutable. The correct
            //    action at this point in time is to do nothing: the final task order in the
            //    device's DrawTask will be pre-notified tasks into the device's target, then the
            //    child's DrawTask when it's drawn back into `device`, and then any post tasks that
            //    further modify the `device`'s target.
            // 2) the scratch device was flushed to a drawContext's local task list, resulting in no
            //    pending work but also no lastTask. The correct action is again to do nothing. In
            //    this case, it is also possible that the device was not registered with the
            //    recorder.
            SkASSERT(!fRecorder || fRecorder == recorder);
        }

        // Scratch devices are often already marked immutable, but they are also the way in which
        // Image finds the last snapped DrawTask so we don't unlink scratch devices. The scratch
        // image view will be short-lived as well, or the device will transition to a non-scratch
        // device in a future Recording and then it will be unlinked then. Thus, we always return
        // false for scratch devices so they are not unlinked from their images.
        return false;
    } else {
        // Automatic flushing of image views only happens when mixing reads and writes on the
        // originating Recorder. Draws of the view on another Recorder will always see the texture
        // content dependent on how Recordings are inserted.
        if (fRecorder == recorder) {
            // Non-scratch devices push their tasks to the root task list to maintain an order
            // consistent with the client-triggering actions. Because of this, there's no need to
            // add references to the `drawContext` that the device is being drawn into.
            this->flushPendingWork(/*drawContext=*/nullptr);

            if (drawContext) {
                // But if we are being drawn into another context, remember that there is an
                // outstanding dependency on the current state of this device, in which case it's
                // next flush must also flush those other devices before its new tasks are added.
                fMustFlushDependencies = true;
            }
        }
        // Return true (to unlink with the image) if the non-scratch surface is immutable since this
        // Device cannot record any more commands that will modify its texture.
        return !SkToBool(fRecorder) || this->unique();
    }
}

bool Device::hasPendingReads(const TextureProxy* texture) const {
    return fRecorder && fDC->readsTexture(texture);
}

const Transform& Device::localToDeviceTransform() {
    if (this->checkLocalToDeviceDirty()) {
        fCachedLocalToDevice = Transform{this->localToDevice44()};
    }
    return fCachedLocalToDevice;
}

SkStrikeDeviceInfo Device::strikeDeviceInfo() const {
    return {this->surfaceProps(), this->scalerContextFlags(), &fSubRunControl};
}

sk_sp<SkDevice> Device::createDevice(const CreateInfo& info, const SkPaint*) {
    // TODO: Inspect the paint and create info to determine if there's anything that has to be
    // modified to support inline subpasses.
    SkSurfaceProps props =
        this->surfaceProps().cloneWithPixelGeometry(info.fPixelGeometry);

    // Skia's convention is to only clear a device if it is non-opaque.
    LoadOp initialLoadOp = info.fInfo.isOpaque() ? LoadOp::kDiscard : LoadOp::kClear;

    std::string label = this->target()->label();
    if (label.empty()) {
        label = "ChildDevice";
    } else {
        label += "_ChildDevice";
    }

    return Make(fRecorder,
                info.fInfo,
                skgpu::Budgeted::kYes,
                Mipmapped::kNo,
                SkBackingFit::kApprox,
                props,
                initialLoadOp,
                label);
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& ii, const SkSurfaceProps& props) {
    return SkSurfaces::RenderTarget(fRecorder, ii, Mipmapped::kNo, &props);
}

// Although we have a drawContext here, we pass a nullptr to both flushPendingWork and Image::Copy
// so that tasks end up on the root task list.
sk_sp<Image> Device::makeImageCopy(const SkIRect& subset,
                                   Budgeted budgeted,
                                   Mipmapped mipmapped,
                                   SkBackingFit backingFit) {
    ASSERT_SINGLE_OWNER
    this->flushPendingWork(/*drawContext=*/nullptr);

    const SkColorInfo& colorInfo = this->imageInfo().colorInfo();
    TextureProxyView srcView = this->readSurfaceView();
    if (!srcView) {
        // readSurfaceView() returns an empty view when the target is not texturable. Create an
        // equivalent view for the blitting operation.
        Swizzle readSwizzle = fRecorder->priv().caps()->getReadSwizzle(
                colorInfo.colorType(), this->target()->textureInfo());
        srcView = {sk_ref_sp(this->target()), readSwizzle};
    }
    std::string label = this->target()->label();
    if (label.empty()) {
        label = "CopyDeviceTexture";
    } else {
        label += "_DeviceCopy";
    }

    return Image::Copy(fRecorder, /*drawContext=*/nullptr, srcView, colorInfo, subset, budgeted,
                       mipmapped, backingFit, label);
}

bool Device::onReadPixels(const SkPixmap& pm, int srcX, int srcY) {
#if defined(GPU_TEST_UTILS)
    // This testing-only function should only be called before the Device has detached from its
    // Recorder, since it's accessed via the test-held Surface.
    ASSERT_SINGLE_OWNER
    if (Context* context = fRecorder->priv().context()) {
        // Add all previous commands generated to the command buffer.
        // If the client snaps later they'll only get post-read commands in their Recording,
        // but since they're doing a readPixels in the middle that shouldn't be unexpected.
        std::unique_ptr<Recording> recording = fRecorder->snap();
        if (!recording) {
            return false;
        }
        InsertRecordingInfo info;
        info.fRecording = recording.get();
        if (!context->insertRecording(info)) {
            return false;
        }
        return context->priv().readPixels(pm, fDC->target(), this->imageInfo(), srcX, srcY);
    }
#endif
    // We have no access to a context to do a read pixels here.
    return false;
}

bool Device::onWritePixels(const SkPixmap& src, int x, int y) {
    ASSERT_SINGLE_OWNER
    // TODO: we may need to share this in a more central place to handle uploads
    // to backend textures

    const TextureProxy* target = fDC->target();

    // TODO: add mipmap support for createBackendTexture

    if (src.colorType() == kUnknown_SkColorType) {
        return false;
    }

    // If one alpha type is unknown and the other isn't, it's too underspecified.
    if ((src.alphaType() == kUnknown_SkAlphaType) !=
        (this->imageInfo().alphaType() == kUnknown_SkAlphaType)) {
        return false;
    }

    // TODO: canvas2DFastPath?

    if (!fRecorder->priv().caps()->supportsWritePixels(target->textureInfo())) {
        auto image = SkImages::RasterFromPixmap(src, nullptr, nullptr);
        image = SkImages::TextureFromImage(fRecorder, image.get());
        if (!image) {
            return false;
        }

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        this->drawImageRect(image.get(),
                            /*src=*/nullptr,
                            SkRect::MakeXYWH(x, y, src.width(), src.height()),
                            SkFilterMode::kNearest,
                            paint,
                            SkCanvas::kFast_SrcRectConstraint);
        return true;
    }

    // TODO: check for flips and either handle here or pass info to UploadTask

    // Determine rect to copy
    SkIRect dstRect = SkIRect::MakePtSize({x, y}, src.dimensions());
    if (!target->isFullyLazy() && !dstRect.intersect(SkIRect::MakeSize(target->dimensions()))) {
        return false;
    }

    // Set up copy location
    const void* addr = src.addr(dstRect.fLeft - x, dstRect.fTop - y);
    std::vector<MipLevel> levels;
    levels.push_back({addr, src.rowBytes()});

    // The writePixels() still respects painter's order, so flush everything to tasks before this
    // recording the upload for the pixel data.
    this->internalFlush();
    // The new upload will be executed before any new draws are recorded and also ensures that
    // the next call to flushDeviceToRecorder() will produce a non-null DrawTask. If this Device's
    // target is mipmapped, mipmap generation tasks will be added automatically at that point.
    const UploadSource uploadSource = UploadSource::Make(fRecorder->priv().caps(),
                                                         *fDC->refTarget(),
                                                         src.info().colorInfo(),
                                                         this->imageInfo().colorInfo(),
                                                         levels,
                                                         dstRect);
    if (!uploadSource.isValid()) {
        return false;
    }
    return fDC->recordUpload(fRecorder,
                             fDC->refTarget(),
                             src.info().colorInfo(),
                             this->imageInfo().colorInfo(),
                             uploadSource,
                             dstRect,
                             nullptr);
}


///////////////////////////////////////////////////////////////////////////////

bool Device::isClipAntiAliased() const {
    // All clips are AA'ed unless it's wide-open, empty, or a device-rect with integer coordinates
    ClipStack::ClipState type = fClip.clipState();
    if (type == ClipStack::ClipState::kWideOpen || type == ClipStack::ClipState::kEmpty) {
        return false;
    } else if (type == ClipStack::ClipState::kDeviceRect) {
        const ClipStack::Element rect = *fClip.begin();
        SkASSERT(rect.fShape.isRect() && rect.fLocalToDevice.type() == Transform::Type::kIdentity);
        return rect.fShape.rect() != rect.fShape.rect().makeRoundOut();
    } else {
        return true;
    }
}

SkIRect Device::devClipBounds() const {
    return rect_to_pixelbounds(fClip.conservativeBounds());
}

// TODO: This is easy enough to support, but do we still need this API in Skia at all?
void Device::android_utils_clipAsRgn(SkRegion* region) const {
    SkIRect bounds = this->devClipBounds();
    // Assume wide open and then perform intersect/difference operations reducing the region
    region->setRect(bounds);
    const SkRegion deviceBounds(bounds);
    for (const ClipStack::Element& e : fClip) {
        SkRegion tmp;
        if (e.fShape.isRect() && e.fLocalToDevice.type() == Transform::Type::kIdentity) {
            tmp.setRect(rect_to_pixelbounds(e.fShape.rect()));
        } else {
            SkPath tmpPath = e.fShape.asPath().makeTransform(e.fLocalToDevice);
            tmp.setPath(tmpPath, deviceBounds);
        }

        region->op(tmp, (SkRegion::Op) e.fOp);
    }
}

void Device::clipRect(const SkRect& rect, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    auto snapping = aa ? ClipStack::PixelSnapping::kNo : ClipStack::PixelSnapping::kYes;
    fClip.clipShape(this->localToDeviceTransform(), Shape{rect}, op, snapping);
}

void Device::clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    auto snapping = aa ? ClipStack::PixelSnapping::kNo : ClipStack::PixelSnapping::kYes;
    fClip.clipShape(this->localToDeviceTransform(), Shape{rrect}, op, snapping);
}

void Device::clipPath(const SkPath& path, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    // TODO: Ensure all path inspection is handled here or in SkCanvas, and that non-AA rects as
    // paths are routed appropriately.
    // TODO: Must also detect paths that are lines so the clip stack can be set to empty
    fClip.clipShape(this->localToDeviceTransform(), Shape{path}, op);
}

void Device::onClipShader(sk_sp<SkShader> shader) {
    fClip.clipShader(std::move(shader));
}

// TODO: Is clipRegion() on the deprecation chopping block. If not it should be...
void Device::clipRegion(const SkRegion& globalRgn, SkClipOp op) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);

    Transform globalToDevice{this->globalToDevice()};

    if (globalRgn.isEmpty()) {
        fClip.clipShape(globalToDevice, Shape{}, op);
    } else if (globalRgn.isRect()) {
        fClip.clipShape(globalToDevice, Shape{SkRect::Make(globalRgn.getBounds())}, op,
                        ClipStack::PixelSnapping::kYes);
    } else {
        // TODO: Can we just iterate the region and do non-AA rects for each chunk?
        SkPath path = globalRgn.getBoundaryPath();
        fClip.clipShape(globalToDevice, Shape{path}, op);
    }
}

void Device::replaceClip(const SkIRect& rect) {
    // ReplaceClip() is currently not intended to be supported in Graphite since it's only used
    // for emulating legacy clip ops in Android Framework, and apps/devices that require that
    // should not use Graphite. However, if it needs to be supported, we could probably implement
    // it by:
    //  1. Flush all pending clip element depth draws.
    //  2. Draw a fullscreen rect to the depth attachment using a Z value greater than what's
    //     been used so far.
    //  3. Make sure all future "unclipped" draws use this Z value instead of 0 so they aren't
    //     sorted before the depth reset.
    //  4. Make sure all prior elements are inactive so they can't affect subsequent draws.
    //
    // For now, just ignore it.
}

///////////////////////////////////////////////////////////////////////////////

void Device::drawPaint(const SkPaint& paint) {
    ASSERT_SINGLE_OWNER

    Shape inverseFill; // defaults to empty
    inverseFill.setInverted(true);
    // An empty shape with an inverse fill completely floods the clip
    SkASSERT(inverseFill.isFloodFill());

    this->drawGeometry(this->localToDeviceTransform(),
                       Geometry(inverseFill),
                       PaintParams(paint),
                       DefaultFillStyle());
}

void Device::drawRect(const SkRect& r, const SkPaint& paint) {
    Rect rectToDraw(r);
    SkStrokeRec style(paint);
    if (!paint.isAntiAlias()) {
        // Graphite assumes everything is anti-aliased. In the case of axis-aligned non-aa requested
        // rectangles, we snap the local geometry to land on pixel boundaries to emulate non-aa.
        if (style.isFillStyle()) {
            rectToDraw = snap_rect_to_pixels(this->localToDeviceTransform(), rectToDraw);
        } else {
            const bool strokeAndFill = style.getStyle() == SkStrokeRec::kStrokeAndFill_Style;
            float strokeWidth = style.getWidth();
            rectToDraw = snap_rect_to_pixels(this->localToDeviceTransform(),
                                             rectToDraw, &strokeWidth);
            style.setStrokeStyle(strokeWidth, strokeAndFill);
        }
    }
    this->drawGeometryWithPathEffect(this->localToDeviceTransform(),
                                     Geometry(Shape(rectToDraw)),
                                     PaintParams(paint),
                                     style,
                                     paint.getPathEffect());
}

void Device::drawVertices(const SkVertices* vertices, sk_sp<SkBlender> blender,
                          const SkPaint& paint, bool skipColorXform)  {
    // A null blender is normally equivalent to SrcOver; coerce it to non-null so that nullity
    // can be used by PaintParamsKeyBuilder to know when to add primitive blending blocks.
    // Use null for the primitive blender if `vertices` does not have per-vertex colors.
    const SkBlender* primitiveBlender =
            !vertices->priv().hasColors() ? nullptr :
                                  blender ? blender.get()
                                          : GetBlendModeSingleton(SkBlendMode::kSrcOver);
    this->drawGeometry(this->localToDeviceTransform(),
                       Geometry(sk_ref_sp(vertices)),
                       PaintParams(paint, primitiveBlender, skipColorXform),
                       DefaultFillStyle());
}

bool Device::drawAsTiledImageRect(SkCanvas* canvas,
                                  const SkImage* image,
                                  const SkRect* src,
                                  const SkRect& dst,
                                  const SkSamplingOptions& sampling,
                                  const SkPaint& paint,
                                  SkCanvas::SrcRectConstraint constraint) {
    auto recorder = canvas->recorder();
    if (!recorder) {
        return false;
    }
    SkASSERT(src);

    // For Graphite this is a pretty loose heuristic. The Recorder-local cache size (relative
    // to the large image's size) is used as a proxy for how conservative we should be when
    // allocating tiles. Since the tiles will actually be owned by the client (via an
    // ImageProvider) they won't actually add any memory pressure directly to Graphite.
    size_t cacheSize = recorder->priv().getResourceCacheLimit();
    size_t maxTextureSize = recorder->priv().caps()->maxTextureSize();

#if defined(GPU_TEST_UTILS)
    if (gOverrideMaxTextureSizeGraphite) {
        maxTextureSize = gOverrideMaxTextureSizeGraphite;
    }
    gNumTilesDrawnGraphite.store(0, std::memory_order_relaxed);
#endif

    // DrawAsTiledImageRect produces per-edge AA quads, which do not participate in non-AA pixel
    // snapping emulation. To match an un-tiled drawImageRect, round the src and dst geometry
    // before any tiling occurs.
    SkRect finalSrc = *src;
    SkRect finalDst = dst;
    if (!paint.isAntiAlias()) {
        snap_src_and_dst_rect_to_pixels(this->localToDeviceTransform(),
                                        &finalSrc, &finalDst);
    }

    [[maybe_unused]] auto [wasTiled, numTiles] =
            skgpu::TiledTextureUtils::DrawAsTiledImageRect(canvas,
                                                           image,
                                                           finalSrc,
                                                           finalDst,
                                                           SkCanvas::kAll_QuadAAFlags,
                                                           sampling,
                                                           &paint,
                                                           constraint,
                                                           /* sharpenMM= */ true,
                                                           cacheSize,
                                                           maxTextureSize);
#if defined(GPU_TEST_UTILS)
    gNumTilesDrawnGraphite.store(numTiles, std::memory_order_relaxed);
#endif
    return wasTiled;
}

void Device::drawOval(const SkRect& oval, const SkPaint& paint) {
    if (paint.getPathEffect()) {
        // Dashing requires that the oval path starts on the right side and travels clockwise. This
        // is the default for the SkPath::Oval constructor, as used by SkBitmapDevice.
        this->drawGeometryWithPathEffect(this->localToDeviceTransform(),
                                         Geometry(Shape(SkPath::Oval(oval))),
                                         PaintParams(paint),
                                         SkStrokeRec(paint),
                                         paint.getPathEffect());
    } else {
        // TODO: This has wasted effort from the SkCanvas level since it instead converts rrects
        // that happen to be ovals into this, only for us to go right back to rrect.
        this->drawRRect(SkRRect::MakeOval(oval), paint);
    }
}

void Device::drawArc(const SkArc& arc, const SkPaint& paint) {
    // For sweeps >= 360°, simple fills and simple strokes without the center point or square caps
    // are ovals. Culling these here simplifies the path processing in Shape.
    if (!paint.getPathEffect() &&
        SkScalarAbs(arc.sweepAngle()) >= 360.f &&
        (paint.getStyle() == SkPaint::kFill_Style ||
         (paint.getStyle() == SkPaint::kStroke_Style &&
          // square caps can stick out from the shape so we can't do this with an rrect draw
          paint.getStrokeCap() != SkPaint::kSquare_Cap &&
          // wedge cases with strokes will draw lines to the center
          !arc.isWedge()))) {
        this->drawRRect(SkRRect::MakeOval(arc.oval()), paint);
    } else {
        this->drawGeometryWithPathEffect(this->localToDeviceTransform(),
                                         Geometry(Shape(arc)),
                                         PaintParams(paint),
                                         SkStrokeRec(paint),
                                         paint.getPathEffect());
    }
}

void Device::drawRRect(const SkRRect& rr, const SkPaint& paint) {
    Shape rrectToDraw;
    SkStrokeRec style(paint);

    if (paint.isAntiAlias()) {
        rrectToDraw.setRRect(rr);
    } else {
        // Snap the horizontal and vertical edges of the rounded rectangle to pixel edges to match
        // the behavior of drawRect(rr.bounds()), to partially emulate non-AA rendering while
        // preserving the anti-aliasing of the curved corners.
        Rect snappedBounds;
        if (style.isFillStyle()) {
            snappedBounds = snap_rect_to_pixels(this->localToDeviceTransform(), rr.rect());
        } else {
            const bool strokeAndFill = style.getStyle() == SkStrokeRec::kStrokeAndFill_Style;
            float strokeWidth = style.getWidth();
            snappedBounds = snap_rect_to_pixels(this->localToDeviceTransform(),
                                                rr.rect(), &strokeWidth);
            style.setStrokeStyle(strokeWidth, strokeAndFill);
        }

        SkRRect snappedRRect;
        snappedRRect.setRectRadii(snappedBounds.asSkRect(), rr.radii().data());
        rrectToDraw.setRRect(snappedRRect);
    }

    this->drawGeometryWithPathEffect(this->localToDeviceTransform(),
                                     Geometry(rrectToDraw),
                                     PaintParams(paint),
                                     style,
                                     paint.getPathEffect());
}

void Device::drawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    // If there's a path effect or inverse fill, fall back to path rendering
    if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style) {
        this->SkDevice::drawDRRect(outer, inner, paint);
        return;
    }

    // This holds the positive insets from `outer` to `inner`
    Rect strokeRect{outer.rect()};
    skvx::float4 gap = Rect(inner.rect()).vals() - strokeRect.vals();
    const float tolerance = Shape::kDefaultPixelTolerance *
                            this->localToDeviceTransform().localAARadius(strokeRect);
    const float strokeWidth = gap[0];
    if (!all((gap > tolerance) & (abs(gap - strokeWidth) <= tolerance))) {
        // Either not approximately equal insets on all sides, or it would create a hairline stroke
        // which is something that should be requested via paint style.
        //
        // But we can try subtracting inner from outer if they are both rectangular to see if it
        // leaves a valid filled rect.
        SkRect diff;
        if (outer.isRect() && inner.isRect() &&
            SkRectPriv::Subtract(outer.rect(), inner.rect(), &diff)) {
            this->drawRect(diff, paint);
            return;
        }

        // Fall through to the draw+clip handling
    } else {
        // The shape is possibly expressible as a stroked [r]rect.
        const float strokeRadius = 0.5f * strokeWidth;
        strokeRect.inset(strokeRadius);

        SkPaint strokePaint = paint;
        strokePaint.setStroke(true);
        strokePaint.setStrokeWidth(strokeWidth);
        strokePaint.setStrokeCap(SkPaint::kButt_Cap);
        strokePaint.setStrokeMiter(4.f); // large enough to not trigger bevels for 90 degree corners

        // Check the corners to see if they are equivalent to a stroked round rect. If `outer` has
        // rounded corners, they must be circular and be at least `strokeRadius`. An outer corner
        // can have a 0 radius if we assume a miter join style, but if an outer corner is exactly
        // the stroke radius then we have to use a round join style. The matching corners of `inner`
        // must also be rounded, and be exactly strokeWidth less, or they must be 0 if the
        // difference would be negative.
        int validCorners = 0;
        int rectCorners = 0;
        SkVector strokeCorners[4];
        std::optional<SkPaint::Join> requiredJoin;
        for (int i = 0; i < 4; ++i) {
            SkVector outerCornerRadii = outer.radii((SkRRect::Corner) i);
            SkVector innerCornerRadii = inner.radii((SkRRect::Corner) i);

            float strokeCorner;
            if (!SkRRectPriv::IsRelativelyCircular(outerCornerRadii.fX,
                                                   outerCornerRadii.fY,
                                                   tolerance)) {
                // Not circular; a stroked ellipse is not just a larger ellipse
                break;
            } else if (SkScalarNearlyZero(outerCornerRadii.fX, tolerance)) {
                // A rectangular outer corner requires miter joins
                if (requiredJoin.has_value() && *requiredJoin != SkPaint::kMiter_Join) {
                    break;
                }
                requiredJoin = SkPaint::kMiter_Join;
                strokeCorner = 0.f;
                rectCorners++;
            } else {
                strokeCorner = outerCornerRadii.fX - strokeRadius;
                if (strokeCorner < -tolerance) {
                    // Corner is rounded but less than the stroke radius, which isn't representable
                    break;
                } else if (strokeCorner <= tolerance) {
                    // Corner is rounded to the stroke radius, which can only be represented as an
                    // underlying rect corner and round join
                    if (requiredJoin.has_value() && *requiredJoin != SkPaint::kRound_Join) {
                        break;
                    }
                    requiredJoin = SkPaint::kRound_Join;
                    strokeCorner = 0.f;
                    rectCorners++;
                }
            }

            float expectedInnerRadius = std::max(0.f, strokeCorner - strokeRadius);
            if (!SkRRectPriv::IsRelativelyCircular(innerCornerRadii.fX,
                                                   expectedInnerRadius,
                                                   tolerance) ||
                !SkRRectPriv::IsRelativelyCircular(innerCornerRadii.fY,
                                                   expectedInnerRadius,
                                                   tolerance)) {
                // Inner corner doesn't match expectation
                break;
            }

            strokeCorners[i] = {strokeCorner, strokeCorner};
            validCorners++;
        }

        if (validCorners == 4) {
            strokePaint.setStrokeJoin(requiredJoin.value_or(SkPaint::kRound_Join));
            if (rectCorners == 4) {
                this->drawRect(strokeRect.asSkRect(), strokePaint);
            } else {
                SkRRect strokeRRect;
                strokeRRect.setRectRadii(strokeRect.asSkRect(), strokeCorners);
                this->drawRRect(strokeRRect, strokePaint);
            }
            return;
        }
        // Otherwise fall through to draw+clip handling
    }

    // To avoid path rendering, treat DRRects as a drawRRect(outer) with a clipRRect(inner, kDiff)
    fClip.save();
    fClip.clipShape(this->localToDeviceTransform(),
                    inner.isRect() ? Shape{inner.rect()} : Shape{inner},
                    SkClipOp::kDifference,
                    paint.isAntiAlias() ? ClipStack::PixelSnapping::kNo
                                        : ClipStack::PixelSnapping::kYes);
    if (outer.isRect()) {
        this->drawRect(outer.rect(), paint);
    } else {
        this->drawRRect(outer, paint);
    }
    fClip.restore();
}

void Device::drawPath(const SkPath& path, const SkPaint& paint) {
    // Alternatively, we could move this analysis to SkCanvas. Also, we could consider applying the
    // path effect, being careful about starting point and direction.
    if (!paint.getPathEffect() && !path.isInverseFillType()) {
        SkPoint linePts[2];
        if (path.isLine(linePts)) {
            // A line has zero area, so stroke and stroke-and-fill are equivalent
            if (paint.getStyle() != SkPaint::kFill_Style) {
                this->drawPoints(SkCanvas::kLines_PointMode, linePts, paint);
            } // and if it's fill, nothing is drawn
            return;
        }
        if (SkRect oval; path.isOval(&oval)) {
            this->drawOval(oval, paint);
            return;
        }
        if (SkRRect rrect; path.isRRect(&rrect)) {
            this->drawRRect(rrect, paint);
            return;
        }
        // For rects, if the path is not explicitly closed and the paint style is stroked then it
        // represents a rectangle with only 3 sides rasterized (and with any caps). If it's filled
        // or is closed+stroked, then the path renders identically to the rectangle.
        bool isClosed = false;
        if (SkRect rect; path.isRect(&rect, &isClosed) &&
            (paint.getStyle() == SkPaint::kFill_Style || isClosed)) {
            this->drawRect(rect, paint);
            return;
        }
        // Detect filled nested rect contours
        SkRect rects[2];
        SkPathDirection dirs[2];
        if (paint.getStyle() == SkPaint::kFill_Style &&
            SkPathPriv::IsNestedFillRects(path, rects, dirs)) {
            // For winding fills with contours going the same direction, there isn't any cutout
            if (path.getFillType() == SkPathFillType::kWinding && dirs[0] == dirs[1]) {
                this->drawRect(rects[0], paint);
                return;
            } else {
                // The inner is cut out from the outer rect. Delegate to drawDRRect.
                this->drawDRRect(SkRRect::MakeRect(rects[0]), SkRRect::MakeRect(rects[1]), paint);
                return;
            }
        }
    }

    // Full path rendering required
    this->drawGeometryWithPathEffect(this->localToDeviceTransform(),
                                     Geometry(Shape(path)),
                                     PaintParams(paint),
                                     SkStrokeRec(paint),
                                     paint.getPathEffect());
}

void Device::drawPoints(SkCanvas::PointMode mode, SkSpan<const SkPoint> points,
                        const SkPaint& paint) {
    if (points.empty()) {
        return;
    }
    size_t count = points.size();

    SkStrokeRec stroke(paint, SkPaint::kStroke_Style);
    size_t next = 0;
    if (mode == SkCanvas::kPoints_PointMode) {
        // Treat kPoints mode as stroking zero-length path segments, which produce caps so that
        // both hairlines and round vs. square geometry are handled entirely on the GPU.
        // TODO: SkCanvas should probably do the butt to square cap correction.
        if (paint.getStrokeCap() == SkPaint::kButt_Cap) {
            stroke.setStrokeParams(SkPaint::kSquare_Cap,
                                   paint.getStrokeJoin(),
                                   paint.getStrokeMiter());
        }
    } else {
        next = 1;
        count--;
    }

    const PaintParams paintParams(paint);
    size_t inc = mode == SkCanvas::kLines_PointMode ? 2 : 1;
    for (size_t i = 0; i < count; i += inc) {
        this->drawGeometryWithPathEffect(this->localToDeviceTransform(),
                                         Geometry(Shape(points[i], points[i + next])),
                                         paintParams,
                                         stroke,
                                         paint.getPathEffect());
    }
}

void Device::drawEdgeAAQuad(const SkRect& rect,
                            const SkPoint clip[4],
                            SkCanvas::QuadAAFlags aaFlags,
                            const SkColor4f& color,
                            SkBlendMode mode) {
    // NOTE: We do not snap edge AA quads that are fully non-AA because we need their edges to seam
    // with quads that have mixed edge flags (so both need to match the GPU rasterization, not our
    // CPU rounding).
    SkEnumBitMask<EdgeAAQuad::Flags> flags = static_cast<EdgeAAQuad::Flags>(aaFlags);
    EdgeAAQuad quad = clip ? EdgeAAQuad(clip, flags) : EdgeAAQuad(rect, flags);
    this->drawGeometry(this->localToDeviceTransform(),
                       Geometry(quad),
                       PaintParams(color, mode),
                       DefaultFillStyle());
}

void Device::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry set[], int count,
                                const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                const SkSamplingOptions& sampling, const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(count > 0);

    const Transform& localToDevice = this->localToDeviceTransform();
    // SkPaint paintWithShader(paint);
    int dstClipIndex = 0;
    for (int i = 0; i < count; ++i) {
        // If the entry is clipped by 'dstClips', that must be provided
        SkASSERT(!set[i].fHasClip || dstClips);
        // Similarly, if it has an extra transform, those must be provided
        SkASSERT(set[i].fMatrixIndex < 0 || preViewMatrices);

        // See SkModifyPaintAndDstForDrawImageRect, as this behavior is consistent but avoids
        // allocating SkShader objects or having to modify the SkPaint.
        // Adjust `dst` such that it only samples from the portion of fSrcRect that overlaps with
        // the image bounds. This "decal" effect is applied geometrically to what is drawn so that
        // actual texture tiling can be clamped to the src rect.
        const SkRect imageBounds = SkRect::Make(set[i].fImage->bounds());
        SkRect dstToDraw = set[i].fDstRect;
        SkRect subset = set[i].fSrcRect;
        SkMatrix localMatrix = SkMatrix::RectToRectOrIdentity(subset, dstToDraw);
        if (!imageBounds.contains(subset)) {
            if (subset.intersect(imageBounds)) {
                // Update dst to match the smaller src
                dstToDraw = localMatrix.mapRect(subset);
            } else {
                dstToDraw.setEmpty();
            }
        }
        if (dstToDraw.isEmpty()) {
            continue; // Nothing to draw for this set entry
        }

        PaintParams::SimpleImage imageShader{set[i].fImage.get(),
                                             &localMatrix,
                                             constraint == SkCanvas::kStrict_SrcRectConstraint ?
                                                    subset : imageBounds,
                                             sampling};

        // NOTE: See drawEdgeAAQuad for details, we do not snap non-AA quads.
        SkEnumBitMask<EdgeAAQuad::Flags> flags = static_cast<EdgeAAQuad::Flags>(set[i].fAAFlags);
        EdgeAAQuad quad = set[i].fHasClip ? EdgeAAQuad(dstClips + dstClipIndex, flags)
                                          : EdgeAAQuad(dstToDraw, flags);

        // TODO: Calling drawGeometry() for each entry re-evaluates the clip stack every time, which
        // is consistent with Ganesh's behavior. It also matches the behavior if edge-AA images were
        // submitted one at a time by SkiaRenderer (a nice client simplification). However, we
        // should explore the performance trade off with doing one bulk evaluation for the whole set
        const SkMatrix* xtraXform = set[i].fMatrixIndex < 0 ? nullptr
                                                            : &preViewMatrices[set[i].fMatrixIndex];
        this->drawGeometry(xtraXform ?  localToDevice.concat(SkM44(*xtraXform)) : localToDevice,
                           Geometry(quad),
                           PaintParams(paint, imageShader, set[i].fAlpha),
                           DefaultFillStyle());

        dstClipIndex += 4 * set[i].fHasClip;
    }
}

void Device::drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                           const SkSamplingOptions& sampling, const SkPaint& paint,
                           SkCanvas::SrcRectConstraint constraint) {
    SkCanvas::ImageSetEntry single{sk_ref_sp(image),
                                   src ? *src : SkRect::Make(image->bounds()),
                                   dst,
                                   /*alpha=*/1.f,
                                   SkCanvas::kAll_QuadAAFlags};
    // While this delegates to drawEdgeAAImageSet() for the image shading logic, semantically a
    // drawImageRect()'s non-AA behavior should match that of drawRect() so we snap dst (and update
    // src to match) if needed before hand.
    if (!paint.isAntiAlias()) {
        snap_src_and_dst_rect_to_pixels(this->localToDeviceTransform(),
                                        &single.fSrcRect, &single.fDstRect);
    }
    this->drawEdgeAAImageSet(&single, 1, nullptr, nullptr, sampling, paint, constraint);
}

sktext::gpu::AtlasDrawDelegate Device::atlasDelegate() {
    return [&](const sktext::gpu::AtlasSubRun* subRun,
               SkPoint drawOrigin,
               const SkPaint& paint,
               sk_sp<SkRefCnt> subRunStorage,
               sktext::gpu::RendererData rendererData) {
        this->drawAtlasSubRun(subRun, drawOrigin, paint, std::move(subRunStorage), rendererData);
    };
}

void Device::onDrawGlyphRunList(SkCanvas* canvas,
                                const sktext::GlyphRunList& glyphRunList,
                                const SkPaint& paint) {
    ASSERT_SINGLE_OWNER
    fRecorder->priv().textBlobCache()->drawGlyphRunList(canvas,
                                                        this->localToDevice(),
                                                        glyphRunList,
                                                        paint,
                                                        this->strikeDeviceInfo(),
                                                        this->atlasDelegate());
}

void Device::drawAtlasSubRun(const sktext::gpu::AtlasSubRun* subRun,
                             SkPoint drawOrigin,
                             const SkPaint& paint,
                             sk_sp<SkRefCnt> subRunStorage,
                             sktext::gpu::RendererData rendererData) {
    ASSERT_SINGLE_OWNER

    // For color emoji, the shading behaves similarly to how drawImageRects override the shader
    // via a SimpleImage. However, for text, the "image" is coming from the atlas and RenderStep as
    // a primitive color and is combined with the paint color using the primitive blender, so we
    // construct the PaintParams to explicitly ignore the paint's set shader. For regular and LCD
    // text, the mask image provides coverage so there is no primitive blender.
    const SkBlender* primitiveBlender = subRun->maskFormat() == MaskFormat::kARGB ?
            GetBlendModeSingleton(SkBlendMode::kDstIn) : nullptr;
    const PaintParams paintParams(paint,
                                  primitiveBlender,
                                  /*skipColorXform=*/false,
                                  /*ignoreShader=*/SkToBool(primitiveBlender));
    const bool useGammaCorrectDistanceTable = this->imageInfo().colorSpace() &&
                                              this->imageInfo().colorSpace()->gammaIsLinear();
    const Transform& localToDevice = this->localToDeviceTransform();

    const int subRunEnd = subRun->glyphCount();
    auto regenerateDelegate = [&](sktext::gpu::GlyphVector* glyphs,
                                  int begin,
                                  int end,
                                  skgpu::MaskFormat maskFormat,
                                  int padding) {
        return glyphs->regenerateAtlasForGraphite(begin, end, maskFormat, padding, fRecorder);
    };
    for (int subRunCursor = 0; subRunCursor < subRunEnd;) {
        // For the remainder of the run, add any atlas uploads to the Recorder's TextAtlasManager
        auto[ok, glyphsRegenerated] = subRun->regenerateAtlas(subRunCursor, subRunEnd,
                                                              regenerateDelegate);
        // There was a problem allocating the glyph in the atlas. Bail.
        if (!ok) {
            return;
        }
        if (glyphsRegenerated) {
            auto [bounds, maskToDevice] =
                    subRun->vertexFiller().boundsAndDeviceMatrix(localToDevice, drawOrigin);


            this->drawGeometry(maskToDevice,
                               Geometry(SubRunData(subRun,
                                                   subRunStorage,
                                                   bounds,
                                                   localToDevice.inverse(),
                                                   subRunCursor,
                                                   glyphsRegenerated,
                                                   SkPaintPriv::ComputeLuminanceColor(paint),
                                                   useGammaCorrectDistanceTable,
                                                   this->surfaceProps().pixelGeometry(),
                                                   fRecorder,
                                                   rendererData)),
                               paintParams,
                               DefaultFillStyle());
        }
        subRunCursor += glyphsRegenerated;

        if (subRunCursor < subRunEnd) {
            // Flush if not all the glyphs are handled because the atlas is out of space.
            // We flush every Device because the glyphs that are being flushed/referenced are not
            // necessarily specific to this Device. This addresses both multiple SkSurfaces within
            // a Recorder, and nested layers.
            TRACE_EVENT_INSTANT0("skia.gpu", "Glyph atlas full", TRACE_EVENT_SCOPE_NAME_THREAD);
            fRecorder->priv().flushTrackedDevices(SK_DUMP_TASKS_CODE("Device::drawAtlasSubRun"));
        }
    }
}

void Device::drawGeometryWithPathEffect(const Transform& localToDevice,
                                        Geometry&& geometry,
                                        const PaintParams& paint,
                                        SkStrokeRec style,
                                        const SkPathEffect* pathEffect) {
    // Path effects are applied on the CPU, which may modify the geometry to draw.
    // TODO(b/238757903): Handle dashing on the GPU when possible (e.g. straight lines)
    if (pathEffect && localToDevice.valid()) {
        // Apply the path effect before anything else, which if we are applying here, means that we
        // are dealing with a Shape. drawVertices (and a SkVertices geometry) should pass in
        // kIgnorePathEffect per SkCanvas spec. Text geometry also should pass in kIgnorePathEffect
        // because the path effect is applied per glyph by the SkStrikeSpec already.
        SkASSERT(geometry.isShape());

        // TODO: If asADash() returns true and the base path matches the dashing fast path, then
        // that should be detected now as well. Maybe add dashPath to Device so canvas can handle it
        float maxScaleFactor = localToDevice.maxScaleFactor();
        if (localToDevice.type() == Transform::Type::kPerspective) {
            auto bounds = geometry.bounds();
            float tl = std::get<1>(localToDevice.scaleFactors({bounds.left(), bounds.top()}));
            float tr = std::get<1>(localToDevice.scaleFactors({bounds.right(), bounds.top()}));
            float br = std::get<1>(localToDevice.scaleFactors({bounds.right(), bounds.bot()}));
            float bl = std::get<1>(localToDevice.scaleFactors({bounds.left(), bounds.bot()}));
            maxScaleFactor = std::max(std::max(tl, tr), std::max(bl, br));
        }

        style.setResScale(maxScaleFactor);
        SkPathBuilder builder;
        if (pathEffect->filterPath(&builder, geometry.shape().asPath(),
                                   &style, nullptr, localToDevice)) {
            SkPath dst = builder.detach();
            dst.setIsVolatile(true);
            geometry.setShape(Shape(dst));
        } else {
            SKGPU_LOG_W("Path effect failed to apply, drawing original path.");
        }

        // Fallthrough, remaining code assumes the effect has been applied to `geometry` and `style`
    }

    this->drawGeometry(localToDevice, std::move(geometry), paint, style);
}

void Device::drawGeometry(const Transform& localToDevice,
                          Geometry&& geometry,
                          const PaintParams& paint,
                          SkStrokeRec style) {
    ASSERT_SINGLE_OWNER

    if (!localToDevice.valid()) {
        // If the transform is not invertible or not finite then drawing isn't well defined.
        SKGPU_LOG_W("Skipping draw with non-invertible/non-finite transform.");
        return;
    }

    // TODO: The tessellating and atlas path renderers haven't implemented perspective yet, so
    // transform to device space so we draw something approximately correct (barring local coord
    // issues).
    if (geometry.isShape() && localToDevice.type() == Transform::Type::kPerspective &&
        !is_simple_shape(geometry.shape(), localToDevice, style.getStyle())) {
        SkPath devicePath = geometry.shape().asPath().makeTransform(localToDevice.matrix().asM33());
        devicePath.setIsVolatile(true);
        // TODO(b/452415460): This fallback breaks perspective interpolation for local coords and
        // it causes strokes to render in device space.
        this->drawGeometry(Transform::Identity(), Geometry(Shape(devicePath)), paint, style);
        return;
    }

    ScopedDrawBuilder scopedDrawBuilder(fRecorder);

    // Calculate the clipped bounds of the draw and determine the clip elements that affect the
    // draw without updating the clip stack.
    ClipStack::ElementList clipElements;
    Clip clip = fClip.visitClipStackForDraw(localToDevice,
                                            &geometry,
                                            style,
                                            fMSAASupported,
                                            &clipElements);
    if (clip.isClippedOut()) {
        // Clipped out, so don't record anything.
        return;
    }

    // We assume that we will receive a renderer, or a PathAtlas. If it's a PathAtlas, then we
    // assume that the renderer chosen in PathAtlas::addShape() will have single-channel coverage,
    // require AA bounds outsetting, and have a single renderStep. The clip's draw bounds are passed
    // in for heuristics, so it's fine if it doesn't include the AA outsetting we add for some
    // analytic coverage renderers.
    auto [renderer, pathAtlas] = this->chooseRenderer(localToDevice,
                                                      geometry,
                                                      style,
                                                      clip.transformedShapeBounds(),
                                                      /*requireMSAA=*/false);
    if (!renderer && !pathAtlas) {
        SKGPU_LOG_W("Skipping draw with no supported renderer or PathAtlas.");
        return;
    }

    // Update the pixel bounds of the draw to include any outsets done by the renderer (or that
    // must be included in the pixels required when using an atlas). This is important so that
    // all bounds overlap checks take into account pixels touched by rasterization, even if the
    // calculated coverage for a pixel is 0.
    if (!renderer || renderer->outsetBoundsForAA()) {
        clip.outsetBoundsForAA();
    }

    // A renderer that emits a primitive color should only be used by a drawX() call that sets a
    // non-null primitive blender.
    SkASSERT(SkToBool(paint.primitiveBlender()) == (renderer && renderer->emitsPrimitiveColor()));

    ShadingParams shading{fRecorder->priv().caps(),
                          paint,
                          clip.nonMSAAClip(),
                          clip.shader(),
                          renderer ? renderer->coverage() : Coverage::kSingleChannel,
                          TextureInfoPriv::ViewFormat(fDC->target()->textureInfo())};

    // Some shapes and styles combine multiple draws so the total render step count is split between
    // the main renderer and possibly a secondaryRenderer. As we can't be sure whether a secondary
    // renderer is required prior to getting the dstUsage from shading.toKey(), we pessimistically
    // assume it's required for needsFlushBeforeDraw().
    int numNewRenderSteps = 1;
    SkStrokeRec::Style styleType = style.getStyle();
    if (renderer) {
        numNewRenderSteps = renderer->numRenderSteps();
        if (styleType == SkStrokeRec::kStrokeAndFill_Style) {
            numNewRenderSteps +=
                fRecorder->priv().rendererProvider()->tessellatedStrokes()->numRenderSteps();
        } else if (style.isFillStyle() && renderer->useNonAAInnerFill()) {
            numNewRenderSteps +=
                fRecorder->priv().rendererProvider()->nonAABounds()->numRenderSteps();
        }
    }

    // Decide if we have any reason to flush pending work. A flush may be necessary for two reasons:
    //      1) A flush is required before updating the clip state or making any permanent changes to
    //         a path atlas, since otherwise clip operations and/or atlas entries for the current
    //         draw will be flushed.
    //      2) A flush is required before shading.toKey() is called so that child tasks required by
    //         this draw are associated with the DrawContext after any instead of being added as a
    //         child of the current draw. See "Layer" tests in NotifyInUseTest.cpp.
    DstReadStrategy dstReadStrategy = shading.dstReadRequired() ?
                                      fDC->dstReadStrategy() : DstReadStrategy::kNoneRequired;
    const bool needsFlush = this->needsFlushBeforeDraw(numNewRenderSteps, dstReadStrategy);
    if (needsFlush) {
        if (pathAtlas != nullptr) {
            // We need to flush work for all devices associated with the current Recorder.
            // Otherwise we may end up with outstanding draws that depend on past atlas state.
            fRecorder->priv().flushTrackedDevices(
                    SK_DUMP_TASKS_CODE("Device::drawGeometry Flush Before Draw"));
        } else {
            this->flushPendingWork(/*drawContext=*/nullptr);
        }
    }

    // Determine the paint ID and collect the paint uniforms now before anything has been recorded.
    // The paint may reference an SkPicture or a Graphite-backed dynamic SkImage that can trigger
    // a flush of the Recorder.
    KeyContext keyContext{fRecorder,
                          fDC.get(),
                          fRecorder->priv().floatStorageManager(),
                          scopedDrawBuilder.builder(),
                          scopedDrawBuilder.gatherer(),
                          localToDevice.matrix(),
                          fDC->colorInfo(),
                          geometry.isShape() || geometry.isEdgeAAQuad()
                                ? KeyGenFlags::kDefault
                                : KeyGenFlags::kDisableSamplingOptimization,
                          paint.color()};
    auto keyResult = shading.toKey(keyContext);
    if (!keyResult) {
        // Converting the SkPaint to a pipeline and set of uniform values + sampled textures failed.
        SKGPU_LOG_W("Key context creation failed in Device::drawGeometry, draw dropped!");
        return;
    }
    auto [paintID, dstUsage] = *keyResult;

    // If we are unclipped, do not depend on the dst, and cover the target, then we can adjust
    // load ops of the renderpass to more optimally handle the draw (and avoid redundant clears).
    // NOTE: We skip this for fully-lazy render targets because the load ops may impact a larger
    // area than the Device's theoretical bounds.
    const bool overwritesAllPixels = dstUsage == DstUsage::kNone &&
                                     geometry.isShape() &&
                                     geometry.shape().isFloodFill() &&
                                     !fDC->target()->isFullyLazy() &&
                                     clipElements.empty() &&
                                     clip.scissor().contains(this->bounds());
    if (overwritesAllPixels) {
        if (std::optional<SkColor4f> color = extract_paint_color(paint, fDC->colorInfo())) {
            // Fullscreen clear, so nothing has to be rendered at all
            fDC->clear(*color);
            return;
        } else {
            // This paint does not depend on the destination and covers the entire surface, so
            // discard everything previously recorded and proceed with the draw. However, if we are
            // here because of a paint with src-over blending that just happens to be opaque, the
            // discarded dst can still be accessed. For non-floating point formats, that is fine,
            // but float formats can have NaNs after a discard that cause blending to fail. To
            // avoid that scenario, we clear to a known value instead.
            if (paint.finalBlendMode() == SkBlendMode::kSrcOver &&
                TextureFormatIsFloatingPoint(
                        TextureInfoPriv::ViewFormat(fDC->target()->textureInfo()))) {
                fDC->clear(SkColors::kMagenta); // This color doesn't matter
            } else {
                fDC->discard();
            }
            // But then continue to render the flood fill with shading
        }
    }

    // If an atlas path renderer was chosen we need to insert the shape into the atlas and schedule
    // it to be drawn.
    std::optional<PathAtlas::MaskAndOrigin> atlasMask;  // only used if `pathAtlas != nullptr`
    if (pathAtlas != nullptr) {
        Rect clippedShapeBounds = clip.transformedShapeBounds().makeIntersect(clip.scissor());
        if (clippedShapeBounds.area() >= 0.8f * clip.transformedShapeBounds().area()) {
            // The clip isn't excluding very much of the original shape, so store the entire path
            // in the atlas to avoid redundant entries with slightly different clips.
            clippedShapeBounds = clip.transformedShapeBounds();
        }
        std::tie(renderer, atlasMask) = pathAtlas->addShape(clippedShapeBounds,
                                                            geometry.shape(),
                                                            localToDevice,
                                                            style);

        // If there was no space in the atlas and we haven't flushed already, then flush pending
        // work to clear up space in the atlas. If we had already flushed once (which would have
        // cleared the atlas) then the atlas is too small for this shape.
        if (!atlasMask && !needsFlush) {
            // We need to flush work for all devices associated with the current Recorder.
            // Otherwise we may end up with outstanding draws that depend on past atlas state.
            fRecorder->priv().flushTrackedDevices(
                    SK_DUMP_TASKS_CODE("Device::drawGeometry Atlas Flush"));

            // Try inserting the shape again.
            std::tie(renderer, atlasMask) = pathAtlas->addShape(clippedShapeBounds,
                                                                geometry.shape(),
                                                                localToDevice,
                                                                style);
        }

        if (!atlasMask) {
            SKGPU_LOG_E("Failed to add shape to atlas!");
            // TODO(b/285195175): This can happen if the atlas is not large enough or a compatible
            // atlas texture cannot be created. Handle the first case in `chooseRenderer` and make
            // sure that the atlas path renderer is not chosen if the path is larger than the atlas
            // texture.
            return;
        }
        // Since addShape() was successful we should have a valid Renderer now.
        SkASSERT(renderer && renderer->numRenderSteps() == 1 && !renderer->emitsPrimitiveColor());
        fAtlasedPathCount++;
    }

#if defined(SK_DEBUG)
    // Renderers and their component RenderSteps have flexibility in defining their
    // DepthStencilSettings. However, the clipping and ordering managed between Device and ClipStack
    // requires that only LESS or LEQUAL depth tests are used for draws recorded through the
    // client-facing, painters-order-oriented API. We assert here vs. in Renderer's constructor to
    // allow internal-oriented Renderers that are never selected for a "regular" draw call to have
    // more flexibility in their settings.
    SkASSERT(renderer);
    for (const RenderStep* step : renderer->steps()) {
        auto dss = step->depthStencilSettings();
        SkASSERT((!step->performsShading() || dss.fDepthTestEnabled) &&
                 (!dss.fDepthTestEnabled ||
                  dss.fDepthCompareOp == CompareOp::kLess ||
                  dss.fDepthCompareOp == CompareOp::kLEqual));
    }
#endif

    // Update the clip stack after issuing a flush (if it was needed). A draw will be recorded after
    // this point.
    DrawOrder order(fCurrentDepth.next());
    CompressedPaintersOrder clipOrder = fClip.updateClipStateForDraw(
            clip, clipElements, fColorDepthBoundsManager.get(), order.depth());

    // A draw's order always depends on the clips that must be drawn before it
    order.dependsOnPaintersOrder(clipOrder);
    // If a draw is not opaque, it must be drawn after the most recent draw it intersects with in
    // order to blend correctly.
    if (shading.rendererCoverage() != Coverage::kNone || dstUsage != DstUsage::kNone) {
        CompressedPaintersOrder prevDraw =
            fColorDepthBoundsManager->getMostRecentDraw(clip.drawBounds());
        order.dependsOnPaintersOrder(prevDraw);
    }

    // Now that the base paint order and draw bounds are finalized, if the Renderer relies on the
    // stencil attachment, we compute a secondary sorting field to allow disjoint draws to reorder
    // the RenderSteps across draws instead of in sequence for each draw.
    if (renderer->depthStencilFlags() & DepthStencilFlags::kStencil) {
        DisjointStencilIndex setIndex = fDisjointStencilSet->add(order.paintOrder(),
                                                                 clip.drawBounds());
        order.dependsOnStencil(setIndex);
    } else if (dstUsage == DstUsage::kNone && renderer->coverage() == Coverage::kNone &&
               style.isFillStyle() &&
               ((geometry.isEdgeAAQuad() && geometry.edgeAAQuad().isRect()) ||
                (geometry.isShape() && geometry.shape().isRect()))) {
        // Sort this draw front to back since it will not blend against what came before it.
        // We could do this for all opaque/non-blending draws but that can hurt the performance of
        // the std::sort in DrawPass::Make if it has to effectively reverse a large list. For now,
        // limit it to filled rectangles (here and for the later non-AA inner fill).
        order.reverseDepthAsStencil();
    }

    // If an atlas path renderer was chosen, then record a single CoverageMaskShape draw.
    // The shape will be scheduled to be rendered or uploaded into the atlas during the
    // next invocation of flushPendingWork().
    if (pathAtlas != nullptr) {
        // Record the draw as a fill since stroking is handled by the atlas render/upload.
        SkASSERT(atlasMask.has_value());
        auto [mask, origin] = *atlasMask;
        fDC->recordDraw(renderer, Transform::Translate(origin.fX, origin.fY), Geometry(mask), clip,
                        order, paintID, dstUsage, scopedDrawBuilder.gatherer(), nullptr);
    } else {
        if (styleType == SkStrokeRec::kStroke_Style ||
            styleType == SkStrokeRec::kHairline_Style ||
            styleType == SkStrokeRec::kStrokeAndFill_Style) {
            // For stroke-and-fill, 'renderer' is used for the fill and we always use the
            // TessellatedStrokes renderer; for stroke and hairline, 'renderer' is used.
            StrokeStyle stroke(style.getWidth(), style.getMiter(), style.getJoin(), style.getCap());
            fDC->recordDraw(styleType == SkStrokeRec::kStrokeAndFill_Style
                                   ? fRecorder->priv().rendererProvider()->tessellatedStrokes()
                                   : renderer,
                            localToDevice, geometry, clip, order, paintID, dstUsage,
                            scopedDrawBuilder.gatherer(), &stroke);
        }
        if (styleType == SkStrokeRec::kFill_Style ||
            styleType == SkStrokeRec::kStrokeAndFill_Style) {
            // Possibly record an additional draw using the non-AA bounds renderer to fill the
            // interior with a renderer that can disable blending entirely.
            Rect innerFillBounds = renderer->useNonAAInnerFill() && dstUsage == DstUsage::kNone &&
                styleType != SkStrokeRec::kStrokeAndFill_Style
                            ? get_inner_bounds(geometry, localToDevice)
                            : Rect::InfiniteInverted();
            if (!innerFillBounds.isEmptyNegativeOrNaN()) {
                SkASSERT(dstUsage == DstUsage::kNone && renderer->useNonAAInnerFill());
                DrawOrder orderWithoutCoverage{order.depth()};
                orderWithoutCoverage.dependsOnPaintersOrder(clipOrder);
                // The regular draw has analytic coverage, so isn't being sorted front to back, but
                // we do want to sort the inner fill to maximize overdraw reduction
                orderWithoutCoverage.reverseDepthAsStencil();
                fDC->recordDraw(fRecorder->priv().rendererProvider()->nonAABounds(), localToDevice,
                                Geometry(Shape(innerFillBounds)), clip, orderWithoutCoverage,
                                paintID, dstUsage, scopedDrawBuilder.gatherer(), nullptr);
                // Force the coverage draw to come after the non-AA draw in order to benefit from
                // early depth testing.
                order.dependsOnPaintersOrder(orderWithoutCoverage.paintOrder());
            }
            fDC->recordDraw(renderer, localToDevice, geometry, clip, order, paintID, dstUsage,
                            scopedDrawBuilder.gatherer(), nullptr);
        }
    }

    // Post-draw book keeping (bounds manager, depth tracking, etc.)
    fColorDepthBoundsManager->recordDraw(clip.drawBounds(), order.paintOrder());
    fCurrentDepth = order.depth();

    // TODO(b/238758897): When we enable layer elision that depends on draws not overlapping, we
    // can use the `getMostRecentDraw()` query to determine that, although that will mean querying
    // even if the draw does not depend on dst (so should be only be used when the Device is an
    // elision candidate).
}

void Device::drawClipShape(const Transform& localToDevice,
                           const Shape& shape,
                           const Clip& clip,
                           DrawOrder order) {
    ScopedDrawBuilder scopedDrawBuilder(fRecorder);

    // A clip draw's state is almost fully defined by the ClipStack. The only thing we need
    // to account for is selecting a Renderer and tracking the stencil buffer usage.
    Geometry geometry{shape};
    auto [renderer, pathAtlas] = this->chooseRenderer(localToDevice,
                                                      geometry,
                                                      DefaultFillStyle(),
                                                      clip.transformedShapeBounds(),
                                                      /*requireMSAA=*/true);
    if (!renderer) {
        SKGPU_LOG_W("Skipping clip with no supported path renderer.");
        return;
    } else if (renderer->depthStencilFlags() & DepthStencilFlags::kStencil) {
        DisjointStencilIndex setIndex = fDisjointStencilSet->add(order.paintOrder(),
                                                                 clip.drawBounds());
        order.dependsOnStencil(setIndex);
    }

    // This call represents one of the deferred clip shapes that's already pessimistically counted
    // in needsFlushBeforeDraw(), so the DrawContext should have room to add it.
    SkASSERT(fDC->pendingRenderSteps() + renderer->numRenderSteps() < DrawList::kMaxRenderSteps);

    // Anti-aliased clipping requires the renderer to use MSAA to modify the depth per sample, so
    // analytic coverage renderers cannot be used.
    SkASSERT(renderer->coverage() == Coverage::kNone && renderer->requiresMSAA());
    SkASSERT(pathAtlas == nullptr);

    // Clips draws are depth-only (invalid UniquePaintParamsID), and filled (null StrokeStyle).
    // The data gatherer must be reset so that the DrawList can use it for any RenderStep data.
    if (localToDevice.type() == Transform::Type::kPerspective) {
        SkPath devicePath = geometry.shape().asPath().makeTransform(localToDevice.matrix().asM33());
        fDC->recordDraw(renderer, Transform::Identity(), Geometry(Shape(devicePath)), clip, order,
                        UniquePaintParamsID::Invalid(), DstUsage::kNone,
                        scopedDrawBuilder.gatherer(), /*stroke=*/nullptr);
    } else {
        fDC->recordDraw(renderer, localToDevice, geometry, clip, order,
                        UniquePaintParamsID::Invalid(), DstUsage::kNone,
                        scopedDrawBuilder.gatherer(), /*stroke=*/nullptr);
    }
    // This ensures that draws recorded after this clip shape has been popped off the stack will
    // be unaffected by the Z value the clip shape wrote to the depth attachment.
    if (order.depth() > fCurrentDepth) {
        fCurrentDepth = order.depth();
    }
}

// TODO: Currently all Renderers are always defined, but with config options and caps that may not
// be the case, in which case chooseRenderer() will have to go through compatible choices.
std::pair<const Renderer*, PathAtlas*> Device::chooseRenderer(const Transform& localToDevice,
                                                              const Geometry& geometry,
                                                              const SkStrokeRec& style,
                                                              const Rect& drawBounds,
                                                              bool requireMSAA) const {
    const RendererProvider* renderers = fRecorder->priv().rendererProvider();
    SkASSERT(renderers);
    SkStrokeRec::Style type = style.getStyle();

    if (geometry.isSubRun()) {
        SkASSERT(!requireMSAA);
        sktext::gpu::RendererData rendererData = geometry.subRunData().rendererData();
        if (!rendererData.isSDF) {
            return {renderers->bitmapText(rendererData.isLCD, rendererData.maskFormat), nullptr};
        }
        // Even though the SkPaint can request subpixel rendering, we still need to match
        // this with the pixel geometry.
        bool useLCD = rendererData.isLCD &&
                      geometry.subRunData().pixelGeometry() != kUnknown_SkPixelGeometry;
        return {renderers->sdfText(useLCD), nullptr};
    } else if (geometry.isVertices()) {
        SkVerticesPriv info(geometry.vertices()->priv());
        return {renderers->vertices(info.mode(), info.hasColors(), info.hasTexCoords()), nullptr};
    } else if (geometry.isCoverageMaskShape()) {
        // drawCoverageMask() passes in CoverageMaskShapes that reference a provided texture.
        // The CoverageMask renderer can also be chosen later on if the shape is assigned to
        // to be rendered into the PathAtlas, in which case the 2nd return value is non-null.
        return {renderers->coverageMask(), nullptr};
    } else if (geometry.isEdgeAAQuad()) {
        SkASSERT(!requireMSAA && style.isFillStyle());
        // handled by specialized system, simplified from rects and round rects
        const EdgeAAQuad& quad = geometry.edgeAAQuad();
        if (quad.isRect() && (quad.edgeFlags() == EdgeAAQuad::Flags::kNone
#if !defined(SK_SKIP_PIXELALIGNED_QUAD_CHECK_GRAPHITE)
                              || is_pixel_aligned(quad.bounds(), localToDevice)
#endif
                             )) {
            // For non-AA rectangular quads, it can always use a coverage-less renderer; there's no
            // need to check for pixel alignment to avoid popping if MSAA is turned on because quad
            // tile edges will seam with each in either mode. We also switch to use the cover bounds
            // when the quad is pixel aligned to be consistent with drawRect Renderer handling.
            return {renderers->nonAABounds(), nullptr};
        } else {
            return {renderers->perEdgeAAQuad(), nullptr};
        }
    } else if (geometry.isAnalyticBlur()) {
        return {renderers->analyticBlur(), nullptr};
    } else if (!geometry.isShape()) {
        // We must account for new Geometry types with specific Renderers
        return {nullptr, nullptr};
    }

    const Shape& shape = geometry.shape();
    // We can't use this renderer if we require MSAA for an effect (i.e. clipping or stroke+fill).
    if (!requireMSAA && is_simple_shape(shape, localToDevice, type)) {
        // For pixel-aligned rects, use the the non-AA bounds renderer to avoid triggering any
        // dst-read requirement due to src blending.
        bool pixelAlignedRect = false;
        if (shape.isRect() && style.isFillStyle()) {
            pixelAlignedRect = is_pixel_aligned(shape.rect(), localToDevice);
        }

        if (shape.isEmpty() || pixelAlignedRect) {
            SkASSERT(!shape.isEmpty() || shape.inverted());
            return {renderers->nonAABounds(), nullptr};
        } else {
            return {renderers->analyticRRect(), nullptr};
        }
    }

    if (!requireMSAA &&
        shape.isArc() &&
        std::abs(shape.arc().sweepAngle()) < 360.f &&
        localToDevice.type() <= Transform::Type::kAffine &&
        SkRRectPriv::IsRelativelyCircular(shape.arc().oval().width(), shape.arc().oval().height(),
                                          Shape::kDefaultPixelTolerance *
                                                localToDevice.localAARadius(drawBounds))) {
        // We aren't perspective, so the point passed to scaleFactors() doesn't matter
        auto [minScale, maxScale] = localToDevice.scaleFactors({0, 0});
        if (SkScalarNearlyEqual(maxScale, minScale)) {
            // Arc support depends on the style.
            SkStrokeRec::Style recStyle = style.getStyle();
            switch (recStyle) {
                case SkStrokeRec::kStrokeAndFill_Style:
                    // This produces a strange result that this op doesn't implement.
                    break;
                case SkStrokeRec::kFill_Style:
                    return {renderers->circularArc(), nullptr};
                case SkStrokeRec::kStroke_Style:
                case SkStrokeRec::kHairline_Style:
                    // Strokes that don't use the center point are supported with butt & round caps.
                    bool isWedge = shape.arc().isWedge();
                    bool isSquareCap = style.getCap() == SkPaint::kSquare_Cap;
                    if (!isWedge && !isSquareCap) {
                        return {renderers->circularArc(), nullptr};
                    }
                    break;
            }
        }
    }

    // Path rendering options. For now the strategy is very simple and not optimal:
    // I. Use tessellation if MSAA is required for an effect.
    // II: otherwise:
    //    1. Always use compute AA if supported unless it was excluded by ContextOptions or the
    //       compute renderer cannot render the shape efficiently yet (based on the result of
    //       `isSuitableForAtlasing`).
    //    2. Fall back to CPU raster AA if hardware MSAA is disabled or it was explicitly requested
    //       via ContextOptions (including if the path is small enough).
    //    3. Otherwise use tessellation.
    std::optional<PathRendererStrategy> strategy;
#if defined(GPU_TEST_UTILS)
    strategy = fRecorder->priv().caps()->requestedPathRendererStrategy();
#endif

    PathAtlas* pathAtlas = nullptr;
    AtlasProvider* atlasProvider = fRecorder->priv().atlasProvider();

    // Prefer compute atlas draws if supported. This currently implicitly filters out clip draws as
    // they require MSAA. Eventually we may want to route clip shapes to the atlas as well but not
    // if hardware MSAA is required.
    if (atlasProvider->isAvailable(AtlasProvider::PathAtlasFlags::kCompute) &&
        use_compute_atlas_when_available(strategy)) {
        PathAtlas* atlas = fDC->getComputePathAtlas(fRecorder);
        SkASSERT(atlas);

        // Don't use the compute renderer if it can't handle the shape efficiently.
        if (atlas->isSuitableForAtlasing(drawBounds, fClip.conservativeBounds())) {
            pathAtlas = atlas;
        }
    }

    // Fall back to CPU rendered paths when multisampling is disabled and the compute atlas is not
    // available.
    static constexpr int kMaxSmallPathAtlasCount = 256;
    const float minPathSizeForMSAA = fRecorder->priv().caps()->minPathSizeForMSAA();
    const bool useRasterAtlasByDefault = !fMSAASupported ||
                                         (fAtlasedPathCount < kMaxSmallPathAtlasCount &&
                                          all(drawBounds.size() <= minPathSizeForMSAA));
    if (!pathAtlas && atlasProvider->isAvailable(AtlasProvider::PathAtlasFlags::kRaster) &&
        (strategy == PathRendererStrategy::kRasterAA ||
         (!strategy.has_value() && useRasterAtlasByDefault))) {
        // NOTE: RasterPathAtlas doesn't implement `PathAtlas::isSuitableForAtlasing` as it doesn't
        // reject paths (unlike ComputePathAtlas).
        pathAtlas = atlasProvider->getRasterPathAtlas();
    }

    if (!requireMSAA && pathAtlas) {
        // If we got here it means that we should draw with an atlas renderer if we can and avoid
        // resorting to one of the tessellating techniques.
        return {nullptr, pathAtlas};
    }

    // If we got here, it requires tessellated path rendering or an MSAA technique applied to a
    // simple shape (so we interpret them as paths to reduce the number of pipelines we need).

    // TODO: All shapes that select a tessellating path renderer need to be "pre-chopped" if they
    // are large enough to exceed the fixed count tessellation limits. Fills are pre-chopped to the
    // viewport bounds, strokes and stroke-and-fills are pre-chopped to the viewport bounds outset
    // by the stroke radius (hence taking the whole style and not just its type).

    if (type == SkStrokeRec::kStroke_Style ||
        type == SkStrokeRec::kHairline_Style) {
        // Unlike in Ganesh, the HW stroke tessellator can work with arbitrary paints since the
        // depth test prevents double-blending when there is transparency, thus we can HW stroke
        // any path regardless of its paint.
        // TODO: We treat inverse-filled strokes as regular strokes. We could handle them by
        // stenciling first with the HW stroke tessellator and then covering their bounds, but
        // inverse-filled strokes are not well-specified in our public canvas behavior so we may be
        // able to remove it.
        return {renderers->tessellatedStrokes(), nullptr};
    }

    // 'type' could be kStrokeAndFill, but in that case chooseRenderer() is meant to return the
    // fill renderer since tessellatedStrokes() will always be used for the stroke pass.
    if (shape.convex() && !shape.inverted()) {
        // TODO: Ganesh doesn't have a curve+middle-out triangles option for convex paths, but it
        // would be pretty trivial to spin up.
        return {renderers->convexTessellatedWedges(), nullptr};
    } else {
        const bool preferWedges =
                // TODO: Combine this heuristic with what is used in PathStencilCoverOp to choose
                // between wedges curves consistently in Graphite and Ganesh.
                (shape.isPath() && shape.path().countVerbs() < 50) ||
                drawBounds.area() <= (256 * 256);

        if (preferWedges) {
            return {renderers->stencilTessellatedWedges(shape.fillType()), nullptr};
        } else {
            return {renderers->stencilTessellatedCurvesAndTris(shape.fillType()), nullptr};
        }
    }
}

sk_sp<Task> Device::lastDrawTask() const {
    SkASSERT(this->isScratchDevice());
    return fLastTask;
}

void Device::flushPendingWork(DrawContext* drawContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    // If this is a scratch device being flushed, it should only be flushing into the expected
    // next recording from when the Device was first created.
    SkASSERT(fRecorder);
    SkASSERT(fScopedRecordingID == 0 || fScopedRecordingID == fRecorder->priv().nextRecordingID());

    // TODO(b/330864257):  flushPendingWork() can be recursively called if this Device
    // recorded a picture shader draw and during a flush (triggered by snap or automatically from
    // reaching limits), the picture shader will be rendered to a new device. If that picture drawn
    // to the temporary device fills up an atlas it can trigger the global
    // recorder->flushTrackedDevices(), which will then encounter this device that is already in
    // the midst of flushing. To avoid crashing we only actually flush the first time this is called
    // and set a bit to early-out on any recursive calls.
    // This is not an ideal solution since the temporary Device's flush-the-world may have reset
    // atlas entries that the current Device's flushed draws will reference. But at this stage it's
    // not possible to split the already recorded draws into a before-list and an after-list that
    // can reference the old and new contents of the atlas. While avoiding the crash, this may cause
    // incorrect accesses to a shared atlas. Once paint data is extracted at draw time, picture
    // shaders will be resolved outside of flushes and then this will be fixed automatically.
    if (fIsFlushing) {
        return;
    } else {
        fIsFlushing = true;
    }

    // Ideally we would just check if `drawTask` was non-null and then call flushTrackedDevices()
    // before we appended `drawTask` afterwards. Unfortunately, internalFlush() is not 100% internal
    // because it can record atlas uploads to the DrawContext. If those uploads were moved to
    // `drawTask` before flushTrackedDevices() is called, any other Devices would incorrectly assume
    // that the uploads would be executed before their tasks, even though that's not the case here.
    if (fDC->modifiesTarget() && fMustFlushDependencies) {
        // If this is a client-owned Device that has also been used as an image in the same Recorder
        // we need flush all tracked devices that have pending reads from this Device, because those
        // need to be resolved *before* `drawTask` would be executed and modify its texture state.
        fMustFlushDependencies = false;
        fRecorder->priv().flushTrackedDevices(this->target());
    }

    this->internalFlush();
    sk_sp<Task> drawTask = fDC->snapDrawTask();
    if (drawContext) {
        drawContext->recordDependency(std::move(drawTask));
    } else {
        if (this->isScratchDevice()) {
            // TODO(b/323887221): Once shared atlas resources are less brittle, scratch devices
            // won't flush to the recorder at all and will only store the snapped task here.
            fLastTask = drawTask;
        } else {
            // Non-scratch devices do not need to point back to the last snapped task since they are
            // always added to the root task list.
            // TODO: It is currently possible for scratch devices to be flushed and instantiated
            // before their work is finished, meaning they will produce additional tasks to be
            // included in a follow-up Recording:
            // https://chat.google.com/room/AAAA2HlH94I/YU0XdFqX2Uw. However, in this case they no
            // longer appear scratch because the first Recording instantiated the targets. When
            // scratch devices are not actually registered with the Recorder and are only included
            // when they are drawn (e.g. restored), we should be able to assert that `fLastTask` is
            // null.
            fLastTask = nullptr;
        }

        if (drawTask) {
            fRecorder->priv().add(std::move(drawTask));
        }
    }

    fIsFlushing = false;
}

void Device::internalFlush() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    ASSERT_SINGLE_OWNER

    // Push any pending uploads from the atlas provider that pending draws reference.
    fRecorder->priv().atlasProvider()->recordUploads(fDC.get());

    // Clip shapes are depth-only draws, but aren't recorded in the DrawContext until a flush in
    // order to determine the Z values for each element.
    fClip.recordDeferredClipDraws();

    // Flush all pending items to the internal task list and reset Device tracking state
    fDC->flush(fRecorder);

    fColorDepthBoundsManager->reset();
    fDisjointStencilSet->reset();
    fCurrentDepth = DrawOrder::kClearDepth;
    fAtlasedPathCount = 0;

    // Any cleanup in the AtlasProvider
    fRecorder->priv().atlasProvider()->compact();
}

bool Device::needsFlushBeforeDraw(int numNewRenderSteps, DstReadStrategy dstReadStrategy) {
    // Must also account for the elements in the clip stack that might need to be recorded.
    numNewRenderSteps += fClip.maxDeferredClipDraws() * Renderer::kMaxRenderSteps;
    bool needsFlush =
            // Need flush if we don't have room to record into the current list.
            (DrawList::kMaxRenderSteps - fDC->pendingRenderSteps()) < numNewRenderSteps ||
            // Need flush if this draw needs to copy the dst surface for reading.
            dstReadStrategy == DstReadStrategy::kTextureCopy;

    return needsFlush;
}

void Device::drawSpecial(SkSpecialImage* special,
                         const SkMatrix& localToDevice,
                         const SkSamplingOptions& sampling,
                         const SkPaint& paint,
                         SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(!paint.getMaskFilter() && !paint.getImageFilter());

    sk_sp<SkImage> img = special->asImage();
    if (!img || !as_IB(img)->isGraphiteBacked()) {
        SKGPU_LOG_W("Couldn't get Graphite-backed special image as image");
        return;
    }

    // drawSpecial could be the same as drawEdgeAAImageSet or drawImageRect except that it
    // ignores the currently assigned local-to-device transform and uses the one provided.
    // But because SkSpecialImage guarantees the subset is already contained in the image and it's
    // not empty, we can skip the src/dst correction.
    SkRect src = SkRect::Make(special->subset());
    SkRect dst = SkRect::MakeIWH(special->width(), special->height());
    SkASSERT(img->bounds().contains(src) && !dst.isEmpty());

    SkMatrix localMatrix = *SkMatrix::Rect2Rect(src, dst);
    SkRect subset = constraint == SkCanvas::kStrict_SrcRectConstraint ?
            src : SkRect::Make(img->bounds());
    PaintParams::SimpleImage imageShader{img.get(), &localMatrix, subset, sampling};

    // The image filtering and layer code paths often rely on the paint being non-AA to avoid
    // coverage operations. To stay consistent with the other backends, we use an edge AA "quad"
    // whose flags match the paint's AA request.
    EdgeAAQuad::Flags aaFlags = paint.isAntiAlias() ? EdgeAAQuad::Flags::kAll
                                                    : EdgeAAQuad::Flags::kNone;
    this->drawGeometry(Transform(SkM44(localToDevice)),
                       Geometry(EdgeAAQuad(dst, aaFlags)),
                       PaintParams(paint, imageShader),
                       DefaultFillStyle());
}

void Device::drawCoverageMask(const SkSpecialImage* mask,
                              const SkMatrix& localToDevice,
                              const SkSamplingOptions& sampling,
                              const SkPaint& paint) {
    CoverageMaskShape::MaskInfo maskInfo{/*fTextureOrigin=*/{SkTo<uint16_t>(mask->subset().fLeft),
                                                             SkTo<uint16_t>(mask->subset().fTop)},
                                         /*fMaskSize=*/{SkTo<uint16_t>(mask->width()),
                                                        SkTo<uint16_t>(mask->height())}};

    auto maskProxyView = AsView(mask->asImage());
    if (!maskProxyView) {
        SKGPU_LOG_W("Couldn't get Graphite-backed special image as texture proxy view");
        return;
    }

    // Every other "Image" draw reaches the underlying texture via AddToKey/NotifyInUse, which
    // handles notifying the image and either flushing the linked surface or attaching draw tasks
    // from a scratch device to the current draw context. In this case, 'mask' is very likely to
    // be linked to a scratch device, but we must perform the same notifyInUse manually here because
    // the texture is consumed by the RenderStep and not part of the PaintParams.
    static_cast<Image_Base*>(mask->asImage().get())->notifyInUse(fRecorder, fDC.get());

    // CoverageMaskShape() wraps a Shape when it's used as a PathAtlas, but in this case the
    // original shape has been long lost, so just use a Rect that bounds the image.
    CoverageMaskShape maskShape{Shape{Rect::WH((float)mask->width(), (float)mask->height())},
                                // We store a ref to the textureProxy to keep it alive.
                                maskProxyView.refProxy(),
                                // Use the active local-to-device transform for this since it
                                // determines the local coords for evaluating the skpaint, whereas
                                // the provided 'localToDevice' just places the coverage mask.
                                this->localToDeviceTransform().inverse(),
                                maskInfo};

    this->drawGeometry(Transform(SkM44(localToDevice)),
                       Geometry(maskShape),
                       PaintParams(paint),
                       DefaultFillStyle());
}

sk_sp<SkSpecialImage> Device::snapSpecial(const SkIRect& subset, bool forceCopy) {
    // NOTE: snapSpecial() can be called even after the device has been marked immutable (null
    // recorder), but in those cases it should not be a copy and just returns the image view.
    sk_sp<Image> deviceImage;
    SkIRect finalSubset;
    if (forceCopy || !this->readSurfaceView() || this->readSurfaceView().proxy()->isFullyLazy()) {
        deviceImage = this->makeImageCopy(
                subset, Budgeted::kYes, Mipmapped::kNo, SkBackingFit::kApprox);
        finalSubset = SkIRect::MakeSize(subset.size());
    } else {
        // TODO(b/323886870): For now snapSpecial() force adds the pending work to the recorder's
        // root task list. Once shared atlas management is solved and DrawTasks can be nested in a
        // graph then this can go away in favor of auto-flushing through the image's linked device.
        if (fRecorder) {
            this->flushPendingWork(/*drawContext=*/nullptr);
        }
        deviceImage = Image::WrapDevice(sk_ref_sp(this));
        finalSubset = subset;
    }

    if (!deviceImage) {
        return nullptr;
    }

    // For non-copying "snapSpecial", the semantics are returning an image view of the surface data,
    // and relying on higher-level draw and restore logic for the contents to make sense.
    return SkSpecialImages::MakeGraphite(
            fRecorder, finalSubset, std::move(deviceImage), this->surfaceProps());
}

sk_sp<skif::Backend> Device::createImageFilteringBackend(const SkSurfaceProps& surfaceProps,
                                                         SkColorType colorType) const {
    return skif::MakeGraphiteBackend(fRecorder, surfaceProps, colorType);
}

TextureProxy* Device::target() { return fDC->target(); }

TextureProxyView Device::readSurfaceView() const { return fDC->readSurfaceView(); }

bool Device::isScratchDevice() const {
    // Scratch device status is inferred from whether or not the Device's target is instantiated.
    // By default devices start out un-instantiated unless they are wrapping an existing backend
    // texture (definitely not a scratch scenario), or Surface explicitly instantiates the target
    // before returning to the client (not a scratch scenario).
    //
    // Scratch device targets are instantiated during the prepareResources() phase of
    // Recorder::snap(). Truly scratch devices that have gone out of scope as intended will have
    // already been destroyed at this point. Scratch devices that become longer-lived (linked to
    // a client-owned object) automatically transition to non-scratch usage.
    return !fDC->target()->isInstantiated() && !fDC->target()->isLazy();
}

sk_sp<sktext::gpu::Slug> Device::convertGlyphRunListToSlug(const sktext::GlyphRunList& glyphRunList,
                                                           const SkPaint& paint) {
    return sktext::gpu::SlugImpl::Make(this->localToDevice(),
                                       glyphRunList,
                                       paint,
                                       this->strikeDeviceInfo(),
                                       SkStrikeCache::GlobalStrikeCache());
}

void Device::drawSlug(SkCanvas* canvas, const sktext::gpu::Slug* slug, const SkPaint& paint) {
    auto slugImpl = static_cast<const sktext::gpu::SlugImpl*>(slug);
    slugImpl->subRuns()->draw(canvas, slugImpl->origin(), paint, slugImpl, this->atlasDelegate());
}

bool Device::drawBlurredRRect(const SkRRect& rrect, const SkPaint& paint, float deviceSigma) {
    if (skgpu::BlurIsEffectivelyIdentity(deviceSigma)) {
        this->drawRRect(rrect, paint);
        return true;
    }

    std::optional<AnalyticBlurMask> analyticBlur = AnalyticBlurMask::Make(
            this->recorder(), this->localToDeviceTransform(), deviceSigma, rrect);
    if (!analyticBlur) {
        return false;
    }

    this->drawGeometry(this->localToDeviceTransform(),
                       Geometry(*analyticBlur),
                       PaintParams(paint),
                       SkStrokeRec(paint));
    return true;
}

} // namespace skgpu::graphite
