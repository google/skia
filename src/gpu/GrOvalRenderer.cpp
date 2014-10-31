/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalRenderer.h"

#include "gl/builders/GrGLProgramBuilder.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLGeometryProcessor.h"
#include "GrProcessor.h"
#include "GrTBackendProcessorFactory.h"

#include "GrDrawState.h"
#include "GrDrawTarget.h"
#include "GrGpu.h"

#include "SkRRect.h"
#include "SkStrokeRec.h"
#include "SkTLazy.h"

#include "GrGeometryProcessor.h"
#include "effects/GrRRectEffect.h"

namespace {
// TODO(joshualitt) add per vertex colors
struct CircleVertex {
    SkPoint  fPos;
    SkPoint  fOffset;
    SkScalar fOuterRadius;
    SkScalar fInnerRadius;
};

struct EllipseVertex {
    SkPoint  fPos;
    SkPoint  fOffset;
    SkPoint  fOuterRadii;
    SkPoint  fInnerRadii;
};

struct DIEllipseVertex {
    SkPoint  fPos;
    SkPoint  fOuterOffset;
    SkPoint  fInnerOffset;
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

class CircleEdgeEffect : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(bool stroke) {
        GR_CREATE_STATIC_PROCESSOR(gCircleStrokeEdge, CircleEdgeEffect, (true));
        GR_CREATE_STATIC_PROCESSOR(gCircleFillEdge, CircleEdgeEffect, (false));

        if (stroke) {
            gCircleStrokeEdge->ref();
            return gCircleStrokeEdge;
        } else {
            gCircleFillEdge->ref();
            return gCircleFillEdge;
        }
    }

    const GrShaderVar& inCircleEdge() const { return fInCircleEdge; }

    virtual const GrBackendGeometryProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendGeometryProcessorFactory<CircleEdgeEffect>::getInstance();
    }

    virtual ~CircleEdgeEffect() {}

    static const char* Name() { return "CircleEdge"; }

    inline bool isStroked() const { return fStroke; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrBackendProcessorFactory& factory, const GrProcessor&)
        : INHERITED (factory) {}

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            const CircleEdgeEffect& circleEffect = args.fGP.cast<CircleEdgeEffect>();
            GrGLVertToFrag v(kVec4f_GrSLType);
            args.fPB->addVarying("CircleEdge", &v);

            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();;
            vsBuilder->codeAppendf("%s = %s;", v.vsOut(), circleEffect.inCircleEdge().c_str());

            GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
            fsBuilder->codeAppendf("float d = length(%s.xy);", v.fsIn());
            fsBuilder->codeAppendf("float edgeAlpha = clamp(%s.z - d, 0.0, 1.0);", v.fsIn());
            if (circleEffect.isStroked()) {
                fsBuilder->codeAppendf("float innerAlpha = clamp(d - %s.w, 0.0, 1.0);",
                                       v.fsIn());
                fsBuilder->codeAppend("edgeAlpha *= innerAlpha;");
            }

            fsBuilder->codeAppendf("%s = %s;\n", args.fOutput,
                                   (GrGLSLExpr4(args.fInput) * GrGLSLExpr1("edgeAlpha")).c_str());
        }

        static void GenKey(const GrProcessor& processor, const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const CircleEdgeEffect& circleEffect = processor.cast<CircleEdgeEffect>();
            b->add32(circleEffect.isStroked());
        }

        virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {}

    private:
        typedef GrGLGeometryProcessor INHERITED;
    };


private:
    CircleEdgeEffect(bool stroke)
        : fInCircleEdge(this->addVertexAttrib(
                GrShaderVar("inCircleEdge",
                            kVec4f_GrSLType,
                            GrShaderVar::kAttribute_TypeModifier))) {
        fStroke = stroke;
    }

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const CircleEdgeEffect& cee = other.cast<CircleEdgeEffect>();
        return cee.fStroke == fStroke;
    }

    virtual void onComputeInvariantOutput(InvariantOutput* inout) const SK_OVERRIDE {
        inout->mulByUnknownAlpha();
    }

    const GrShaderVar& fInCircleEdge;
    bool fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(CircleEdgeEffect);

