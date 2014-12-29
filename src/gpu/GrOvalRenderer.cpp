/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalRenderer.h"

#include "GrProcessor.h"
#include "GrDrawState.h"
#include "GrDrawTarget.h"
#include "GrGeometryProcessor.h"
#include "GrGpu.h"
#include "GrInvariantOutput.h"
#include "SkRRect.h"
#include "SkStrokeRec.h"
#include "SkTLazy.h"
#include "effects/GrRRectEffect.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

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
 * The output of this effect is a modulation of the input color and coverage for a circle. It
 * operates in a space normalized by the circle radius (outer radius in the case of a stroke)
 * with origin at the circle center. Two   vertex attributes are used:
 *    vec2f : position in device space of the bounding geometry vertices
 *    vec4f : (p.xy, outerRad, innerRad)
 *             p is the position in the normalized space.
 *             outerRad is the outerRadius in device space.
 *             innerRad is the innerRadius in normalized space (ignored if not stroking).
 */

class CircleEdgeEffect : public GrGeometryProcessor {
public:
    static GrGeometryProcessor* Create(GrColor color, bool stroke, const SkMatrix& localMatrix) {
        return SkNEW_ARGS(CircleEdgeEffect, (color, stroke, localMatrix));
    }

    const GrAttribute* inPosition() const { return fInPosition; }
    const GrAttribute* inCircleEdge() const { return fInCircleEdge; }
    virtual ~CircleEdgeEffect() {}

    virtual const char* name() const SK_OVERRIDE { return "CircleEdge"; }

    inline bool isStroked() const { return fStroke; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor&,
                    const GrBatchTracker&)
            : fColor(GrColor_ILLEGAL) {}

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            const CircleEdgeEffect& ce = args.fGP.cast<CircleEdgeEffect>();
            GrGLGPBuilder* pb = args.fPB;
            const BatchTracker& local = args.fBT.cast<BatchTracker>();
            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

            GrGLVertToFrag v(kVec4f_GrSLType);
            args.fPB->addVarying("CircleEdge", &v);
            vsBuilder->codeAppendf("%s = %s;", v.vsOut(), ce.inCircleEdge()->fName);

            // Setup pass through color
            this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, NULL,
                                        &fColorUniform);

            // setup coord outputs
            vsBuilder->codeAppendf("%s = %s;", vsBuilder->positionCoords(), ce.inPosition()->fName);
            vsBuilder->codeAppendf("%s = %s;", vsBuilder->localCoords(), ce.inPosition()->fName);

            // setup position varying
            vsBuilder->codeAppendf("%s = %s * vec3(%s, 1);", vsBuilder->glPosition(),
                                   vsBuilder->uViewM(), ce.inPosition()->fName);

            GrGLGPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
            fsBuilder->codeAppendf("float d = length(%s.xy);", v.fsIn());
            fsBuilder->codeAppendf("float edgeAlpha = clamp(%s.z * (1.0 - d), 0.0, 1.0);", v.fsIn());
            if (ce.isStroked()) {
                fsBuilder->codeAppendf("float innerAlpha = clamp(%s.z * (d - %s.w), 0.0, 1.0);",
                                       v.fsIn(), v.fsIn());
                fsBuilder->codeAppend("edgeAlpha *= innerAlpha;");
            }

            fsBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& processor,
                           const GrBatchTracker& bt,
                           const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const BatchTracker& local = bt.cast<BatchTracker>();
            const CircleEdgeEffect& circleEffect = processor.cast<CircleEdgeEffect>();
            uint16_t key = circleEffect.isStroked() ? 0x1 : 0x0;
            key |= local.fUsesLocalCoords && processor.localMatrix().hasPerspective() ? 0x2 : 0x0;
            b->add32(key << 16 | local.fInputColorType);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) SK_OVERRIDE {
            const BatchTracker& local = bt.cast<BatchTracker>();
            if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(local.fColor, c);
                pdman.set4fv(fColorUniform, 1, c);
                fColor = local.fColor;
            }
        }

    private:
        GrColor fColor;
        UniformHandle fColorUniform;
        typedef GrGLGeometryProcessor INHERITED;
    };

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE {
        GLProcessor::GenKey(*this, bt, caps, b);
    }

    virtual GrGLGeometryProcessor* createGLInstance(const GrBatchTracker& bt) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

    void initBatchTracker(GrBatchTracker* bt, const InitBT& init) const SK_OVERRIDE {
        BatchTracker* local = bt->cast<BatchTracker>();
        local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
        local->fUsesLocalCoords = init.fUsesLocalCoords;
    }

    bool onCanMakeEqual(const GrBatchTracker& m,
                        const GrGeometryProcessor& that,
                        const GrBatchTracker& t) const SK_OVERRIDE {
        const BatchTracker& mine = m.cast<BatchTracker>();
        const BatchTracker& theirs = t.cast<BatchTracker>();
        return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                       that, theirs.fUsesLocalCoords) &&
               CanCombineOutput(mine.fInputColorType, mine.fColor,
                                theirs.fInputColorType, theirs.fColor);
    }

