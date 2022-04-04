/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Device.h"

#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/include/SkStuff.h"
#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/DrawContext.h"
#include "experimental/graphite/src/DrawGeometry.h"
#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/Log.h"
#include "experimental/graphite/src/RecorderPriv.h"
#include "experimental/graphite/src/Renderer.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/TextureUtils.h"
#include "experimental/graphite/src/geom/BoundsManager.h"
#include "experimental/graphite/src/geom/IntersectionTree.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStrokeRec.h"

#include "src/core/SkConvertPixels.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkStroke.h"

#include <unordered_map>
#include <vector>

namespace skgpu {

namespace {

static const SkStrokeRec kFillStyle(SkStrokeRec::kFill_InitStyle);

bool paint_depends_on_dst(const PaintParams& paintParams) {
    std::optional<SkBlendMode> bm = paintParams.asBlendMode();
    if (!bm.has_value()) {
        return true;
    }

    if (bm.value() == SkBlendMode::kSrc || bm.value() == SkBlendMode::kClear) {
        // src and clear blending never depends on dst
        return false;
    } else if (bm.value() == SkBlendMode::kSrcOver) {
        // src-over does not depend on dst if src is opaque (a = 1)
        // TODO: This will get more complicated when PaintParams has color filters and blenders
        return !paintParams.color().isOpaque() ||
               (paintParams.shader() && !paintParams.shader()->isOpaque());
    } else {
        // TODO: Are their other modes that don't depend on dst that can be trivially detected?
        return true;
    }
}

SkIRect rect_to_pixelbounds(const Rect& r) {
    return skvx::bit_pun<SkIRect>(skvx::cast<int>(r.makeRoundOut().ltrb()));
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

sk_sp<Device> Device::Make(Recorder* recorder, const SkImageInfo& ii) {
    if (!recorder) {
        return nullptr;
    }
    auto textureInfo = recorder->priv().caps()->getDefaultSampledTextureInfo(ii.colorType(),
                                                                             /*levelCount=*/1,
                                                                             Protected::kNo,
                                                                             Renderable::kYes);
    sk_sp<TextureProxy> target(new TextureProxy(ii.dimensions(), textureInfo));
    return Make(recorder,
                std::move(target),
                ii.refColorSpace(),
                ii.colorType(),
                ii.alphaType());
}

sk_sp<Device> Device::Make(Recorder* recorder,
                           sk_sp<TextureProxy> target,
                           sk_sp<SkColorSpace> colorSpace,
                           SkColorType colorType,
                           SkAlphaType alphaType) {
    if (!recorder) {
        return nullptr;
    }

    sk_sp<DrawContext> dc = DrawContext::Make(std::move(target),
                                              std::move(colorSpace),
                                              colorType,
                                              alphaType);
    if (!dc) {
        return nullptr;
    }

    return sk_sp<Device>(new Device(recorder, std::move(dc)));
}

Device::Device(Recorder* recorder, sk_sp<DrawContext> dc)
        : SkBaseDevice(dc->imageInfo(), SkSurfaceProps())
        , fRecorder(recorder)
        , fDC(std::move(dc))
        , fClip(SkIRect::MakeSize(fDC->target()->dimensions()))
        , fColorDepthBoundsManager(std::make_unique<NaiveBoundsManager>())
        , fDisjointStencilSet(std::make_unique<IntersectionTreeSet>())
        , fCurrentDepth(DrawOrder::kClearDepth)
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

SkBaseDevice* Device::onCreateDevice(const CreateInfo& info, const SkPaint*) {
    // TODO: Inspect the paint and create info to determine if there's anything that has to be
    // modified to support inline subpasses.
    // TODO: onCreateDevice really should return sk_sp<SkBaseDevice>...
    return Make(fRecorder, info.fInfo).release();
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& ii, const SkSurfaceProps& /* props */) {
    return MakeGraphite(fRecorder, ii);
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
    // TODO: Cached transform?
    Transform localToDevice(this->localToDevice44());
    // TODO: Snap rect edges to pixel bounds if non-AA and axis-aligned?
    fClip.clipShape(localToDevice, Shape{rect}, op);
}

void Device::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    // TODO: Cached transform?
    Transform localToDevice(this->localToDevice44());
    // TODO: Snap rrect edges to pixel bounds if non-AA and axis-aligned? Is that worth doing to
    // seam with non-AA rects even if the curves themselves are AA'ed?
    fClip.clipShape(localToDevice, Shape{rrect}, op);
}

void Device::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    SkASSERT(op == SkClipOp::kIntersect || op == SkClipOp::kDifference);
    // TODO: Cached transform?
    Transform localToDevice(this->localToDevice44());
    // TODO: Ensure all path inspection is handled here or in SkCanvas, and that non-AA rects as
    // paths are routed appropriately.
    // TODO: Must also detect paths that are lines so the clip stack can be set to empty
    fClip.clipShape(localToDevice, Shape{path}, op);
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
    SkRect deviceBounds = SkRect::Make(this->devClipBounds());
    // TODO: Should be able to get the inverse from the matrix cache
    SkM44 devToLocal;
    if (!this->localToDevice44().invert(&devToLocal)) {
        // TBD: This matches legacy behavior for drawPaint() that requires local coords, although
        // v1 handles arbitrary transforms when the paint is solid color because it just fills the
        // device bounds directly. In the new world it might be nice to have non-invertible
        // transforms formalized (i.e. no drawing ever, handled at SkCanvas level possibly?)
        return;
    }
    SkRect localCoveringBounds = SkMatrixPriv::MapRect(devToLocal, deviceBounds);
    this->drawShape(Shape(localCoveringBounds), paint, kFillStyle,
                    DrawFlags::kIgnorePathEffect | DrawFlags::kIgnoreMaskFilter);
}

void Device::drawRect(const SkRect& r, const SkPaint& paint) {
    this->drawShape(Shape(r), paint, SkStrokeRec(paint));
}

void Device::drawOval(const SkRect& oval, const SkPaint& paint) {
    // TODO: This has wasted effort from the SkCanvas level since it instead converts rrects that
    // happen to be ovals into this, only for us to go right back to rrect.
    this->drawShape(Shape(SkRRect::MakeOval(oval)), paint, SkStrokeRec(paint));
}

void Device::drawRRect(const SkRRect& rr, const SkPaint& paint) {
    this->drawShape(Shape(rr), paint, SkStrokeRec(paint));
}

void Device::drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) {
    // TODO: If we do try to inspect the path, it should happen here and possibly after computing
    // the path effect. Alternatively, all that should be handled in SkCanvas.
    this->drawShape(Shape(path), paint, SkStrokeRec(paint));
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
                this->drawShape(Shape(SkRRect::MakeOval(pointRect)), paint, kFillStyle);
            } else {
                this->drawShape(Shape(pointRect), paint, kFillStyle);
            }
        }
    } else {
        // Force the style to be a stroke, using the radius and cap from the paint
        SkStrokeRec stroke(paint, SkPaint::kStroke_Style);
        size_t inc = (mode == SkCanvas::kLines_PointMode) ? 2 : 1;
        for (size_t i = 0; i < count; i += inc) {
            this->drawShape(Shape(points[i], points[(i + 1) % count]), paint, stroke);
        }
    }
}

