/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Device.h"

#include "experimental/graphite/include/SkStuff.h"
#include "experimental/graphite/src/DrawContext.h"
#include "experimental/graphite/src/DrawList.h"
#include "experimental/graphite/src/Recorder.h"
#include "experimental/graphite/src/geom/BoundsManager.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStrokeRec.h"

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
    sk_sp<DrawContext> dc = DrawContext::Make(ii);
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
        , fMaxPaintOrder(0)
        , fMaxZ(0)
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
    // TODO: If we're reading back pixels we need to push deferred clip draws and snap off the draw
    // task so we can have a read back task added to the graph, the same as will be done if the
    // Device has a snapped special image or is drawn into another device directly.
    // TODO: actually do a read back
    pm.erase(SK_ColorGREEN);
    return true;
}

void Device::drawPaint(const SkPaint& paint) {
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
            this->drawShape(Shape(points[i], points[i + 1]), paint, stroke);
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

    uint16_t drawZ = fMaxZ + 1;
    ClipResult clip = this->applyClipToDraw(localToDevice, shape, style, drawZ);
    if (clip.fDrawBounds.isEmptyNegativeOrNaN()) {
        // Clipped out, so don't record anything
        return;
    }

    auto blendMode = paint.asBlendMode();
    PaintParams shading{paint.getColor4f(),
                        blendMode.has_value() ? *blendMode : SkBlendMode::kSrcOver,
                        paint.refShader()};

    // If a draw is opaque, its ordering only depends on clipping; otherwise it must be drawn after
    // the most recent draw it intersects with, in order to blend correctly.
    const bool opaque = is_opaque(shading);
    CompressedPaintersOrder prevDrawOrder =
            fColorDepthBoundsManager->getMostRecentDraw(clip.fDrawBounds);
    CompressedPaintersOrder drawOrder =
            1 + (opaque ? clip.fOrder : std::max(clip.fOrder, prevDrawOrder));

    SkStrokeRec::Style styleType = style.getStyle();
    if (styleType == SkStrokeRec::kStroke_Style ||
        styleType == SkStrokeRec::kHairline_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        // TODO: If DC supports stroked primitives, Device could choose one of those based on shape
        StrokeParams stroke(style.getWidth(), style.getMiter(), style.getJoin(), style.getCap());
        fDC->strokePath(localToDevice, shape, stroke, clip.fScissor,
                        drawOrder, drawZ, &shading);
    }
    if (styleType == SkStrokeRec::kFill_Style ||
        styleType == SkStrokeRec::kStrokeAndFill_Style) {
        // TODO: If DC supports filled primitives, Device could choose one of those based on shape
        if (shape.convex()) {
            fDC->fillConvexPath(localToDevice, shape, clip.fScissor,
                                drawOrder, drawZ, &shading);
        } else {
            // FIXME must determine stencil order; a separate bounds manager? a rect tree? defer?
            fDC->stencilAndFillPath(localToDevice, shape, clip.fScissor,
                                    drawOrder, 0, drawZ, &shading);
        }
    }

    // Record the painters order and Z used for this draw
    const bool fullyOpaque = opaque && shape.isRect() &&
                             localToDevice.type() <= Transform::Type::kRectStaysRect;
    fColorDepthBoundsManager->recordDraw(shape.bounds(), drawOrder, drawZ, fullyOpaque);
    fMaxPaintOrder = std::max(fMaxPaintOrder, drawOrder);
    fMaxZ = drawZ;
    fDrawsOverlap |= (prevDrawOrder != 0);
}

Device::ClipResult Device::applyClipToDraw(const Transform& localToDevice,
                                           const Shape& shape,
                                           const SkStrokeRec& style,
                                           uint16_t z) {
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
        return {scissor, drawBounds, 0};
    }

    // TODO: iterate the clip stack and accumulate draw bounds into clip usage
    return {scissor, drawBounds, 0};
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkBitmap&) {
    return nullptr;
}

sk_sp<SkSpecialImage> Device::makeSpecial(const SkImage*) {
    return nullptr;
}

sk_sp<SkSpecialImage> Device::snapSpecial(const SkIRect& subset, bool forceCopy) {
    return nullptr;
}

} // namespace skgpu