private:
    CircleEdgeEffect(GrColor color, bool stroke, const SkMatrix& localMatrix)
        : INHERITED(color, SkMatrix::I(), localMatrix) {
        this->initClassID<CircleEdgeEffect>();
        fInPosition = &this->addVertexAttrib(GrAttribute("inPosition", kVec2f_GrVertexAttribType));
        fInCircleEdge = &this->addVertexAttrib(GrAttribute("inCircleEdge",
                                                           kVec4f_GrVertexAttribType));
        fStroke = stroke;
    }

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const CircleEdgeEffect& cee = other.cast<CircleEdgeEffect>();
        return cee.fStroke == fStroke;
    }

    virtual void onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    struct BatchTracker {
        GrGPInput fInputColorType;
        GrColor fColor;
        bool fUsesLocalCoords;
    };

    const GrAttribute* fInPosition;
    const GrAttribute* fInCircleEdge;
    bool fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(CircleEdgeEffect);

GrGeometryProcessor* CircleEdgeEffect::TestCreate(SkRandom* random,
                                                  GrContext* context,
                                                  const GrDrawTargetCaps&,
                                                  GrTexture* textures[]) {
    return CircleEdgeEffect::Create(GrRandomColor(random),
                                    random->nextBool(),
                                    GrProcessorUnitTest::TestMatrix(random));
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
    static GrGeometryProcessor* Create(GrColor color, bool stroke, const SkMatrix& localMatrix) {
        return SkNEW_ARGS(EllipseEdgeEffect, (color, stroke, localMatrix));
    }

    virtual ~EllipseEdgeEffect() {}

    virtual const char* name() const SK_OVERRIDE { return "EllipseEdge"; }

    const GrAttribute* inPosition() const { return fInPosition; }
    const GrAttribute* inEllipseOffset() const { return fInEllipseOffset; }
    const GrAttribute* inEllipseRadii() const { return fInEllipseRadii; }

    inline bool isStroked() const { return fStroke; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor&,
                    const GrBatchTracker&)
            : fColor(GrColor_ILLEGAL) {}

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            const EllipseEdgeEffect& ee = args.fGP.cast<EllipseEdgeEffect>();
            GrGLGPBuilder* pb = args.fPB;
            const BatchTracker& local = args.fBT.cast<BatchTracker>();
            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

            GrGLVertToFrag ellipseOffsets(kVec2f_GrSLType);
            args.fPB->addVarying("EllipseOffsets", &ellipseOffsets);
            vsBuilder->codeAppendf("%s = %s;", ellipseOffsets.vsOut(),
                                   ee.inEllipseOffset()->fName);

            GrGLVertToFrag ellipseRadii(kVec4f_GrSLType);
            args.fPB->addVarying("EllipseRadii", &ellipseRadii);
            vsBuilder->codeAppendf("%s = %s;", ellipseRadii.vsOut(),
                                   ee.inEllipseRadii()->fName);

            // Setup pass through color
            this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, NULL,
                                        &fColorUniform);

            // setup coord outputs
            vsBuilder->codeAppendf("%s = %s;", vsBuilder->positionCoords(), ee.inPosition()->fName);
            vsBuilder->codeAppendf("%s = %s;", vsBuilder->localCoords(), ee.inPosition()->fName);

            // setup position varying
            vsBuilder->codeAppendf("%s = %s * vec3(%s, 1);", vsBuilder->glPosition(),
                                   vsBuilder->uViewM(), ee.inPosition()->fName);

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
            if (ee.isStroked()) {
                fsBuilder->codeAppendf("scaledOffset = %s*%s.zw;",
                                       ellipseOffsets.fsIn(), ellipseRadii.fsIn());
                fsBuilder->codeAppend("test = dot(scaledOffset, scaledOffset) - 1.0;");
                fsBuilder->codeAppendf("grad = 2.0*scaledOffset*%s.zw;",
                                       ellipseRadii.fsIn());
                fsBuilder->codeAppend("invlen = inversesqrt(dot(grad, grad));");
                fsBuilder->codeAppend("edgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);");
            }

            fsBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& processor,
                           const GrBatchTracker& bt,
                           const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const BatchTracker& local = bt.cast<BatchTracker>();
            const EllipseEdgeEffect& ellipseEffect = processor.cast<EllipseEdgeEffect>();
            uint16_t key = ellipseEffect.isStroked() ? 0x1 : 0x0;
            key |= local.fUsesLocalCoords && processor.localMatrix().hasPerspective() ? 0x2 : 0x0;
            b->add32(key << 16 | local.fInputColorType);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) SK_OVERRIDE {
            const BatchTracker& local = bt.cast<BatchTracker>();
            if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(local.fColor, c);
                pdman.set4fv(fColorUniform, 1, c);
                fColor = local.fColor;
            }
        }

    private:
        GrColor fColor;
        UniformHandle fColorUniform;

        typedef GrGLGeometryProcessor INHERITED;
    };

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE {
        GLProcessor::GenKey(*this, bt, caps, b);
    }

    virtual GrGLGeometryProcessor* createGLInstance(const GrBatchTracker& bt) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

    void initBatchTracker(GrBatchTracker* bt, const InitBT& init) const SK_OVERRIDE {
        BatchTracker* local = bt->cast<BatchTracker>();
        local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
        local->fUsesLocalCoords = init.fUsesLocalCoords;
    }

    bool onCanMakeEqual(const GrBatchTracker& m,
                        const GrGeometryProcessor& that,
                        const GrBatchTracker& t) const SK_OVERRIDE {
        const BatchTracker& mine = m.cast<BatchTracker>();
        const BatchTracker& theirs = t.cast<BatchTracker>();
        return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                       that, theirs.fUsesLocalCoords) &&
               CanCombineOutput(mine.fInputColorType, mine.fColor,
                                theirs.fInputColorType, theirs.fColor);
    }