GrGeometryProcessor* CircleEdgeEffect::TestCreate(SkRandom* random,
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

class EllipseEdgeEffect : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(bool stroke) {
        GR_CREATE_STATIC_PROCESSOR(gEllipseStrokeEdge, EllipseEdgeEffect, (true));
        GR_CREATE_STATIC_PROCESSOR(gEllipseFillEdge, EllipseEdgeEffect, (false));

        if (stroke) {
            gEllipseStrokeEdge->ref();
            return gEllipseStrokeEdge;
        } else {
            gEllipseFillEdge->ref();
            return gEllipseFillEdge;
        }
    }

    virtual const GrBackendGeometryProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendGeometryProcessorFactory<EllipseEdgeEffect>::getInstance();
    }

    virtual ~EllipseEdgeEffect() {}

    static const char* Name() { return "EllipseEdge"; }

    const GrShaderVar& inEllipseOffset() const { return fInEllipseOffset; }
    const GrShaderVar& inEllipseRadii() const { return fInEllipseRadii; }

    inline bool isStroked() const { return fStroke; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrBackendProcessorFactory& factory, const GrProcessor&)
        : INHERITED (factory) {}

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            const EllipseEdgeEffect& ellipseEffect = args.fGP.cast<EllipseEdgeEffect>();

            GrGLVertToFrag ellipseOffsets(kVec2f_GrSLType);
            args.fPB->addVarying("EllipseOffsets", &ellipseOffsets);

            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();
            vsBuilder->codeAppendf("%s = %s;", ellipseOffsets.vsOut(),
                                   ellipseEffect.inEllipseOffset().c_str());

            GrGLVertToFrag ellipseRadii(kVec4f_GrSLType);
            args.fPB->addVarying("EllipseRadii", &ellipseRadii);
            vsBuilder->codeAppendf("%s = %s;", ellipseRadii.vsOut(),
                                   ellipseEffect.inEllipseRadii().c_str());

            // for outer curve
            GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
            fsBuilder->codeAppendf("vec2 scaledOffset = %s*%s.xy;", ellipseOffsets.fsIn(),
                                   ellipseRadii.fsIn());
            fsBuilder->codeAppend("float test = dot(scaledOffset, scaledOffset) - 1.0;");
            fsBuilder->codeAppendf("vec2 grad = 2.0*scaledOffset*%s.xy;", ellipseRadii.fsIn());
            fsBuilder->codeAppend("float grad_dot = dot(grad, grad);");

            // avoid calling inversesqrt on zero.
            fsBuilder->codeAppend("grad_dot = max(grad_dot, 1.0e-4);");
            fsBuilder->codeAppend("float invlen = inversesqrt(grad_dot);");
            fsBuilder->codeAppend("float edgeAlpha = clamp(0.5-test*invlen, 0.0, 1.0);");

            // for inner curve
            if (ellipseEffect.isStroked()) {
                fsBuilder->codeAppendf("scaledOffset = %s*%s.zw;",
                                       ellipseOffsets.fsIn(), ellipseRadii.fsIn());
                fsBuilder->codeAppend("test = dot(scaledOffset, scaledOffset) - 1.0;");
                fsBuilder->codeAppendf("grad = 2.0*scaledOffset*%s.zw;",
                                       ellipseRadii.fsIn());
                fsBuilder->codeAppend("invlen = inversesqrt(dot(grad, grad));");
                fsBuilder->codeAppend("edgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);");
            }

            fsBuilder->codeAppendf("%s = %s;", args.fOutput,
                                   (GrGLSLExpr4(args.fInput) * GrGLSLExpr1("edgeAlpha")).c_str());
        }

        static void GenKey(const GrProcessor& processor, const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const EllipseEdgeEffect& ellipseEffect = processor.cast<EllipseEdgeEffect>();
            b->add32(ellipseEffect.isStroked());
        }

        virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {
        }

    private:
        typedef GrGLGeometryProcessor INHERITED;
    };

