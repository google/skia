/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Device.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/SkStuff.h"
#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/ContextPriv.h"
#include "experimental/graphite/src/CopyTask.h"
#include "experimental/graphite/src/DrawContext.h"
#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/Gpu.h"
#include "experimental/graphite/src/Recorder.h"
#include "experimental/graphite/src/Recording.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/TextureProxy.h"
#include "experimental/graphite/src/geom/BoundsManager.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStrokeRec.h"

#include "src/core/SkConvertPixels.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkSpecialImage.h"

namespace skgpu {

namespace {

static const SkStrokeRec kFillStyle(SkStrokeRec::kFill_InitStyle);

bool is_opaque(const PaintParams& paint) {
    // TODO: implement this
    return false;
}

} // anonymous namespace

sk_sp<Device> Device::Make(sk_sp<Recorder> recorder, const SkImageInfo& ii) {
    const Gpu* gpu = recorder->context()->priv().gpu();
    auto textureInfo = gpu->caps()->getDefaultSampledTextureInfo(ii.colorType(), /*levelCount=*/1,
                                                                 Protected::kNo, Renderable::kYes);
    auto target = sk_sp<TextureProxy>(new TextureProxy(ii.dimensions(), textureInfo));
    sk_sp<DrawContext> dc = DrawContext::Make(target,
                                              ii.refColorSpace(),
                                              ii.colorType(),
                                              ii.alphaType());
    if (!dc) {
        return nullptr;
    }

    return sk_sp<Device>(new Device(std::move(recorder), std::move(dc)));
}

Device::Device(sk_sp<Recorder> recorder, sk_sp<DrawContext> dc)
        : SkBaseDevice(dc->imageInfo(), SkSurfaceProps())
        , fRecorder(std::move(recorder))
        , fDC(std::move(dc))
        , fColorDepthBoundsManager(std::make_unique<NaiveBoundsManager>())
        , fCurrentDepth(DrawOrder::kClearDepth)
        , fMaxStencilIndex(DrawOrder::kUnassigned)
        , fDrawsOverlap(false) {
    SkASSERT(SkToBool(fDC) && SkToBool(fRecorder));
}

Device::~Device() = default;

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
    // TODO: Support more formats that we can read back into
    if (pm.colorType() != kRGBA_8888_SkColorType) {
        return false;
    }

    auto context = fRecorder->context();
    auto resourceProvider = context->priv().resourceProvider();

    TextureProxy* srcProxy = fDC->target();
    if(!srcProxy->instantiate(resourceProvider)) {
        return false;
    }
    sk_sp<Texture> srcTexture = srcProxy->refTexture();
    SkASSERT(srcTexture);

    size_t rowBytes = pm.rowBytes();
    size_t size = rowBytes * pm.height();
    sk_sp<Buffer> dstBuffer = resourceProvider->findOrCreateBuffer(size,
                                                                   BufferType::kXferGpuToCpu,
                                                                   PrioritizeGpuReads::kNo);
    if (!dstBuffer) {
        return false;
    }

    SkIRect srcRect = SkIRect::MakeXYWH(x, y, pm.width(), pm.height());
    sk_sp<CopyTextureToBufferTask> task =
            CopyTextureToBufferTask::Make(std::move(srcTexture),
                                          srcRect,
                                          dstBuffer,
                                          /*bufferOffset=*/0,
                                          rowBytes);
    if (!task) {
        return false;
    }

    this->flushPendingWorkToRecorder();
    fRecorder->add(std::move(task));

    // TODO: Can snapping ever fail?
    context->insertRecording(fRecorder->snap());
    context->submit(SyncToCpu::kYes);

    void* mappedMemory = dstBuffer->map();

    memcpy(pm.writable_addr(), mappedMemory, size);

    return true;
}

SkIRect Device::onDevClipBounds() const {
    auto target = fDC->target();
    return SkIRect::MakeSize(target->dimensions());
}

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
        // TBD: This warning should go through the general purpose graphite logging system
        SkDebugf("[graphite] WARNING - Skipping draw with non-invertible/non-finite transform.\n");
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
            // TBD: This warning should go through the general purpose graphite logging system
            SkDebugf("[graphite] WARNING - Path effect failed to apply, drawing original path.\n");
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

    auto blendMode = paint.asBlendMode();
    PaintParams shading{paint.getColor4f(),
                        blendMode.has_value() ? *blendMode : SkBlendMode::kSrcOver,
                        paint.refShader()};

    // If a draw is not opaque, it must be drawn after the most recent draw it intersects with in
    // order to blend correctly. We always query the most recent draw (even when opaque) because it
    // also lets Device easily track whether or not there are any overlapping draws.
    const bool opaque = is_opaque(shading);
    CompressedPaintersOrder prevDraw =
            fColorDepthBoundsManager->getMostRecentDraw(clip.drawBounds());
    if (!opaque) {
        order.dependsOnPaintersOrder(prevDraw);
    }

    if (styleType == SkStrokeRec::kStroke_Style ||
        styleType == SkStrokeRec::kHairline_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        // TODO: If DC supports stroked primitives, Device could choose one of those based on shape
        StrokeParams stroke(style.getWidth(), style.getMiter(), style.getJoin(), style.getCap());
        fDC->strokePath(localToDevice, shape, stroke, clip, order, &shading);
    }
    if (styleType == SkStrokeRec::kFill_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        // TODO: If DC supports filled primitives, Device could choose one of those based on shape

        // TODO: Route all filled shapes to stencil-and-cover for the sprint; convex will draw
        // correctly but uses an unnecessary stencil step.
        // if (shape.convex()) {
        //     fDC->fillConvexPath(localToDevice, shape, clip, order, &shading);
        // } else {
            order.dependsOnStencil(fMaxStencilIndex.next());
            fDC->stencilAndFillPath(localToDevice, shape, clip, order, &shading);
        // }
    }

    // Record the painters order and depth used for this draw
    const bool fullyOpaque = opaque && shape.isRect() &&
                             localToDevice.type() <= Transform::Type::kRectStaysRect;
    fColorDepthBoundsManager->recordDraw(shape.bounds(),
                                         order.paintOrder(),
                                         order.depth(),
                                         fullyOpaque);

    fCurrentDepth = order.depth();
    if (order.stencilIndex() != DrawOrder::kUnassigned) {
        fMaxStencilIndex = std::max(fMaxStencilIndex, order.stencilIndex());
    }
    fDrawsOverlap |= (prevDraw != DrawOrder::kNoIntersection);
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
    // TODO: we may need to further split this function up since device->device drawList and
    // DrawPass stealing will need to share some of the same logic w/o becoming a Task.

    // TODO: iterate the clip stack and issue a depth-only draw for every clip element that has
    // a non-empty usage bounds, using that bounds as the scissor.
    auto drawTask = fDC->snapRenderPassTask(fRecorder.get(), fColorDepthBoundsManager.get());
    if (drawTask) {
        fRecorder->add(std::move(drawTask));
    }
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