private:
    EllipseEdgeEffect(GrColor color, bool stroke, const SkMatrix& localMatrix)
        : INHERITED(color, SkMatrix::I(), localMatrix) {
        this->initClassID<EllipseEdgeEffect>();
        fInPosition = &this->addVertexAttrib(GrAttribute("inPosition", kVec2f_GrVertexAttribType));
        fInEllipseOffset = &this->addVertexAttrib(GrAttribute("inEllipseOffset",
                                                              kVec2f_GrVertexAttribType));
        fInEllipseRadii = &this->addVertexAttrib(GrAttribute("inEllipseRadii",
                                                             kVec4f_GrVertexAttribType));
        fStroke = stroke;
    }

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const EllipseEdgeEffect& eee = other.cast<EllipseEdgeEffect>();
        return eee.fStroke == fStroke;
    }

    virtual void onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    struct BatchTracker {
        GrGPInput fInputColorType;
        GrColor fColor;
        bool fUsesLocalCoords;
    };

    const GrAttribute* fInPosition;
    const GrAttribute* fInEllipseOffset;
    const GrAttribute* fInEllipseRadii;
    bool fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(EllipseEdgeEffect);

GrGeometryProcessor* EllipseEdgeEffect::TestCreate(SkRandom* random,
                                                   GrContext* context,
                                                   const GrDrawTargetCaps&,
                                                   GrTexture* textures[]) {
    return EllipseEdgeEffect::Create(GrRandomColor(random),
                                     random->nextBool(),
                                     GrProcessorUnitTest::TestMatrix(random));
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

    static GrGeometryProcessor* Create(GrColor color, const SkMatrix& viewMatrix, Mode mode) {
        return SkNEW_ARGS(DIEllipseEdgeEffect, (color, viewMatrix, mode));
    }

    virtual ~DIEllipseEdgeEffect() {}

    virtual const char* name() const SK_OVERRIDE { return "DIEllipseEdge"; }

    const GrAttribute* inPosition() const { return fInPosition; }
    const GrAttribute* inEllipseOffsets0() const { return fInEllipseOffsets0; }
    const GrAttribute* inEllipseOffsets1() const { return fInEllipseOffsets1; }

    inline Mode getMode() const { return fMode; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor&,
                    const GrBatchTracker&)
            : fColor(GrColor_ILLEGAL) {}

        virtual void emitCode(const EmitArgs& args) SK_OVERRIDE {
            const DIEllipseEdgeEffect& ee = args.fGP.cast<DIEllipseEdgeEffect>();
            GrGLGPBuilder* pb = args.fPB;
            const BatchTracker& local = args.fBT.cast<BatchTracker>();
            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

            GrGLVertToFrag offsets0(kVec2f_GrSLType);
            args.fPB->addVarying("EllipseOffsets0", &offsets0);
            vsBuilder->codeAppendf("%s = %s;", offsets0.vsOut(),
                                   ee.inEllipseOffsets0()->fName);

            GrGLVertToFrag offsets1(kVec2f_GrSLType);
            args.fPB->addVarying("EllipseOffsets1", &offsets1);
            vsBuilder->codeAppendf("%s = %s;", offsets1.vsOut(),
                                   ee.inEllipseOffsets1()->fName);

            // Setup pass through color
            this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, NULL,
                                        &fColorUniform);

            // setup coord outputs
            vsBuilder->codeAppendf("%s = %s;", vsBuilder->positionCoords(), ee.inPosition()->fName);
            vsBuilder->codeAppendf("%s = %s;", vsBuilder->localCoords(), ee.inPosition()->fName);

            // setup position varying
            vsBuilder->codeAppendf("%s = %s * vec3(%s, 1);", vsBuilder->glPosition(),
                                   vsBuilder->uViewM(), ee.inPosition()->fName);

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
            if (kHairline == ee.getMode()) {
                // can probably do this with one step
                fsBuilder->codeAppend("float edgeAlpha = clamp(1.0-test*invlen, 0.0, 1.0);");
                fsBuilder->codeAppend("edgeAlpha *= clamp(1.0+test*invlen, 0.0, 1.0);");
            } else {
                fsBuilder->codeAppend("float edgeAlpha = clamp(0.5-test*invlen, 0.0, 1.0);");
            }

            // for inner curve
            if (kStroke == ee.getMode()) {
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

            fsBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& processor,
                           const GrBatchTracker& bt,
                           const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const BatchTracker& local = bt.cast<BatchTracker>();
            const DIEllipseEdgeEffect& ellipseEffect = processor.cast<DIEllipseEdgeEffect>();
            uint16_t key = ellipseEffect.getMode();
            key |= local.fUsesLocalCoords && processor.localMatrix().hasPerspective() ? 0x1 << 8 :
                                                                                        0x0;
            b->add32(key << 16 | local.fInputColorType);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) SK_OVERRIDE {
            const BatchTracker& local = bt.cast<BatchTracker>();
            if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
                GrGLfloat c[4];
                GrColorToRGBAFloat(local.fColor, c);
                pdman.set4fv(fColorUniform, 1, c);
                fColor = local.fColor;
            }
        }

    private:
        GrColor fColor;
        UniformHandle fColorUniform;

        typedef GrGLGeometryProcessor INHERITED;
    };

    virtual void getGLProcessorKey(const GrBatchTracker& bt,
                                   const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE {
        GLProcessor::GenKey(*this, bt, caps, b);
    }

    virtual GrGLGeometryProcessor* createGLInstance(const GrBatchTracker& bt) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

    void initBatchTracker(GrBatchTracker* bt, const InitBT& init) const SK_OVERRIDE {
        BatchTracker* local = bt->cast<BatchTracker>();
        local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
        local->fUsesLocalCoords = init.fUsesLocalCoords;
    }

    bool onCanMakeEqual(const GrBatchTracker& m,
                        const GrGeometryProcessor& that,
                        const GrBatchTracker& t) const SK_OVERRIDE {
        const BatchTracker& mine = m.cast<BatchTracker>();
        const BatchTracker& theirs = t.cast<BatchTracker>();
        return CanCombineLocalMatrices(*this, mine.fUsesLocalCoords,
                                       that, theirs.fUsesLocalCoords) &&
               CanCombineOutput(mine.fInputColorType, mine.fColor,
                                theirs.fInputColorType, theirs.fColor);
    }

private:
    DIEllipseEdgeEffect(GrColor color, const SkMatrix& viewMatrix, Mode mode)
        : INHERITED(color, viewMatrix) {
        this->initClassID<DIEllipseEdgeEffect>();
        fInPosition = &this->addVertexAttrib(GrAttribute("inPosition", kVec2f_GrVertexAttribType));
        fInEllipseOffsets0 = &this->addVertexAttrib(GrAttribute("inEllipseOffsets0",
                                                                kVec2f_GrVertexAttribType));
        fInEllipseOffsets1 = &this->addVertexAttrib(GrAttribute("inEllipseOffsets1",
                                                                kVec2f_GrVertexAttribType));
        fMode = mode;
    }

    virtual bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const DIEllipseEdgeEffect& eee = other.cast<DIEllipseEdgeEffect>();
        return eee.fMode == fMode;
    }

    virtual void onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    struct BatchTracker {
        GrGPInput fInputColorType;
        GrColor fColor;
        bool fUsesLocalCoords;
    };

    const GrAttribute* fInPosition;
    const GrAttribute* fInEllipseOffsets0;
    const GrAttribute* fInEllipseOffsets1;
    Mode fMode;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DIEllipseEdgeEffect);