private:
    EllipseEdgeEffect(bool stroke)
        : fInEllipseOffset(this->addVertexAttrib(
                GrShaderVar("inEllipseOffset",
                            kVec2f_GrSLType,
                            GrShaderVar::kAttribute_TypeModifier)))
        , fInEllipseRadii(this->addVertexAttrib(
                GrShaderVar("inEllipseRadii",
                            kVec4f_GrSLType,
                            GrShaderVar::kAttribute_TypeModifier))) {
        fStroke = stroke;
    }

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const EllipseEdgeEffect& eee = other.cast<EllipseEdgeEffect>();
        return eee.fStroke == fStroke;
    }

    virtual void onComputeInvariantOutput(InvariantOutput* inout) const SK_OVERRIDE {
        inout->mulByUnknownAlpha();
    }

    const GrShaderVar& fInEllipseOffset;
    const GrShaderVar& fInEllipseRadii;
    bool fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(EllipseEdgeEffect);

GrGeometryProcessor* EllipseEdgeEffect::TestCreate(SkRandom* random,
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

class DIEllipseEdgeEffect : public GrGeometryProcessor {
public:
    enum Mode { kStroke = 0, kHairline, kFill };

    static GrGeometryProcessor* Create(Mode mode) {
        GR_CREATE_STATIC_PROCESSOR(gEllipseStrokeEdge, DIEllipseEdgeEffect, (kStroke));
        GR_CREATE_STATIC_PROCESSOR(gEllipseHairlineEdge, DIEllipseEdgeEffect, (kHairline));
        GR_CREATE_STATIC_PROCESSOR(gEllipseFillEdge, DIEllipseEdgeEffect, (kFill));

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

    virtual const GrBackendGeometryProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendGeometryProcessorFactory<DIEllipseEdgeEffect>::getInstance();
    }

    virtual ~DIEllipseEdgeEffect() {}

    static const char* Name() { return "DIEllipseEdge"; }

    const GrShaderVar& inEllipseOffsets0() const { return fInEllipseOffsets0; }
    const GrShaderVar& inEllipseOffsets1() const { return fInEllipseOffsets1; }

    inline Mode getMode() const { return fMode; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrBackendProcessorFactory& factory, const GrProcessor&)
        : INHERITED (factory) {}

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            const DIEllipseEdgeEffect& ellipseEffect = args.fGP.cast<DIEllipseEdgeEffect>();

            GrGLVertToFrag offsets0(kVec2f_GrSLType);
            args.fPB->addVarying("EllipseOffsets0", &offsets0);

            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();
            vsBuilder->codeAppendf("%s = %s;", offsets0.vsOut(),
                                   ellipseEffect.inEllipseOffsets0().c_str());

            GrGLVertToFrag offsets1(kVec2f_GrSLType);
            args.fPB->addVarying("EllipseOffsets1", &offsets1);
            vsBuilder->codeAppendf("%s = %s;", offsets1.vsOut(),
                                   ellipseEffect.inEllipseOffsets1().c_str());

            GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            // for outer curve
            fsBuilder->codeAppendf("vec2 scaledOffset = %s.xy;", offsets0.fsIn());
            fsBuilder->codeAppend("float test = dot(scaledOffset, scaledOffset) - 1.0;");
            fsBuilder->codeAppendf("vec2 duvdx = dFdx(%s);", offsets0.fsIn());
            fsBuilder->codeAppendf("vec2 duvdy = dFdy(%s);", offsets0.fsIn());
            fsBuilder->codeAppendf("vec2 grad = vec2(2.0*%s.x*duvdx.x + 2.0*%s.y*duvdx.y,"
                                   "                 2.0*%s.x*duvdy.x + 2.0*%s.y*duvdy.y);",
                                   offsets0.fsIn(), offsets0.fsIn(), offsets0.fsIn(), offsets0.fsIn());

            fsBuilder->codeAppend("float grad_dot = dot(grad, grad);");
            // avoid calling inversesqrt on zero.
            fsBuilder->codeAppend("grad_dot = max(grad_dot, 1.0e-4);");
            fsBuilder->codeAppend("float invlen = inversesqrt(grad_dot);");
            if (kHairline == ellipseEffect.getMode()) {
                // can probably do this with one step
                fsBuilder->codeAppend("float edgeAlpha = clamp(1.0-test*invlen, 0.0, 1.0);");
                fsBuilder->codeAppend("edgeAlpha *= clamp(1.0+test*invlen, 0.0, 1.0);");
            } else {
                fsBuilder->codeAppend("float edgeAlpha = clamp(0.5-test*invlen, 0.0, 1.0);");
            }

            // for inner curve
            if (kStroke == ellipseEffect.getMode()) {
                fsBuilder->codeAppendf("scaledOffset = %s.xy;", offsets1.fsIn());
                fsBuilder->codeAppend("test = dot(scaledOffset, scaledOffset) - 1.0;");
                fsBuilder->codeAppendf("duvdx = dFdx(%s);", offsets1.fsIn());
                fsBuilder->codeAppendf("duvdy = dFdy(%s);", offsets1.fsIn());
                fsBuilder->codeAppendf("grad = vec2(2.0*%s.x*duvdx.x + 2.0*%s.y*duvdx.y,"
                                       "            2.0*%s.x*duvdy.x + 2.0*%s.y*duvdy.y);",
                                       offsets1.fsIn(), offsets1.fsIn(), offsets1.fsIn(),
                                       offsets1.fsIn());
                fsBuilder->codeAppend("invlen = inversesqrt(dot(grad, grad));");
                fsBuilder->codeAppend("edgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);");
            }

            fsBuilder->codeAppendf("%s = %s;", args.fOutput,
                                   (GrGLSLExpr4(args.fInput) * GrGLSLExpr1("edgeAlpha")).c_str());
        }

        static void GenKey(const GrProcessor& processor, const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const DIEllipseEdgeEffect& ellipseEffect = processor.cast<DIEllipseEdgeEffect>();

            b->add32(ellipseEffect.getMode());
        }

        virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {
        }

    private:
        typedef GrGLGeometryProcessor INHERITED;
    };

