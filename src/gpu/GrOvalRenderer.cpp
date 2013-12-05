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
#include "gl/GrGLVertexEffect.h"
#include "GrTBackendEffectFactory.h"

#include "GrDrawState.h"
#include "GrDrawTarget.h"
#include "GrGpu.h"

#include "SkRRect.h"
#include "SkStrokeRec.h"

#include "effects/GrVertexEffect.h"

namespace {

struct CircleVertex {
    GrPoint  fPos;
    GrPoint  fOffset;
    SkScalar fOuterRadius;
    SkScalar fInnerRadius;
};

struct EllipseVertex {
    GrPoint  fPos;
    GrPoint  fOffset;
    GrPoint  fOuterRadii;
    GrPoint  fInnerRadii;
};

struct DIEllipseVertex {
    GrPoint  fPos;
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

class CircleEdgeEffect : public GrVertexEffect {
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

    class GLEffect : public GrGLVertexEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
        : INHERITED (factory) {}

        virtual void emitCode(GrGLFullShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
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

            builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            const CircleEdgeEffect& circleEffect = drawEffect.castEffect<CircleEdgeEffect>();

            return circleEffect.isStroked() ? 0x1 : 0x0;
        }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

    private:
        typedef GrGLVertexEffect INHERITED;
    };


private:
    CircleEdgeEffect(bool stroke) : GrVertexEffect() {
        this->addVertexAttrib(kVec4f_GrSLType);
        fStroke = stroke;
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const CircleEdgeEffect& cee = CastEffect<CircleEdgeEffect>(other);
        return cee.fStroke == fStroke;
    }

    bool fStroke;

    GR_DECLARE_EFFECT_TEST;

    typedef GrVertexEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(CircleEdgeEffect);

GrEffectRef* CircleEdgeEffect::TestCreate(SkRandom* random,
                                          GrContext* context,
                                          const GrDrawTargetCaps&,
                                          GrTexture* textures[]) {
    return CircleEdgeEffect::Create(random->nextBool());
}

///////////////////////////////////////////////////////////////////////////////

/**
 * The output of this effect is a modulation of the input color and coverage for an axis-aligned
 * ellipse, specified as a 2D offset from center, and the reciprocals of the outer and inner radii,
 * in both x and y directions.
 *
 * We are using an implicit function of x^2/a^2 + y^2/b^2 - 1 = 0.
 */

class EllipseEdgeEffect : public GrVertexEffect {
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

    class GLEffect : public GrGLVertexEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
        : INHERITED (factory) {}

        virtual void emitCode(GrGLFullShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            const EllipseEdgeEffect& ellipseEffect = drawEffect.castEffect<EllipseEdgeEffect>();

            const char *vsOffsetName, *fsOffsetName;
            const char *vsRadiiName, *fsRadiiName;

            builder->addVarying(kVec2f_GrSLType, "EllipseOffsets", &vsOffsetName, &fsOffsetName);
            const SkString* attr0Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsOffsetName, attr0Name->c_str());

            builder->addVarying(kVec4f_GrSLType, "EllipseRadii", &vsRadiiName, &fsRadiiName);
            const SkString* attr1Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[1]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsRadiiName, attr1Name->c_str());

            // for outer curve
            builder->fsCodeAppendf("\tvec2 scaledOffset = %s*%s.xy;\n", fsOffsetName, fsRadiiName);
            builder->fsCodeAppend("\tfloat test = dot(scaledOffset, scaledOffset) - 1.0;\n");
            builder->fsCodeAppendf("\tvec2 grad = 2.0*scaledOffset*%s.xy;\n", fsRadiiName);
            builder->fsCodeAppend("\tfloat grad_dot = dot(grad, grad);\n");
            // we need to clamp the length^2 of the gradiant vector to a non-zero value, because
            // on the Nexus 4 the undefined result of inversesqrt(0) drops out an entire tile
            // TODO: restrict this to Adreno-only
            builder->fsCodeAppend("\tgrad_dot = max(grad_dot, 1.0e-4);\n");
            builder->fsCodeAppend("\tfloat invlen = inversesqrt(grad_dot);\n");
            builder->fsCodeAppend("\tfloat edgeAlpha = clamp(0.5-test*invlen, 0.0, 1.0);\n");

            // for inner curve
            if (ellipseEffect.isStroked()) {
                builder->fsCodeAppendf("\tscaledOffset = %s*%s.zw;\n", fsOffsetName, fsRadiiName);
                builder->fsCodeAppend("\ttest = dot(scaledOffset, scaledOffset) - 1.0;\n");
                builder->fsCodeAppendf("\tgrad = 2.0*scaledOffset*%s.zw;\n", fsRadiiName);
                builder->fsCodeAppend("\tinvlen = inversesqrt(dot(grad, grad));\n");
                builder->fsCodeAppend("\tedgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);\n");
            }

            builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            const EllipseEdgeEffect& ellipseEffect = drawEffect.castEffect<EllipseEdgeEffect>();

            return ellipseEffect.isStroked() ? 0x1 : 0x0;
        }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {
        }

    private:
        typedef GrGLVertexEffect INHERITED;
    };

private:
    EllipseEdgeEffect(bool stroke) : GrVertexEffect() {
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

    typedef GrVertexEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(EllipseEdgeEffect);

GrEffectRef* EllipseEdgeEffect::TestCreate(SkRandom* random,
                                           GrContext* context,
                                           const GrDrawTargetCaps&,
                                           GrTexture* textures[]) {
    return EllipseEdgeEffect::Create(random->nextBool());
}

///////////////////////////////////////////////////////////////////////////////

/**
 * The output of this effect is a modulation of the input color and coverage for an ellipse,
 * specified as a 2D offset from center for both the outer and inner paths (if stroked). The
 * implict equation used is for a unit circle (x^2 + y^2 - 1 = 0) and the edge corrected by
 * using differentials.
 *
 * The result is device-independent and can be used with any affine matrix.
 */

class DIEllipseEdgeEffect : public GrVertexEffect {
public:
    enum Mode { kStroke = 0, kHairline, kFill };

    static GrEffectRef* Create(Mode mode) {
        GR_CREATE_STATIC_EFFECT(gEllipseStrokeEdge, DIEllipseEdgeEffect, (kStroke));
        GR_CREATE_STATIC_EFFECT(gEllipseHairlineEdge, DIEllipseEdgeEffect, (kHairline));
        GR_CREATE_STATIC_EFFECT(gEllipseFillEdge, DIEllipseEdgeEffect, (kFill));

        if (kStroke == mode) {
            gEllipseStrokeEdge->ref();
            return gEllipseStrokeEdge;
        } else if (kHairline == mode) {
            gEllipseHairlineEdge->ref();
            return gEllipseHairlineEdge;
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
        return GrTBackendEffectFactory<DIEllipseEdgeEffect>::getInstance();
    }

    virtual ~DIEllipseEdgeEffect() {}

    static const char* Name() { return "DIEllipseEdge"; }

    inline Mode getMode() const { return fMode; }

    class GLEffect : public GrGLVertexEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
        : INHERITED (factory) {}

        virtual void emitCode(GrGLFullShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            const DIEllipseEdgeEffect& ellipseEffect = drawEffect.castEffect<DIEllipseEdgeEffect>();

            SkAssertResult(builder->enableFeature(
                                              GrGLShaderBuilder::kStandardDerivatives_GLSLFeature));

            const char *vsOffsetName0, *fsOffsetName0;
            builder->addVarying(kVec2f_GrSLType, "EllipseOffsets0",
                                      &vsOffsetName0, &fsOffsetName0);
            const SkString* attr0Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[0]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsOffsetName0, attr0Name->c_str());
            const char *vsOffsetName1, *fsOffsetName1;
            builder->addVarying(kVec2f_GrSLType, "EllipseOffsets1",
                                      &vsOffsetName1, &fsOffsetName1);
            const SkString* attr1Name =
                builder->getEffectAttributeName(drawEffect.getVertexAttribIndices()[1]);
            builder->vsCodeAppendf("\t%s = %s;\n", vsOffsetName1, attr1Name->c_str());

            // for outer curve
            builder->fsCodeAppendf("\tvec2 scaledOffset = %s.xy;\n", fsOffsetName0);
            builder->fsCodeAppend("\tfloat test = dot(scaledOffset, scaledOffset) - 1.0;\n");
            builder->fsCodeAppendf("\tvec2 duvdx = dFdx(%s);\n", fsOffsetName0);
            builder->fsCodeAppendf("\tvec2 duvdy = dFdy(%s);\n", fsOffsetName0);
            builder->fsCodeAppendf("\tvec2 grad = vec2(2.0*%s.x*duvdx.x + 2.0*%s.y*duvdx.y,\n"
                                   "\t                 2.0*%s.x*duvdy.x + 2.0*%s.y*duvdy.y);\n",
                                   fsOffsetName0, fsOffsetName0, fsOffsetName0, fsOffsetName0);

            builder->fsCodeAppend("\tfloat grad_dot = dot(grad, grad);\n");
            // we need to clamp the length^2 of the gradiant vector to a non-zero value, because
            // on the Nexus 4 the undefined result of inversesqrt(0) drops out an entire tile
            // TODO: restrict this to Adreno-only
            builder->fsCodeAppend("\tgrad_dot = max(grad_dot, 1.0e-4);\n");
            builder->fsCodeAppend("\tfloat invlen = inversesqrt(grad_dot);\n");
            if (kHairline == ellipseEffect.getMode()) {
                // can probably do this with one step
                builder->fsCodeAppend("\tfloat edgeAlpha = clamp(1.0-test*invlen, 0.0, 1.0);\n");
                builder->fsCodeAppend("\tedgeAlpha *= clamp(1.0+test*invlen, 0.0, 1.0);\n");
            } else {
                builder->fsCodeAppend("\tfloat edgeAlpha = clamp(0.5-test*invlen, 0.0, 1.0);\n");
            }

            // for inner curve
            if (kStroke == ellipseEffect.getMode()) {
                builder->fsCodeAppendf("\tscaledOffset = %s.xy;\n", fsOffsetName1);
                builder->fsCodeAppend("\ttest = dot(scaledOffset, scaledOffset) - 1.0;\n");
                builder->fsCodeAppendf("\tduvdx = dFdx(%s);\n", fsOffsetName1);
                builder->fsCodeAppendf("\tduvdy = dFdy(%s);\n", fsOffsetName1);
                builder->fsCodeAppendf("\tgrad = vec2(2.0*%s.x*duvdx.x + 2.0*%s.y*duvdx.y,\n"
                                       "\t            2.0*%s.x*duvdy.x + 2.0*%s.y*duvdy.y);\n",
                                       fsOffsetName1, fsOffsetName1, fsOffsetName1, fsOffsetName1);
                builder->fsCodeAppend("\tinvlen = inversesqrt(dot(grad, grad));\n");
                builder->fsCodeAppend("\tedgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);\n");
            }

            builder->fsCodeAppendf("\t%s = %s;\n", outputColor,
                                   (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
        }

        static inline EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
            const DIEllipseEdgeEffect& ellipseEffect = drawEffect.castEffect<DIEllipseEdgeEffect>();

            return ellipseEffect.getMode();
        }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {
        }

    private:
        typedef GrGLVertexEffect INHERITED;
    };

private:
    DIEllipseEdgeEffect(Mode mode) : GrVertexEffect() {
        this->addVertexAttrib(kVec2f_GrSLType);
        this->addVertexAttrib(kVec2f_GrSLType);
        fMode = mode;
    }

    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE {
        const DIEllipseEdgeEffect& eee = CastEffect<DIEllipseEdgeEffect>(other);
        return eee.fMode == fMode;
    }

    Mode fMode;

    GR_DECLARE_EFFECT_TEST;

    typedef GrVertexEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(DIEllipseEdgeEffect);

GrEffectRef* DIEllipseEdgeEffect::TestCreate(SkRandom* random,
                                             GrContext* context,
                                             const GrDrawTargetCaps&,
                                             GrTexture* textures[]) {
    return DIEllipseEdgeEffect::Create((Mode)(random->nextRangeU(0,2)));
}

///////////////////////////////////////////////////////////////////////////////

void GrOvalRenderer::reset() {
    SkSafeSetNull(fRRectIndexBuffer);
}

bool GrOvalRenderer::drawOval(GrDrawTarget* target, const GrContext* context, bool useAA,
                              const SkRect& oval, const SkStrokeRec& stroke)
{
    bool useCoverageAA = useAA &&
        !target->getDrawState().getRenderTarget()->isMultisampled() &&
        !target->shouldDisableCoverageAAForBlend();

    if (!useCoverageAA) {
        return false;
    }

    const SkMatrix& vm = context->getMatrix();

    // we can draw circles
    if (SkScalarNearlyEqual(oval.width(), oval.height())
        && circle_stays_circle(vm)) {
        this->drawCircle(target, useCoverageAA, oval, stroke);
    // if we have shader derivative support, render as device-independent
    } else if (target->caps()->shaderDerivativeSupport()) {
        return this->drawDIEllipse(target, useCoverageAA, oval, stroke);
    // otherwise axis-aligned ellipses only
    } else if (vm.rectStaysRect()) {
        return this->drawEllipse(target, useCoverageAA, oval, stroke);
    } else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

// position + edge
extern const GrVertexAttrib gCircleVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,               kPosition_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, sizeof(GrPoint), kEffect_GrVertexAttribBinding}
};

void GrOvalRenderer::drawCircle(GrDrawTarget* target,
                                bool useCoverageAA,
                                const SkRect& circle,
                                const SkStrokeRec& stroke)
{
    GrDrawState* drawState = target->drawState();

    const SkMatrix& vm = drawState->getViewMatrix();
    GrPoint center = GrPoint::Make(circle.centerX(), circle.centerY());
    vm.mapPoints(&center, 1);
    SkScalar radius = vm.mapRadius(SkScalarHalf(circle.width()));
    SkScalar strokeWidth = vm.mapRadius(stroke.getWidth());

    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(drawState)) {
        return;
    }

    drawState->setVertexAttribs<gCircleVertexAttribs>(SK_ARRAY_COUNT(gCircleVertexAttribs));
    SkASSERT(sizeof(CircleVertex) == drawState->getVertexSize());

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

    SkStrokeRec::Style style = stroke.getStyle();
    bool isStroked = (SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style);

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
            innerRadius = radius - halfWidth;
        }
    }

    GrEffectRef* effect = CircleEdgeEffect::Create(isStroked && innerRadius > 0);
    static const int kCircleEdgeAttrIndex = 1;
    drawState->addCoverageEffect(effect, kCircleEdgeAttrIndex)->unref();

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

