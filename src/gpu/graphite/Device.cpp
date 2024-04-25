/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Device.h"

#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "include/private/gpu/graphite/ContextOptionsPriv.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/RasterPathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/IntersectionTree.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"
#include "src/gpu/graphite/text/TextAtlasManager.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStrokeRec.h"

#include "src/core/SkBlenderBase.h"
#include "src/core/SkBlurMaskFilterImpl.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkVerticesPriv.h"
#include "src/gpu/TiledTextureUtils.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"
#include "src/text/gpu/VertexFiller.h"

#include <functional>
#include <tuple>
#include <unordered_map>
#include <vector>

using RescaleGamma       = SkImage::RescaleGamma;
using RescaleMode        = SkImage::RescaleMode;
using ReadPixelsCallback = SkImage::ReadPixelsCallback;
using ReadPixelsContext  = SkImage::ReadPixelsContext;

#if defined(GRAPHITE_TEST_UTILS)
int gOverrideMaxTextureSizeGraphite = 0;
// Allows tests to check how many tiles were drawn on the most recent call to
// Device::drawAsTiledImageRect. This is an atomic because we can write to it from
// multiple threads during "normal" operations. However, the tests that actually
// read from it are done single-threaded.
std::atomic<int> gNumTilesDrawnGraphite{0};
#endif

namespace skgpu::graphite {

#define ASSERT_SINGLE_OWNER SkASSERT(fRecorder); SKGPU_ASSERT_SINGLE_OWNER(fRecorder->singleOwner())

namespace {

const SkStrokeRec& DefaultFillStyle() {
    static const SkStrokeRec kFillStyle(SkStrokeRec::kFill_InitStyle);
    return kFillStyle;
}

bool blender_depends_on_dst(const SkBlender* blender, bool srcIsTransparent) {
    std::optional<SkBlendMode> bm = blender ? as_BB(blender)->asBlendMode() : SkBlendMode::kSrcOver;
    if (!bm.has_value()) {
        return true;
    }
    if (bm.value() == SkBlendMode::kSrc || bm.value() == SkBlendMode::kClear) {
        // src and clear blending never depends on dst
        return false;
    }
    if (bm.value() == SkBlendMode::kSrcOver) {
        // src-over depends on dst if src is transparent (a != 1)
        return srcIsTransparent;
    }
    // TODO: Are their other modes that don't depend on dst that can be trivially detected?
    return true;
}

bool paint_depends_on_dst(SkColor4f color,
                          const SkShader* shader,
                          const SkColorFilter* colorFilter,
                          const SkBlender* finalBlender,
                          const SkBlender* primitiveBlender) {
    const bool srcIsTransparent = !color.isOpaque() || (shader && !shader->isOpaque()) ||
                                  (colorFilter && !colorFilter->isAlphaUnchanged());

    if (primitiveBlender && blender_depends_on_dst(primitiveBlender, srcIsTransparent)) {
        return true;
    }

    return blender_depends_on_dst(finalBlender, srcIsTransparent);
}

bool paint_depends_on_dst(const PaintParams& paintParams) {
    return paint_depends_on_dst(paintParams.color(),
                                paintParams.shader(),
                                paintParams.colorFilter(),
                                paintParams.finalBlender(),
                                paintParams.primitiveBlender());
}

bool paint_depends_on_dst(const SkPaint& paint) {
    // CAUTION: getMaskFilter is intentionally ignored here.
    SkASSERT(!paint.getImageFilter());  // no paints in SkDevice should have an image filter
    return paint_depends_on_dst(paint.getColor4f(),
                                paint.getShader(),
                                paint.getColorFilter(),
                                paint.getBlender(),
                                /*primitiveBlender=*/nullptr);
}

/** If the paint can be reduced to a solid flood-fill, determine the correct color to fill with. */
std::optional<SkColor4f> extract_paint_color(const SkPaint& paint,
                                             const SkColorInfo& dstColorInfo) {
    SkASSERT(!paint_depends_on_dst(paint));
    if (paint.getShader()) {
        return std::nullopt;
    }

    SkColor4f dstPaintColor = PaintParams::Color4fPrepForDst(paint.getColor4f(), dstColorInfo);

    if (SkColorFilter* filter = paint.getColorFilter()) {
        SkColorSpace* dstCS = dstColorInfo.colorSpace();
        return filter->filterColor4f(dstPaintColor, dstCS, dstCS);
    }
    return dstPaintColor;
}

SkIRect rect_to_pixelbounds(const Rect& r) {
    return r.makeRoundOut().asSkIRect();
}

bool is_simple_shape(const Shape& shape, SkStrokeRec::Style type) {
    // We send regular filled and hairline [round] rectangles, stroked/hairline lines, and stroked
    // [r]rects with circular corners to a single Renderer that does not trigger MSAA.
    // Per-edge AA quadrilaterals also use the same Renderer but those are not "Shapes".
    return !shape.inverted() && type != SkStrokeRec::kStrokeAndFill_Style &&
            (shape.isRect() ||
             (shape.isLine() && type != SkStrokeRec::kFill_Style) ||
             (shape.isRRect() && (type != SkStrokeRec::kStroke_Style ||
                                  SkRRectPriv::AllCornersCircular(shape.rrect()))));
}

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
                                   backingDimensions, textureInfo, budgeted),
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
        SkDEBUGCODE(, fPreRecorderSentinel(reinterpret_cast<intptr_t>(recorder) - 1))
        , fRecorder(recorder)
        SkDEBUGCODE(, fPostRecorderSentinel(reinterpret_cast<intptr_t>(recorder) + 1))
        , fDC(std::move(dc))
        , fClip(this)
        , fColorDepthBoundsManager(std::make_unique<HybridBoundsManager>(
                  fDC->imageInfo().dimensions(), kGridCellSize, kMaxBruteForceN, kMaxGridSize))
        , fDisjointStencilSet(std::make_unique<IntersectionTreeSet>())
        , fCachedLocalToDevice(SkM44())
        , fCurrentDepth(DrawOrder::kClearDepth)
        , fSDFTControl(recorder->priv().caps()->getSDFTControl(false)) {
    SkASSERT(SkToBool(fDC) && SkToBool(fRecorder));
    if (fRecorder->priv().caps()->defaultMSAASamplesCount() > 1) {
        if (fRecorder->priv().caps()->msaaRenderToSingleSampledSupport()) {
            fMSAASupported = true;
        } else {
            TextureInfo msaaTexInfo =
                   fRecorder->priv().caps()->getDefaultMSAATextureInfo(fDC->target()->textureInfo(),
                                                                       Discardable::kYes);
            fMSAASupported = msaaTexInfo.isValid();
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
        this->flushPendingWorkToRecorder();
        fRecorder->deregisterDevice(this);
        // Abandoning the recorder ensures that there are no further operations that can be recorded
        // and is relied on by Image::notifyInUse() to detect when it can unlink from a Device.
        this->abandonRecorder();
    }
}

const Transform& Device::localToDeviceTransform() {
    if (this->checkLocalToDeviceDirty()) {
        fCachedLocalToDevice = Transform{this->localToDevice44()};
    }
    return fCachedLocalToDevice;
}

SkStrikeDeviceInfo Device::strikeDeviceInfo() const {
    return {this->surfaceProps(), this->scalerContextFlags(), &fSDFTControl};
}

sk_sp<SkDevice> Device::createDevice(const CreateInfo& info, const SkPaint*) {
    // TODO: Inspect the paint and create info to determine if there's anything that has to be
    // modified to support inline subpasses.
    SkSurfaceProps props =
        this->surfaceProps().cloneWithPixelGeometry(info.fPixelGeometry);

    // Skia's convention is to only clear a device if it is non-opaque.
    LoadOp initialLoadOp = info.fInfo.isOpaque() ? LoadOp::kDiscard : LoadOp::kClear;

    return Make(fRecorder,
                info.fInfo,
                skgpu::Budgeted::kYes,
                Mipmapped::kNo,
#if defined(GRAPHITE_USE_APPROX_FIT_FOR_FILTERS)
                SkBackingFit::kApprox,
#else
                SkBackingFit::kExact,
#endif
                props,
                initialLoadOp);
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& ii, const SkSurfaceProps& props) {
    return SkSurfaces::RenderTarget(fRecorder, ii, Mipmapped::kNo, &props);
}

sk_sp<Image> Device::makeImageCopy(const SkIRect& subset,
                                   Budgeted budgeted,
                                   Mipmapped mipmapped,
                                   SkBackingFit backingFit) {
    ASSERT_SINGLE_OWNER
    this->flushPendingWorkToRecorder();

    const SkColorInfo& colorInfo = this->imageInfo().colorInfo();
    TextureProxyView srcView = this->readSurfaceView();
    if (!srcView) {
        // readSurfaceView() returns an empty view when the target is not texturable. Create an
        // equivalent view for the blitting operation.
        Swizzle readSwizzle = fRecorder->priv().caps()->getReadSwizzle(
                colorInfo.colorType(), this->target()->textureInfo());
        srcView = {sk_ref_sp(this->target()), readSwizzle};
    }
    return Image::Copy(fRecorder, srcView, colorInfo, subset, budgeted, mipmapped, backingFit);
}

bool Device::onReadPixels(const SkPixmap& pm, int srcX, int srcY) {
#if defined(GRAPHITE_TEST_UTILS)
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
    return fDC->recordUpload(fRecorder, fDC->refTarget(), src.info().colorInfo(),
                             this->imageInfo().colorInfo(), levels, dstRect, nullptr);
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
            SkPath tmpPath = e.fShape.asPath();
            tmpPath.transform(e.fLocalToDevice);
            tmp.setPath(tmpPath, deviceBounds);
        }

        region->op(tmp, (SkRegion::Op) e.fOp);
    }
}