private:
    DIEllipseEdgeEffect(Mode mode)
        : fInEllipseOffsets0(this->addVertexAttrib(
                GrShaderVar("inEllipseOffsets0",
                            kVec2f_GrSLType,
                            GrShaderVar::kAttribute_TypeModifier)))
        , fInEllipseOffsets1(this->addVertexAttrib(
                GrShaderVar("inEllipseOffsets1",
                            kVec2f_GrSLType,
                            GrShaderVar::kAttribute_TypeModifier))) {
        fMode = mode;
    }

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const DIEllipseEdgeEffect& eee = other.cast<DIEllipseEdgeEffect>();
        return eee.fMode == fMode;
    }

    virtual void onComputeInvariantOutput(InvariantOutput* inout) const SK_OVERRIDE {
        inout->mulByUnknownAlpha();
    }

    const GrShaderVar& fInEllipseOffsets0;
    const GrShaderVar& fInEllipseOffsets1;
    Mode fMode;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DIEllipseEdgeEffect);

GrGeometryProcessor* DIEllipseEdgeEffect::TestCreate(SkRandom* random,
                                                     GrContext* context,
                                                     const GrDrawTargetCaps&,
                                                     GrTexture* textures[]) {
    return DIEllipseEdgeEffect::Create((Mode)(random->nextRangeU(0,2)));
}

///////////////////////////////////////////////////////////////////////////////

void GrOvalRenderer::reset() {
    SkSafeSetNull(fRRectIndexBuffer);
    SkSafeSetNull(fStrokeRRectIndexBuffer);
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
        this->drawCircle(target, context, useCoverageAA, oval, stroke);
    // if we have shader derivative support, render as device-independent
    } else if (target->caps()->shaderDerivativeSupport()) {
        return this->drawDIEllipse(target, context, useCoverageAA, oval, stroke);
    // otherwise axis-aligned ellipses only
    } else if (vm.rectStaysRect()) {
        return this->drawEllipse(target, context, useCoverageAA, oval, stroke);
    } else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

// position + edge
extern const GrVertexAttrib gCircleVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,               kPosition_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, sizeof(SkPoint), kGeometryProcessor_GrVertexAttribBinding}
};

