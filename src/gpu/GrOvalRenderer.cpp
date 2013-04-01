/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalRenderer.h"

#include "effects/GrCircleEdgeEffect.h"
#include "effects/GrEllipseEdgeEffect.h"

#include "GrDrawState.h"
#include "GrDrawTarget.h"
#include "SkStrokeRec.h"

SK_DEFINE_INST_COUNT(GrOvalRenderer)

namespace {

struct CircleVertex {
    GrPoint fPos;
    GrPoint fCenter;
    SkScalar fOuterRadius;
    SkScalar fInnerRadius;
};

struct EllipseVertex {
    GrPoint fPos;
    GrPoint fCenter;
    SkScalar fOuterXRadius;
    SkScalar fOuterXYRatio;
    SkScalar fInnerXRadius;
    SkScalar fInnerXYRatio;
};

inline bool circle_stays_circle(const SkMatrix& m) {
    return m.isSimilarity();
}

}

bool GrOvalRenderer::drawOval(GrDrawTarget* target, const GrContext* context, const GrPaint& paint,
                    const GrRect& oval, const SkStrokeRec& stroke)
{
    if (!paint.isAntiAlias()) {
        return false;
    }

    const SkMatrix& vm = context->getMatrix();

    // we can draw circles
    if (SkScalarNearlyEqual(oval.width(), oval.height())
        && circle_stays_circle(vm)) {
        drawCircle(target, paint, oval, stroke);

    // and axis-aligned ellipses only
    } else if (vm.rectStaysRect()) {
        drawEllipse(target, paint, oval, stroke);

    } else {
        return false;
    }

    return true;
}

void GrOvalRenderer::drawCircle(GrDrawTarget* target,
                                const GrPaint& paint,
                                const GrRect& circle,
                                const SkStrokeRec& stroke)
{
    GrDrawState* drawState = target->drawState();

    const SkMatrix& vm = drawState->getViewMatrix();
    GrPoint center = GrPoint::Make(circle.centerX(), circle.centerY());
    vm.mapPoints(&center, 1);
    SkScalar radius = vm.mapRadius(SkScalarHalf(circle.width()));
    SkScalar strokeWidth = vm.mapRadius(stroke.getWidth());

    GrDrawState::AutoDeviceCoordDraw adcd(drawState);
    if (!adcd.succeeded()) {
        return;
    }

    // position + edge
    static const GrVertexAttrib kVertexAttribs[] = {
        {kVec2f_GrVertexAttribType, 0},
        {kVec4f_GrVertexAttribType, sizeof(GrPoint)}
    };
    drawState->setVertexAttribs(kVertexAttribs, SK_ARRAY_COUNT(kVertexAttribs));
    drawState->setAttribIndex(GrDrawState::kPosition_AttribIndex, 0);
    GrAssert(sizeof(CircleVertex) == drawState->getVertexSize());

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

    SkStrokeRec::Style style = stroke.getStyle();
    bool isStroked = (SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style);
    enum {
        // the edge effects share this stage with glyph rendering
        // (kGlyphMaskStage in GrTextContext) && SW path rendering
        // (kPathMaskStage in GrSWMaskHelper)
        kEdgeEffectStage = GrPaint::kTotalStages,
    };
    drawState->setAttribBindings(GrDrawState::kDefault_AttribBindings);

    GrEffectRef* effect = GrCircleEdgeEffect::Create(isStroked);
    static const int kCircleEdgeAttrIndex = 1;
    drawState->setEffect(kEdgeEffectStage, effect, kCircleEdgeAttrIndex)->unref();

    SkScalar innerRadius = 0.0f;
    SkScalar outerRadius = radius;
    SkScalar halfWidth = 0;
    if (style != SkStrokeRec::kFill_Style) {
        if (SkScalarNearlyZero(strokeWidth)) {
            halfWidth = SK_ScalarHalf;
        } else {
            halfWidth = SkScalarHalf(strokeWidth);
        }

        outerRadius += halfWidth;
        if (isStroked) {
            innerRadius = SkMaxScalar(0, radius - halfWidth);
        }
    }

    for (int i = 0; i < 4; ++i) {
        verts[i].fCenter = center;
        verts[i].fOuterRadius = outerRadius + 0.5f;
        verts[i].fInnerRadius = innerRadius - 0.5f;
    }

    // We've extended the outer radius out half a pixel to antialias.
    // Expand the drawn rect here so all the pixels will be captured.
    SkRect bounds = SkRect::MakeLTRB(
        center.fX - outerRadius - SK_ScalarHalf,
        center.fY - outerRadius - SK_ScalarHalf,
        center.fX + outerRadius + SK_ScalarHalf,
        center.fY + outerRadius + SK_ScalarHalf
    );

    verts[0].fPos = SkPoint::Make(bounds.fLeft,  bounds.fTop);
    verts[1].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[2].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);

    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, 4, &bounds);
}