///////////////////////////////////////////////////////////////////////////////

// position + offset + 1/radii
extern const GrVertexAttrib gEllipseVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(GrPoint),   kEffect_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, 2*sizeof(GrPoint), kEffect_GrVertexAttribBinding}
};

// position + offsets
extern const GrVertexAttrib gDIEllipseVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(GrPoint),   kEffect_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, 2*sizeof(GrPoint), kEffect_GrVertexAttribBinding},
};

bool GrOvalRenderer::drawEllipse(GrDrawTarget* target,
                                 bool useCoverageAA,
                                 const SkRect& ellipse,
                                 const SkStrokeRec& stroke)
{
    GrDrawState* drawState = target->drawState();
#ifdef SK_DEBUG
    {
        // we should have checked for this previously
        bool isAxisAlignedEllipse = drawState->getViewMatrix().rectStaysRect();
        SkASSERT(useCoverageAA && isAxisAlignedEllipse);
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

    // do (potentially) anisotropic mapping of stroke
    SkVector scaledStroke;
    SkScalar strokeWidth = stroke.getWidth();
    scaledStroke.fX = SkScalarAbs(strokeWidth*(vm[SkMatrix::kMScaleX] + vm[SkMatrix::kMSkewY]));
    scaledStroke.fY = SkScalarAbs(strokeWidth*(vm[SkMatrix::kMSkewX] + vm[SkMatrix::kMScaleY]));

    SkStrokeRec::Style style = stroke.getStyle();
    bool isStroked = (SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style);

    SkScalar innerXRadius = 0;
    SkScalar innerYRadius = 0;
    if (SkStrokeRec::kFill_Style != style) {
        if (SkScalarNearlyZero(scaledStroke.length())) {
            scaledStroke.set(SK_ScalarHalf, SK_ScalarHalf);
        } else {
            scaledStroke.scale(SK_ScalarHalf);
        }

        // we only handle thick strokes for near-circular ellipses
        if (scaledStroke.length() > SK_ScalarHalf &&
            (SK_ScalarHalf*xRadius > yRadius || SK_ScalarHalf*yRadius > xRadius)) {
            return false;
        }

        // we don't handle it if curvature of the stroke is less than curvature of the ellipse
        if (scaledStroke.fX*(yRadius*yRadius) < (scaledStroke.fY*scaledStroke.fY)*xRadius ||
            scaledStroke.fY*(xRadius*xRadius) < (scaledStroke.fX*scaledStroke.fX)*yRadius) {
            return false;
        }

        // this is legit only if scale & translation (which should be the case at the moment)
        if (isStroked) {
            innerXRadius = xRadius - scaledStroke.fX;
            innerYRadius = yRadius - scaledStroke.fY;
        }

        xRadius += scaledStroke.fX;
        yRadius += scaledStroke.fY;
    }

    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(drawState)) {
        return false;
    }

    drawState->setVertexAttribs<gEllipseVertexAttribs>(SK_ARRAY_COUNT(gEllipseVertexAttribs));
    SkASSERT(sizeof(EllipseVertex) == drawState->getVertexSize());

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return false;
    }

    EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(geo.vertices());

    GrEffectRef* effect = EllipseEdgeEffect::Create(isStroked &&
                                                    innerXRadius > 0 && innerYRadius > 0);

    static const int kEllipseCenterAttrIndex = 1;
    static const int kEllipseEdgeAttrIndex = 2;
    drawState->addCoverageEffect(effect, kEllipseCenterAttrIndex, kEllipseEdgeAttrIndex)->unref();

    // Compute the reciprocals of the radii here to save time in the shader
    SkScalar xRadRecip = SkScalarInvert(xRadius);
    SkScalar yRadRecip = SkScalarInvert(yRadius);
    SkScalar xInnerRadRecip = SkScalarInvert(innerXRadius);
    SkScalar yInnerRadRecip = SkScalarInvert(innerYRadius);

    // We've extended the outer x radius out half a pixel to antialias.
    // This will also expand the rect so all the pixels will be captured.
    // TODO: Consider if we should use sqrt(2)/2 instead
    xRadius += SK_ScalarHalf;
    yRadius += SK_ScalarHalf;

    SkRect bounds = SkRect::MakeLTRB(
        center.fX - xRadius,
        center.fY - yRadius,
        center.fX + xRadius,
        center.fY + yRadius
    );

    verts[0].fPos = SkPoint::Make(bounds.fLeft,  bounds.fTop);
    verts[0].fOffset = SkPoint::Make(-xRadius, -yRadius);
    verts[0].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
    verts[0].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

    verts[1].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[1].fOffset = SkPoint::Make(xRadius, -yRadius);
    verts[1].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
    verts[1].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

    verts[2].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[2].fOffset = SkPoint::Make(-xRadius, yRadius);
    verts[2].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
    verts[2].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
    verts[3].fOffset = SkPoint::Make(xRadius, yRadius);
    verts[3].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
    verts[3].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, 4, &bounds);

    return true;
}