GrGeometryProcessor* DIEllipseEdgeEffect::TestCreate(SkRandom* random,
                                                     GrContext* context,
                                                     const GrDrawTargetCaps&,
                                                     GrTexture* textures[]) {
    return DIEllipseEdgeEffect::Create(GrRandomColor(random),
                                       GrProcessorUnitTest::TestMatrix(random),
                                       (Mode)(random->nextRangeU(0,2)));
}

///////////////////////////////////////////////////////////////////////////////

void GrOvalRenderer::reset() {
    SkSafeSetNull(fRRectIndexBuffer);
    SkSafeSetNull(fStrokeRRectIndexBuffer);
}

bool GrOvalRenderer::drawOval(GrDrawTarget* target,
                              GrDrawState* drawState,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              bool useAA,
                              const SkRect& oval,
                              const SkStrokeRec& stroke)
{
    bool useCoverageAA = useAA &&
        !drawState->getRenderTarget()->isMultisampled() &&
        drawState->canUseFracCoveragePrimProc(color, *target->caps());

    if (!useCoverageAA) {
        return false;
    }

    // we can draw circles
    if (SkScalarNearlyEqual(oval.width(), oval.height()) && circle_stays_circle(viewMatrix)) {
        this->drawCircle(target, drawState, color, viewMatrix, useCoverageAA, oval, stroke);
    // if we have shader derivative support, render as device-independent
    } else if (target->caps()->shaderDerivativeSupport()) {
        return this->drawDIEllipse(target, drawState, color, viewMatrix, useCoverageAA, oval,
                                   stroke);
    // otherwise axis-aligned ellipses only
    } else if (viewMatrix.rectStaysRect()) {
        return this->drawEllipse(target, drawState, color, viewMatrix, useCoverageAA, oval, stroke);
    } else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void GrOvalRenderer::drawCircle(GrDrawTarget* target,
                                GrDrawState* drawState,
                                GrColor color,
                                const SkMatrix& viewMatrix,
                                bool useCoverageAA,
                                const SkRect& circle,
                                const SkStrokeRec& stroke) {
    SkPoint center = SkPoint::Make(circle.centerX(), circle.centerY());
    viewMatrix.mapPoints(&center, 1);
    SkScalar radius = viewMatrix.mapRadius(SkScalarHalf(circle.width()));
    SkScalar strokeWidth = viewMatrix.mapRadius(stroke.getWidth());

    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return;
    }

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

    SkAutoTUnref<GrGeometryProcessor> gp(
            CircleEdgeEffect::Create(color, isStrokeOnly && innerRadius > 0,invert));

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, gp->getVertexStride(),  0);
    SkASSERT(gp->getVertexStride() == sizeof(CircleVertex));
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return;
    }

    CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

    // The radii are outset for two reasons. First, it allows the shader to simply perform simpler
    // computation because the computed alpha is zero, rather than 50%, at the radius.
    // Second, the outer radius is used to compute the verts of the bounding box that is rendered
    // and the outset ensures the box will cover all partially covered by the circle.
    outerRadius += SK_ScalarHalf;
    innerRadius -= SK_ScalarHalf;

    SkRect bounds = SkRect::MakeLTRB(
        center.fX - outerRadius,
        center.fY - outerRadius,
        center.fX + outerRadius,
        center.fY + outerRadius
    );

    // The inner radius in the vertex data must be specified in normalized space.
    innerRadius = innerRadius / outerRadius;
    verts[0].fPos = SkPoint::Make(bounds.fLeft,  bounds.fTop);
    verts[0].fOffset = SkPoint::Make(-1, -1);
    verts[0].fOuterRadius = outerRadius;
    verts[0].fInnerRadius = innerRadius;

    verts[1].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
    verts[1].fOffset = SkPoint::Make(-1, 1);
    verts[1].fOuterRadius = outerRadius;
    verts[1].fInnerRadius = innerRadius;

    verts[2].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
    verts[2].fOffset = SkPoint::Make(1, 1);
    verts[2].fOuterRadius = outerRadius;
    verts[2].fInnerRadius = innerRadius;

    verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
    verts[3].fOffset = SkPoint::Make(1, -1);
    verts[3].fOuterRadius = outerRadius;
    verts[3].fInnerRadius = innerRadius;

    target->setIndexSourceToBuffer(fGpu->getQuadIndexBuffer());
    target->drawIndexedInstances(drawState, gp, kTriangles_GrPrimitiveType, 1, 4, 6, &bounds);
    target->resetIndexSource();
}