void Device::clipRect(const SkRect& rect, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    // TODO: Snap rect edges to pixel bounds if non-AA and axis-aligned?
    fClip.clipShape(this->localToDeviceTransform(), Shape{rect}, op);
}

void Device::clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    // TODO: Snap rrect edges to pixel bounds if non-AA and axis-aligned? Is that worth doing to
    // seam with non-AA rects even if the curves themselves are AA'ed?
    fClip.clipShape(this->localToDeviceTransform(), Shape{rrect}, op);
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
        // TODO: Region clips are non-AA so this should match non-AA onClipRect(), but we use a
        // different transform so can't just call that instead.
        fClip.clipShape(globalToDevice, Shape{SkRect::Make(globalRgn.getBounds())}, op);
    } else {
        // TODO: Can we just iterate the region and do non-AA rects for each chunk?
        SkPath path;
        globalRgn.getBoundaryPath(&path);
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
    // We never want to do a fullscreen clear on a fully-lazy render target, because the device size
    // may be smaller than the final surface we draw to, in which case we don't want to fill the
    // entire final surface.
    if (this->isClipWideOpen() && !fDC->target()->isFullyLazy()) {
        if (!paint_depends_on_dst(paint)) {
            if (std::optional<SkColor4f> color = extract_paint_color(paint, fDC->colorInfo())) {
                // do fullscreen clear
                fDC->clear(*color);
                return;
            } else {
                // This paint does not depend on the destination and covers the entire surface, so
                // discard everything previously recorded and proceed with the draw.
                fDC->discard();
            }
        }
    }

    const Transform& localToDevice = this->localToDeviceTransform();
    if (!localToDevice.valid()) {
        // TBD: This matches legacy behavior for drawPaint() that requires local coords, although
        // v1 handles arbitrary transforms when the paint is solid color because it just fills the
        // device bounds directly. In the new world it might be nice to have non-invertible
        // transforms formalized (i.e. no drawing ever, handled at SkCanvas level possibly?)
        return;
    }
    Rect localCoveringBounds = localToDevice.inverseMapRect(fClip.conservativeBounds());
    this->drawGeometry(localToDevice,
                       Geometry(Shape(localCoveringBounds)),
                       paint,
                       DefaultFillStyle(),
                       DrawFlags::kIgnorePathEffect);
}