bool GrOvalRenderer::drawDIEllipse(GrDrawTarget* target,
                                   bool useCoverageAA,
                                   const SkRect& ellipse,
                                   const SkStrokeRec& stroke)
{
    GrDrawState* drawState = target->drawState();
    const SkMatrix& vm = drawState->getViewMatrix();

    GrPoint center = GrPoint::Make(ellipse.centerX(), ellipse.centerY());
    SkScalar xRadius = SkScalarHalf(ellipse.width());
    SkScalar yRadius = SkScalarHalf(ellipse.height());

    SkStrokeRec::Style style = stroke.getStyle();
    DIEllipseEdgeEffect::Mode mode = (SkStrokeRec::kStroke_Style == style) ?
                                    DIEllipseEdgeEffect::kStroke :
                                    (SkStrokeRec::kHairline_Style == style) ?
                                    DIEllipseEdgeEffect::kHairline : DIEllipseEdgeEffect::kFill;

    SkScalar innerXRadius = 0;
    SkScalar innerYRadius = 0;
    if (SkStrokeRec::kFill_Style != style && SkStrokeRec::kHairline_Style != style) {
        SkScalar strokeWidth = stroke.getWidth();

        if (SkScalarNearlyZero(strokeWidth)) {
            strokeWidth = SK_ScalarHalf;
        } else {
            strokeWidth *= SK_ScalarHalf;
        }

        // we only handle thick strokes for near-circular ellipses
        if (strokeWidth > SK_ScalarHalf &&
            (SK_ScalarHalf*xRadius > yRadius || SK_ScalarHalf*yRadius > xRadius)) {
            return false;
        }

        // we don't handle it if curvature of the stroke is less than curvature of the ellipse
        if (strokeWidth*(yRadius*yRadius) < (strokeWidth*strokeWidth)*xRadius ||
            strokeWidth*(xRadius*xRadius) < (strokeWidth*strokeWidth)*yRadius) {
            return false;
        }

        // set inner radius (if needed)
        if (SkStrokeRec::kStroke_Style == style) {
            innerXRadius = xRadius - strokeWidth;
            innerYRadius = yRadius - strokeWidth;
        }

        xRadius += strokeWidth;
        yRadius += strokeWidth;
    }
    if (DIEllipseEdgeEffect::kStroke == mode) {
        mode = (innerXRadius > 0 && innerYRadius > 0) ? DIEllipseEdgeEffect::kStroke :
                                                        DIEllipseEdgeEffect::kFill;
    }
    SkScalar innerRatioX = SkScalarDiv(xRadius, innerXRadius);
    SkScalar innerRatioY = SkScalarDiv(yRadius, innerYRadius);

    drawState->setVertexAttribs<gDIEllipseVertexAttribs>(SK_ARRAY_COUNT(gDIEllipseVertexAttribs));
    SkASSERT(sizeof(DIEllipseVertex) == drawState->getVertexSize());

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return false;
    }

    DIEllipseVertex* verts = reinterpret_cast<DIEllipseVertex*>(geo.vertices());

    GrEffectRef* effect = DIEllipseEdgeEffect::Create(mode);

    static const int kEllipseOuterOffsetAttrIndex = 1;
    static const int kEllipseInnerOffsetAttrIndex = 2;
    drawState->addCoverageEffect(effect, kEllipseOuterOffsetAttrIndex,
                                         kEllipseInnerOffsetAttrIndex)->unref();

    // This expands the outer rect so that after CTM we end up with a half-pixel border
    SkScalar a = vm[SkMatrix::kMScaleX];
    SkScalar b = vm[SkMatrix::kMSkewX];
    SkScalar c = vm[SkMatrix::kMSkewY];
    SkScalar d = vm[SkMatrix::kMScaleY];
    SkScalar geoDx = SkScalarDiv(SK_ScalarHalf, SkScalarSqrt(a*a + c*c));
    SkScalar geoDy = SkScalarDiv(SK_ScalarHalf, SkScalarSqrt(b*b + d*d));
    // This adjusts the "radius" to include the half-pixel border
    SkScalar offsetDx = SkScalarDiv(geoDx, xRadius);
    SkScalar offsetDy = SkScalarDiv(geoDy, yRadius);

    SkRect bounds = SkRect::MakeLTRB(
        center.fX - xRadius - geoDx,
        center.fY - yRadius - geoDy,
        center.fX + xRadius + geoDx,
        center.fY + yRadius + geoDy
    );

    verts[0].fPos = SkPoint::Make(bounds.fLeft, bounds.fTop);
    verts[0].fOuterOffset = SkPoint::Make(-1.0f - offsetDx, -1.0f - offsetDy);
    verts[0].fInnerOffset = SkPoint::Make(-innerRatioX - offsetDx, -innerRatioY - offsetDy);

    verts[1].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[1].fOuterOffset = SkPoint::Make(1.0f + offsetDx, -1.0f - offsetDy);
    verts[1].fInnerOffset = SkPoint::Make(innerRatioX + offsetDx, -innerRatioY - offsetDy);

    verts[2].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[2].fOuterOffset = SkPoint::Make(-1.0f - offsetDx, 1.0f + offsetDy);
    verts[2].fInnerOffset = SkPoint::Make(-innerRatioX - offsetDx, innerRatioY + offsetDy);

    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
    verts[3].fOuterOffset = SkPoint::Make(1.0f + offsetDx, 1.0f + offsetDy);
    verts[3].fInnerOffset = SkPoint::Make(innerRatioX + offsetDx, innerRatioY + offsetDy);

    target->drawNonIndexed(kTriangleStrip_GrPrimitiveType, 0, 4, &bounds);

    return true;
}

