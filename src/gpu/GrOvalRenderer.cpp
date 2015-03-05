/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalRenderer.h"

#include "GrBatch.h"
#include "GrBatchTarget.h"
#include "GrBufferAllocPool.h"
#include "GrDrawTarget.h"
#include "GrGeometryProcessor.h"
#include "GrGpu.h"
#include "GrInvariantOutput.h"
#include "GrPipelineBuilder.h"
#include "GrProcessor.h"
#include "SkRRect.h"
#include "SkStrokeRec.h"
#include "SkTLazy.h"
#include "effects/GrRRectEffect.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

// TODO(joshualitt) - Break this file up during GrBatch post implementation cleanup

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

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inCircleEdge() const { return fInCircleEdge; }
    virtual ~CircleEdgeEffect() {}

    const char* name() const SK_OVERRIDE { return "CircleEdge"; }

    inline bool isStroked() const { return fStroke; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor&,
                    const GrBatchTracker&)
            : fColor(GrColor_ILLEGAL) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) SK_OVERRIDE{
            const CircleEdgeEffect& ce = args.fGP.cast<CircleEdgeEffect>();
            GrGLGPBuilder* pb = args.fPB;
            const BatchTracker& local = args.fBT.cast<BatchTracker>();
            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

            // emit attributes
            vsBuilder->emitAttributes(ce);

            GrGLVertToFrag v(kVec4f_GrSLType);
            args.fPB->addVarying("CircleEdge", &v);
            vsBuilder->codeAppendf("%s = %s;", v.vsOut(), ce.inCircleEdge()->fName);

            // Setup pass through color
            this->setupColorPassThrough(pb, local.fInputColorType, args.fOutputColor, NULL,
                                        &fColorUniform);

            // Setup position
            this->setupPosition(pb, gpArgs, ce.inPosition()->fName, ce.viewMatrix());

            // emit transforms
            this->emitTransforms(args.fPB, gpArgs->fPositionVar, ce.inPosition()->fName,
                                 ce.localMatrix(), args.fTransformsIn, args.fTransformsOut);;

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

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrBatchTracker& bt,
                           const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const BatchTracker& local = bt.cast<BatchTracker>();
            const CircleEdgeEffect& circleEffect = gp.cast<CircleEdgeEffect>();
            uint16_t key = circleEffect.isStroked() ? 0x1 : 0x0;
            key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x2 : 0x0;
            key |= ComputePosKey(gp.viewMatrix()) << 2;
            b->add32(key << 16 | local.fInputColorType);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) SK_OVERRIDE {
            this->setUniformViewMatrix(pdman, gp.viewMatrix());

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

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLCaps&) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

    void initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const SK_OVERRIDE {
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
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
        fInCircleEdge = &this->addVertexAttrib(Attribute("inCircleEdge",
                                                           kVec4f_GrVertexAttribType));
        fStroke = stroke;
    }

    bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const CircleEdgeEffect& cee = other.cast<CircleEdgeEffect>();
        return cee.fStroke == fStroke;
    }

    void onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    struct BatchTracker {
        GrGPInput fInputColorType;
        GrColor fColor;
        bool fUsesLocalCoords;
    };

    const Attribute* fInPosition;
    const Attribute* fInCircleEdge;
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

    const char* name() const SK_OVERRIDE { return "EllipseEdge"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inEllipseOffset() const { return fInEllipseOffset; }
    const Attribute* inEllipseRadii() const { return fInEllipseRadii; }

    inline bool isStroked() const { return fStroke; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor&,
                    const GrBatchTracker&)
            : fColor(GrColor_ILLEGAL) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) SK_OVERRIDE{
            const EllipseEdgeEffect& ee = args.fGP.cast<EllipseEdgeEffect>();
            GrGLGPBuilder* pb = args.fPB;
            const BatchTracker& local = args.fBT.cast<BatchTracker>();
            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

            // emit attributes
            vsBuilder->emitAttributes(ee);

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

            // Setup position
            this->setupPosition(pb, gpArgs, ee.inPosition()->fName, ee.viewMatrix());

            // emit transforms
            this->emitTransforms(args.fPB, gpArgs->fPositionVar, ee.inPosition()->fName,
                                 ee.localMatrix(), args.fTransformsIn, args.fTransformsOut);

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

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrBatchTracker& bt,
                           const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const BatchTracker& local = bt.cast<BatchTracker>();
            const EllipseEdgeEffect& ellipseEffect = gp.cast<EllipseEdgeEffect>();
            uint16_t key = ellipseEffect.isStroked() ? 0x1 : 0x0;
            key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x2 : 0x0;
            key |= ComputePosKey(gp.viewMatrix()) << 2;
            b->add32(key << 16 | local.fInputColorType);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) SK_OVERRIDE {
            this->setUniformViewMatrix(pdman, gp.viewMatrix());

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

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLCaps&) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

    void initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const SK_OVERRIDE {
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
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
        fInEllipseOffset = &this->addVertexAttrib(Attribute("inEllipseOffset",
                                                              kVec2f_GrVertexAttribType));
        fInEllipseRadii = &this->addVertexAttrib(Attribute("inEllipseRadii",
                                                             kVec4f_GrVertexAttribType));
        fStroke = stroke;
    }

    bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const EllipseEdgeEffect& eee = other.cast<EllipseEdgeEffect>();
        return eee.fStroke == fStroke;
    }

    void onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    struct BatchTracker {
        GrGPInput fInputColorType;
        GrColor fColor;
        bool fUsesLocalCoords;
    };

    const Attribute* fInPosition;
    const Attribute* fInEllipseOffset;
    const Attribute* fInEllipseRadii;
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

    const char* name() const SK_OVERRIDE { return "DIEllipseEdge"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inEllipseOffsets0() const { return fInEllipseOffsets0; }
    const Attribute* inEllipseOffsets1() const { return fInEllipseOffsets1; }

    inline Mode getMode() const { return fMode; }

    class GLProcessor : public GrGLGeometryProcessor {
    public:
        GLProcessor(const GrGeometryProcessor&,
                    const GrBatchTracker&)
            : fColor(GrColor_ILLEGAL) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) SK_OVERRIDE{
            const DIEllipseEdgeEffect& ee = args.fGP.cast<DIEllipseEdgeEffect>();
            GrGLGPBuilder* pb = args.fPB;
            const BatchTracker& local = args.fBT.cast<BatchTracker>();
            GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();

            // emit attributes
            vsBuilder->emitAttributes(ee);

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

            // Setup position
            this->setupPosition(pb, gpArgs, ee.inPosition()->fName, ee.viewMatrix());

            // emit transforms
            this->emitTransforms(args.fPB, gpArgs->fPositionVar, ee.inPosition()->fName,
                                 ee.localMatrix(), args.fTransformsIn, args.fTransformsOut);

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

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrBatchTracker& bt,
                           const GrGLCaps&,
                           GrProcessorKeyBuilder* b) {
            const BatchTracker& local = bt.cast<BatchTracker>();
            const DIEllipseEdgeEffect& ellipseEffect = gp.cast<DIEllipseEdgeEffect>();
            uint16_t key = ellipseEffect.getMode();
            key |= local.fUsesLocalCoords && gp.localMatrix().hasPerspective() ? 0x1 << 8 : 0x0;
            key |= ComputePosKey(gp.viewMatrix()) << 9;
            b->add32(key << 16 | local.fInputColorType);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrPrimitiveProcessor& gp,
                             const GrBatchTracker& bt) SK_OVERRIDE {
            this->setUniformViewMatrix(pdman, gp.viewMatrix());

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

    virtual GrGLPrimitiveProcessor* createGLInstance(const GrBatchTracker& bt,
                                                     const GrGLCaps&) const SK_OVERRIDE {
        return SkNEW_ARGS(GLProcessor, (*this, bt));
    }

    void initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const SK_OVERRIDE {
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
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
        fInEllipseOffsets0 = &this->addVertexAttrib(Attribute("inEllipseOffsets0",
                                                                kVec2f_GrVertexAttribType));
        fInEllipseOffsets1 = &this->addVertexAttrib(Attribute("inEllipseOffsets1",
                                                                kVec2f_GrVertexAttribType));
        fMode = mode;
    }

    bool onIsEqual(const GrGeometryProcessor& other) const SK_OVERRIDE {
        const DIEllipseEdgeEffect& eee = other.cast<DIEllipseEdgeEffect>();
        return eee.fMode == fMode;
    }

    void onGetInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    struct BatchTracker {
        GrGPInput fInputColorType;
        GrColor fColor;
        bool fUsesLocalCoords;
    };

    const Attribute* fInPosition;
    const Attribute* fInEllipseOffsets0;
    const Attribute* fInEllipseOffsets1;
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
                              GrPipelineBuilder* pipelineBuilder,
                              GrColor color,
                              const SkMatrix& viewMatrix,
                              bool useAA,
                              const SkRect& oval,
                              const SkStrokeRec& stroke)
{
    bool useCoverageAA = useAA &&
        !pipelineBuilder->getRenderTarget()->isMultisampled();

    if (!useCoverageAA) {
        return false;
    }

    // we can draw circles
    if (SkScalarNearlyEqual(oval.width(), oval.height()) && circle_stays_circle(viewMatrix)) {
        this->drawCircle(target, pipelineBuilder, color, viewMatrix, useCoverageAA, oval, stroke);
    // if we have shader derivative support, render as device-independent
    } else if (target->caps()->shaderDerivativeSupport()) {
        return this->drawDIEllipse(target, pipelineBuilder, color, viewMatrix, useCoverageAA, oval,
                                   stroke);
    // otherwise axis-aligned ellipses only
    } else if (viewMatrix.rectStaysRect()) {
        return this->drawEllipse(target, pipelineBuilder, color, viewMatrix, useCoverageAA, oval,
                                 stroke);
    } else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

class CircleBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkScalar fInnerRadius;
        SkScalar fOuterRadius;
        bool fStroke;
        SkRect fDevBounds;
    };

    static GrBatch* Create(const Geometry& geometry) {
        return SkNEW_ARGS(CircleBatch, (geometry));
    }

    const char* name() const SK_OVERRIDE { return "CircleBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) SK_OVERRIDE {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fGeoData[0].fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fStroke = fGeoData[0].fStroke;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        SkMatrix invert;
        if (!this->viewMatrix().invert(&invert)) {
            return;
        }

        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(CircleEdgeEffect::Create(this->color(),
                                                                      this->stroke(),
                                                                      invert));

        batchTarget->initDraw(gp, pipeline);

        // TODO this is hacky, but the only way we have to initialize the GP is to use the
        // GrPipelineInfo struct so we can generate the correct shader.  Once we have GrBatch
        // everywhere we can remove this nastiness
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        gp->initBatchTracker(batchTarget->currentBatchTracker(), init);

        int instanceCount = fGeoData.count();
        int vertexCount = kVertsPerCircle * instanceCount;
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(CircleVertex));

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void *vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices || !batchTarget->quadIndexBuffer()) {
            SkDebugf("Could not allocate buffers\n");
            return;
        }

        CircleVertex* verts = reinterpret_cast<CircleVertex*>(vertices);

        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];

            SkScalar innerRadius = args.fInnerRadius;
            SkScalar outerRadius = args.fOuterRadius;

            const SkRect& bounds = args.fDevBounds;

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

            verts += kVertsPerCircle;
        }

        const GrIndexBuffer* quadIndexBuffer = batchTarget->quadIndexBuffer();

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(kVertsPerCircle);
        drawInfo.setIndicesPerInstance(kIndicesPerCircle);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(quadIndexBuffer);

        int maxInstancesPerDraw = quadIndexBuffer->maxQuads();

        while (instanceCount) {
            drawInfo.setInstanceCount(SkTMin(instanceCount, maxInstancesPerDraw));
            drawInfo.setVertexCount(drawInfo.instanceCount() * drawInfo.verticesPerInstance());
            drawInfo.setIndexCount(drawInfo.instanceCount() * drawInfo.indicesPerInstance());

            batchTarget->draw(drawInfo);

            drawInfo.setStartVertex(drawInfo.startVertex() + drawInfo.vertexCount());
            instanceCount -= drawInfo.instanceCount();
        }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    CircleBatch(const Geometry& geometry) {
        this->initClassID<CircleBatch>();
        fGeoData.push_back(geometry);
    }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        CircleBatch* that = t->cast<CircleBatch>();

        // TODO use vertex color to avoid breaking batches
        if (this->color() != that->color()) {
            return false;
        }

        if (this->stroke() != that->stroke()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool stroke() const { return fBatch.fStroke; }

    struct BatchTracker {
        GrColor fColor;
        bool fStroke;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    static const int kVertsPerCircle = 4;
    static const int kIndicesPerCircle = 6;

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

void GrOvalRenderer::drawCircle(GrDrawTarget* target,
                                GrPipelineBuilder* pipelineBuilder,
                                GrColor color,
                                const SkMatrix& viewMatrix,
                                bool useCoverageAA,
                                const SkRect& circle,
                                const SkStrokeRec& stroke) {
    SkPoint center = SkPoint::Make(circle.centerX(), circle.centerY());
    viewMatrix.mapPoints(&center, 1);
    SkScalar radius = viewMatrix.mapRadius(SkScalarHalf(circle.width()));
    SkScalar strokeWidth = viewMatrix.mapRadius(stroke.getWidth());

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

    CircleBatch::Geometry geometry;
    geometry.fViewMatrix = viewMatrix;
    geometry.fColor = color;
    geometry.fInnerRadius = innerRadius;
    geometry.fOuterRadius = outerRadius;
    geometry.fStroke = isStrokeOnly && innerRadius > 0;
    geometry.fDevBounds = bounds;

    SkAutoTUnref<GrBatch> batch(CircleBatch::Create(geometry));
    target->drawBatch(pipelineBuilder, batch, &bounds);
}

///////////////////////////////////////////////////////////////////////////////

class EllipseBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        bool fStroke;
        SkRect fDevBounds;
    };

    static GrBatch* Create(const Geometry& geometry) {
        return SkNEW_ARGS(EllipseBatch, (geometry));
    }

    const char* name() const SK_OVERRIDE { return "EllipseBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) SK_OVERRIDE {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fGeoData[0].fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fStroke = fGeoData[0].fStroke;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        SkMatrix invert;
        if (!this->viewMatrix().invert(&invert)) {
            return;
        }

        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(EllipseEdgeEffect::Create(this->color(),
                                                                       this->stroke(),
                                                                       invert));

        batchTarget->initDraw(gp, pipeline);

        // TODO this is hacky, but the only way we have to initialize the GP is to use the
        // GrPipelineInfo struct so we can generate the correct shader.  Once we have GrBatch
        // everywhere we can remove this nastiness
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        gp->initBatchTracker(batchTarget->currentBatchTracker(), init);

        int instanceCount = fGeoData.count();
        int vertexCount = kVertsPerEllipse * instanceCount;
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(EllipseVertex));

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void *vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices || !batchTarget->quadIndexBuffer()) {
            SkDebugf("Could not allocate buffers\n");
            return;
        }

        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(vertices);

        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];

            SkScalar xRadius = args.fXRadius;
            SkScalar yRadius = args.fYRadius;

            // Compute the reciprocals of the radii here to save time in the shader
            SkScalar xRadRecip = SkScalarInvert(xRadius);
            SkScalar yRadRecip = SkScalarInvert(yRadius);
            SkScalar xInnerRadRecip = SkScalarInvert(args.fInnerXRadius);
            SkScalar yInnerRadRecip = SkScalarInvert(args.fInnerYRadius);

            const SkRect& bounds = args.fDevBounds;

            // The inner radius in the vertex data must be specified in normalized space.
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

            verts += kVertsPerEllipse;
        }

        const GrIndexBuffer* quadIndexBuffer = batchTarget->quadIndexBuffer();

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(kVertsPerEllipse);
        drawInfo.setIndicesPerInstance(kIndicesPerEllipse);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(quadIndexBuffer);

        int maxInstancesPerDraw = quadIndexBuffer->maxQuads();

        while (instanceCount) {
            drawInfo.setInstanceCount(SkTMin(instanceCount, maxInstancesPerDraw));
            drawInfo.setVertexCount(drawInfo.instanceCount() * drawInfo.verticesPerInstance());
            drawInfo.setIndexCount(drawInfo.instanceCount() * drawInfo.indicesPerInstance());

            batchTarget->draw(drawInfo);

            drawInfo.setStartVertex(drawInfo.startVertex() + drawInfo.vertexCount());
            instanceCount -= drawInfo.instanceCount();
        }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    EllipseBatch(const Geometry& geometry) {
        this->initClassID<EllipseBatch>();
        fGeoData.push_back(geometry);
    }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        EllipseBatch* that = t->cast<EllipseBatch>();

        // TODO use vertex color to avoid breaking batches
        if (this->color() != that->color()) {
            return false;
        }

        if (this->stroke() != that->stroke()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool stroke() const { return fBatch.fStroke; }

    struct BatchTracker {
        GrColor fColor;
        bool fStroke;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    static const int kVertsPerEllipse = 4;
    static const int kIndicesPerEllipse = 6;

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

bool GrOvalRenderer::drawEllipse(GrDrawTarget* target,
                                 GrPipelineBuilder* pipelineBuilder,
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

    EllipseBatch::Geometry geometry;
    geometry.fViewMatrix = viewMatrix;
    geometry.fColor = color;
    geometry.fXRadius = xRadius;
    geometry.fYRadius = yRadius;
    geometry.fInnerXRadius = innerXRadius;
    geometry.fInnerYRadius = innerYRadius;
    geometry.fStroke = isStrokeOnly && innerXRadius > 0 && innerYRadius > 0;
    geometry.fDevBounds = bounds;

    SkAutoTUnref<GrBatch> batch(EllipseBatch::Create(geometry));
    target->drawBatch(pipelineBuilder, batch, &bounds);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

class DIEllipseBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        SkScalar fGeoDx;
        SkScalar fGeoDy;
        DIEllipseEdgeEffect::Mode fMode;
        SkRect fDevBounds;
    };

    static GrBatch* Create(const Geometry& geometry) {
        return SkNEW_ARGS(DIEllipseBatch, (geometry));
    }

    const char* name() const SK_OVERRIDE { return "DIEllipseBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) SK_OVERRIDE {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fGeoData[0].fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fMode = fGeoData[0].fMode;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(DIEllipseEdgeEffect::Create(this->color(),
                                                                         this->viewMatrix(),
                                                                         this->mode()));

        batchTarget->initDraw(gp, pipeline);

        // TODO this is hacky, but the only way we have to initialize the GP is to use the
        // GrPipelineInfo struct so we can generate the correct shader.  Once we have GrBatch
        // everywhere we can remove this nastiness
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        gp->initBatchTracker(batchTarget->currentBatchTracker(), init);

        int instanceCount = fGeoData.count();
        int vertexCount = kVertsPerEllipse * instanceCount;
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(DIEllipseVertex));

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void *vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices || !batchTarget->quadIndexBuffer()) {
            SkDebugf("Could not allocate buffers\n");
            return;
        }

        DIEllipseVertex* verts = reinterpret_cast<DIEllipseVertex*>(vertices);

        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];

            SkScalar xRadius = args.fXRadius;
            SkScalar yRadius = args.fYRadius;

            const SkRect& bounds = args.fDevBounds;

            // This adjusts the "radius" to include the half-pixel border
            SkScalar offsetDx = SkScalarDiv(args.fGeoDx, xRadius);
            SkScalar offsetDy = SkScalarDiv(args.fGeoDy, yRadius);

            SkScalar innerRatioX = SkScalarDiv(xRadius, args.fInnerXRadius);
            SkScalar innerRatioY = SkScalarDiv(yRadius, args.fInnerYRadius);

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

            verts += kVertsPerEllipse;
        }

        const GrIndexBuffer* quadIndexBuffer = batchTarget->quadIndexBuffer();

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(kVertsPerEllipse);
        drawInfo.setIndicesPerInstance(kIndicesPerEllipse);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(quadIndexBuffer);

        int maxInstancesPerDraw = quadIndexBuffer->maxQuads();

        while (instanceCount) {
            drawInfo.setInstanceCount(SkTMin(instanceCount, maxInstancesPerDraw));
            drawInfo.setVertexCount(drawInfo.instanceCount() * drawInfo.verticesPerInstance());
            drawInfo.setIndexCount(drawInfo.instanceCount() * drawInfo.indicesPerInstance());

            batchTarget->draw(drawInfo);

            drawInfo.setStartVertex(drawInfo.startVertex() + drawInfo.vertexCount());
            instanceCount -= drawInfo.instanceCount();
        }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    DIEllipseBatch(const Geometry& geometry) {
        this->initClassID<DIEllipseBatch>();
        fGeoData.push_back(geometry);
    }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        DIEllipseBatch* that = t->cast<DIEllipseBatch>();

        // TODO use vertex color to avoid breaking batches
        if (this->color() != that->color()) {
            return false;
        }

        if (this->mode() != that->mode()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    DIEllipseEdgeEffect::Mode mode() const { return fBatch.fMode; }

    struct BatchTracker {
        GrColor fColor;
        DIEllipseEdgeEffect::Mode fMode;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    static const int kVertsPerEllipse = 4;
    static const int kIndicesPerEllipse = 6;

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

bool GrOvalRenderer::drawDIEllipse(GrDrawTarget* target,
                                   GrPipelineBuilder* pipelineBuilder,
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

    // This expands the outer rect so that after CTM we end up with a half-pixel border
    SkScalar a = viewMatrix[SkMatrix::kMScaleX];
    SkScalar b = viewMatrix[SkMatrix::kMSkewX];
    SkScalar c = viewMatrix[SkMatrix::kMSkewY];
    SkScalar d = viewMatrix[SkMatrix::kMScaleY];
    SkScalar geoDx = SkScalarDiv(SK_ScalarHalf, SkScalarSqrt(a*a + c*c));
    SkScalar geoDy = SkScalarDiv(SK_ScalarHalf, SkScalarSqrt(b*b + d*d));

    SkRect bounds = SkRect::MakeLTRB(
        center.fX - xRadius - geoDx,
        center.fY - yRadius - geoDy,
        center.fX + xRadius + geoDx,
        center.fY + yRadius + geoDy
    );

    DIEllipseBatch::Geometry geometry;
    geometry.fViewMatrix = viewMatrix;
    geometry.fColor = color;
    geometry.fXRadius = xRadius;
    geometry.fYRadius = yRadius;
    geometry.fInnerXRadius = innerXRadius;
    geometry.fInnerYRadius = innerYRadius;
    geometry.fGeoDx = geoDx;
    geometry.fGeoDy = geoDy;
    geometry.fMode = mode;
    geometry.fDevBounds = bounds;

    SkAutoTUnref<GrBatch> batch(DIEllipseBatch::Create(geometry));
    target->drawBatch(pipelineBuilder, batch, &bounds);

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
                                GrPipelineBuilder* pipelineBuilder,
                                GrColor color,
                                const SkMatrix& viewMatrix,
                                bool useAA,
                                const SkRRect& origOuter,
                                const SkRRect& origInner) {
    bool applyAA = useAA &&
                   !pipelineBuilder->getRenderTarget()->isMultisampled();
    GrPipelineBuilder::AutoRestoreFragmentProcessors arfp;
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
        arfp.set(pipelineBuilder);
        pipelineBuilder->addCoverageProcessor(fp)->unref();
    }

    SkStrokeRec fillRec(SkStrokeRec::kFill_InitStyle);
    if (this->drawRRect(target, pipelineBuilder, color, viewMatrix, useAA, origOuter, fillRec)) {
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
    if (!arfp.isSet()) {
        arfp.set(pipelineBuilder);
    }

    SkMatrix invert;
    if (!viewMatrix.invert(&invert)) {
        return false;
    }

    pipelineBuilder->addCoverageProcessor(effect)->unref();
    SkRect bounds = outer->getBounds();
    if (applyAA) {
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);
    }
    target->drawRect(pipelineBuilder, color, SkMatrix::I(), bounds, NULL, &invert);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class RRectCircleRendererBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkScalar fInnerRadius;
        SkScalar fOuterRadius;
        bool fStroke;
        SkRect fDevBounds;
    };

    static GrBatch* Create(const Geometry& geometry, const GrIndexBuffer* indexBuffer) {
        return SkNEW_ARGS(RRectCircleRendererBatch, (geometry, indexBuffer));
    }

    const char* name() const SK_OVERRIDE { return "RRectCircleBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) SK_OVERRIDE {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fGeoData[0].fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fStroke = fGeoData[0].fStroke;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        // reset to device coordinates
        SkMatrix invert;
        if (!this->viewMatrix().invert(&invert)) {
            SkDebugf("Failed to invert\n");
            return;
        }

        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(CircleEdgeEffect::Create(this->color(),
                                                                      this->stroke(),
                                                                      invert));

        batchTarget->initDraw(gp, pipeline);

        // TODO this is hacky, but the only way we have to initialize the GP is to use the
        // GrPipelineInfo struct so we can generate the correct shader.  Once we have GrBatch
        // everywhere we can remove this nastiness
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        gp->initBatchTracker(batchTarget->currentBatchTracker(), init);

        int instanceCount = fGeoData.count();
        int vertexCount = kVertsPerRRect * instanceCount;
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(CircleVertex));

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void *vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        CircleVertex* verts = reinterpret_cast<CircleVertex*>(vertices);

        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];

            SkScalar outerRadius = args.fOuterRadius;

            const SkRect& bounds = args.fDevBounds;

            SkScalar yCoords[4] = {
                bounds.fTop,
                bounds.fTop + outerRadius,
                bounds.fBottom - outerRadius,
                bounds.fBottom
            };

            SkScalar yOuterRadii[4] = {-1, 0, 0, 1 };
            // The inner radius in the vertex data must be specified in normalized space.
            SkScalar innerRadius = args.fInnerRadius / args.fOuterRadius;
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
        }

        // drop out the middle quad if we're stroked
        int indexCnt = this->stroke() ? SK_ARRAY_COUNT(gRRectIndices) - 6 :
                                        SK_ARRAY_COUNT(gRRectIndices);


        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(kVertsPerRRect);
        drawInfo.setIndicesPerInstance(indexCnt);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(fIndexBuffer);

        int maxInstancesPerDraw = kNumRRectsInIndexBuffer;

        while (instanceCount) {
            drawInfo.setInstanceCount(SkTMin(instanceCount, maxInstancesPerDraw));
            drawInfo.setVertexCount(drawInfo.instanceCount() * drawInfo.verticesPerInstance());
            drawInfo.setIndexCount(drawInfo.instanceCount() * drawInfo.indicesPerInstance());

            batchTarget->draw(drawInfo);

            drawInfo.setStartVertex(drawInfo.startVertex() + drawInfo.vertexCount());
            instanceCount -= drawInfo.instanceCount();
        }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    RRectCircleRendererBatch(const Geometry& geometry, const GrIndexBuffer* indexBuffer)
        : fIndexBuffer(indexBuffer) {
        this->initClassID<RRectCircleRendererBatch>();
        fGeoData.push_back(geometry);
    }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        RRectCircleRendererBatch* that = t->cast<RRectCircleRendererBatch>();

        // TODO use vertex color to avoid breaking batches
        if (this->color() != that->color()) {
            return false;
        }

        if (this->stroke() != that->stroke()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool stroke() const { return fBatch.fStroke; }

    struct BatchTracker {
        GrColor fColor;
        bool fStroke;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
    const GrIndexBuffer* fIndexBuffer;
};

class RRectEllipseRendererBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        bool fStroke;
        SkRect fDevBounds;
    };

    static GrBatch* Create(const Geometry& geometry, const GrIndexBuffer* indexBuffer) {
        return SkNEW_ARGS(RRectEllipseRendererBatch, (geometry, indexBuffer));
    }

    const char* name() const SK_OVERRIDE { return "RRectEllipseRendererBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }
    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
    }

    void initBatchTracker(const GrPipelineInfo& init) SK_OVERRIDE {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fGeoData[0].fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fStroke = fGeoData[0].fStroke;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        // reset to device coordinates
        SkMatrix invert;
        if (!this->viewMatrix().invert(&invert)) {
            SkDebugf("Failed to invert\n");
            return;
        }

        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(EllipseEdgeEffect::Create(this->color(),
                                                                       this->stroke(),
                                                                       invert));

        batchTarget->initDraw(gp, pipeline);

        // TODO this is hacky, but the only way we have to initialize the GP is to use the
        // GrPipelineInfo struct so we can generate the correct shader.  Once we have GrBatch
        // everywhere we can remove this nastiness
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        gp->initBatchTracker(batchTarget->currentBatchTracker(), init);

        int instanceCount = fGeoData.count();
        int vertexCount = kVertsPerRRect * instanceCount;
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(EllipseVertex));

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void *vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(vertices);

        for (int i = 0; i < instanceCount; i++) {
            Geometry& args = fGeoData[i];

            // Compute the reciprocals of the radii here to save time in the shader
            SkScalar xRadRecip = SkScalarInvert(args.fXRadius);
            SkScalar yRadRecip = SkScalarInvert(args.fYRadius);
            SkScalar xInnerRadRecip = SkScalarInvert(args.fInnerXRadius);
            SkScalar yInnerRadRecip = SkScalarInvert(args.fInnerYRadius);

            // Extend the radii out half a pixel to antialias.
            SkScalar xOuterRadius = args.fXRadius + SK_ScalarHalf;
            SkScalar yOuterRadius = args.fYRadius + SK_ScalarHalf;

            const SkRect& bounds = args.fDevBounds;

            SkScalar yCoords[4] = {
                bounds.fTop,
                bounds.fTop + yOuterRadius,
                bounds.fBottom - yOuterRadius,
                bounds.fBottom
            };
            SkScalar yOuterOffsets[4] = {
                yOuterRadius,
                SK_ScalarNearlyZero, // we're using inversesqrt() in shader, so can't be exactly 0
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
        }

        // drop out the middle quad if we're stroked
        int indexCnt = this->stroke() ? SK_ARRAY_COUNT(gRRectIndices) - 6 :
                                        SK_ARRAY_COUNT(gRRectIndices);

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(kVertsPerRRect);
        drawInfo.setIndicesPerInstance(indexCnt);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(fIndexBuffer);

        int maxInstancesPerDraw = kNumRRectsInIndexBuffer;

        while (instanceCount) {
            drawInfo.setInstanceCount(SkTMin(instanceCount, maxInstancesPerDraw));
            drawInfo.setVertexCount(drawInfo.instanceCount() * drawInfo.verticesPerInstance());
            drawInfo.setIndexCount(drawInfo.instanceCount() * drawInfo.indicesPerInstance());

            batchTarget->draw(drawInfo);

            drawInfo.setStartVertex(drawInfo.startVertex() + drawInfo.vertexCount());
            instanceCount -= drawInfo.instanceCount();
        }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    RRectEllipseRendererBatch(const Geometry& geometry, const GrIndexBuffer* indexBuffer)
        : fIndexBuffer(indexBuffer) {
        this->initClassID<RRectEllipseRendererBatch>();
        fGeoData.push_back(geometry);
    }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        RRectEllipseRendererBatch* that = t->cast<RRectEllipseRendererBatch>();

        // TODO use vertex color to avoid breaking batches
        if (this->color() != that->color()) {
            return false;
        }

        if (this->stroke() != that->stroke()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool stroke() const { return fBatch.fStroke; }

    struct BatchTracker {
        GrColor fColor;
        bool fStroke;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
    const GrIndexBuffer* fIndexBuffer;
};

bool GrOvalRenderer::drawRRect(GrDrawTarget* target,
                               GrPipelineBuilder* pipelineBuilder,
                               GrColor color,
                               const SkMatrix& viewMatrix,
                               bool useAA,
                               const SkRRect& rrect,
                               const SkStrokeRec& stroke) {
    if (rrect.isOval()) {
        return this->drawOval(target, pipelineBuilder, color, viewMatrix, useAA, rrect.getBounds(),
                              stroke);
    }

    bool useCoverageAA = useAA &&
        !pipelineBuilder->getRenderTarget()->isMultisampled();

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

        // The radii are outset for two reasons. First, it allows the shader to simply perform
        // simpler computation because the computed alpha is zero, rather than 50%, at the radius.
        // Second, the outer radius is used to compute the verts of the bounding box that is
        // rendered and the outset ensures the box will cover all partially covered by the rrect
        // corners.
        outerRadius += SK_ScalarHalf;
        innerRadius -= SK_ScalarHalf;

        // Expand the rect so all the pixels will be captured.
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);

        RRectCircleRendererBatch::Geometry geometry;
        geometry.fViewMatrix = viewMatrix;
        geometry.fColor = color;
        geometry.fInnerRadius = innerRadius;
        geometry.fOuterRadius = outerRadius;
        geometry.fStroke = isStrokeOnly;
        geometry.fDevBounds = bounds;

        SkAutoTUnref<GrBatch> batch(RRectCircleRendererBatch::Create(geometry, indexBuffer));
        target->drawBatch(pipelineBuilder, batch, &bounds);

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

        // Expand the rect so all the pixels will be captured.
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);

        RRectEllipseRendererBatch::Geometry geometry;
        geometry.fViewMatrix = viewMatrix;
        geometry.fColor = color;
        geometry.fXRadius = xRadius;
        geometry.fYRadius = yRadius;
        geometry.fInnerXRadius = innerXRadius;
        geometry.fInnerYRadius = innerYRadius;
        geometry.fStroke = isStrokeOnly;
        geometry.fDevBounds = bounds;

        SkAutoTUnref<GrBatch> batch(RRectEllipseRendererBatch::Create(geometry, indexBuffer));
        target->drawBatch(pipelineBuilder, batch, &bounds);
    }
    return true;
}