void GrOvalRenderer::drawCircle(GrDrawTarget* target,
                                const GrContext* context,
                                bool useCoverageAA,
                                const SkRect& circle,
                                const SkStrokeRec& stroke)
{
    GrDrawState* drawState = target->drawState();

    const SkMatrix& vm = drawState->getViewMatrix();
    SkPoint center = SkPoint::Make(circle.centerX(), circle.centerY());
    vm.mapPoints(&center, 1);
    SkScalar radius = vm.mapRadius(SkScalarHalf(circle.width()));
    SkScalar strokeWidth = vm.mapRadius(stroke.getWidth());

    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(drawState)) {
        return;
    }

    drawState->setVertexAttribs<gCircleVertexAttribs>(SK_ARRAY_COUNT(gCircleVertexAttribs),
                                                      sizeof(CircleVertex));

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return;
    }

    CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

    SkStrokeRec::Style style = stroke.getStyle();
    bool isStrokeOnly = SkStrokeRec::kStroke_Style == style ||
                        SkStrokeRec::kHairline_Style == style;
    bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == style;

    SkScalar innerRadius = 0.0f;
    SkScalar outerRadius = radius;
    SkScalar halfWidth = 0;
    if (hasStroke) {
        if (SkScalarNearlyZero(strokeWidth)) {
            halfWidth = SK_ScalarHalf;
        } else {
            halfWidth = SkScalarHalf(strokeWidth);
        }

        outerRadius += halfWidth;
        if (isStrokeOnly) {
            innerRadius = radius - halfWidth;
        }
    }

    GrGeometryProcessor* gp = CircleEdgeEffect::Create(isStrokeOnly && innerRadius > 0);
    drawState->setGeometryProcessor(gp)->unref();

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

    verts[1].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[1].fOffset = SkPoint::Make(-outerRadius, outerRadius);
    verts[1].fOuterRadius = outerRadius;
    verts[1].fInnerRadius = innerRadius;

    verts[2].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
    verts[2].fOffset = SkPoint::Make(outerRadius, outerRadius);
    verts[2].fOuterRadius = outerRadius;
    verts[2].fInnerRadius = innerRadius;

    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[3].fOffset = SkPoint::Make(outerRadius, -outerRadius);
    verts[3].fOuterRadius = outerRadius;
    verts[3].fInnerRadius = innerRadius;

    target->setIndexSourceToBuffer(context->getGpu()->getQuadIndexBuffer());
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 4, 6, &bounds);
    target->resetIndexSource();
}

///////////////////////////////////////////////////////////////////////////////

// position + offset + 1/radii
extern const GrVertexAttrib gEllipseVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(SkPoint),   kGeometryProcessor_GrVertexAttribBinding},
    {kVec4f_GrVertexAttribType, 2*sizeof(SkPoint), kGeometryProcessor_GrVertexAttribBinding}
};

// position + offsets
extern const GrVertexAttrib gDIEllipseVertexAttribs[] = {
    {kVec2f_GrVertexAttribType, 0,                 kPosition_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, sizeof(SkPoint),   kGeometryProcessor_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType, 2*sizeof(SkPoint), kGeometryProcessor_GrVertexAttribBinding},
};

bool GrOvalRenderer::drawEllipse(GrDrawTarget* target,
                                 const GrContext* context,
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
    SkPoint center = SkPoint::Make(ellipse.centerX(), ellipse.centerY());
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
    bool isStrokeOnly = SkStrokeRec::kStroke_Style == style ||
                        SkStrokeRec::kHairline_Style == style;
    bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == style;

    SkScalar innerXRadius = 0;
    SkScalar innerYRadius = 0;
    if (hasStroke) {
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
        if (isStrokeOnly) {
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

    drawState->setVertexAttribs<gEllipseVertexAttribs>(SK_ARRAY_COUNT(gEllipseVertexAttribs),
                                                       sizeof(EllipseVertex));

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return false;
    }

    EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(geo.vertices());

    GrGeometryProcessor* gp = EllipseEdgeEffect::Create(isStrokeOnly &&
                                                        innerXRadius > 0 && innerYRadius > 0);

    drawState->setGeometryProcessor(gp)->unref();

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

    verts[1].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[1].fOffset = SkPoint::Make(-xRadius, yRadius);
    verts[1].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
    verts[1].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

    verts[2].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
    verts[2].fOffset = SkPoint::Make(xRadius, yRadius);
    verts[2].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
    verts[2].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[3].fOffset = SkPoint::Make(xRadius, -yRadius);
    verts[3].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
    verts[3].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

    target->setIndexSourceToBuffer(context->getGpu()->getQuadIndexBuffer());
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 4, 6, &bounds);
    target->resetIndexSource();

    return true;
}