void Device::drawRect(const SkRect& r, const SkPaint& paint) {
    this->drawGeometry(this->localToDeviceTransform(), Geometry(Shape(r)),
                       paint, SkStrokeRec(paint));
}

void Device::drawVertices(const SkVertices* vertices, sk_sp<SkBlender> blender,
                          const SkPaint& paint, bool skipColorXform)  {
  // TODO - Add GPU handling of skipColorXform once Graphite has its color system more fleshed out.
    this->drawGeometry(this->localToDeviceTransform(),
                       Geometry(sk_ref_sp(vertices)),
                       paint,
                       DefaultFillStyle(),
                       DrawFlags::kIgnorePathEffect,
                       std::move(blender),
                       skipColorXform);
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

#if defined(GRAPHITE_TEST_UTILS)
    if (gOverrideMaxTextureSizeGraphite) {
        maxTextureSize = gOverrideMaxTextureSizeGraphite;
    }
    gNumTilesDrawnGraphite.store(0, std::memory_order_relaxed);
#endif

    [[maybe_unused]] auto [wasTiled, numTiles] =
            skgpu::TiledTextureUtils::DrawAsTiledImageRect(canvas,
                                                           image,
                                                           *src,
                                                           dst,
                                                           SkCanvas::kAll_QuadAAFlags,
                                                           sampling,
                                                           &paint,
                                                           constraint,
                                                           cacheSize,
                                                           maxTextureSize);
#if defined(GRAPHITE_TEST_UTILS)
    gNumTilesDrawnGraphite.store(numTiles, std::memory_order_relaxed);
#endif
    return wasTiled;
}

void Device::drawOval(const SkRect& oval, const SkPaint& paint) {
    if (paint.getPathEffect()) {
        // Dashing requires that the oval path starts on the right side and travels clockwise. This
        // is the default for the SkPath::Oval constructor, as used by SkBitmapDevice.
        this->drawGeometry(this->localToDeviceTransform(), Geometry(Shape(SkPath::Oval(oval))),
                           paint, SkStrokeRec(paint));
    } else {
        // TODO: This has wasted effort from the SkCanvas level since it instead converts rrects
        // that happen to be ovals into this, only for us to go right back to rrect.
        this->drawGeometry(this->localToDeviceTransform(), Geometry(Shape(SkRRect::MakeOval(oval))),
                           paint, SkStrokeRec(paint));
    }
}

void Device::drawRRect(const SkRRect& rr, const SkPaint& paint) {
    this->drawGeometry(this->localToDeviceTransform(), Geometry(Shape(rr)),
                       paint, SkStrokeRec(paint));
}

void Device::drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) {
    // TODO: If we do try to inspect the path, it should happen here and possibly after computing
    // the path effect. Alternatively, all that should be handled in SkCanvas.
    this->drawGeometry(this->localToDeviceTransform(), Geometry(Shape(path)),
                       paint, SkStrokeRec(paint));
}

void Device::drawPoints(SkCanvas::PointMode mode, size_t count,
                        const SkPoint* points, const SkPaint& paint) {
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

    size_t inc = mode == SkCanvas::kLines_PointMode ? 2 : 1;
    for (size_t i = 0; i < count; i += inc) {
        this->drawGeometry(this->localToDeviceTransform(),
                           Geometry(Shape(points[i], points[i + next])),
                           paint, stroke);
    }
}

void Device::drawEdgeAAQuad(const SkRect& rect,
                            const SkPoint clip[4],
                            SkCanvas::QuadAAFlags aaFlags,
                            const SkColor4f& color,
                            SkBlendMode mode) {
    SkPaint solidColorPaint;
    solidColorPaint.setColor4f(color, /*colorSpace=*/nullptr);
    solidColorPaint.setBlendMode(mode);

    auto flags = SkEnumBitMask<EdgeAAQuad::Flags>(static_cast<EdgeAAQuad::Flags>(aaFlags));
    EdgeAAQuad quad = clip ? EdgeAAQuad(clip, flags) : EdgeAAQuad(rect, flags);
    this->drawGeometry(this->localToDeviceTransform(),
                       Geometry(quad),
                       solidColorPaint,
                       DefaultFillStyle(),
                       DrawFlags::kIgnorePathEffect);
}