///////////////////////////////////////////////////////////////////////////////

bool GrOvalRenderer::drawEllipse(GrDrawTarget* target,
                                 GrDrawState* drawState,
                                 GrColor color,
                                 const SkMatrix& viewMatrix,
                                 bool useCoverageAA,
                                 const SkRect& ellipse,
                                 const SkStrokeRec& stroke) {
#ifdef SK_DEBUG
    {
        // we should have checked for this previously
        bool isAxisAlignedEllipse = viewMatrix.rectStaysRect();
        SkASSERT(useCoverageAA && isAxisAlignedEllipse);
    }
#endif

    // do any matrix crunching before we reset the draw state for device coords
    SkPoint center = SkPoint::Make(ellipse.centerX(), ellipse.centerY());
    viewMatrix.mapPoints(&center, 1);
    SkScalar ellipseXRadius = SkScalarHalf(ellipse.width());
    SkScalar ellipseYRadius = SkScalarHalf(ellipse.height());
    SkScalar xRadius = SkScalarAbs(viewMatrix[SkMatrix::kMScaleX]*ellipseXRadius +
                                   viewMatrix[SkMatrix::kMSkewY]*ellipseYRadius);
    SkScalar yRadius = SkScalarAbs(viewMatrix[SkMatrix::kMSkewX]*ellipseXRadius +
                                   viewMatrix[SkMatrix::kMScaleY]*ellipseYRadius);

    // do (potentially) anisotropic mapping of stroke
    SkVector scaledStroke;
    SkScalar strokeWidth = stroke.getWidth();
    scaledStroke.fX = SkScalarAbs(strokeWidth*(viewMatrix[SkMatrix::kMScaleX] +
                                               viewMatrix[SkMatrix::kMSkewY]));
    scaledStroke.fY = SkScalarAbs(strokeWidth*(viewMatrix[SkMatrix::kMSkewX] +
                                               viewMatrix[SkMatrix::kMScaleY]));

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

    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return false;
    }

    SkAutoTUnref<GrGeometryProcessor> gp(
            EllipseEdgeEffect::Create(color,
                                      isStrokeOnly && innerXRadius > 0 && innerYRadius > 0,
                                      invert));

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, gp->getVertexStride(),  0);
    SkASSERT(gp->getVertexStride() == sizeof(EllipseVertex));
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return false;
    }

    EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(geo.vertices());

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

    target->setIndexSourceToBuffer(fGpu->getQuadIndexBuffer());
    target->drawIndexedInstances(drawState, gp, kTriangles_GrPrimitiveType, 1, 4, 6, &bounds);
    target->resetIndexSource();

    return true;
}