bool GrOvalRenderer::drawDIEllipse(GrDrawTarget* target,
                                   const GrContext* context,
                                   bool useCoverageAA,
                                   const SkRect& ellipse,
                                   const SkStrokeRec& stroke)
{
    GrDrawState* drawState = target->drawState();
    const SkMatrix& vm = drawState->getViewMatrix();

    SkPoint center = SkPoint::Make(ellipse.centerX(), ellipse.centerY());
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

    drawState->setVertexAttribs<gDIEllipseVertexAttribs>(SK_ARRAY_COUNT(gDIEllipseVertexAttribs),
                                                         sizeof(DIEllipseVertex));

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return false;
    }

    DIEllipseVertex* verts = reinterpret_cast<DIEllipseVertex*>(geo.vertices());

    GrGeometryProcessor* gp = DIEllipseEdgeEffect::Create(mode);

    drawState->setGeometryProcessor(gp)->unref();

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

    verts[1].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[1].fOuterOffset = SkPoint::Make(-1.0f - offsetDx, 1.0f + offsetDy);
    verts[1].fInnerOffset = SkPoint::Make(-innerRatioX - offsetDx, innerRatioY + offsetDy);

    verts[2].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
    verts[2].fOuterOffset = SkPoint::Make(1.0f + offsetDx, 1.0f + offsetDy);
    verts[2].fInnerOffset = SkPoint::Make(innerRatioX + offsetDx, innerRatioY + offsetDy);

    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[3].fOuterOffset = SkPoint::Make(1.0f + offsetDx, -1.0f - offsetDy);
    verts[3].fInnerOffset = SkPoint::Make(innerRatioX + offsetDx, -innerRatioY - offsetDy);

    target->setIndexSourceToBuffer(context->getGpu()->getQuadIndexBuffer());
    target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 4, 6, &bounds);
    target->resetIndexSource();

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

static const int kIndicesPerStrokeRRect = SK_ARRAY_COUNT(gRRectIndices) - 6;
static const int kIndicesPerRRect = SK_ARRAY_COUNT(gRRectIndices);
static const int kVertsPerRRect = 16;
static const int kNumRRectsInIndexBuffer = 256;

GrIndexBuffer* GrOvalRenderer::rRectIndexBuffer(bool isStrokeOnly, GrGpu* gpu) {
    if (isStrokeOnly) {
        if (NULL == fStrokeRRectIndexBuffer) {
            fStrokeRRectIndexBuffer = gpu->createInstancedIndexBuffer(gRRectIndices,
                                                                      kIndicesPerStrokeRRect,
                                                                      kNumRRectsInIndexBuffer,
                                                                      kVertsPerRRect);
        }
        return fStrokeRRectIndexBuffer;
    } else {
        if (NULL == fRRectIndexBuffer) {
            fRRectIndexBuffer = gpu->createInstancedIndexBuffer(gRRectIndices,
                                                                kIndicesPerRRect,
                                                                kNumRRectsInIndexBuffer,
                                                                kVertsPerRRect);
        }
        return fRRectIndexBuffer;
    }
}