void Device::drawShape(const Shape& shape,
                       const SkPaint& paint,
                       const SkStrokeRec& style,
                       Mask<DrawFlags> flags) {
    // TODO: Device will cache the Transform or otherwise ensure it's computed once per change to
    // its local-to-device matrix, but that requires updating SkDevice's virtuals. Right now we
    // re-compute the Transform every draw, as well as any time we recurse on drawShape(), but that
    // goes away with the caching.
    Transform localToDevice(this->localToDevice44());
    if (!localToDevice.valid()) {
        // If the transform is not invertible or not finite then drawing isn't well defined.
        SKGPU_LOG_W("Skipping draw with non-invertible/non-finite transform.");
        return;
    }

    // Heavy weight paint options like path effects, mask filters, and stroke-and-fill style are
    // applied on the CPU by generating a new shape and recursing on drawShape() with updated flags
    if (!(flags & DrawFlags::kIgnorePathEffect) && paint.getPathEffect()) {
        // Apply the path effect before anything else
        // TODO: If asADash() returns true and the base path matches the dashing fast path, then
        // that should be detected now as well. Maybe add dashPath to Device so canvas can handle it
        SkStrokeRec newStyle = style;
        newStyle.setResScale(localToDevice.maxScaleFactor());
        SkPath dst;
        if (paint.getPathEffect()->filterPath(&dst, shape.asPath(), &newStyle,
                                              nullptr, localToDevice)) {
            // Recurse using the path and new style, while disabling downstream path effect handling
            this->drawShape(Shape(dst), paint, newStyle, flags | DrawFlags::kIgnorePathEffect);
            return;
        } else {
            SKGPU_LOG_W("Path effect failed to apply, drawing original path.");
            this->drawShape(shape, paint, style, flags | DrawFlags::kIgnorePathEffect);
            return;
        }
    }

    if (!(flags & DrawFlags::kIgnoreMaskFilter) && paint.getMaskFilter()) {
        // TODO: Handle mask filters, ignored for the sprint.
        // TODO: Could this be handled by SkCanvas by drawing a mask, blurring, and then sampling
        // with a rect draw? What about fast paths for rrect blur masks...
        this->drawShape(shape, paint, style, flags | DrawFlags::kIgnoreMaskFilter);
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
    const SkStrokeRec::Style styleType = style.getStyle();
    if (this->needsFlushBeforeDraw(styleType == SkStrokeRec::kStrokeAndFill_Style ? 2 : 1)) {
        this->flushPendingWorkToRecorder();
    }

    DrawOrder order(fCurrentDepth.next());
    auto [clip, clipOrder] = this->applyClipToDraw(localToDevice, shape, style, order.depth());
    if (clip.drawBounds().isEmptyNegativeOrNaN()) {
        // Clipped out, so don't record anything
        return;
    }

    // A draw's order always depends on the clips that must be drawn before it
    order.dependsOnPaintersOrder(clipOrder);

    // If a draw is not opaque, it must be drawn after the most recent draw it intersects with in
    // order to blend correctly. We always query the most recent draw (even when opaque) because it
    // also lets Device easily track whether or not there are any overlapping draws.
    PaintParams shading{paint};
    const bool dependsOnDst = paint_depends_on_dst(shading);
    CompressedPaintersOrder prevDraw =
            fColorDepthBoundsManager->getMostRecentDraw(clip.drawBounds());
    if (dependsOnDst) {
        order.dependsOnPaintersOrder(prevDraw);
    }

    if (styleType == SkStrokeRec::kStroke_Style ||
        styleType == SkStrokeRec::kHairline_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        StrokeStyle stroke(style.getWidth(), style.getMiter(), style.getJoin(), style.getCap());
        this->recordDraw(localToDevice, shape, clip, order, &shading, &stroke);
    }
    if (styleType == SkStrokeRec::kFill_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        this->recordDraw(localToDevice, shape, clip, order, &shading, nullptr);
    }

    // Record the painters order and depth used for this draw
    // TODO: If recordDraw picked a coverage AA renderer, 'dependsOnDst' is out of date.
    const bool fullyOpaque = !dependsOnDst &&
                             shape.isRect() &&
                             localToDevice.type() <= Transform::Type::kRectStaysRect;
    fColorDepthBoundsManager->recordDraw(shape.bounds(),
                                         order.paintOrder(),
                                         order.depth(),
                                         fullyOpaque);

    fCurrentDepth = order.depth();
    fDrawsOverlap |= (prevDraw != DrawOrder::kNoIntersection);
}

void Device::recordDraw(const Transform& localToDevice,
                        const Shape& shape,
                        const Clip& clip,
                        DrawOrder ordering,
                        const PaintParams* paint,
                        const StrokeStyle* stroke) {
    // TODO: remove after CPU-transform fallbacks are no longer needed
    static const Transform kIdentity{SkM44()};

    // TODO: For now stroked paths are converted to fills on the CPU since the fixed count
    // stroke path renderer hasn't been ported to Graphite yet.
    if (stroke) {
        SkPath strokeAsPath = shape.asPath();
        SkStroke stroker;
        stroker.setCap(stroke->cap());
        stroker.setJoin(stroke->join());
        stroker.setMiterLimit(stroke->miterLimit());
        stroker.setDoFill(false);
        const Transform* transform;
        if (stroke->halfWidth() == 0.f) {
            // Manually transform to device space and then generate a 1px stroke filled path, which
            // would require applying a local matrix to the paint but we skip that for now since all
            // of this is temporary anyways and most hairlines aren't spatially-varying.
            strokeAsPath.transform(localToDevice.matrix().asM33());
            stroker.setWidth(1.f);
            stroker.strokePath(strokeAsPath, &strokeAsPath);
            transform = &kIdentity;
        } else {
            stroker.setResScale(localToDevice.maxScaleFactor());
            stroker.setWidth(stroke->width());
            stroker.strokePath(strokeAsPath, &strokeAsPath);
            transform = &localToDevice;
        }
        // Strokes as fills shouldn't be inverse filled
        if (strokeAsPath.isInverseFillType()) {
            strokeAsPath.toggleInverseFillType();
        }
        // Stroked paths with just moveTos may produce an empty path, which shouldn't be sent on
        if (!strokeAsPath.isEmpty()) {
            this->recordDraw(*transform, Shape(strokeAsPath), clip, ordering, paint, nullptr);
        }
        return;
    }
    // TODO: The tessellating path renderers haven't implemented perspective yet, so transform to
    // device space so we draw something approximately correct (barring local coord issues).
    if (localToDevice.type() == Transform::Type::kProjection) {
        SkPath devicePath = shape.asPath();
        devicePath.transform(localToDevice.matrix().asM33());
        this->recordDraw(kIdentity, Shape(devicePath), clip, ordering, paint, nullptr);
        return;
    }

    // TODO: Eventually the Renderer selection logic should be lifted to some external
    // RendererSelector that can be reused between Device and other wrappers around DrawContext.
    // TODO: All shapes that select a tessellating path renderer need to be "pre-chopped" if they
    // are large enough to exceed the fixed count tessellation limits.
    const Renderer* renderer = nullptr;

    if (shape.convex() && !shape.inverted()) {
        // TODO: Ganesh doesn't have a curve+middle-out triangles option for convex paths, but it
        // would be pretty trivial to spin up.
        renderer = &Renderer::ConvexTessellatedWedges();
    } else {
        // TODO: Combine this heuristic with what is used in PathStencilCoverOp to choose between
        // wedges curves consistently in Graphite and Ganesh.
        const bool preferWedges = (shape.isPath() && shape.path().countVerbs() < 50) ||
                                   clip.drawBounds().area() <= (256 * 256);

        if (preferWedges) {
            renderer = &Renderer::StencilTessellatedWedges(shape.fillType());
        } else {
            renderer = &Renderer::StencilTessellatedCurvesAndTris(shape.fillType());
        }
    }

    if (!renderer) {
        SKGPU_LOG_W("Skipping draw with no supported path renderer.");
        return;
    }

    if (renderer->depthStencilFlags() & DepthStencilFlags::kStencil) {
        DisjointStencilIndex setIndex = fDisjointStencilSet->add(ordering.paintOrder(),
                                                                 clip.drawBounds());
        ordering.dependsOnStencil(setIndex);
    }
    // TODO: if the chosen Renderer uses coverage AA, then 'ordering' depends on painter's order,
    // so we will need to take into account the previous draw. Since no Renderer uses coverage AA
    // right now, it's not an issue yet.
    fDC->recordDraw(*renderer, localToDevice, shape, clip, ordering, paint, stroke);
}

std::pair<Clip, CompressedPaintersOrder> Device::applyClipToDraw(const Transform& localToDevice,
                                                                 const Shape& shape,
                                                                 const SkStrokeRec& style,
                                                                 PaintersDepth z) {
    SkIRect scissor = this->devClipBounds();

    Rect drawBounds = shape.bounds();
    if (!style.isHairlineStyle()) {
        float localStyleOutset = style.getInflationRadius();
        drawBounds.outset(localStyleOutset);
    }
    drawBounds = localToDevice.mapRect(drawBounds);

    // Hairlines get an extra pixel *after* transforming to device space
    if (style.isHairlineStyle()) {
        drawBounds.outset(0.5f);
    }

    drawBounds.intersect(SkRect::Make(scissor));
    if (drawBounds.isEmptyNegativeOrNaN()) {
        // Trivially clipped out, so return now
        return {{drawBounds, scissor}, DrawOrder::kNoIntersection};
    }

    // TODO: iterate the clip stack and accumulate draw bounds into clip usage
    return {{drawBounds, scissor}, DrawOrder::kNoIntersection};
}

void Device::flushPendingWorkToRecorder() {
    SkASSERT(fRecorder);

    // TODO: we may need to further split this function up since device->device drawList and
    // DrawPass stealing will need to share some of the same logic w/o becoming a Task.

    auto uploadTask = fDC->snapUploadTask(fRecorder);
    if (uploadTask) {
        fRecorder->priv().add(std::move(uploadTask));
    }

    // TODO: iterate the clip stack and issue a depth-only draw for every clip element that has
    // a non-empty usage bounds, using that bounds as the scissor.
    auto drawTask = fDC->snapRenderPassTask(fRecorder, fColorDepthBoundsManager.get());
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
    // TODO: iterate the clip stack and count the number of clip elements (both w/ and w/o usage
    // since we want to know the max # of clip shapes that flushing might add as draws).
    // numNewDraws += clip element count...
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

} // namespace skgpu