void Device::drawEdgeAAImageSet(const SkCanvas::ImageSetEntry set[], int count,
                                const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                const SkSamplingOptions& sampling, const SkPaint& paint,
                                SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(count > 0);

    SkPaint paintWithShader(paint);
    int dstClipIndex = 0;
    for (int i = 0; i < count; ++i) {
        // If the entry is clipped by 'dstClips', that must be provided
        SkASSERT(!set[i].fHasClip || dstClips);
        // Similarly, if it has an extra transform, those must be provided
        SkASSERT(set[i].fMatrixIndex < 0 || preViewMatrices);

        auto [ imageToDraw, newSampling ] =
                skgpu::graphite::GetGraphiteBacked(this->recorder(), set[i].fImage.get(), sampling);
        if (!imageToDraw) {
            SKGPU_LOG_W("Device::drawImageRect: Creation of Graphite-backed image failed");
            return;
        }

        // TODO: Produce an image shading paint key and data directly without having to reconstruct
        // the equivalent SkPaint for each entry. Reuse the key and data between entries if possible
        paintWithShader.setShader(paint.refShader());
        paintWithShader.setAlphaf(paint.getAlphaf() * set[i].fAlpha);
        SkRect dst = SkModifyPaintAndDstForDrawImageRect(
                    imageToDraw.get(), newSampling, set[i].fSrcRect, set[i].fDstRect,
                    constraint == SkCanvas::kStrict_SrcRectConstraint,
                    &paintWithShader);
        if (dst.isEmpty()) {
            return;
        }

        auto flags =
                SkEnumBitMask<EdgeAAQuad::Flags>(static_cast<EdgeAAQuad::Flags>(set[i].fAAFlags));
        EdgeAAQuad quad = set[i].fHasClip ? EdgeAAQuad(dstClips + dstClipIndex, flags)
                                          : EdgeAAQuad(dst, flags);

        // TODO: Calling drawGeometry() for each entry re-evaluates the clip stack every time, which
        // is consistent with Ganesh's behavior. It also matches the behavior if edge-AA images were
        // submitted one at a time by SkiaRenderer (a nice client simplification). However, we
        // should explore the performance trade off with doing one bulk evaluation for the whole set
        if (set[i].fMatrixIndex < 0) {
            this->drawGeometry(this->localToDeviceTransform(),
                               Geometry(quad),
                               paintWithShader,
                               DefaultFillStyle(),
                               DrawFlags::kIgnorePathEffect);
        } else {
            SkM44 xtraTransform(preViewMatrices[set[i].fMatrixIndex]);
            this->drawGeometry(this->localToDeviceTransform().concat(xtraTransform),
                               Geometry(quad),
                               paintWithShader,
                               DefaultFillStyle(),
                               DrawFlags::kIgnorePathEffect);
        }

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
    this->drawEdgeAAImageSet(&single, 1, nullptr, nullptr, sampling, paint, constraint);
}

sktext::gpu::AtlasDrawDelegate Device::atlasDelegate() {
    return [&](const sktext::gpu::AtlasSubRun* subRun,
               SkPoint drawOrigin,
               const SkPaint& paint,
               sk_sp<SkRefCnt> subRunStorage,
               sktext::gpu::RendererData rendererData) {
        this->drawAtlasSubRun(subRun, drawOrigin, paint, subRunStorage, rendererData);
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
            auto [bounds, localToDevice] = subRun->vertexFiller().boundsAndDeviceMatrix(
                                                   this->localToDeviceTransform(), drawOrigin);
            SkPaint subRunPaint = paint;
            // For color emoji, only the paint alpha affects the final color
            if (subRun->maskFormat() == skgpu::MaskFormat::kARGB) {
                subRunPaint.setColor(SK_ColorWHITE);
                subRunPaint.setAlphaf(paint.getAlphaf());
            }
            this->drawGeometry(localToDevice,
                               Geometry(SubRunData(subRun,
                                                   subRunStorage,
                                                   bounds,
                                                   this->localToDeviceTransform().inverse(),
                                                   subRunCursor,
                                                   glyphsRegenerated,
                                                   fRecorder,
                                                   rendererData)),
                               subRunPaint,
                               DefaultFillStyle(),
                               DrawFlags::kIgnorePathEffect);
        }
        subRunCursor += glyphsRegenerated;

        if (subRunCursor < subRunEnd) {
            // Flush if not all the glyphs are handled because the atlas is out of space.
            // We flush every Device because the glyphs that are being flushed/referenced are not
            // necessarily specific to this Device. This addresses both multiple SkSurfaces within
            // a Recorder, and nested layers.
            TRACE_EVENT_INSTANT0("skia.gpu", "Glyph atlas full", TRACE_EVENT_SCOPE_NAME_THREAD);
            fRecorder->priv().flushTrackedDevices();
        }
    }
}

