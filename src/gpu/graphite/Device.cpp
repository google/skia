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
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/PathAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
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
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkVerticesPriv.h"
#include "src/shaders/SkImageShader.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"
#include "src/text/gpu/VertexFiller.h"

#include <functional>
#include <unordered_map>
#include <vector>

using RescaleGamma       = SkImage::RescaleGamma;
using RescaleMode        = SkImage::RescaleMode;
using ReadPixelsCallback = SkImage::ReadPixelsCallback;
using ReadPixelsContext  = SkImage::ReadPixelsContext;

namespace skgpu::graphite {

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

bool create_img_shader_paint(sk_sp<SkImage> image,
                             const SkRect& subset,
                             SkCanvas::SrcRectConstraint constraint,
                             const SkSamplingOptions& sampling,
                             const SkMatrix* localMatrix,
                             SkPaint* paint) {
    bool imageIsAlphaOnly = SkColorTypeIsAlphaOnly(image->colorType());

    sk_sp<SkShader> imgShader;
    if (constraint == SkCanvas::kStrict_SrcRectConstraint) {
        imgShader = SkImageShader::MakeSubset(std::move(image), subset,
                                              SkTileMode::kClamp, SkTileMode::kClamp,
                                              sampling, localMatrix);
    } else {
        imgShader = image->makeShader(SkTileMode::kClamp, SkTileMode::kClamp,
                                      sampling, localMatrix);
    }
    if (!imgShader) {
        SKGPU_LOG_W("Couldn't create subset image shader");
        return false;
    }
    if (imageIsAlphaOnly && paint->getShader()) {
        // Compose the image shader with the paint's shader. Alpha images+shaders should output the
        // texture's alpha multiplied by the shader's color. DstIn (d*sa) will achieve this with
        // the source image and dst shader (MakeBlend takes dst first, src second).
        imgShader = SkShaders::Blend(SkBlendMode::kDstIn, paint->refShader(), std::move(imgShader));
    }

    paint->setStyle(SkPaint::kFill_Style);
    paint->setShader(std::move(imgShader));
    paint->setPathEffect(nullptr);  // neither drawSpecial nor drawImageRect support path effects
    return true;
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
                           const SkSurfaceProps& props,
                           bool addInitialClear) {
    if (!recorder) {
        return nullptr;
    }

    sk_sp<TextureProxy> target = TextureProxy::Make(recorder->priv().caps(),
                                                    ii.dimensions(),
                                                    ii.colorType(),
                                                    mipmapped,
                                                    Protected::kNo,
                                                    Renderable::kYes,
                                                    budgeted);
    if (!target) {
        return nullptr;
    }

    return Make(recorder, std::move(target), ii.colorInfo(), props, addInitialClear);
}

sk_sp<Device> Device::Make(Recorder* recorder,
                           sk_sp<TextureProxy> target,
                           const SkColorInfo& colorInfo,
                           const SkSurfaceProps& props,
                           bool addInitialClear) {
    return Make(recorder, target, target->dimensions(), colorInfo, props, addInitialClear);
}

sk_sp<Device> Device::Make(Recorder* recorder,
                           sk_sp<TextureProxy> target,
                           SkISize deviceSize,
                           const SkColorInfo& colorInfo,
                           const SkSurfaceProps& props,
                           bool addInitialClear) {
    if (!recorder) {
        return nullptr;
    }
    // We don't render to unknown or unpremul alphatypes
    if (colorInfo.alphaType() == kUnknown_SkAlphaType ||
        colorInfo.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }

    sk_sp<DrawContext> dc = DrawContext::Make(std::move(target), deviceSize, colorInfo, props);
    if (!dc) {
        return nullptr;
    }

    return sk_sp<Device>(new Device(recorder, std::move(dc), addInitialClear));
}

// These default tuning numbers for the HybridBoundsManager were chosen from looking at performance
// and accuracy curves produced by the BoundsManagerBench for random draw bounding boxes. This
// config will use brute force for the first 64 draw calls to the Device and then switch to a grid
// that is dynamically sized to produce cells that are 16x16, which seemed to be in the sweet spot
// for maintaining good performance without becoming too inaccurate.
// TODO: These could be exposed as context options or surface options, and we may want to have
// different strategies in place for a base device vs. a layer's device.
static constexpr int kGridCellSize = 16;
static constexpr int kMaxBruteForceN = 64;

Device::Device(Recorder* recorder, sk_sp<DrawContext> dc, bool addInitialClear)
        : SkDevice(dc->imageInfo(), dc->surfaceProps())
        , fRecorder(recorder)
        , fDC(std::move(dc))
        , fClip(this)
        , fColorDepthBoundsManager(
                    std::make_unique<HybridBoundsManager>(fDC->imageInfo().dimensions(),
                                                          kGridCellSize,
                                                          kMaxBruteForceN))
        , fDisjointStencilSet(std::make_unique<IntersectionTreeSet>())
        , fCachedLocalToDevice(SkM44())
        , fCurrentDepth(DrawOrder::kClearDepth)
        , fSDFTControl(recorder->priv().caps()->getSDFTControl(false))
        , fDrawsOverlap(false) {
    SkASSERT(SkToBool(fDC) && SkToBool(fRecorder));
    fRecorder->registerDevice(this);

    if (addInitialClear) {
        fDC->clear(SkColors::kTransparent);
    }
}

Device::~Device() {
    if (fRecorder) {
        this->flushPendingWorkToRecorder();
        fRecorder->deregisterDevice(this);
    }
}

void Device::abandonRecorder() {
    fRecorder = nullptr;
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
    SkSurfaceProps props(this->surfaceProps().flags(), info.fPixelGeometry);

    // Skia's convention is to only clear a device if it is non-opaque.
    bool addInitialClear = !info.fInfo.isOpaque();

    return Make(fRecorder,
                info.fInfo,
                skgpu::Budgeted::kYes,
                Mipmapped::kNo,
                props,
                addInitialClear);
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& ii, const SkSurfaceProps& props) {
    return SkSurfaces::RenderTarget(fRecorder, ii, Mipmapped::kNo, &props);
}

TextureProxyView Device::createCopy(const SkIRect* subset, Mipmapped mipmapped) {
    this->flushPendingWorkToRecorder();

    TextureProxyView srcView = this->readSurfaceView();
    if (!srcView) {
        return {};
    }

    SkIRect srcRect = subset ? *subset : SkIRect::MakeSize(this->imageInfo().dimensions());
    return TextureProxyView::Copy(this->recorder(),
                                  this->imageInfo().colorInfo(),
                                  srcView,
                                  srcRect,
                                  mipmapped);
}

TextureProxyView TextureProxyView::Copy(Recorder* recorder,
                                        const SkColorInfo& srcColorInfo,
                                        const TextureProxyView& srcView,
                                        SkIRect srcRect,
                                        Mipmapped mipmapped) {
    SkASSERT(srcView.proxy()->isFullyLazy() ||
             SkIRect::MakeSize(srcView.proxy()->dimensions()).contains(srcRect));

    skgpu::graphite::TextureInfo textureInfo =
            recorder->priv().caps()->getTextureInfoForSampledCopy(srcView.proxy()->textureInfo(),
                                                                  mipmapped);
    sk_sp<TextureProxy> dest = TextureProxy::Make(
            recorder->priv().caps(), srcRect.size(), textureInfo, skgpu::Budgeted::kNo);
    if (!dest) {
        return {};
    }

    sk_sp<CopyTextureToTextureTask> copyTask = CopyTextureToTextureTask::Make(srcView.refProxy(),
                                                                              srcRect,
                                                                              dest,
                                                                              {0, 0});
    if (!copyTask) {
        return {};
    }

    recorder->priv().add(std::move(copyTask));

    if (mipmapped == Mipmapped::kYes) {
        if (!GenerateMipmaps(recorder, dest, srcColorInfo)) {
            SKGPU_LOG_W("TextureProxyView::Copy: Failed to generate mipmaps");
        }
    }

    return { std::move(dest), srcView.swizzle() };
}

bool Device::onReadPixels(const SkPixmap& pm, int srcX, int srcY) {
#if defined(GRAPHITE_TEST_UTILS)
    if (Context* context = fRecorder->priv().context()) {
        this->flushPendingWorkToRecorder();
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

    // TODO: check for readOnly or framebufferOnly target and return false if so

    // TODO: canvas2DFastPath?
    // TODO: check that surface supports writePixels
    // TODO: handle writePixels as draw if needed (e.g., canvas2DFastPath || !supportsWritePixels)

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

    this->flushPendingWorkToRecorder();

    return fDC->recordUpload(fRecorder, sk_ref_sp(target), src.info().colorInfo(),
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
    // We never want to do a fullscreen clear on a fully-lazy render target, because the device size
    // may be smaller than the final surface we draw to, in which case we don't want to fill the
    // entire final surface.
    if (this->isClipWideOpen() && !fDC->target()->isFullyLazy()) {
        if (!paint_depends_on_dst(paint)) {
            if (std::optional<SkColor4f> color = extract_paint_color(paint, fDC->colorInfo())) {
                // do fullscreen clear
                fDC->clear(*color);
                return;
            }
            // TODO(michaelludwig): this paint doesn't depend on the destination, so we can reset
            // the DrawContext to use a discard load op. The drawPaint will cover anything else
            // entirely. We still need shader evaluation to get per-pixel colors (since the paint
            // couldn't be reduced to a solid color).
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
                       DrawFlags::kIgnorePathEffect | DrawFlags::kIgnoreMaskFilter);
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
                       DrawFlags::kIgnorePathEffect | DrawFlags::kIgnoreMaskFilter,
                       std::move(blender),
                       skipColorXform);
}

void Device::drawOval(const SkRect& oval, const SkPaint& paint) {
    // TODO: This has wasted effort from the SkCanvas level since it instead converts rrects that
    // happen to be ovals into this, only for us to go right back to rrect.
    this->drawGeometry(this->localToDeviceTransform(), Geometry(Shape(SkRRect::MakeOval(oval))),
                       paint, SkStrokeRec(paint));
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
                       DrawFlags::kIgnoreMaskFilter | DrawFlags::kIgnorePathEffect);
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

        SkRect imgBounds = SkRect::Make(set[i].fImage->bounds());
        SkRect src = set[i].fSrcRect;
        SkRect dst = set[i].fDstRect;

        // TODO: All of this logic should be handled in SkCanvas, since it's the same for every
        // backend.
        SkASSERT(src.isFinite() && dst.isFinite() && dst.isSorted());
        SkMatrix localMatrix = SkMatrix::RectToRect(src, dst);
        if (!imgBounds.contains(src)) {
            if (!src.intersect(imgBounds)) {
                continue; // Nothing to draw for this entry
            }
            // Update dst to match smaller src
            dst = localMatrix.mapRect(src);
        }

        auto [ imageToDraw, newSampling ] =
                skgpu::graphite::GetGraphiteBacked(this->recorder(), set[i].fImage.get(), sampling);
        if (!imageToDraw) {
            SKGPU_LOG_W("Device::drawImageRect: Creation of Graphite-backed image failed");
            return;
        }

        // TODO: Produce an image shading paint key and data directly without having to reconstruct
        // the equivalent SkPaint for each entry. Reuse the key and data between entries if possible
        paintWithShader.setShader(paint.refShader());
        if (!create_img_shader_paint(std::move(imageToDraw), src, constraint, newSampling,
                                     &localMatrix, &paintWithShader)) {
            return;
        }
        paintWithShader.setAlphaf(paint.getAlphaf() * set[i].fAlpha);

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
                                const SkPaint& initialPaint,
                                const SkPaint& drawingPaint) {
    fRecorder->priv().textBlobCache()->drawGlyphRunList(canvas,
                                                        this->localToDevice(),
                                                        glyphRunList,
                                                        drawingPaint,
                                                        this->strikeDeviceInfo(),
                                                        this->atlasDelegate());
}

void Device::drawAtlasSubRun(const sktext::gpu::AtlasSubRun* subRun,
                             SkPoint drawOrigin,
                             const SkPaint& paint,
                             sk_sp<SkRefCnt> subRunStorage,
                             sktext::gpu::RendererData rendererData) {
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
                               DrawFlags::kIgnorePathEffect | DrawFlags::kIgnoreMaskFilter);
        }
        subRunCursor += glyphsRegenerated;

