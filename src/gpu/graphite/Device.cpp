/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Device.h"

#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/DrawList.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/ImageUtils.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/SharedContext.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/geom/BoundsManager.h"
#include "src/gpu/graphite/geom/Geometry.h"
#include "src/gpu/graphite/geom/IntersectionTree.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"
#include "src/gpu/graphite/text/AtlasManager.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkVerticesPriv.h"

#include "src/core/SkConvertPixels.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkTraceEvent.h"
#include "src/shaders/SkImageShader.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

#include <unordered_map>
#include <vector>

namespace skgpu::graphite {

namespace {

static const SkStrokeRec kFillStyle(SkStrokeRec::kFill_InitStyle);

bool paint_depends_on_dst(const PaintParams& paintParams) {
    std::optional<SkBlendMode> bm = paintParams.asFinalBlendMode();
    if (!bm.has_value()) {
        return true;
    }

    if (bm.value() == SkBlendMode::kSrc || bm.value() == SkBlendMode::kClear) {
        // src and clear blending never depends on dst
        return false;
    } else if (bm.value() == SkBlendMode::kSrcOver) {
        // src-over does not depend on dst if src is opaque (a = 1)
        return !paintParams.color().isOpaque() ||
               (paintParams.shader() && !paintParams.shader()->isOpaque()) ||
               (paintParams.colorFilter() && !paintParams.colorFilter()->isAlphaUnchanged());
    } else {
        // TODO: Are their other modes that don't depend on dst that can be trivially detected?
        return true;
    }
}

SkIRect rect_to_pixelbounds(const Rect& r) {
    return r.makeRoundOut().asSkIRect();
}

// TODO: this doesn't support the SrcRectConstraint option.
sk_sp<SkShader> make_img_shader_for_paint(const SkPaint& paint,
                                          sk_sp<SkImage> image,
                                          const SkRect& subset,
                                          SkTileMode tmx, SkTileMode tmy,
                                          const SkSamplingOptions& sampling,
                                          const SkMatrix* localMatrix) {
    bool imageIsAlphaOnly = SkColorTypeIsAlphaOnly(image->colorType());

    auto s = SkImageShader::MakeSubset(std::move(image), subset, tmx, tmy, sampling, localMatrix);
    if (!s) {
        return nullptr;
    }
    if (imageIsAlphaOnly && paint.getShader()) {
        // Compose the image shader with the paint's shader. Alpha images+shaders should output the
        // texture's alpha multiplied by the shader's color. DstIn (d*sa) will achieve this with
        // the source image and dst shader (MakeBlend takes dst first, src second).
        s = SkShaders::Blend(SkBlendMode::kDstIn, paint.refShader(), std::move(s));
    }
    return s;
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
                           SkBudgeted budgeted,
                           const SkSurfaceProps& props) {
    if (!recorder) {
        return nullptr;
    }
    auto textureInfo = recorder->priv().caps()->getDefaultSampledTextureInfo(ii.colorType(),
                                                                             /*levelCount=*/1,
                                                                             Protected::kNo,
                                                                             Renderable::kYes);
    sk_sp<TextureProxy> target(new TextureProxy(ii.dimensions(), textureInfo, budgeted));
    return Make(recorder, std::move(target), ii.colorInfo(), props);
}

sk_sp<Device> Device::Make(Recorder* recorder,
                           sk_sp<TextureProxy> target,
                           const SkColorInfo& colorInfo,
                           const SkSurfaceProps& props) {
    if (!recorder) {
        return nullptr;
    }

    sk_sp<DrawContext> dc = DrawContext::Make(std::move(target), colorInfo, props);
    if (!dc) {
        return nullptr;
    }

    return sk_sp<Device>(new Device(recorder, std::move(dc)));
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

Device::Device(Recorder* recorder, sk_sp<DrawContext> dc)
        : SkBaseDevice(dc->imageInfo(), dc->surfaceProps())
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

SkBaseDevice* Device::onCreateDevice(const CreateInfo& info, const SkPaint*) {
    // TODO: Inspect the paint and create info to determine if there's anything that has to be
    // modified to support inline subpasses.
    // TODO: onCreateDevice really should return sk_sp<SkBaseDevice>...
    SkSurfaceProps props(this->surfaceProps().flags(), info.fPixelGeometry);
    return Make(fRecorder, info.fInfo, SkBudgeted::kYes, props).release();
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& ii, const SkSurfaceProps& /* props */) {
    return SkSurface::MakeGraphite(fRecorder, ii);
}

bool Device::onReadPixels(const SkPixmap& pm, int x, int y) {
    // We have no access to a context to do a read pixels here.
    return false;
}

bool Device::readPixels(Context* context,
                        Recorder* recorder,
                        const SkPixmap& pm,
                        int srcX,
                        int srcY) {
    return ReadPixelsHelper([this]() {
                                this->flushPendingWorkToRecorder();
                            },
                            context,
                            recorder,
                            fDC->target(),
                            pm.info(),
                            pm.writable_addr(),
                            pm.rowBytes(),
                            srcX,
                            srcY);
}

bool Device::onWritePixels(const SkPixmap& src, int x, int y) {
    // TODO: we may need to share this in a more central place to handle uploads
    // to backend textures

    const TextureProxy* target = fDC->target();

    // TODO: add mipmap support for createBackendTexture

    if (src.colorType() == kUnknown_SkColorType) {
        return false;
    }

    // TODO: check for readOnly or framebufferOnly target and return false if so

    const Caps* caps = fRecorder->priv().caps();

    // TODO: canvas2DFastPath?
    // TODO: check that surface supports writePixels
    // TODO: handle writePixels as draw if needed (e.g., canvas2DFastPath || !supportsWritePixels)

    // TODO: check for flips and conversions and either handle here or pass info to UploadTask

    // for now, until conversions are supported
    if (!caps->areColorTypeAndTextureInfoCompatible(src.colorType(),
                                                    target->textureInfo())) {
        return false;
    }

    std::vector<MipLevel> levels;
    levels.push_back({src.addr(), src.rowBytes()});

    SkIRect dstRect = SkIRect::MakePtSize({x, y}, src.dimensions());

    this->flushPendingWorkToRecorder();
    return fDC->recordUpload(fRecorder, sk_ref_sp(target), src.colorType(), levels, dstRect);
}


///////////////////////////////////////////////////////////////////////////////

bool Device::onClipIsAA() const {
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

SkBaseDevice::ClipType Device::onGetClipType() const {
    ClipStack::ClipState state = fClip.clipState();
    if (state == ClipStack::ClipState::kEmpty) {
        return ClipType::kEmpty;
    } else if (state == ClipStack::ClipState::kDeviceRect ||
               state == ClipStack::ClipState::kWideOpen) {
        return ClipType::kRect;
    } else {
        return ClipType::kComplex;
    }
}

SkIRect Device::onDevClipBounds() const {
    return rect_to_pixelbounds(fClip.conservativeBounds());
}

// TODO: This is easy enough to support, but do we still need this API in Skia at all?
void Device::onAsRgnClip(SkRegion* region) const {
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

void Device::onClipRect(const SkRect& rect, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    // TODO: Snap rect edges to pixel bounds if non-AA and axis-aligned?
    fClip.clipShape(this->localToDeviceTransform(), Shape{rect}, op);
}

void Device::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    // TODO: Snap rrect edges to pixel bounds if non-AA and axis-aligned? Is that worth doing to
    // seam with non-AA rects even if the curves themselves are AA'ed?
    fClip.clipShape(this->localToDeviceTransform(), Shape{rrect}, op);
}

void Device::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
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
void Device::onClipRegion(const SkRegion& globalRgn, SkClipOp op) {
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

void Device::onReplaceClip(const SkIRect& rect) {
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
    // TODO: check paint params as well
     if (this->clipIsWideOpen()) {
        // do fullscreen clear
        fDC->clear(paint.getColor4f());
        return;
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
    this->drawGeometry(localToDevice, Geometry(Shape(localCoveringBounds)), paint, kFillStyle,
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
                     kFillStyle,
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
    // TODO: I'm [ml] not sure either CPU or GPU backend really has a fast path for this that
    // isn't captured by drawOval and drawLine, so could easily be moved into SkCanvas.
    if (mode == SkCanvas::kPoints_PointMode) {
        float radius = 0.5f * paint.getStrokeWidth();
        for (size_t i = 0; i < count; ++i) {
            SkRect pointRect = SkRect::MakeLTRB(points[i].fX - radius, points[i].fY - radius,
                                                points[i].fX + radius, points[i].fY + radius);
            // drawOval/drawRect with a forced fill style
            if (paint.getStrokeCap() == SkPaint::kRound_Cap) {
                this->drawGeometry(this->localToDeviceTransform(),
                                   Geometry(Shape(SkRRect::MakeOval(pointRect))),
                                   paint, kFillStyle);
            } else {
                this->drawGeometry(this->localToDeviceTransform(), Geometry(Shape(pointRect)),
                                   paint, kFillStyle);
            }
        }
    } else {
        // Force the style to be a stroke, using the radius and cap from the paint
        SkStrokeRec stroke(paint, SkPaint::kStroke_Style);
        size_t inc = (mode == SkCanvas::kLines_PointMode) ? 2 : 1;
        for (size_t i = 0; i < count; i += inc) {
            this->drawGeometry(this->localToDeviceTransform(),
                               Geometry(Shape(points[i], points[(i + 1) % count])),
                               paint, stroke);
        }
    }
}

void Device::drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                           const SkSamplingOptions& sampling, const SkPaint& paint,
                           SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(dst.isFinite());
    SkASSERT(dst.isSorted());

    // TODO: All of this logic should be handled in SkCanvas, since it's the same for every backend
    SkRect tmpSrc, tmpDst = dst;
    SkRect imgBounds = SkRect::Make(image->bounds());

    if (src) {
        tmpSrc = *src;
    } else {
        tmpSrc = SkRect::Make(image->bounds());
    }
    SkMatrix matrix = SkMatrix::RectToRect(tmpSrc, dst);

    // clip the tmpSrc to the bounds of the image, and recompute the dest rect if
    // needed (i.e., if the src was clipped). No check needed if src==null.
    if (src) {
        if (!imgBounds.contains(tmpSrc)) {
            if (!tmpSrc.intersect(imgBounds)) {
                return; // nothing to draw
            }
            // recompute dst, based on the smaller tmpSrc
            matrix.mapRect(&tmpDst, tmpSrc);
            if (!tmpDst.isFinite()) {
                return;
            }
        }
    }

    auto [ imageToDraw, newSampling ] = skgpu::graphite::GetGraphiteBacked(this->recorder(),
                                                                           image, sampling);
    if (!imageToDraw) {
        SKGPU_LOG_W("Device::drawImageRect: Creation of Graphite-backed image failed");
        return;
    }

    // construct a shader, so we can call drawRect with the dst
    auto s = make_img_shader_for_paint(paint, std::move(imageToDraw), tmpSrc,
                                       SkTileMode::kClamp, SkTileMode::kClamp,
                                       newSampling, &matrix);
    if (!s) {
        return;
    }

    SkPaint paintWithShader(paint);
    paintWithShader.setStyle(SkPaint::kFill_Style);
    paintWithShader.setShader(std::move(s));
    paintWithShader.setPathEffect(nullptr);  // drawImageRect doesn't support path effects

    this->drawRect(tmpDst, paintWithShader);
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
                                                        this);
}

void Device::drawAtlasSubRun(const sktext::gpu::AtlasSubRun* subRun,
                             SkPoint drawOrigin,
                             const SkPaint& paint,
                             sk_sp<SkRefCnt> subRunStorage) {
    const int subRunEnd = subRun->glyphCount();
    for (int subRunCursor = 0; subRunCursor < subRunEnd;) {
        // For the remainder of the run, add any atlas uploads to the Recorder's AtlasManager
        auto[ok, glyphsRegenerated] = subRun->regenerateAtlas(subRunCursor, subRunEnd, fRecorder);
        // There was a problem allocating the glyph in the atlas. Bail.
        if (!ok) {
            return;
        }
        if (glyphsRegenerated) {
            auto [bounds, localToDevice] = subRun->boundsAndDeviceMatrix(
                                                   this->localToDeviceTransform(), drawOrigin);
            SkPaint subRunPaint = paint;
            // For color emoji, only the paint alpha affects the final color
            if (subRun->maskFormat() == skgpu::MaskFormat::kARGB) {
                subRunPaint.setColor(SK_ColorWHITE);
                subRunPaint.setAlphaf(paint.getAlphaf());
            }
            this->drawGeometry(localToDevice,
                               Geometry(SubRunData(subRun, subRunStorage, bounds, subRunCursor,
                                                   glyphsRegenerated, fRecorder)),
                               subRunPaint,
                               kFillStyle,
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
            this->drawGeometry(localToDevice, Geometry(Shape(dst)), paint, style,
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

    // TODO: The tessellating path renderers haven't implemented perspective yet, so transform to
    // device space so we draw something approximately correct (barring local coord issues).
    if (geometry.isShape() && localToDevice.type() == Transform::Type::kProjection) {
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

    // Check if we have room to record into the current list before determining clipping and order
    SkStrokeRec::Style styleType = style.getStyle();
    if (this->needsFlushBeforeDraw(styleType == SkStrokeRec::kStrokeAndFill_Style ? 2 : 1)) {
        this->flushPendingWorkToRecorder();
    }

    DrawOrder order(fCurrentDepth.next());
    auto [clip, clipOrder] = fClip.applyClipToDraw(
            fColorDepthBoundsManager.get(), localToDevice, geometry, style, order.depth());
    if (clip.drawBounds().isEmptyNegativeOrNaN()) {
        // Clipped out, so don't record anything
        return;
    }
    // Some Renderer decisions are based on estimated fill rate, which requires the clipped bounds.
    // Since the fallbacks shouldn't change the bounds of the draw, it's okay to have evaluated the
    // clip stack before calling ChooseRenderer.
    const Renderer* renderer = ChooseRenderer(geometry, clip, style);
    if (!renderer) {
        SKGPU_LOG_W("Skipping draw with no supported renderer.");
        return;
    }

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

    // If a draw is not opaque, it must be drawn after the most recent draw it intersects with in
    // order to blend correctly. We always query the most recent draw (even when opaque) because it
    // also lets Device easily track whether or not there are any overlapping draws.
    PaintParams shading{paint, std::move(primitiveBlender), skipColorXform};
    const bool dependsOnDst = renderer->emitsCoverage() || paint_depends_on_dst(shading);
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

    if (styleType == SkStrokeRec::kStroke_Style ||
        styleType == SkStrokeRec::kHairline_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        // For stroke-and-fill, 'renderer' is used for the fill and we always use the
        // TessellatedStrokes renderer; for stroke and hairline, 'renderer' is used.
        StrokeStyle stroke(style.getWidth(), style.getMiter(), style.getJoin(), style.getCap());
        fDC->recordDraw(styleType == SkStrokeRec::kStrokeAndFill_Style ?
                                Renderer::TessellatedStrokes() : *renderer,
                        localToDevice, geometry, clip, order, &shading, &stroke);
    }
    if (styleType == SkStrokeRec::kFill_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        fDC->recordDraw(*renderer, localToDevice, geometry, clip, order, &shading, nullptr);
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
    const Renderer* renderer = ChooseRenderer(geometry, clip, kFillStyle);
    if (!renderer) {
        SKGPU_LOG_W("Skipping clip with no supported path renderer.");
        return;
    } else if (renderer->depthStencilFlags() & DepthStencilFlags::kStencil) {
        DisjointStencilIndex setIndex = fDisjointStencilSet->add(order.paintOrder(),
                                                                 clip.drawBounds());
        order.dependsOnStencil(setIndex);
    }

    // Clips draws are depth-only (null PaintParams), and filled (null StrokeStyle).
    // TODO: Remove this CPU-transform once perspective is supported for all path renderers
    if (localToDevice.type() == Transform::Type::kProjection) {
        SkPath devicePath = geometry.shape().asPath();
        devicePath.transform(localToDevice.matrix().asM33());
        fDC->recordDraw(*renderer, Transform::Identity(), Geometry(Shape(devicePath)), clip, order,
                        nullptr, nullptr);
    } else {
        fDC->recordDraw(*renderer, localToDevice, geometry, clip, order, nullptr, nullptr);
    }
    // This ensures that draws recorded after this clip shape has been popped off the stack will
    // be unaffected by the Z value the clip shape wrote to the depth attachment.
    if (order.depth() > fCurrentDepth) {
        fCurrentDepth = order.depth();
    }
}

const Renderer* Device::ChooseRenderer(const Geometry& geometry,
                                       const Clip& clip,
                                       const SkStrokeRec& style) {
    SkStrokeRec::Style type = style.getStyle();

    if (geometry.isSubRun()) {
        return geometry.subRunData().subRun()->renderer();
    } else if (geometry.isVertices()) {
        SkVerticesPriv info(geometry.vertices()->priv());
        return &Renderer::Vertices(info.mode(), info.hasColors(), info.hasTexCoords());
    }

    if (!geometry.isShape()) {
        // TODO: Other Geometry types will have pretty specific Renderers
        return nullptr;
    }

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
        // TODO: For non-stroke-and-fill strokes, we may add coverage-AA renderers for primitives
        // that we want to avoid triggering MSAA on.
        return &Renderer::TessellatedStrokes();
    }

    // TODO: stroke-and-fill returns the fill renderer, but if there is ever a case where a shape
    // can't be stroked with the TessellatedStrokes renderer, we should return null even if we there
    // is a valid renderer for the fill. Additionally, if we have coverage-AA renderers for filled
    // primitives to avoid triggering MSAA, we do NOT want to select them for stroke-and-fill or
    // we'll add AA seams where the separate fill and stroke draws overlap.
    //
    // For now, neither of these cases apply so stroke-and-fill and fill are handled the same.

    const Shape& shape = geometry.shape();
    if (shape.convex() && !shape.inverted()) {
        // TODO: Ganesh doesn't have a curve+middle-out triangles option for convex paths, but it
        // would be pretty trivial to spin up.
        return &Renderer::ConvexTessellatedWedges();
    } else {
        // TODO: Combine this heuristic with what is used in PathStencilCoverOp to choose between
        // wedges curves consistently in Graphite and Ganesh.
        const bool preferWedges = (shape.isPath() && shape.path().countVerbs() < 50) ||
                                   clip.drawBounds().area() <= (256 * 256);

        if (preferWedges) {
            return &Renderer::StencilTessellatedWedges(shape.fillType());
        } else {
            return &Renderer::StencilTessellatedCurvesAndTris(shape.fillType());
        }
    }
}

void Device::flushPendingWorkToRecorder() {
    SkASSERT(fRecorder);

    // TODO: we may need to further split this function up since device->device drawList and
    // DrawPass stealing will need to share some of the same logic w/o becoming a Task.

    // push any pending uploads from the atlasmanager
    auto atlasManager = fRecorder->priv().atlasManager();
    if (!atlasManager->recordUploads(fDC.get())) {
        SKGPU_LOG_E("AtlasManager uploads have failed -- may see invalid results.");
    }

    auto uploadTask = fDC->snapUploadTask(fRecorder);
    if (uploadTask) {
        fRecorder->priv().add(std::move(uploadTask));
    }

#ifdef SK_ENABLE_PIET_GPU
    auto pietTask = fDC->snapPietRenderTask(fRecorder);
    if (pietTask) {
        fRecorder->priv().add(std::move(pietTask));
    }
#endif

    fClip.recordDeferredClipDraws();
    auto drawTask = fDC->snapRenderPassTask(fRecorder);
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

bool Device::needsFlushBeforeDraw(int numNewDraws) const {
    // Must also account for the elements in the clip stack that might need to be recorded.
    numNewDraws += fClip.maxDeferredClipDraws();
    return (DrawList::kMaxDraws - fDC->pendingDrawCount()) < numNewDraws;
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkBitmap&) {
    return nullptr;
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkImage*) {
    return nullptr;
}

sk_sp<SkSpecialImage> Device::snapSpecial(const SkIRect& subset, bool forceCopy) {
    this->flushPendingWorkToRecorder();
    return nullptr;
}

#if GRAPHITE_TEST_UTILS
TextureProxy* Device::proxy() {
    return fDC->target();
}

#endif

} // namespace skgpu::graphite
