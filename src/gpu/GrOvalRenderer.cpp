/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalRenderer.h"

#include "GrEffect.h"
#include "gl/GrGLEffect.h"
#include "gl/GrGLSL.h"
#include "GrTBackendEffectFactory.h"

#include "GrDrawState.h"
#include "GrDrawTarget.h"
#include "SkStrokeRec.h"

SK_DEFINE_INST_COUNT(GrOvalRenderer)

namespace {

struct CircleVertex {
    GrPoint  fPos;
    GrPoint  fOffset;
    SkScalar fOuterRadius;
    SkScalar fInnerRadius;
};

struct EllipseVertex {
    GrPoint  fPos;
    SkScalar fOuterXRadius;
    SkScalar fInnerXRadius;
    GrPoint  fOuterOffset;
    GrPoint  fInnerOffset;
};

inline bool circle_stays_circle(const SkMatrix& m) {
    return m.isSimilarity();
}

}

///////////////////////////////////////////////////////////////////////////////

/**
 * The output of this effect is a modulation of the input color and coverage for a circle,
 * specified as offset_x, offset_y (both from center point), outer radius and inner radius.
 */

class CircleEdgeEffect : public GrEffect {
public:
    static GrEffectRef* Create(bool stroke) {
        GR_CREATE_STATIC_EFFECT(gCircleStrokeEdge, CircleEdgeEffect, (true));
        GR_CREATE_STATIC_EFFECT(gCircleFillEdge, CircleEdgeEffect, (false));

        if (stroke) {
            gCircleStrokeEdge->ref();
            return gCircleStrokeEdge;
        } else {
            gCircleFillEdge->ref();
            return gCircleFillEdge;
        }
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<CircleEdgeEffect>::getInstance();
    }

    virtual ~CircleEdgeEffect() {}

    static const char* Name() { return "CircleEdge"; }

    inline bool isStroked() const { return fStroke; }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
        : INHERITED (factory) {}

        virtual void emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            const CircleEdgeEffect& circleEffect = drawEffect.castEffect<CircleEdgeEffect>();
            const char *vsName, *fsName;
            builder->addVarying(kVec4f_GrSLType, "CircleEdge", &vsName, &fsName);

            const SkString* attrName =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsName, attrName->c_str());

            builder->fsCodeAppendf("\tfloat d = length(%s.xy);\n", fsName);
            builder->fsCodeAppendf("\tfloat edgeAlpha = clamp(%s.z - d, 0.0, 1.0);\n", fsName);
            if (circleEffect.isStroked()) {
                builder->fsCodeAppendf("\tfloat innerAlpha = clamp(d - %s.w, 0.0, 1.0);\n", fsName);
                builder->fsCodeAppend("\tedgeAlpha *= innerAlpha;\n");
            }

            SkString modulate;
            GrGLSLModulatef<4>(&modulate, inputColor, "edgeAlpha");
            builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            const CircleEdgeEffect& circleEffect = drawEffect.castEffect<CircleEdgeEffect>();

            return circleEffect.isStroked() ? 0x1 : 0x0;
        }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

    private:
        typedef GrGLEffect INHERITED;
    };


private:
    CircleEdgeEffect(bool stroke) : GrEffect() {
        this->addVertexAttrib(kVec4f_GrSLType);
        fStroke = stroke;
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const CircleEdgeEffect& cee = CastEffect<CircleEdgeEffect>(other);
        return cee.fStroke == fStroke;
    }

    bool fStroke;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(CircleEdgeEffect);

GrEffectRef* CircleEdgeEffect::TestCreate(SkMWCRandom* random,
                                          GrContext* context,
                                          const GrDrawTargetCaps&,
                                          GrTexture* textures[]) {
    return CircleEdgeEffect::Create(random->nextBool());
}

///////////////////////////////////////////////////////////////////////////////

/**
 * The output of this effect is a modulation of the input color and coverage for an axis-aligned
 * ellipse, specified as  outer and inner radii, and outer and inner offsets from center.
 */

class EllipseEdgeEffect : public GrEffect {
public:
    static GrEffectRef* Create(bool stroke) {
        GR_CREATE_STATIC_EFFECT(gEllipseStrokeEdge, EllipseEdgeEffect, (true));
        GR_CREATE_STATIC_EFFECT(gEllipseFillEdge, EllipseEdgeEffect, (false));

        if (stroke) {
            gEllipseStrokeEdge->ref();
            return gEllipseStrokeEdge;
        } else {
            gEllipseFillEdge->ref();
            return gEllipseFillEdge;
        }
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<EllipseEdgeEffect>::getInstance();
    }

    virtual ~EllipseEdgeEffect() {}

    static const char* Name() { return "EllipseEdge"; }

    inline bool isStroked() const { return fStroke; }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
        : INHERITED (factory) {}