        if (subRunCursor < subRunEnd) {
            // Flush if not all the glyphs are handled because the atlas is out of space.
            // We flush every Device because the glyphs that are being flushed/referenced are not
            // necessarily specific to this Device. This addresses both multiple SkSurfaces within
            // a Recorder, and nested layers.
            ATRACE_ANDROID_FRAMEWORK_ALWAYS("Atlas full");
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
    if (!localToDevice.valid()) {
        // If the transform is not invertible or not finite then drawing isn't well defined.
        SKGPU_LOG_W("Skipping draw with non-invertible/non-finite transform.");
        return;
    }

    // Heavy weight paint options like path effects, mask filters, and stroke-and-fill style are
    // applied on the CPU by generating a new shape and recursing on drawShape() with updated flags
    if (!(flags & DrawFlags::kIgnorePathEffect) && paint.getPathEffect()) {
        // Apply the path effect before anything else, which if we are applying here, means that we
        // are dealing with a Shape. drawVertices (and a SkVertices geometry) should pass in
        // kIgnorePathEffect per SkCanvas spec. Text geometry also should pass in kIgnorePathEffect
        // because the path effect is applied per glyph by the SkStrikeSpec already.
        SkASSERT(geometry.isShape());

        // TODO: If asADash() returns true and the base path matches the dashing fast path, then
        // that should be detected now as well. Maybe add dashPath to Device so canvas can handle it
        SkStrokeRec newStyle = style;
        newStyle.setResScale(localToDevice.maxScaleFactor());
        SkPath dst;
        if (paint.getPathEffect()->filterPath(&dst, geometry.shape().asPath(), &newStyle,
                                              nullptr, localToDevice)) {
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

    if (!(flags & DrawFlags::kIgnoreMaskFilter) && paint.getMaskFilter()) {
        // TODO: Handle mask filters, ignored for the sprint.
        // TODO: Could this be handled by SkCanvas by drawing a mask, blurring, and then sampling
        // with a rect draw? What about fast paths for rrect blur masks...
        this->drawGeometry(localToDevice, geometry, paint, style,
                           flags | DrawFlags::kIgnoreMaskFilter, std::move(primitiveBlender),
                           skipColorXform);
        return;
    }

    // TODO: The tessellating and atlas path renderers haven't implemented perspective yet, so
    // transform to device space so we draw something approximately correct (barring local coord
    // issues).
    if (geometry.isShape() && localToDevice.type() == Transform::Type::kProjection &&
        !is_simple_shape(geometry.shape(), style.getStyle())) {
        SkPath devicePath = geometry.shape().asPath();
        devicePath.transform(localToDevice.matrix().asM33());
        this->drawGeometry(Transform::Identity(), Geometry(Shape(devicePath)), paint, style, flags,
                           std::move(primitiveBlender), skipColorXform);
        return;
    }

    // TODO: Manually snap pixels for rects, rrects, and lines if paint is non-AA (ideally also
    // consider snapping stroke width and/or adjusting geometry for hairlines). This pixel snapping
    // math should be consistent with how non-AA clip [r]rects are handled.

    // If we got here, then path effects and mask filters should have been handled and the style
    // should be fill or stroke/hairline. Stroke-and-fill is not handled by DrawContext, but is
    // emulated here by drawing twice--one stroke and one fill--using the same depth value.
    SkASSERT(!SkToBool(paint.getPathEffect()) || (flags & DrawFlags::kIgnorePathEffect));
    SkASSERT(!SkToBool(paint.getMaskFilter()) || (flags & DrawFlags::kIgnoreMaskFilter));

    auto [renderer, pathAtlas] =
            this->chooseRenderer(localToDevice, geometry, style, /*requireMSAA=*/false);
    if (!renderer) {
        SKGPU_LOG_W("Skipping draw with no supported renderer.");
        return;
    }

    // Calculate the clipped bounds of the draw and determine the clip elements that affect the
    // draw without updating the clip stack.
    ClipStack::ElementList clipElements;
    const Clip clip =
            fClip.visitClipStackForDraw(localToDevice, geometry, style, *renderer, &clipElements);
    if (clip.isClippedOut()) {
        // Clipped out, so don't record anything.
        return;
    }

    // Figure out what dst color requirements we have, if any.
    DstReadRequirement dstReadReq = DstReadRequirement::kNone;
    const SkBlenderBase* blender = as_BB(paint.getBlender());
    const std::optional<SkBlendMode> blendMode = blender ? blender->asBlendMode()
                                                         : SkBlendMode::kSrcOver;
    const Coverage rendererCoverage = renderer->coverage();
    dstReadReq = GetDstReadRequirement(recorder()->priv().caps(), blendMode, rendererCoverage);

    // When using a tessellating path renderer a stroke-and-fill is rendered using two draws. When
    // drawing from an atlas we issue a single draw as the atlas mask covers both styles.
    SkStrokeRec::Style styleType = style.getStyle();
    const int numNewDraws = !pathAtlas && (styleType == SkStrokeRec::kStrokeAndFill_Style) ? 2 : 1;

    // Decide if we have any reason to flush pending work. We want to flush before updating the clip
    // state or making any permanent changes to a path atlas, since otherwise clip operations and/or
    // atlas entries for the current draw will be flushed.
    const bool needsFlush = this->needsFlushBeforeDraw(numNewDraws, dstReadReq);
    if (needsFlush) {
        this->flushPendingWorkToRecorder();
    }

    // If an atlas path renderer was chosen we need to insert the shape into the atlas and schedule
    // it to be drawn.
    AtlasShape::MaskInfo atlasMaskInfo;  // only used if `pathAtlas != nullptr`

    // It is possible for the transformed shape bounds to be fully clipped out while the draw still
    // produces coverage due to an inverse fill. In this case, don't render any mask;
    // AtlasShapeRenderStep will automatically handle the simple fill.
    if (pathAtlas != nullptr && !clip.transformedShapeBounds().isEmptyNegativeOrNaN()) {
        bool foundAtlasSpace = pathAtlas->addShape(recorder(),
                                                   clip.transformedShapeBounds(),
                                                   geometry.shape(),
                                                   localToDevice,
                                                   style,
                                                   &atlasMaskInfo);

        // If there was no space in the atlas and we haven't flushed already, then flush pending
        // work to clear up space in the atlas. If we had already flushed once (which would have
        // cleared the atlas) then the atlas is too small for this shape.
        if (!foundAtlasSpace && !needsFlush) {
            this->flushPendingWorkToRecorder();

            // Try inserting the shape again.
            foundAtlasSpace = pathAtlas->addShape(recorder(),
                                                  clip.transformedShapeBounds(),
                                                  geometry.shape(),
                                                  localToDevice,
                                                  style,
                                                  &atlasMaskInfo);
        }

        if (!foundAtlasSpace) {
            SKGPU_LOG_E("Shape is too large for path atlas!");
            // TODO(b/285195175): This can happen if the atlas is not large enough. Handle this case
            // in `chooseRenderer` and make sure that the atlas path renderer is not chosen if the
            // path is larger than the atlas texture.
            return;
        }
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
    PaintParams shading{paint, std::move(primitiveBlender), dstReadReq, skipColorXform};
    const bool dependsOnDst = rendererCoverage != Coverage::kNone || paint_depends_on_dst(shading);
    CompressedPaintersOrder prevDraw =
            fColorDepthBoundsManager->getMostRecentDraw(clip.drawBounds());
    if (dependsOnDst) {
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

    // If the atlas path renderer was chosen, then schedule the shape to be rendered into the atlas
    // and record a single AtlashShape draw.
    if (pathAtlas != nullptr) {
        // Record the draw as a fill since stroking is handled by the atlas render.
        Geometry atlasShape(
                AtlasShape(geometry.shape(), pathAtlas, localToDevice.inverse(), atlasMaskInfo));
        fDC->recordDraw(
                renderer, Transform::Identity(), atlasShape, clip, order, &shading, nullptr);
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
    fDrawsOverlap |= (prevDraw != DrawOrder::kNoIntersection);
}

void Device::drawClipShape(const Transform& localToDevice,
                           const Shape& shape,
                           const Clip& clip,
                           DrawOrder order) {
    // This call represents one of the deferred clip shapes that's already pessimistically counted
    // in needsFlushBeforeDraw(), so the DrawContext should have room to add it.
    SkASSERT(fDC->pendingDrawCount() + 1 < DrawList::kMaxDraws);

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

    // Anti-aliased clipping requires the renderer to use MSAA to modify the depth per sample, so
    // analytic coverage renderers cannot be used.
    SkASSERT(renderer->coverage() == Coverage::kNone && renderer->requiresMSAA());
    SkASSERT(pathAtlas == nullptr);

    // Clips draws are depth-only (null PaintParams), and filled (null StrokeStyle).
    // TODO: Remove this CPU-transform once perspective is supported for all path renderers
    if (localToDevice.type() == Transform::Type::kProjection) {
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
    } else if (geometry.isEdgeAAQuad()) {
        SkASSERT(!requireMSAA && style.isFillStyle());
        // handled by the same system as rects and round rects
        return {renderers->analyticRRect(), nullptr};
    } else if (!geometry.isShape()) {
        // We must account for new Geometry types with specific Renderers
        return {nullptr, nullptr};
    }

    const Shape& shape = geometry.shape();
    // We can't use this renderer if we require MSAA for an effect (i.e. clipping or stroke+fill).
    if (!requireMSAA && is_simple_shape(shape, type)) {
        return {renderers->analyticRRect(), nullptr};
    }

    // Prefer compute atlas draws if supported. This currently implicitly filters out clip draws as
    // they require MSAA. Eventually we may want to route clip shapes to the atlas as well but not
    // if hardware MSAA is required.
    // TODO(b/285195175): There may be reasons to prefer tessellation, e.g. if the shape is large
    // and hardware MSAA looks acceptable.
    // TODO(b/280927548): Currently we assume `pathAtlas` is a GPU compute path atlas and select it
    // if it's supported (this should be always nullptr if SK_ENABLE_VELLO_SHADERS isn't defined).
    // This will likely need to provide more information about the PathAtlas' rendering algorithm
    // when we support non-compute PathAtlases, which may factor into the renderer choice.
    PathAtlas* pathAtlas = fDC->getOrCreatePathAtlas(fRecorder);
    if (!requireMSAA && pathAtlas) {
        // TODO: vello can't do correct strokes yet. Maybe this shouldn't get selected for stroke
        // renders until all stroke styles are supported?
        return {renderers->atlasShape(), pathAtlas};
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

void Device::flushPendingWorkToRecorder() {
    SkASSERT(fRecorder);

    // TODO: we may need to further split this function up since device->device drawList and
    // DrawPass stealing will need to share some of the same logic w/o becoming a Task.

    // push any pending uploads from the atlasmanager
    auto textAtlasManager = fRecorder->priv().atlasProvider()->textAtlasManager();
    if (!fDC->recordTextUploads(textAtlasManager)) {
        SKGPU_LOG_E("TextAtlasManager uploads have failed -- may see invalid results.");
    }

    auto uploadTask = fDC->snapUploadTask(fRecorder);
    if (uploadTask) {
        fRecorder->priv().add(std::move(uploadTask));
    }

    fClip.recordDeferredClipDraws();

    // Snap the render pass task before snapping the compute task because creating a DrawPass may
    // record DispatchGroups that it depends on (e.g. to process geometry or atlas draws).
    auto drawTask = fDC->snapRenderPassTask(fRecorder);
    auto computeTask = fDC->snapComputeTask(fRecorder);

    // Execute the compute task before the draw task.
    if (computeTask) {
        fRecorder->priv().add(std::move(computeTask));
    }
    if (drawTask) {
        fRecorder->priv().add(std::move(drawTask));
    }

    // Reset accumulated state tracking since everything that it referred to has been moved into
    // an immutable DrawPass.
    fColorDepthBoundsManager->reset();
    fDisjointStencilSet->reset();
    fCurrentDepth = DrawOrder::kClearDepth;
    // NOTE: fDrawsOverlap is not reset here because that is a persistent property of everything
    // drawn into the Device, and not just the currently accumulating pass.
}

bool Device::needsFlushBeforeDraw(int numNewDraws, DstReadRequirement dstReadReq) const {
    // Must also account for the elements in the clip stack that might need to be recorded.
    numNewDraws += fClip.maxDeferredClipDraws();
    return
            // Need flush if we don't have room to record into the current list.
            (DrawList::kMaxDraws - fDC->pendingDrawCount()) < numNewDraws ||
            // Need flush if this draw needs to copy the dst surface for reading.
            dstReadReq == DstReadRequirement::kTextureCopy;
}

void Device::drawSpecial(SkSpecialImage* special,
                         const SkMatrix& localToDevice,
                         const SkSamplingOptions& sampling,
                         const SkPaint& paint) {
    SkASSERT(!paint.getMaskFilter() && !paint.getImageFilter());

    sk_sp<SkImage> img = special->asImage();
    if (!img) {
        SKGPU_LOG_W("Couldn't get Graphite-backed special image as image");
        return;
    }

    // TODO: remove this check once Graphite has image filter support.
    if (!img->isTextureBacked()) {
        return;
    }

    SkRect src = SkRect::Make(special->subset());
    SkRect dst = SkRect::MakeWH(special->width(), special->height());
    SkMatrix srcToDst = SkMatrix::RectToRect(src, dst);
    SkASSERT(srcToDst.isTranslate());

    SkPaint paintWithShader(paint);
    if (!create_img_shader_paint(std::move(img), src, SkCanvas::kStrict_SrcRectConstraint ,sampling,
                                 &srcToDst, &paintWithShader)) {
        return;
    }

    this->drawGeometry(Transform(SkM44(localToDevice)),
                       Geometry(Shape(dst)),
                       paintWithShader,
                       DefaultFillStyle(),
                       DrawFlags::kIgnorePathEffect | DrawFlags::kIgnoreMaskFilter);
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkBitmap&) {
    return nullptr;
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkImage*) {
    return nullptr;
}

sk_sp<SkSpecialImage> Device::snapSpecial(const SkIRect& subset, bool forceCopy) {
    this->flushPendingWorkToRecorder();

    SkIRect finalSubset = subset;
    TextureProxyView view = this->readSurfaceView();
    if (forceCopy || !view || view.proxy()->isFullyLazy()) {
        // TODO: this doesn't address the non-readable surface view case, in which view is empty and
        // createCopy will return an empty view as well.
        view = this->createCopy(&subset, Mipmapped::kNo);
        if (!view) {
            return nullptr;
        }
        finalSubset = SkIRect::MakeWH(view.width(), view.height());
    }

    return SkSpecialImages::MakeGraphite(finalSubset,
                                         kNeedNewImageUniqueID_SpecialImage,
                                         std::move(view),
                                         this->imageInfo().colorInfo(),
                                         this->surfaceProps());
}

skif::Context Device::createContext(const skif::ContextInfo& ctxInfo) const {
    return skif::MakeGraphiteContext(fRecorder, ctxInfo);
}

TextureProxy* Device::target() { return fDC->target(); }

TextureProxyView Device::readSurfaceView() const {
    if (!fRecorder) {
        return {};
    }
    return fDC->readSurfaceView(fRecorder->priv().caps());
}

sk_sp<sktext::gpu::Slug> Device::convertGlyphRunListToSlug(const sktext::GlyphRunList& glyphRunList,
                                                           const SkPaint& initialPaint,
                                                           const SkPaint& drawingPaint) {
    return sktext::gpu::SlugImpl::Make(this->localToDevice(),
                                       glyphRunList,
                                       initialPaint,
                                       drawingPaint,
                                       this->strikeDeviceInfo(),
                                       SkStrikeCache::GlobalStrikeCache());
}

void Device::drawSlug(SkCanvas* canvas, const sktext::gpu::Slug* slug,
                      const SkPaint& drawingPaint) {
    auto slugImpl = static_cast<const sktext::gpu::SlugImpl*>(slug);
    slugImpl->subRuns()->draw(canvas, slugImpl->origin(), drawingPaint, slugImpl,
                              this->atlasDelegate());
}

} // namespace skgpu::graphite