bool GrOvalRenderer::drawDRRect(GrDrawTarget* target, GrContext* context, bool useAA,
                                const SkRRect& origOuter, const SkRRect& origInner) {
    bool applyAA = useAA &&
                   !target->getDrawState().getRenderTarget()->isMultisampled() &&
                   !target->shouldDisableCoverageAAForBlend();
    GrDrawState::AutoRestoreEffects are;
    if (!origInner.isEmpty()) {
        SkTCopyOnFirstWrite<SkRRect> inner(origInner);
        if (!context->getMatrix().isIdentity()) {
            if (!origInner.transform(context->getMatrix(), inner.writable())) {
                return false;
            }
        }
        GrPrimitiveEdgeType edgeType = applyAA ?
                kInverseFillAA_GrProcessorEdgeType :
                kInverseFillBW_GrProcessorEdgeType;
        GrFragmentProcessor* fp = GrRRectEffect::Create(edgeType, *inner);
        if (NULL == fp) {
            return false;
        }
        are.set(target->drawState());
        target->drawState()->addCoverageProcessor(fp)->unref();
    }

    SkStrokeRec fillRec(SkStrokeRec::kFill_InitStyle);
    if (this->drawRRect(target, context, useAA, origOuter, fillRec)) {
        return true;
    }

    SkASSERT(!origOuter.isEmpty());
    SkTCopyOnFirstWrite<SkRRect> outer(origOuter);
    if (!context->getMatrix().isIdentity()) {
        if (!origOuter.transform(context->getMatrix(), outer.writable())) {
            return false;
        }
    }
    GrPrimitiveEdgeType edgeType = applyAA ? kFillAA_GrProcessorEdgeType :
                                          kFillBW_GrProcessorEdgeType;
    GrFragmentProcessor* effect = GrRRectEffect::Create(edgeType, *outer);
    if (NULL == effect) {
        return false;
    }
    if (!are.isSet()) {
        are.set(target->drawState());
    }
    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(target->drawState())) {
        return false;
    }
    target->drawState()->addCoverageProcessor(effect)->unref();
    SkRect bounds = outer->getBounds();
    if (applyAA) {
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);
    }
    target->drawRect(bounds, NULL, NULL);
    return true;
}