        virtual void emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            const EllipseEdgeEffect& ellipseEffect = drawEffect.castEffect<EllipseEdgeEffect>();

            const char *vsRadiiName, *fsRadiiName;
            const char *vsOffsetsName, *fsOffsetsName;

            builder->addVarying(kVec2f_GrSLType, "EllipseRadii", &vsRadiiName, &fsRadiiName);
            const SkString* attr0Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsRadiiName, attr0Name->c_str());

            builder->addVarying(kVec4f_GrSLType, "EllipseOffsets", &vsOffsetsName, &fsOffsetsName);
            const SkString* attr1Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[1]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsOffsetsName, attr1Name->c_str());

            // get length of offset
            builder->fsCodeAppendf("\tfloat dOuter = length(%s.xy);\n", fsOffsetsName);
            // compare outer lengths against xOuterRadius
            builder->fsCodeAppendf("\tfloat edgeAlpha = clamp(%s.x-dOuter, 0.0, 1.0);\n",
                                   fsRadiiName);

            if (ellipseEffect.isStroked()) {
                builder->fsCodeAppendf("\tfloat dInner = length(%s.zw);\n", fsOffsetsName);

                // compare inner lengths against xInnerRadius
                builder->fsCodeAppendf("\tfloat innerAlpha = clamp(dInner-%s.y, 0.0, 1.0);\n",
                                       fsRadiiName);
                builder->fsCodeAppend("\tedgeAlpha *= innerAlpha;\n");
            }

            SkString modulate;
            GrGLSLModulatef<4>(&modulate, inputColor, "edgeAlpha");
            builder->fsCodeAppendf("\t%s = %s;\n", outputColor, modulate.c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            const EllipseEdgeEffect& ellipseEffect = drawEffect.castEffect<EllipseEdgeEffect>();

            return ellipseEffect.isStroked() ? 0x1 : 0x0;
        }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {
        }

    private:
        typedef GrGLEffect INHERITED;
    };

private:
    EllipseEdgeEffect(bool stroke) : GrEffect() {
        this->addVertexAttrib(kVec2f_GrSLType);
        this->addVertexAttrib(kVec4f_GrSLType);
        fStroke = stroke;
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const EllipseEdgeEffect& eee = CastEffect<EllipseEdgeEffect>(other);
        return eee.fStroke == fStroke;
    }

    bool fStroke;

    GR_DECLARE_EFFECT_TEST;

    typedef GrEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(EllipseEdgeEffect);

GrEffectRef* EllipseEdgeEffect::TestCreate(SkMWCRandom* random,
                                           GrContext* context,
                                           const GrDrawTargetCaps&,
                                           GrTexture* textures[]) {
    return EllipseEdgeEffect::Create(random->nextBool());
}

///////////////////////////////////////////////////////////////////////////////

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
        return drawEllipse(target, paint, oval, stroke);

    } else {
        return false;
    }

    return true;
}

namespace {

// position + edge
extern const GrVertexAttrib gCircleVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,               kPosition_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, sizeof(GrPoint), kEffect_GrVertexAttribBinding}
};

};

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

    drawState->setVertexAttribs<gCircleVertexAttribs>(SK_ARRAY_COUNT(gCircleVertexAttribs));
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

    GrEffectRef* effect = CircleEdgeEffect::Create(isStroked);
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

    // The radii are outset for two reasons. First, it allows the shader to simply perform
    // clamp(distance-to-center - radius, 0, 1). Second, the outer radius is used to compute the
    // verts of the bounding box that is rendered and the outset ensures the box will cover all
    // pixels partially covered by the circle.
    outerRadius += SK_ScalarHalf;
    innerRadius -= SK_ScalarHalf;

    SkRect bounds = SkRect::MakeLTRB(
        center.fX - outerRadius,
        center.fY - outerRadius,
        center.fX + outerRadius,
        center.fY + outerRadius
    );

    verts[0].fPos = SkPoint::Make(bounds.fLeft,  bounds.fTop);
    verts[0].fOffset = SkPoint::Make(-outerRadius, -outerRadius);
    verts[0].fOuterRadius = outerRadius;
    verts[0].fInnerRadius = innerRadius;

    verts[1].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[1].fOffset = SkPoint::Make(outerRadius, -outerRadius);
    verts[1].fOuterRadius = outerRadius;
    verts[1].fInnerRadius = innerRadius;

    verts[2].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[2].fOffset = SkPoint::Make(-outerRadius, outerRadius);
    verts[2].fOuterRadius = outerRadius;
    verts[2].fInnerRadius = innerRadius;

    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
    verts[3].fOffset = SkPoint::Make(outerRadius, outerRadius);
    verts[3].fOuterRadius = outerRadius;
    verts[3].fInnerRadius = innerRadius;

    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, 4, &bounds);
}