void GrOvalRenderer::drawEllipse(GrDrawTarget* target,
                                 const GrPaint& paint,
                                 const GrRect& ellipse,
                                 const SkStrokeRec& stroke)
{
    GrDrawState* drawState = target->drawState();
#ifdef SK_DEBUG
    {
        // we should have checked for this previously
        bool isAxisAlignedEllipse = drawState->getViewMatrix().rectStaysRect();
        SkASSERT(paint.isAntiAlias() && isAxisAlignedEllipse);
    }
#endif

    const SkMatrix& vm = drawState->getViewMatrix();
    GrPoint center = GrPoint::Make(ellipse.centerX(), ellipse.centerY());
    vm.mapPoints(&center, 1);
    SkRect xformedRect;
    vm.mapRect(&xformedRect, ellipse);

    GrDrawState::AutoDeviceCoordDraw adcd(drawState);
    if (!adcd.succeeded()) {
        return;
    }

    // position + edge
    static const GrVertexAttrib kVertexAttribs[] = {
        {kVec2f_GrVertexAttribType, 0},
        {kVec2f_GrVertexAttribType, sizeof(GrPoint)},
        {kVec4f_GrVertexAttribType, 2*sizeof(GrPoint)}
    };
    drawState->setVertexAttribs(kVertexAttribs, SK_ARRAY_COUNT(kVertexAttribs));
    drawState->setAttribIndex(GrDrawState::kPosition_AttribIndex, 0);
    GrAssert(sizeof(EllipseVertex) == drawState->getVertexSize());

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(geo.vertices());

    SkStrokeRec::Style style = stroke.getStyle();
    bool isStroked = (SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style);
    enum {
        // the edge effects share this stage with glyph rendering
        // (kGlyphMaskStage in GrTextContext) && SW path rendering
        // (kPathMaskStage in GrSWMaskHelper)
        kEdgeEffectStage = GrPaint::kTotalStages,
    };
    drawState->setAttribBindings(GrDrawState::kDefault_AttribBindings);

    GrEffectRef* effect = GrEllipseEdgeEffect::Create(isStroked);
    static const int kEllipseCenterAttrIndex = 1;
    static const int kEllipseEdgeAttrIndex = 2;
    drawState->setEffect(kEdgeEffectStage, effect,
                         kEllipseCenterAttrIndex, kEllipseEdgeAttrIndex)->unref();

    SkScalar xRadius = SkScalarHalf(xformedRect.width());
    SkScalar yRadius = SkScalarHalf(xformedRect.height());
    SkScalar innerXRadius = 0.0f;
    SkScalar innerRatio = 1.0f;

    if (SkStrokeRec::kFill_Style != style) {
        SkScalar strokeWidth = stroke.getWidth();

        // do (potentially) anisotropic mapping
        SkVector scaledStroke;
        scaledStroke.set(strokeWidth, strokeWidth);
        vm.mapVectors(&scaledStroke, 1);

        if (SkScalarNearlyZero(scaledStroke.length())) {
            scaledStroke.set(SK_ScalarHalf, SK_ScalarHalf);
        } else {
            scaledStroke.scale(0.5f);
        }

        // this is legit only if scale & translation (which should be the case at the moment)
        if (SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style) {
            SkScalar innerYRadius = SkMaxScalar(0, yRadius - scaledStroke.fY);
            if (innerYRadius > SK_ScalarNearlyZero) {
                innerXRadius = SkMaxScalar(0, xRadius - scaledStroke.fX);
                innerRatio = innerXRadius/innerYRadius;
            }
        }
        xRadius += scaledStroke.fX;
        yRadius += scaledStroke.fY;
    }

    SkScalar outerRatio = SkScalarDiv(xRadius, yRadius);

    for (int i = 0; i < 4; ++i) {
        verts[i].fCenter = center;
        verts[i].fOuterXRadius = xRadius + 0.5f;
        verts[i].fOuterXYRatio = outerRatio;
        verts[i].fInnerXRadius = innerXRadius - 0.5f;
        verts[i].fInnerXYRatio = innerRatio;
    }

    SkScalar L = -xRadius;
    SkScalar R = +xRadius;
    SkScalar T = -yRadius;
    SkScalar B = +yRadius;

    // We've extended the outer x radius out half a pixel to antialias.
    // Expand the drawn rect here so all the pixels will be captured.
    L += center.fX - SK_ScalarHalf;
    R += center.fX + SK_ScalarHalf;
    T += center.fY - SK_ScalarHalf;
    B += center.fY + SK_ScalarHalf;

    verts[0].fPos = SkPoint::Make(L, T);
    verts[1].fPos = SkPoint::Make(R, T);
    verts[2].fPos = SkPoint::Make(L, B);
    verts[3].fPos = SkPoint::Make(R, B);

    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, 4);
}