bool GrOvalRenderer::drawRRect(GrDrawTarget* target, GrContext* context, bool useAA,
                               const SkRRect& rrect, const SkStrokeRec& stroke) {
    if (rrect.isOval()) {
        return this->drawOval(target, context, useAA, rrect.getBounds(), stroke);
    }

    bool useCoverageAA = useAA &&
        !target->getDrawState().getRenderTarget()->isMultisampled() &&
        !target->shouldDisableCoverageAAForBlend();

    // only anti-aliased rrects for now
    if (!useCoverageAA) {
        return false;
    }

    const SkMatrix& vm = context->getMatrix();

    if (!vm.rectStaysRect() || !rrect.isSimple()) {
        return false;
    }

    // do any matrix crunching before we reset the draw state for device coords
    const SkRect& rrectBounds = rrect.getBounds();
    SkRect bounds;
    vm.mapRect(&bounds, rrectBounds);

    SkVector radii = rrect.getSimpleRadii();
    SkScalar xRadius = SkScalarAbs(vm[SkMatrix::kMScaleX]*radii.fX +
                                   vm[SkMatrix::kMSkewY]*radii.fY);
    SkScalar yRadius = SkScalarAbs(vm[SkMatrix::kMSkewX]*radii.fX +
                                   vm[SkMatrix::kMScaleY]*radii.fY);

    SkStrokeRec::Style style = stroke.getStyle();

    // do (potentially) anisotropic mapping of stroke
    SkVector scaledStroke;
    SkScalar strokeWidth = stroke.getWidth();

    bool isStrokeOnly = SkStrokeRec::kStroke_Style == style ||
                        SkStrokeRec::kHairline_Style == style;
    bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == style;

    if (hasStroke) {
        if (SkStrokeRec::kHairline_Style == style) {
            scaledStroke.set(1, 1);
        } else {
            scaledStroke.fX = SkScalarAbs(strokeWidth*(vm[SkMatrix::kMScaleX] +
                                                       vm[SkMatrix::kMSkewY]));
            scaledStroke.fY = SkScalarAbs(strokeWidth*(vm[SkMatrix::kMSkewX] +
                                                       vm[SkMatrix::kMScaleY]));
        }

        // if half of strokewidth is greater than radius, we don't handle that right now
        if (SK_ScalarHalf*scaledStroke.fX > xRadius || SK_ScalarHalf*scaledStroke.fY > yRadius) {
            return false;
        }
    }

    // The way the effect interpolates the offset-to-ellipse/circle-center attribute only works on
    // the interior of the rrect if the radii are >= 0.5. Otherwise, the inner rect of the nine-
    // patch will have fractional coverage. This only matters when the interior is actually filled.
    // We could consider falling back to rect rendering here, since a tiny radius is
    // indistinguishable from a square corner.
    if (!isStrokeOnly && (SK_ScalarHalf > xRadius || SK_ScalarHalf > yRadius)) {
        return false;
    }

    // reset to device coordinates
    GrDrawState* drawState = target->drawState();
    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(drawState)) {
        return false;
    }

    GrIndexBuffer* indexBuffer = this->rRectIndexBuffer(isStrokeOnly, context->getGpu());
    if (NULL == indexBuffer) {
        SkDebugf("Failed to create index buffer!\n");
        return false;
    }

    // if the corners are circles, use the circle renderer
    if ((!hasStroke || scaledStroke.fX == scaledStroke.fY) && xRadius == yRadius) {
        drawState->setVertexAttribs<gCircleVertexAttribs>(SK_ARRAY_COUNT(gCircleVertexAttribs),
                                                          sizeof(CircleVertex));

        GrDrawTarget::AutoReleaseGeometry geo(target, 16, 0);
        if (!geo.succeeded()) {
            SkDebugf("Failed to get space for vertices!\n");
            return false;
        }
        CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

        SkScalar innerRadius = 0.0f;
        SkScalar outerRadius = xRadius;
        SkScalar halfWidth = 0;
        if (hasStroke) {
            if (SkScalarNearlyZero(scaledStroke.fX)) {
                halfWidth = SK_ScalarHalf;
            } else {
                halfWidth = SkScalarHalf(scaledStroke.fX);
            }

            if (isStrokeOnly) {
                innerRadius = xRadius - halfWidth;
            }
            outerRadius += halfWidth;
            bounds.outset(halfWidth, halfWidth);
        }

        isStrokeOnly = (isStrokeOnly && innerRadius >= 0);

        GrGeometryProcessor* effect = CircleEdgeEffect::Create(isStrokeOnly);
        drawState->setGeometryProcessor(effect)->unref();

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
        int indexCnt = isStrokeOnly ? SK_ARRAY_COUNT(gRRectIndices) - 6 :
                                      SK_ARRAY_COUNT(gRRectIndices);
        target->setIndexSourceToBuffer(indexBuffer);
        target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 16, indexCnt, &bounds);

    // otherwise we use the ellipse renderer
    } else {
        drawState->setVertexAttribs<gEllipseVertexAttribs>(SK_ARRAY_COUNT(gEllipseVertexAttribs),
                                                           sizeof(EllipseVertex));

        SkScalar innerXRadius = 0.0f;
        SkScalar innerYRadius = 0.0f;
        if (hasStroke) {
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
            if (isStrokeOnly) {
                innerXRadius = xRadius - scaledStroke.fX;
                innerYRadius = yRadius - scaledStroke.fY;
            }

            xRadius += scaledStroke.fX;
            yRadius += scaledStroke.fY;
            bounds.outset(scaledStroke.fX, scaledStroke.fY);
        }

        isStrokeOnly = (isStrokeOnly && innerXRadius >= 0 && innerYRadius >= 0);

        GrDrawTarget::AutoReleaseGeometry geo(target, 16, 0);
        if (!geo.succeeded()) {
            SkDebugf("Failed to get space for vertices!\n");
            return false;
        }
        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(geo.vertices());

        GrGeometryProcessor* effect = EllipseEdgeEffect::Create(isStrokeOnly);
        drawState->setGeometryProcessor(effect)->unref();

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
        int indexCnt = isStrokeOnly ? SK_ARRAY_COUNT(gRRectIndices) - 6 :
                                      SK_ARRAY_COUNT(gRRectIndices);
        target->setIndexSourceToBuffer(indexBuffer);
        target->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 16, indexCnt, &bounds);
    }

    target->resetIndexSource();
    return true;
}