bool GrOvalRenderer::drawDIEllipse(GrDrawTarget* target,
                                   GrDrawState* drawState,
                                   GrColor color,
                                   const SkMatrix& viewMatrix,
                                   bool useCoverageAA,
                                   const SkRect& ellipse,
                                   const SkStrokeRec& stroke) {
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

    SkAutoTUnref<GrGeometryProcessor> gp(DIEllipseEdgeEffect::Create(color, viewMatrix, mode));

    GrDrawTarget::AutoReleaseGeometry geo(target, 4, gp->getVertexStride(),  0);
    SkASSERT(gp->getVertexStride() == sizeof(DIEllipseVertex));
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return false;
    }

    DIEllipseVertex* verts = reinterpret_cast<DIEllipseVertex*>(geo.vertices());

    // This expands the outer rect so that after CTM we end up with a half-pixel border
    SkScalar a = viewMatrix[SkMatrix::kMScaleX];
    SkScalar b = viewMatrix[SkMatrix::kMSkewX];
    SkScalar c = viewMatrix[SkMatrix::kMSkewY];
    SkScalar d = viewMatrix[SkMatrix::kMScaleY];
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

    target->setIndexSourceToBuffer(fGpu->getQuadIndexBuffer());
    target->drawIndexedInstances(drawState, gp, kTriangles_GrPrimitiveType, 1, 4, 6, &bounds);
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