void Device::drawGeometry(const Transform& localToDevice,
                          const Geometry& geometry,
                          const SkPaint& paint,
                          const SkStrokeRec& style,
                          SkEnumBitMask<DrawFlags> flags,
                          sk_sp<SkBlender> primitiveBlender,
                          bool skipColorXform) {
    ASSERT_SINGLE_OWNER

    if (!localToDevice.valid()) {
        // If the transform is not invertible or not finite then drawing isn't well defined.
        SKGPU_LOG_W("Skipping draw with non-invertible/non-finite transform.");
        return;
    }

    // Heavy weight paint options like path effects, mask filters, and stroke-and-fill style are
    // applied on the CPU by generating a new shape and recursing on drawGeometry with updated flags
    if (!(flags & DrawFlags::kIgnorePathEffect) && paint.getPathEffect()) {
        // Apply the path effect before anything else, which if we are applying here, means that we
        // are dealing with a Shape. drawVertices (and a SkVertices geometry) should pass in
        // kIgnorePathEffect per SkCanvas spec. Text geometry also should pass in kIgnorePathEffect
        // because the path effect is applied per glyph by the SkStrikeSpec already.
        SkASSERT(geometry.isShape());

        // TODO: If asADash() returns true and the base path matches the dashing fast path, then
        // that should be detected now as well. Maybe add dashPath to Device so canvas can handle it
        SkStrokeRec newStyle = style;
        float maxScaleFactor = localToDevice.maxScaleFactor();
        if (localToDevice.type() == Transform::Type::kPerspective) {
            auto bounds = geometry.bounds();
            float tl = std::get<1>(localToDevice.scaleFactors({bounds.left(), bounds.top()}));
            float tr = std::get<1>(localToDevice.scaleFactors({bounds.right(), bounds.top()}));
            float br = std::get<1>(localToDevice.scaleFactors({bounds.right(), bounds.bot()}));
            float bl = std::get<1>(localToDevice.scaleFactors({bounds.left(), bounds.bot()}));
            maxScaleFactor = std::max(std::max(tl, tr), std::max(bl, br));
        }
        newStyle.setResScale(maxScaleFactor);
        SkPath dst;
        if (paint.getPathEffect()->filterPath(&dst, geometry.shape().asPath(), &newStyle,
                                              nullptr, localToDevice)) {
            dst.setIsVolatile(true);
            // Recurse using the path and new style, while disabling downstream path effect handling
            this->drawGeometry(localToDevice, Geometry(Shape(dst)), paint, newStyle,
                               flags | DrawFlags::kIgnorePathEffect, std::move(primitiveBlender),
                               skipColorXform);
            return;
        } else {
            SKGPU_LOG_W("Path effect failed to apply, drawing original path.");
            this->drawGeometry(localToDevice, geometry, paint, style,
                               flags | DrawFlags::kIgnorePathEffect, std::move(primitiveBlender),
                               skipColorXform);
            return;
        }
    }

    // TODO: The tessellating and atlas path renderers haven't implemented perspective yet, so
    // transform to device space so we draw something approximately correct (barring local coord
    // issues).
    if (geometry.isShape() && localToDevice.type() == Transform::Type::kPerspective &&
        !is_simple_shape(geometry.shape(), style.getStyle())) {
        SkPath devicePath = geometry.shape().asPath();
        devicePath.transform(localToDevice.matrix().asM33());
        devicePath.setIsVolatile(true);
        this->drawGeometry(Transform::Identity(), Geometry(Shape(devicePath)), paint, style, flags,
                           std::move(primitiveBlender), skipColorXform);
        return;
    }

    // TODO: Manually snap pixels for rects, rrects, and lines if paint is non-AA (ideally also
    // consider snapping stroke width and/or adjusting geometry for hairlines). This pixel snapping
    // math should be consistent with how non-AA clip [r]rects are handled.

    // If we got here, then path effects should have been handled and the style should be fill or
    // stroke/hairline. Stroke-and-fill is not handled by DrawContext, but is emulated here by
    // drawing twice--one stroke and one fill--using the same depth value.
    SkASSERT(!SkToBool(paint.getPathEffect()) || (flags & DrawFlags::kIgnorePathEffect));

    // TODO: Some renderer decisions could depend on the clip (see PathAtlas::addShape for
    // one workaround) so we should figure out how to remove this circular dependency.

    // We assume that we will receive a renderer, or a PathAtlas. If it's a PathAtlas,
    // then we assume that the renderer chosen in PathAtlas::addShape() will have
    // single-channel coverage, require AA bounds outsetting, and have a single renderStep.
    auto [renderer, pathAtlas] =
            this->chooseRenderer(localToDevice, geometry, style, /*requireMSAA=*/false);
    if (!renderer && !pathAtlas) {
        SKGPU_LOG_W("Skipping draw with no supported renderer or PathAtlas.");
        return;
    }

    // Calculate the clipped bounds of the draw and determine the clip elements that affect the
    // draw without updating the clip stack.
    const bool outsetBoundsForAA = renderer ? renderer->outsetBoundsForAA() : true;
    ClipStack::ElementList clipElements;
    const Clip clip =
            fClip.visitClipStackForDraw(localToDevice, geometry, style, outsetBoundsForAA,
                                        &clipElements);
    if (clip.isClippedOut()) {
        // Clipped out, so don't record anything.
        return;
    }

    // Figure out what dst color requirements we have, if any.
    DstReadRequirement dstReadReq = DstReadRequirement::kNone;
    const SkBlenderBase* blender = as_BB(paint.getBlender());
    const std::optional<SkBlendMode> blendMode = blender ? blender->asBlendMode()
                                                         : SkBlendMode::kSrcOver;
    const Coverage rendererCoverage = renderer ? renderer->coverage()
                                               : Coverage::kSingleChannel;
    dstReadReq = GetDstReadRequirement(recorder()->priv().caps(), blendMode, rendererCoverage);

    // When using a tessellating path renderer a stroke-and-fill is rendered using two draws. When
    // drawing from an atlas we issue a single draw as the atlas mask covers both styles.
    SkStrokeRec::Style styleType = style.getStyle();
    const int numNewRenderSteps =
            renderer ? renderer->numRenderSteps() : 1 +
            (!pathAtlas && (styleType == SkStrokeRec::kStrokeAndFill_Style)
                     ? fRecorder->priv().rendererProvider()->tessellatedStrokes()->numRenderSteps()
                     : 0);

    // Decide if we have any reason to flush pending work. We want to flush before updating the clip
    // state or making any permanent changes to a path atlas, since otherwise clip operations and/or
    // atlas entries for the current draw will be flushed.
    const bool needsFlush = this->needsFlushBeforeDraw(numNewRenderSteps, dstReadReq);
    if (needsFlush) {
        if (pathAtlas != nullptr) {
            // We need to flush work for all devices associated with the current Recorder.
            // Otherwise we may end up with outstanding draws that depend on past atlas state.
            fRecorder->priv().flushTrackedDevices();
        } else {
            this->flushPendingWorkToRecorder();
        }
    }

    // If an atlas path renderer was chosen we need to insert the shape into the atlas and schedule
    // it to be drawn.
    std::optional<PathAtlas::MaskAndOrigin> atlasMask;  // only used if `pathAtlas != nullptr`
    if (pathAtlas != nullptr) {
        std::tie(renderer, atlasMask) = pathAtlas->addShape(clip.transformedShapeBounds(),
                                                            geometry.shape(),
                                                            localToDevice,
                                                            style);

        // If there was no space in the atlas and we haven't flushed already, then flush pending
        // work to clear up space in the atlas. If we had already flushed once (which would have
        // cleared the atlas) then the atlas is too small for this shape.
        if (!atlasMask && !needsFlush) {
            // We need to flush work for all devices associated with the current Recorder.
            // Otherwise we may end up with outstanding draws that depend on past atlas state.
            fRecorder->priv().flushTrackedDevices();

            // Try inserting the shape again.
            std::tie(renderer, atlasMask) = pathAtlas->addShape(clip.transformedShapeBounds(),
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
        SkASSERT(renderer);
    }

    // Update the clip stack after issuing a flush (if it was needed). A draw will be recorded after
    // this point.
    DrawOrder order(fCurrentDepth.next());
    CompressedPaintersOrder clipOrder = fClip.updateClipStateForDraw(
            clip, clipElements, fColorDepthBoundsManager.get(), order.depth());

#if defined(SK_DEBUG)
    // Renderers and their component RenderSteps have flexibility in defining their
    // DepthStencilSettings. However, the clipping and ordering managed between Device and ClipStack
    // requires that only GREATER or GEQUAL depth tests are used for draws recorded through the
    // client-facing, painters-order-oriented API. We assert here vs. in Renderer's constructor to
    // allow internal-oriented Renderers that are never selected for a "regular" draw call to have
    // more flexibility in their settings.
    for (const RenderStep* step : renderer->steps()) {
        auto dss = step->depthStencilSettings();
        SkASSERT((!step->performsShading() || dss.fDepthTestEnabled) &&
                 (!dss.fDepthTestEnabled ||
                  dss.fDepthCompareOp == CompareOp::kGreater ||
                  dss.fDepthCompareOp == CompareOp::kGEqual));
    }
#endif

    // A draw's order always depends on the clips that must be drawn before it
    order.dependsOnPaintersOrder(clipOrder);

    // A primitive blender should be ignored if there is no primitive color to blend against.
    // Additionally, if a renderer emits a primitive color, then a null primitive blender should
    // be interpreted as SrcOver blending mode.
    if (!renderer->emitsPrimitiveColor()) {
        primitiveBlender = nullptr;
    } else if (!SkToBool(primitiveBlender)) {
        primitiveBlender = SkBlender::Mode(SkBlendMode::kSrcOver);
    }

    // If a draw is not opaque, it must be drawn after the most recent draw it intersects with in
    // order to blend correctly. We always query the most recent draw (even when opaque) because it
    // also lets Device easily track whether or not there are any overlapping draws.
    PaintParams shading{paint,
                        std::move(primitiveBlender),
                        sk_ref_sp(clip.shader()),
                        dstReadReq,
                        skipColorXform};
    const bool dependsOnDst = rendererCoverage != Coverage::kNone || paint_depends_on_dst(shading);
    if (dependsOnDst) {
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
    }

    // TODO(b/330864257): This is an extra traversal of all paint effects, that can be avoided when
    // the paint key itself is determined inside this function.
    shading.notifyImagesInUse(fRecorder, fDC.get());

    // If an atlas path renderer was chosen, then record a single CoverageMaskShape draw.
    // The shape will be scheduled to be rendered or uploaded into the atlas during the
    // next invocation of flushPendingWorkToRecorder().
    if (pathAtlas != nullptr) {
        // Record the draw as a fill since stroking is handled by the atlas render/upload.
        SkASSERT(atlasMask.has_value());
        auto [mask, origin] = *atlasMask;
        fDC->recordDraw(renderer, Transform::Translate(origin.fX, origin.fY), Geometry(mask),
                        clip, order, &shading, nullptr);
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
                            localToDevice, geometry, clip, order, &shading, &stroke);
        }
        if (styleType == SkStrokeRec::kFill_Style ||
            styleType == SkStrokeRec::kStrokeAndFill_Style) {
            fDC->recordDraw(renderer, localToDevice, geometry, clip, order, &shading, nullptr);
        }
    }

    // TODO: If 'fullyOpaque' is true, it might be useful to store the draw bounds and Z in a
    // special occluders list for filtering the DrawList/DrawPass when flushing.
    // const bool fullyOpaque = !dependsOnDst &&
    //                          clipOrder == DrawOrder::kNoIntersection &&
    //                          shape.isRect() &&
    //                          localToDevice.type() <= Transform::Type::kRectStaysRect;

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
    // A clip draw's state is almost fully defined by the ClipStack. The only thing we need
    // to account for is selecting a Renderer and tracking the stencil buffer usage.
    Geometry geometry{shape};
    auto [renderer, pathAtlas] = this->chooseRenderer(localToDevice,
                                                      geometry,
                                                      DefaultFillStyle(),
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

    // Clips draws are depth-only (null PaintParams), and filled (null StrokeStyle).
    // TODO: Remove this CPU-transform once perspective is supported for all path renderers
    if (localToDevice.type() == Transform::Type::kPerspective) {
        SkPath devicePath = geometry.shape().asPath();
        devicePath.transform(localToDevice.matrix().asM33());
        fDC->recordDraw(renderer, Transform::Identity(), Geometry(Shape(devicePath)), clip, order,
                        nullptr, nullptr);
    } else {
        fDC->recordDraw(renderer, localToDevice, geometry, clip, order, nullptr, nullptr);
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
                                                              bool requireMSAA) const {
    const RendererProvider* renderers = fRecorder->priv().rendererProvider();
    SkASSERT(renderers);
    SkStrokeRec::Style type = style.getStyle();

    if (geometry.isSubRun()) {
        SkASSERT(!requireMSAA);
        sktext::gpu::RendererData rendererData = geometry.subRunData().rendererData();
        if (!rendererData.isSDF) {
            return {renderers->bitmapText(rendererData.isLCD), nullptr};
        }
        return {renderers->sdfText(rendererData.isLCD), nullptr};
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
        return {renderers->perEdgeAAQuad(), nullptr};
    } else if (geometry.isAnalyticBlur()) {
        return {renderers->analyticBlur(), nullptr};
    } else if (!geometry.isShape()) {
        // We must account for new Geometry types with specific Renderers
        return {nullptr, nullptr};
    }

    const Shape& shape = geometry.shape();
    // We can't use this renderer if we require MSAA for an effect (i.e. clipping or stroke+fill).
    if (!requireMSAA && is_simple_shape(shape, type)) {
        return {renderers->analyticRRect(), nullptr};
    }

    // Path rendering options. For now the strategy is very simple and not optimal:
    // I. Use tessellation if MSAA is required for an effect.
    // II: otherwise:
    //    1. Always use compute AA if supported unless it was excluded by ContextOptions.
    //    2. Use CPU raster AA if hardware MSAA is disabled or it was explicitly requested by
    //       ContextOptions.
    //    3. Otherwise use tessellation.
#if defined(GRAPHITE_TEST_UTILS)
    PathRendererStrategy strategy = fRecorder->priv().caps()->requestedPathRendererStrategy();
#else
    PathRendererStrategy strategy = PathRendererStrategy::kDefault;
#endif

    PathAtlas* pathAtlas = nullptr;

    // Prefer compute atlas draws if supported. This currently implicitly filters out clip draws as
    // they require MSAA. Eventually we may want to route clip shapes to the atlas as well but not
    // if hardware MSAA is required.
    AtlasProvider* atlasProvider = fRecorder->priv().atlasProvider();
    if (atlasProvider->isAvailable(AtlasProvider::PathAtlasFlags::kCompute) &&
        (strategy == PathRendererStrategy::kComputeAnalyticAA ||
         strategy == PathRendererStrategy::kComputeMSAA16 ||
         strategy == PathRendererStrategy::kComputeMSAA8 ||
         strategy == PathRendererStrategy::kDefault)) {
        pathAtlas = fDC->getComputePathAtlas(fRecorder);
    // Only use CPU rendered paths when multisampling is disabled
    // TODO: enable other uses of the software path renderer
    } else if (atlasProvider->isAvailable(AtlasProvider::PathAtlasFlags::kRaster) &&
               (strategy == PathRendererStrategy::kRasterAA ||
                (strategy == PathRendererStrategy::kDefault && !fMSAASupported))) {
        pathAtlas = atlasProvider->getRasterPathAtlas();
    }

    // Use an atlas only if an MSAA technique isn't required.
    if (!requireMSAA && pathAtlas) {
        // Don't use a coverage mask renderer if the shape is too large for the atlas such that it
        // cannot be efficiently rasterized. The only exception is if hardware MSAA is not supported
        // as a fallback or one of the atlas strategies was explicitly requested.
        //
        // If the hardware doesn't support MSAA and anti-aliasing is required, then we always render
        // paths with atlasing.
        if (!fMSAASupported || strategy != PathRendererStrategy::kDefault) {
            return {nullptr, pathAtlas};
        }

        // Use the conservative clip bounds for a rough estimate of the mask size (this avoids
        // having to evaluate the entire clip stack before choosing the renderer as it will have to
        // get evaluated again if we fall back to a different renderer).
        Rect drawBounds = localToDevice.mapRect(shape.bounds());
        drawBounds.intersect(fClip.conservativeBounds());
        if (pathAtlas->isSuitableForAtlasing(drawBounds)) {
            return {nullptr, pathAtlas};
        }
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
        Rect drawBounds = localToDevice.mapRect(shape.bounds());
        drawBounds.intersect(fClip.conservativeBounds());
        const bool preferWedges =
                // If the draw bounds don't intersect with the clip stack's conservative bounds,
                // we'll be drawing a very small area at most, accounting for coverage, so just
                // stick with drawing wedges in that case.
                drawBounds.isEmptyNegativeOrNaN() ||

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

void Device::flushPendingWorkToRecorder(Recorder* recorder) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    // Confirm sentinels match the original values set from fRecorder
    SkDEBUGCODE(intptr_t expected = reinterpret_cast<intptr_t>(recorder ? recorder : fRecorder);)
    SkASSERT(fPreRecorderSentinel == expected - 1);
    SkASSERT(fPostRecorderSentinel == expected + 1);

    SkASSERT(recorder == nullptr || recorder == fRecorder);
    if (recorder && recorder != fRecorder) {
        // TODO(b/333073673): This should not happen but if the Device were corrupted exit now
        // to avoid further access of Device's state.
        return;
    }

    // If this is a scratch device being flushed, it should only be flushing into the expected
    // next recording from when the Device was first created.
    SkASSERT(fRecorder);
    // TODO(b/333073673):
    // The only time flushPendingWorkToRecorder() is called with a non-null Recorder is from
    // flushTrackedDevices(), so the scoped recording ID of the device should be 0 or it means a
    // non-tracked device got added to the recorder or something has stomped the heap.
    SkASSERT(!recorder || fScopedRecordingID == 0);
    SkASSERT(fScopedRecordingID == 0 || fScopedRecordingID == fRecorder->priv().nextRecordingID());

    // TODO(b/330864257):  flushPendingWorkToRecorder() can be recursively called if this Device
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

    this->internalFlush();
    sk_sp<Task> drawTask = fDC->snapDrawTask(fRecorder);
    if (this->isScratchDevice()) {
        // TODO(b/323887221): Once shared atlas resources are less brittle, scratch devices won't
        // flush to the recorder at all and will only store the snapped task here.
        fLastTask = drawTask;
    } else {
        // Non-scratch devices do not need to point back to the last snapped task since they are
        // always added to the root task list.
        // TODO: It is currently possible for scratch devices to be flushed and instantiated before
        // their work is finished, meaning they will produce additional tasks to be included in
        // a follow-up Recording: https://chat.google.com/room/AAAA2HlH94I/YU0XdFqX2Uw.
        // However, in this case they no longer appear scratch because the first Recording
        // instantiated the targets. When scratch devices are not actually registered with the
        // Recorder and are only included when they are drawn (e.g. restored), we should be able to
        // assert that `fLastTask` is null.
        fLastTask = nullptr;
    }

    if (drawTask) {
        fRecorder->priv().add(std::move(drawTask));

        // TODO(b/297344089): This always regenerates mipmaps on the draw target when it's drawn to.
        // This could be wasteful if we draw to a target multiple times before reading from it with
        // downscaling.
        if (fDC->target()->mipmapped() == Mipmapped::kYes) {
            if (!GenerateMipmaps(fRecorder, fDC->refTarget(), fDC->colorInfo())) {
                SKGPU_LOG_W("Device::flushPendingWorkToRecorder: Failed to generate mipmaps");
            }
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

     // Any cleanup in the AtlasProvider
    fRecorder->priv().atlasProvider()->postFlush();
}

bool Device::needsFlushBeforeDraw(int numNewRenderSteps, DstReadRequirement dstReadReq) const {
    // Must also account for the elements in the clip stack that might need to be recorded.
    numNewRenderSteps += fClip.maxDeferredClipDraws() * Renderer::kMaxRenderSteps;
    return // Need flush if we don't have room to record into the current list.
           (DrawList::kMaxRenderSteps - fDC->pendingRenderSteps()) < numNewRenderSteps ||
           // Need flush if this draw needs to copy the dst surface for reading.
           dstReadReq == DstReadRequirement::kTextureCopy;
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

    SkPaint paintWithShader(paint);
    SkRect dst = SkModifyPaintAndDstForDrawImageRect(
            img.get(),
            sampling,
            /*src=*/SkRect::Make(special->subset()),
            /*dst=*/SkRect::MakeIWH(special->width(), special->height()),
            /*strictSrcSubset=*/constraint == SkCanvas::kStrict_SrcRectConstraint,
            &paintWithShader);
    if (dst.isEmpty()) {
        return;
    }

    this->drawGeometry(Transform(SkM44(localToDevice)),
                       Geometry(Shape(dst)),
                       paintWithShader,
                       DefaultFillStyle(),
                       DrawFlags::kIgnorePathEffect);
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

    // 'mask' logically has 0 coverage outside of its pixels, which is equivalent to kDecal tiling.
    // However, since we draw geometry tightly fitting 'mask', we can use the better-supported
    // kClamp tiling and behave effectively the same way.
    const SkTileMode kClamp[2] = {SkTileMode::kClamp, SkTileMode::kClamp};

    // Ensure this is kept alive; normally textures are kept alive by the PipelineDataGatherer for
    // image shaders, or by the PathAtlas. This is a unique circumstance.
    // TODO: Find a cleaner way to ensure 'maskProxyView' is transferred to the final Recording.
    TextureDataBlock tdb;
    // NOTE: CoverageMaskRenderStep controls the final sampling options; this texture data block
    // serves only to keep the mask alive so the sampling passed to add() doesn't matter.
    tdb.add(SkFilterMode::kLinear, kClamp, maskProxyView.refProxy());
    fRecorder->priv().textureDataCache()->insert(tdb);

    // CoverageMaskShape() wraps a Shape when it's used as a PathAtlas, but in this case the
    // original shape has been long lost, so just use a Rect that bounds the image.
    CoverageMaskShape maskShape{Shape{Rect::WH((float)mask->width(), (float)mask->height())},
                                maskProxyView.proxy(),
                                // Use the active local-to-device transform for this since it
                                // determines the local coords for evaluating the skpaint, whereas
                                // the provided 'localToDevice' just places the coverage mask.
                                this->localToDeviceTransform().inverse(),
                                maskInfo};

    this->drawGeometry(Transform(SkM44(localToDevice)),
                       Geometry(maskShape),
                       paint,
                       DefaultFillStyle(),
                       DrawFlags::kIgnorePathEffect);
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkBitmap&) {
    return nullptr;
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkImage*) {
    return nullptr;
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
            this->flushPendingWorkToRecorder();
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
    SkStrokeRec style(paint);
    if (skgpu::BlurIsEffectivelyIdentity(deviceSigma)) {
        this->drawGeometry(this->localToDeviceTransform(),
                           Geometry(rrect.isRect() ? Shape(rrect.rect()) : Shape(rrect)),
                           paint,
                           style);
        return true;
    }

    std::optional<AnalyticBlurMask> analyticBlur = AnalyticBlurMask::Make(
            this->recorder(), this->localToDeviceTransform(), deviceSigma, rrect);
    if (!analyticBlur) {
        return false;
    }

    this->drawGeometry(this->localToDeviceTransform(), Geometry(*analyticBlur), paint, style);
    return true;
}

} // namespace skgpu::graphite
