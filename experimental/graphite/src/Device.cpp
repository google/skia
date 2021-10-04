/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Device.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/SkStuff.h"
#include "experimental/graphite/src/DrawContext.h"
#include "experimental/graphite/src/DrawList.h"

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkStrokeRec.h"

#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkSpecialImage.h"

namespace skgpu {

sk_sp<Device> Device::Make(sk_sp<Context> context, const SkImageInfo& ii) {
    sk_sp<DrawContext> dc = DrawContext::Make(ii);
    if (!dc) {
        return nullptr;
    }

    return sk_sp<Device>(new Device(std::move(context), std::move(dc)));
}

Device::Device(sk_sp<Context> context, sk_sp<DrawContext> dc)
        : SkBaseDevice(dc->imageInfo(), SkSurfaceProps())
        , fContext(std::move(context))
        , fDC(std::move(dc)) {
    SkASSERT(SkToBool(fDC));
}

SkBaseDevice* Device::onCreateDevice(const CreateInfo& info, const SkPaint*) {
    // TODO: Inspect the paint and create info to determine if there's anything that has to be
    // modified to support inline subpasses.
    // TODO: onCreateDevice really should return sk_sp<SkBaseDevice>...
    return Make(fContext, info.fInfo).release();
}

sk_sp<SkSurface> Device::makeSurface(const SkImageInfo& ii, const SkSurfaceProps& /* props */) {
    return MakeGraphite(fContext, ii);
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
    SkM44 devToLocal;
    if (!this->localToDevice44().invert(&devToLocal)) {
        // TBD: This matches legacy behavior for drawPaint() that requires local coords, although
        // v1 handles arbitrary transforms when the paint is solid color because it just fills the
        // device bounds directly. In the new world it might be nice to have non-invertible
        // transforms formalized (i.e. no drawing ever, handled at SkCanvas level possibly?)
        return;
    }
    SkRect localCoveringBounds = SkMatrixPriv::MapRect(devToLocal, deviceBounds);
    this->drawRect(localCoveringBounds, paint);
}

void Device::drawRect(const SkRect& r, const SkPaint& paint) {
    // TODO: If the SDC primitive is a rrect (and no simpler), this can be wasted effort since
    // SkCanvas checks SkRRects for being a rect and reduces it, only for Device to rebuild it
    // It would be less effort if we can skip the validation of SkRRect ctors here.
    // TBD: For now rects are paths too, but they may become an SDC primitive
    this->drawPath(SkPath::Rect(r), paint, /*pathIsMutable=*/true);
}

void Device::drawOval(const SkRect& oval, const SkPaint& paint) {
    // TODO: This has wasted effort from the SkCanvas level since it instead converts rrects that
    // happen to be ovals into this, only for us to go right back to rrect.

    // Ovals are always a simplification of round rects
    this->drawRRect(SkRRect::MakeOval(oval), paint);
}

void Device::drawRRect(const SkRRect& rr, const SkPaint& paint) {
    // TBD: If the SDC has a rrect primitive, this won't need to be converted to a path
    this->drawPath(SkPath::RRect(rr), paint, /*pathIsMutable=*/true);
}

void Device::drawPoints(SkCanvas::PointMode mode, size_t count,
                        const SkPoint* points, const SkPaint& paint) {
    // TODO: I'm [ml] not sure either CPU or GPU backend really has a fast path for this that
    // isn't captured by drawOval and drawLine, so could easily be moved into SkCanvas.
    if (mode == SkCanvas::kPoints_PointMode) {
        SkPaint filled = paint;
        filled.setStyle(SkPaint::kFill_Style);
        float radius = 0.5f * paint.getStrokeWidth();
        for (size_t i = 0; i < count; ++i) {
            SkRect cap = SkRect::MakeLTRB(points[i].fX - radius, points[i].fY - radius,
                                          points[i].fX + radius, points[i].fY + radius);
            if (paint.getStrokeCap() == SkPaint::kRound_Cap) {
                this->drawOval(cap, filled);
            } else {
                this->drawRect(cap, filled);
            }
        }
    } else {
        size_t inc = (mode == SkCanvas::kLines_PointMode) ? 2 : 1;
        SkPathBuilder builder;
        for (size_t i = 0; i < count; i += inc) {
            builder.moveTo(points[i]);
            builder.lineTo(points[i + 1]);
            this->drawPath(builder.detach(), paint, /*pathIsMutable=*/true);
        }
    }
}

void Device::drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable) {
    // Heavy weight paint options like path effects, mask filters, and stroke-and-fill style are
    // applied on the CPU by generating a new path and recursing on drawPath().
    if (paint.getPathEffect()) {
        // Apply the path effect before anything else
        // TODO: If asADash() returns true and the base path matches the dashing fast path, then
        // that should be detected now as well.
        // TODO: This logic is also a candidate for moving to SkCanvas if SkDevice exposes a faster
        // dash path.

        // Strip off path effect
        SkPaint noPE = paint;
        noPE.setPathEffect(nullptr);

        float scaleFactor = SkPaintPriv::ComputeResScaleForStroking(this->localToDevice());
        SkStrokeRec stroke(paint, scaleFactor);
        SkPath dst;
        if (paint.getPathEffect()->filterPath(&dst, path, &stroke,
                                              nullptr, this->localToDevice())) {
            // Adjust paint style to reflect modifications to stroke rec
            stroke.applyToPaint(&noPE);
            this->drawPath(dst, noPE, /*pathIsMutable=*/true);
            return;
        } else {
            // TBD: This warning should go through the general purpose graphite logging system
            SkDebugf("[graphite] WARNING - Path effect failed to apply, drawing original path.\n");
            this->drawPath(path, noPE, pathIsMutable);
            return;
        }
    }

    // TODO: Handle mask filters, ignored for now but would be applied at this point. My[ml]
    // thinking is that if there's a mask filter we call a helper function with the path and the
    // paint, which returns a coverage mask. Then we do a rectangular draw sampling the mask and
    // handling the rest of the paint's shading. I don't think that's really any different from
    // the way it is right now. (not 100% sure, but this may also be a reasonable approach for CPU
    // so could make SkCanvas handle all path effects, image filters, and mask filters and Devices
    // only need to handle shaders, color filters, and blenders).
    if (paint.getMaskFilter()) {
        return;
    }

    // Resolve stroke-and-fill -> fill, and hairline -> stroke since the SDC only supports stroke
    // or fill for path rendering.
    if (paint.getStyle() == SkPaint::kStrokeAndFill_Style) {
        // TODO: Could const-cast path when pathIsMutable is true, might not be worth complexity...
        SkPath simplified;
        SkPaint styledPaint = paint;
        if (paint.getFillPath(path, &simplified, nullptr, this->localToDevice())) {
            styledPaint.setStyle(SkPaint::kFill_Style);
        } else {
            styledPaint.setStyle(SkPaint::kStroke_Style);
            styledPaint.setStrokeWidth(0.f);
        }
        this->drawPath(simplified, styledPaint, /*pathIsMutable=*/true);
        return;
    }

    // TODO: Implement clipping and z determination
    SkIRect scissor = this->devClipBounds();

    auto blendMode = paint.asBlendMode();
    PaintParams shading{paint.getColor4f(),
                        blendMode.has_value() ? *blendMode : SkBlendMode::kSrcOver,
                        paint.refShader()};
    if (paint.getStyle() == SkPaint::kStroke_Style) {
        StrokeParams stroke{paint.getStrokeWidth(), paint.getStrokeMiter(),
                            paint.getStrokeJoin(), paint.getStrokeCap()};
        if (paint.getStrokeWidth() <= 0.f) {
            // Handle hairlines by transforming the control points into device space and drawing
            // that path with a stroke width of 1 and the identity matrix
            // FIXME: This doesn't work if the shading requires local coords...
            SkPath devicePath = path.makeTransform(this->localToDevice());
            stroke.fWidth = 1.f;
            fDC->strokePath(SkM44(), devicePath, stroke, scissor, 0, 0, &shading);
        } else {
            fDC->strokePath(this->localToDevice44(), path, stroke, scissor, 0, 0, &shading);
        }
    } else if (path.isConvex()) {
        fDC->fillConvexPath(this->localToDevice44(), path, scissor, 0, 0, &shading);
    } else {
        fDC->stencilAndFillPath(this->localToDevice44(), path, scissor, 0, 0, 0, &shading);
    }
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