GrIndexBuffer* GrOvalRenderer::rRectIndexBuffer(bool isStrokeOnly) {
    if (isStrokeOnly) {
        if (NULL == fStrokeRRectIndexBuffer) {
            fStrokeRRectIndexBuffer = fGpu->createInstancedIndexBuffer(gRRectIndices,
                                                                       kIndicesPerStrokeRRect,
                                                                       kNumRRectsInIndexBuffer,
                                                                       kVertsPerRRect);
        }
        return fStrokeRRectIndexBuffer;
    } else {
        if (NULL == fRRectIndexBuffer) {
            fRRectIndexBuffer = fGpu->createInstancedIndexBuffer(gRRectIndices,
                                                                 kIndicesPerRRect,
                                                                 kNumRRectsInIndexBuffer,
                                                                 kVertsPerRRect);
        }
        return fRRectIndexBuffer;
    }
}

bool GrOvalRenderer::drawDRRect(GrDrawTarget* target,
                                GrDrawState* drawState,
                                GrColor color,
                                const SkMatrix& viewMatrix,
                                bool useAA,
                                const SkRRect& origOuter,
                                const SkRRect& origInner) {
    bool applyAA = useAA &&
                   !drawState->getRenderTarget()->isMultisampled() &&
                   drawState->canUseFracCoveragePrimProc(color, *target->caps());
    GrDrawState::AutoRestoreEffects are;
    if (!origInner.isEmpty()) {
        SkTCopyOnFirstWrite<SkRRect> inner(origInner);
        if (!viewMatrix.isIdentity()) {
            if (!origInner.transform(viewMatrix, inner.writable())) {
                return false;
            }
        }
        GrPrimitiveEdgeType edgeType = applyAA ?
                kInverseFillAA_GrProcessorEdgeType :
                kInverseFillBW_GrProcessorEdgeType;
        // TODO this needs to be a geometry processor
        GrFragmentProcessor* fp = GrRRectEffect::Create(edgeType, *inner);
        if (NULL == fp) {
            return false;
        }
        are.set(drawState);
        drawState->addCoverageProcessor(fp)->unref();
    }

    SkStrokeRec fillRec(SkStrokeRec::kFill_InitStyle);
    if (this->drawRRect(target, drawState, color, viewMatrix, useAA, origOuter, fillRec)) {
        return true;
    }

    SkASSERT(!origOuter.isEmpty());
    SkTCopyOnFirstWrite<SkRRect> outer(origOuter);
    if (!viewMatrix.isIdentity()) {
        if (!origOuter.transform(viewMatrix, outer.writable())) {
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
        are.set(drawState);
    }

    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return false;
    }

    drawState->addCoverageProcessor(effect)->unref();
    SkRect bounds = outer->getBounds();
    if (applyAA) {
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);
    }
    target->drawRect(drawState, color, SkMatrix::I(), bounds, NULL, &invert);
    return true;
}