namespace {

// position + edge
extern const GrVertexAttrib gEllipseVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(GrPoint),   kEffect_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, 2*sizeof(GrPoint), kEffect_GrVertexAttribBinding}
};

};

bool GrOvalRenderer::drawEllipse(GrDrawTarget* target,
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

    // do any matrix crunching before we reset the draw state for device coords
    const SkMatrix& vm = drawState->getViewMatrix();
    GrPoint center = GrPoint::Make(ellipse.centerX(), ellipse.centerY());
    vm.mapPoints(&center, 1);
    SkScalar ellipseXRadius = SkScalarHalf(ellipse.width());
    SkScalar ellipseYRadius = SkScalarHalf(ellipse.height());
    SkScalar xRadius = SkScalarAbs(vm[SkMatrix::kMScaleX]*ellipseXRadius +
                                   vm[SkMatrix::kMSkewY]*ellipseYRadius);
    SkScalar yRadius = SkScalarAbs(vm[SkMatrix::kMSkewX]*ellipseXRadius +
                                   vm[SkMatrix::kMScaleY]*ellipseYRadius);
    if (SkScalarDiv(xRadius, yRadius) > 2 || SkScalarDiv(yRadius, xRadius) > 2) {
        return false;
    }

    // do (potentially) anisotropic mapping of stroke
    SkVector scaledStroke;
    SkScalar strokeWidth = stroke.getWidth();
    scaledStroke.fX = SkScalarAbs(strokeWidth*(vm[SkMatrix::kMScaleX] + vm[SkMatrix::kMSkewY]));
    scaledStroke.fY = SkScalarAbs(strokeWidth*(vm[SkMatrix::kMSkewX] + vm[SkMatrix::kMScaleY]));

    GrDrawState::AutoDeviceCoordDraw adcd(drawState);
    if (!adcd.succeeded()) {
        return false;
    }

    drawState->setVertexAttribs<gEllipseVertexAttribs>(SK_ARRAY_COUNT(gEllipseVertexAttribs));
    GrAssert(sizeof(EllipseVertex) == drawState->getVertexSize());

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return false;
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

    GrEffectRef* effect = EllipseEdgeEffect::Create(isStroked);
    static const int kEllipseCenterAttrIndex = 1;
    static const int kEllipseEdgeAttrIndex = 2;
    drawState->setEffect(kEdgeEffectStage, effect,
                         kEllipseCenterAttrIndex, kEllipseEdgeAttrIndex)->unref();

    SkScalar innerXRadius = 0.0f;
    SkScalar innerRatio = 1.0f;

    if (SkStrokeRec::kFill_Style != style) {


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

    // We've extended the outer x radius out half a pixel to antialias.
    // This will also expand the rect so all the pixels will be captured.
    xRadius += SK_ScalarHalf;
    yRadius += SK_ScalarHalf;
    innerXRadius -= SK_ScalarHalf;

    SkRect bounds = SkRect::MakeLTRB(
        center.fX - xRadius,
        center.fY - yRadius,
        center.fX + xRadius,
        center.fY + yRadius
    );

    // The offsets are created by scaling the y radius by the appropriate ratio. This way we end up
    // with a circle equation which can be checked quickly in the shader. We need one offset for
    // outer and one for inner because they have different scale factors -- otherwise we end up with
    // non-uniform strokes.
    verts[0].fPos = SkPoint::Make(bounds.fLeft,  bounds.fTop);
    verts[0].fOuterXRadius = xRadius;
    verts[0].fInnerXRadius = innerXRadius;
    verts[0].fOuterOffset = SkPoint::Make(-xRadius, -outerRatio*yRadius);
    verts[0].fInnerOffset = SkPoint::Make(-xRadius, -innerRatio*yRadius);

    verts[1].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[1].fOuterXRadius = xRadius;
    verts[1].fInnerXRadius = innerXRadius;
    verts[1].fOuterOffset = SkPoint::Make(xRadius, -outerRatio*yRadius);
    verts[1].fInnerOffset = SkPoint::Make(xRadius, -innerRatio*yRadius);

    verts[2].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[2].fOuterXRadius = xRadius;
    verts[2].fInnerXRadius = innerXRadius;
    verts[2].fOuterOffset = SkPoint::Make(-xRadius, outerRatio*yRadius);
    verts[2].fInnerOffset = SkPoint::Make(-xRadius, innerRatio*yRadius);

    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
    verts[3].fOuterXRadius = xRadius;
    verts[3].fInnerXRadius = innerXRadius;
    verts[3].fOuterOffset = SkPoint::Make(xRadius, outerRatio*yRadius);
    verts[3].fInnerOffset = SkPoint::Make(xRadius, innerRatio*yRadius);

    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, 4, &bounds);

    return true;
}