///////////////////////////////////////////////////////////////////////////////

static const uint16_t gRRectIndices[] = {
    // corners
    0, 1, 5, 0, 5, 4,
    2, 3, 7, 2, 7, 6,
    8, 9, 13, 8, 13, 12,
    10, 11, 15, 10, 15, 14,

    // edges
    1, 2, 6, 1, 6, 5,
    4, 5, 9, 4, 9, 8,
    6, 7, 11, 6, 11, 10,
    9, 10, 14, 9, 14, 13,

    // center
    // we place this at the end so that we can ignore these indices when rendering stroke-only
    5, 6, 10, 5, 10, 9
};


GrIndexBuffer* GrOvalRenderer::rRectIndexBuffer(GrGpu* gpu) {
    if (NULL == fRRectIndexBuffer) {
        fRRectIndexBuffer =
        gpu->createIndexBuffer(sizeof(gRRectIndices), false);
        if (NULL != fRRectIndexBuffer) {
#ifdef SK_DEBUG
            bool updated =
#endif
            fRRectIndexBuffer->updateData(gRRectIndices,
                                          sizeof(gRRectIndices));
            GR_DEBUGASSERT(updated);
        }
    }
    return fRRectIndexBuffer;
}

bool GrOvalRenderer::drawSimpleRRect(GrDrawTarget* target, GrContext* context, bool useAA,
                                     const SkRRect& rrect, const SkStrokeRec& stroke)
{
    bool useCoverageAA = useAA &&
        !target->getDrawState().getRenderTarget()->isMultisampled() &&
        !target->shouldDisableCoverageAAForBlend();

    // only anti-aliased rrects for now
    if (!useCoverageAA) {
        return false;
    }

    const SkMatrix& vm = context->getMatrix();
#ifdef SK_DEBUG
    {
        // we should have checked for this previously
        SkASSERT(useCoverageAA && vm.rectStaysRect() && rrect.isSimple());
    }
#endif

    // do any matrix crunching before we reset the draw state for device coords
    const SkRect& rrectBounds = rrect.getBounds();
    SkRect bounds;
    vm.mapRect(&bounds, rrectBounds);

    SkVector radii = rrect.getSimpleRadii();
    SkScalar xRadius = SkScalarAbs(vm[SkMatrix::kMScaleX]*radii.fX +
                                   vm[SkMatrix::kMSkewY]*radii.fY);
    SkScalar yRadius = SkScalarAbs(vm[SkMatrix::kMSkewX]*radii.fX +
                                   vm[SkMatrix::kMScaleY]*radii.fY);

    // if hairline stroke is greater than radius, we don't handle that right now
    SkStrokeRec::Style style = stroke.getStyle();
    if (SkStrokeRec::kHairline_Style == style &&
        (SK_ScalarHalf > xRadius || SK_ScalarHalf > yRadius)) {
        return false;
    }

    // do (potentially) anisotropic mapping of stroke
    SkVector scaledStroke;
    SkScalar strokeWidth = stroke.getWidth();
    scaledStroke.fX = SkScalarAbs(strokeWidth*(vm[SkMatrix::kMScaleX] + vm[SkMatrix::kMSkewY]));
    scaledStroke.fY = SkScalarAbs(strokeWidth*(vm[SkMatrix::kMSkewX] + vm[SkMatrix::kMScaleY]));

    // if half of strokewidth is greater than radius, we don't handle that right now
    if (SK_ScalarHalf*scaledStroke.fX > xRadius || SK_ScalarHalf*scaledStroke.fY > yRadius) {
        return false;
    }

    // reset to device coordinates
    GrDrawState* drawState = target->drawState();
    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(drawState)) {
        return false;
    }

    bool isStroked = (SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style);

    GrIndexBuffer* indexBuffer = this->rRectIndexBuffer(context->getGpu());
    if (NULL == indexBuffer) {
        GrPrintf("Failed to create index buffer!\n");
        return false;
    }

    // if the corners are circles, use the circle renderer
    if ((!isStroked || scaledStroke.fX == scaledStroke.fY) && xRadius == yRadius) {
        drawState->setVertexAttribs<gCircleVertexAttribs>(SK_ARRAY_COUNT(gCircleVertexAttribs));
        SkASSERT(sizeof(CircleVertex) == drawState->getVertexSize());

        GrDrawTarget::AutoReleaseGeometry geo(target, 16, 0);
        if (!geo.succeeded()) {
            GrPrintf("Failed to get space for vertices!\n");
            return false;
        }
        CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

        SkScalar innerRadius = 0.0f;
        SkScalar outerRadius = xRadius;
        SkScalar halfWidth = 0;
        if (style != SkStrokeRec::kFill_Style) {
            if (SkScalarNearlyZero(scaledStroke.fX)) {
                halfWidth = SK_ScalarHalf;
            } else {
                halfWidth = SkScalarHalf(scaledStroke.fX);
            }

            if (isStroked) {
                innerRadius = xRadius - halfWidth;
            }
            outerRadius += halfWidth;
            bounds.outset(halfWidth, halfWidth);
        }

        isStroked = (isStroked && innerRadius >= 0);

        GrEffectRef* effect = CircleEdgeEffect::Create(isStroked);
        static const int kCircleEdgeAttrIndex = 1;
        drawState->addCoverageEffect(effect, kCircleEdgeAttrIndex)->unref();

        // The radii are outset for two reasons. First, it allows the shader to simply perform
        // clamp(distance-to-center - radius, 0, 1). Second, the outer radius is used to compute the
        // verts of the bounding box that is rendered and the outset ensures the box will cover all
        // pixels partially covered by the circle.
        outerRadius += SK_ScalarHalf;
        innerRadius -= SK_ScalarHalf;

        // Expand the rect so all the pixels will be captured.
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);

        SkScalar yCoords[4] = {
            bounds.fTop,
            bounds.fTop + outerRadius,
            bounds.fBottom - outerRadius,
            bounds.fBottom
        };
        SkScalar yOuterRadii[4] = {
            -outerRadius,
            0,
            0,
            outerRadius
        };
        for (int i = 0; i < 4; ++i) {
            verts->fPos = SkPoint::Make(bounds.fLeft, yCoords[i]);
            verts->fOffset = SkPoint::Make(-outerRadius, yOuterRadii[i]);
            verts->fOuterRadius = outerRadius;
            verts->fInnerRadius = innerRadius;
            verts++;

            verts->fPos = SkPoint::Make(bounds.fLeft + outerRadius, yCoords[i]);
            verts->fOffset = SkPoint::Make(0, yOuterRadii[i]);
            verts->fOuterRadius = outerRadius;
            verts->fInnerRadius = innerRadius;
            verts++;

            verts->fPos = SkPoint::Make(bounds.fRight - outerRadius, yCoords[i]);
            verts->fOffset = SkPoint::Make(0, yOuterRadii[i]);
            verts->fOuterRadius = outerRadius;
            verts->fInnerRadius = innerRadius;
            verts++;

            verts->fPos = SkPoint::Make(bounds.fRight, yCoords[i]);
            verts->fOffset = SkPoint::Make(outerRadius, yOuterRadii[i]);
            verts->fOuterRadius = outerRadius;
            verts->fInnerRadius = innerRadius;
            verts++;
        }

        // drop out the middle quad if we're stroked
        int indexCnt = isStroked ? GR_ARRAY_COUNT(gRRectIndices)-6 : GR_ARRAY_COUNT(gRRectIndices);
        target->setIndexSourceToBuffer(indexBuffer);
        target->drawIndexed(kTriangles_GrPrimitiveType, 0, 0, 16, indexCnt, &bounds);

    // otherwise we use the ellipse renderer
    } else {
        drawState->setVertexAttribs<gEllipseVertexAttribs>(SK_ARRAY_COUNT(gEllipseVertexAttribs));
        SkASSERT(sizeof(EllipseVertex) == drawState->getVertexSize());

        SkScalar innerXRadius = 0.0f;
        SkScalar innerYRadius = 0.0f;
        if (SkStrokeRec::kFill_Style != style) {
            if (SkScalarNearlyZero(scaledStroke.length())) {
                scaledStroke.set(SK_ScalarHalf, SK_ScalarHalf);
            } else {
                scaledStroke.scale(SK_ScalarHalf);
            }

            // we only handle thick strokes for near-circular ellipses
            if (scaledStroke.length() > SK_ScalarHalf &&
                (SK_ScalarHalf*xRadius > yRadius || SK_ScalarHalf*yRadius > xRadius)) {
                return false;
            }

            // we don't handle it if curvature of the stroke is less than curvature of the ellipse
            if (scaledStroke.fX*(yRadius*yRadius) < (scaledStroke.fY*scaledStroke.fY)*xRadius ||
                scaledStroke.fY*(xRadius*xRadius) < (scaledStroke.fX*scaledStroke.fX)*yRadius) {
                return false;
            }

            // this is legit only if scale & translation (which should be the case at the moment)
            if (isStroked) {
                innerXRadius = xRadius - scaledStroke.fX;
                innerYRadius = yRadius - scaledStroke.fY;
            }

            xRadius += scaledStroke.fX;
            yRadius += scaledStroke.fY;
            bounds.outset(scaledStroke.fX, scaledStroke.fY);
        }

        isStroked = (isStroked && innerXRadius >= 0 && innerYRadius >= 0);

        GrDrawTarget::AutoReleaseGeometry geo(target, 16, 0);
        if (!geo.succeeded()) {
            GrPrintf("Failed to get space for vertices!\n");
            return false;
        }
        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(geo.vertices());

        GrEffectRef* effect = EllipseEdgeEffect::Create(isStroked);
        static const int kEllipseOffsetAttrIndex = 1;
        static const int kEllipseRadiiAttrIndex = 2;
        drawState->addCoverageEffect(effect,
                                     kEllipseOffsetAttrIndex, kEllipseRadiiAttrIndex)->unref();

        // Compute the reciprocals of the radii here to save time in the shader
        SkScalar xRadRecip = SkScalarInvert(xRadius);
        SkScalar yRadRecip = SkScalarInvert(yRadius);
        SkScalar xInnerRadRecip = SkScalarInvert(innerXRadius);
        SkScalar yInnerRadRecip = SkScalarInvert(innerYRadius);

        // Extend the radii out half a pixel to antialias.
        SkScalar xOuterRadius = xRadius + SK_ScalarHalf;
        SkScalar yOuterRadius = yRadius + SK_ScalarHalf;

        // Expand the rect so all the pixels will be captured.
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);

        SkScalar yCoords[4] = {
            bounds.fTop,
            bounds.fTop + yOuterRadius,
            bounds.fBottom - yOuterRadius,
            bounds.fBottom
        };
        SkScalar yOuterOffsets[4] = {
            yOuterRadius,
            SK_ScalarNearlyZero, // we're using inversesqrt() in the shader, so can't be exactly 0
            SK_ScalarNearlyZero,
            yOuterRadius
        };

        for (int i = 0; i < 4; ++i) {
            verts->fPos = SkPoint::Make(bounds.fLeft, yCoords[i]);
            verts->fOffset = SkPoint::Make(xOuterRadius, yOuterOffsets[i]);
            verts->fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts->fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);
            verts++;

            verts->fPos = SkPoint::Make(bounds.fLeft + xOuterRadius, yCoords[i]);
            verts->fOffset = SkPoint::Make(SK_ScalarNearlyZero, yOuterOffsets[i]);
            verts->fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts->fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);
            verts++;

            verts->fPos = SkPoint::Make(bounds.fRight - xOuterRadius, yCoords[i]);
            verts->fOffset = SkPoint::Make(SK_ScalarNearlyZero, yOuterOffsets[i]);
            verts->fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts->fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);
            verts++;

            verts->fPos = SkPoint::Make(bounds.fRight, yCoords[i]);
            verts->fOffset = SkPoint::Make(xOuterRadius, yOuterOffsets[i]);
            verts->fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts->fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);
            verts++;
        }

        // drop out the middle quad if we're stroked
        int indexCnt = isStroked ? GR_ARRAY_COUNT(gRRectIndices)-6 : GR_ARRAY_COUNT(gRRectIndices);
        target->setIndexSourceToBuffer(indexBuffer);
        target->drawIndexed(kTriangles_GrPrimitiveType, 0, 0, 16, indexCnt, &bounds);
    }

    return true;
}