bool GrOvalRenderer::drawRRect(GrDrawTarget* target,
                               GrDrawState* drawState,
                               GrColor color,
                               const SkMatrix& viewMatrix,
                               bool useAA,
                               const SkRRect& rrect,
                               const SkStrokeRec& stroke) {
    if (rrect.isOval()) {
        return this->drawOval(target, drawState, color, viewMatrix, useAA, rrect.getBounds(),
                              stroke);
    }

    bool useCoverageAA = useAA &&
        !drawState->getRenderTarget()->isMultisampled() &&
        drawState->canUseFracCoveragePrimProc(color, *target->caps());

    // only anti-aliased rrects for now
    if (!useCoverageAA) {
        return false;
    }

    if (!viewMatrix.rectStaysRect() || !rrect.isSimple()) {
        return false;
    }

    // do any matrix crunching before we reset the draw state for device coords
    const SkRect& rrectBounds = rrect.getBounds();
    SkRect bounds;
    viewMatrix.mapRect(&bounds, rrectBounds);

    SkVector radii = rrect.getSimpleRadii();
    SkScalar xRadius = SkScalarAbs(viewMatrix[SkMatrix::kMScaleX]*radii.fX +
                                   viewMatrix[SkMatrix::kMSkewY]*radii.fY);
    SkScalar yRadius = SkScalarAbs(viewMatrix[SkMatrix::kMSkewX]*radii.fX +
                                   viewMatrix[SkMatrix::kMScaleY]*radii.fY);

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
            scaledStroke.fX = SkScalarAbs(strokeWidth*(viewMatrix[SkMatrix::kMScaleX] +
                                                       viewMatrix[SkMatrix::kMSkewY]));
            scaledStroke.fY = SkScalarAbs(strokeWidth*(viewMatrix[SkMatrix::kMSkewX] +
                                                       viewMatrix[SkMatrix::kMScaleY]));
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
    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        SkDebugf("Failed to invert\n");
        return false;
    }

    GrIndexBuffer* indexBuffer = this->rRectIndexBuffer(isStrokeOnly);
    if (NULL == indexBuffer) {
        SkDebugf("Failed to create index buffer!\n");
        return false;
    }

    // if the corners are circles, use the circle renderer
    if ((!hasStroke || scaledStroke.fX == scaledStroke.fY) && xRadius == yRadius) {
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

        SkAutoTUnref<GrGeometryProcessor> effect(CircleEdgeEffect::Create(color,
                                                                          isStrokeOnly,
                                                                          invert));

        GrDrawTarget::AutoReleaseGeometry geo(target, 16, effect->getVertexStride(),  0);
        SkASSERT(effect->getVertexStride() == sizeof(CircleVertex));
        if (!geo.succeeded()) {
            SkDebugf("Failed to get space for vertices!\n");
            return false;
        }
        CircleVertex* verts = reinterpret_cast<CircleVertex*>(geo.vertices());

        // The radii are outset for two reasons. First, it allows the shader to simply perform
        // simpler computation because the computed alpha is zero, rather than 50%, at the radius.
        // Second, the outer radius is used to compute the verts of the bounding box that is
        // rendered and the outset ensures the box will cover all partially covered by the rrect
        // corners.
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
        SkScalar yOuterRadii[4] = {-1, 0, 0, 1 };
        // The inner radius in the vertex data must be specified in normalized space.
        innerRadius = innerRadius / outerRadius;
        for (int i = 0; i < 4; ++i) {
            verts->fPos = SkPoint::Make(bounds.fLeft, yCoords[i]);
            verts->fOffset = SkPoint::Make(-1, yOuterRadii[i]);
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
            verts->fOffset = SkPoint::Make(1, yOuterRadii[i]);
            verts->fOuterRadius = outerRadius;
            verts->fInnerRadius = innerRadius;
            verts++;
        }

        // drop out the middle quad if we're stroked
        int indexCnt = isStrokeOnly ? SK_ARRAY_COUNT(gRRectIndices) - 6 :
                                      SK_ARRAY_COUNT(gRRectIndices);
        target->setIndexSourceToBuffer(indexBuffer);
        target->drawIndexedInstances(drawState, effect, kTriangles_GrPrimitiveType, 1, 16, indexCnt,
                                     &bounds);

    // otherwise we use the ellipse renderer
    } else {
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

        SkAutoTUnref<GrGeometryProcessor> effect(EllipseEdgeEffect::Create(color,
                                                                           isStrokeOnly,
                                                                           invert));

        GrDrawTarget::AutoReleaseGeometry geo(target, 16, effect->getVertexStride(),  0);
        SkASSERT(effect->getVertexStride() == sizeof(EllipseVertex));
        if (!geo.succeeded()) {
            SkDebugf("Failed to get space for vertices!\n");
            return false;
        }

        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(geo.vertices());

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
        target->drawIndexedInstances(drawState, effect, kTriangles_GrPrimitiveType, 1, 16, indexCnt,
                                     &bounds);
    }

    target->resetIndexSource();
    return true;
}
