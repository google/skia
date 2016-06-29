/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalRenderer.h"

#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrGeometryProcessor.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "GrResourceProvider.h"
#include "SkRRect.h"
#include "SkStrokeRec.h"
#include "batches/GrVertexBatch.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLUtil.h"

// TODO(joshualitt) - Break this file up during GrBatch post implementation cleanup

namespace {

struct CircleVertex {
    SkPoint  fPos;
    GrColor  fColor;
    SkPoint  fOffset;
    SkScalar fOuterRadius;
    SkScalar fInnerRadius;
};

struct EllipseVertex {
    SkPoint  fPos;
    GrColor  fColor;
    SkPoint  fOffset;
    SkPoint  fOuterRadii;
    SkPoint  fInnerRadii;
};

struct DIEllipseVertex {
    SkPoint  fPos;
    GrColor  fColor;
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
 * with origin at the circle center. Three vertex attributes are used:
 *    vec2f : position in device space of the bounding geometry vertices
 *    vec4ub: color
 *    vec4f : (p.xy, outerRad, innerRad)
 *             p is the position in the normalized space.
 *             outerRad is the outerRadius in device space.
 *             innerRad is the innerRadius in normalized space (ignored if not stroking).
 */

class CircleGeometryProcessor : public GrGeometryProcessor {
public:
    CircleGeometryProcessor(bool stroke, const SkMatrix& localMatrix) : fLocalMatrix(localMatrix){
        this->initClassID<CircleGeometryProcessor>();
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                       kHigh_GrSLPrecision));
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
        fInCircleEdge = &this->addVertexAttrib(Attribute("inCircleEdge",
                                                         kVec4f_GrVertexAttribType));
        fStroke = stroke;
    }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inCircleEdge() const { return fInCircleEdge; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }
    virtual ~CircleGeometryProcessor() {}

    const char* name() const override { return "CircleEdge"; }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor() {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
            const CircleGeometryProcessor& cgp = args.fGP.cast<CircleGeometryProcessor>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(cgp);

            GrGLSLVertToFrag v(kVec4f_GrSLType);
            varyingHandler->addVarying("CircleEdge", &v);
            vertBuilder->codeAppendf("%s = %s;", v.vsOut(), cgp.inCircleEdge()->fName);

            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
            // setup pass through color
            varyingHandler->addPassThroughAttribute(cgp.inColor(), args.fOutputColor);

            // Setup position
            this->setupPosition(vertBuilder, gpArgs, cgp.inPosition()->fName);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 gpArgs->fPositionVar,
                                 cgp.inPosition()->fName,
                                 cgp.localMatrix(),
                                 args.fTransformsIn,
                                 args.fTransformsOut);

            fragBuilder->codeAppendf("float d = length(%s.xy);", v.fsIn());
            fragBuilder->codeAppendf("float edgeAlpha = clamp(%s.z * (1.0 - d), 0.0, 1.0);",
                                     v.fsIn());
            if (cgp.fStroke) {
                fragBuilder->codeAppendf("float innerAlpha = clamp(%s.z * (d - %s.w), 0.0, 1.0);",
                                         v.fsIn(), v.fsIn());
                fragBuilder->codeAppend("edgeAlpha *= innerAlpha;");
            }

            fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            const CircleGeometryProcessor& cgp = gp.cast<CircleGeometryProcessor>();
            uint16_t key = cgp.fStroke ? 0x1 : 0x0;
            key |= cgp.localMatrix().hasPerspective() ? 0x2 : 0x0;
            b->add32(key);
        }

        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrPrimitiveProcessor& gp) override {
        }

        void setTransformData(const GrPrimitiveProcessor& primProc,
                              const GrGLSLProgramDataManager& pdman,
                              int index,
                              const SkTArray<const GrCoordTransform*, true>& transforms) override {
            this->setTransformDataHelper<CircleGeometryProcessor>(primProc, pdman, index,
                                                                  transforms);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;
    };

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override {
        return new GLSLProcessor();
    }

private:
    SkMatrix         fLocalMatrix;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInCircleEdge;
    bool             fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(CircleGeometryProcessor);

sk_sp<GrGeometryProcessor> CircleGeometryProcessor::TestCreate(GrProcessorTestData* d) {
    return sk_sp<GrGeometryProcessor>(
        new CircleGeometryProcessor(d->fRandom->nextBool(), GrTest::TestMatrix(d->fRandom)));
}

///////////////////////////////////////////////////////////////////////////////

/**
 * The output of this effect is a modulation of the input color and coverage for an axis-aligned
 * ellipse, specified as a 2D offset from center, and the reciprocals of the outer and inner radii,
 * in both x and y directions.
 *
 * We are using an implicit function of x^2/a^2 + y^2/b^2 - 1 = 0.
 */

class EllipseGeometryProcessor : public GrGeometryProcessor {
public:
    EllipseGeometryProcessor(bool stroke, const SkMatrix& localMatrix)
        : fLocalMatrix(localMatrix) {
        this->initClassID<EllipseGeometryProcessor>();
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType));
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
        fInEllipseOffset = &this->addVertexAttrib(Attribute("inEllipseOffset",
                                                            kVec2f_GrVertexAttribType));
        fInEllipseRadii = &this->addVertexAttrib(Attribute("inEllipseRadii",
                                                           kVec4f_GrVertexAttribType));
        fStroke = stroke;
    }

    virtual ~EllipseGeometryProcessor() {}

    const char* name() const override { return "EllipseEdge"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inEllipseOffset() const { return fInEllipseOffset; }
    const Attribute* inEllipseRadii() const { return fInEllipseRadii; }
    const SkMatrix& localMatrix() const { return fLocalMatrix; }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor() {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override{
            const EllipseGeometryProcessor& egp = args.fGP.cast<EllipseGeometryProcessor>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(egp);

            GrGLSLVertToFrag ellipseOffsets(kVec2f_GrSLType);
            varyingHandler->addVarying("EllipseOffsets", &ellipseOffsets);
            vertBuilder->codeAppendf("%s = %s;", ellipseOffsets.vsOut(),
                                     egp.inEllipseOffset()->fName);

            GrGLSLVertToFrag ellipseRadii(kVec4f_GrSLType);
            varyingHandler->addVarying("EllipseRadii", &ellipseRadii);
            vertBuilder->codeAppendf("%s = %s;", ellipseRadii.vsOut(),
                                     egp.inEllipseRadii()->fName);

            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
            // setup pass through color
            varyingHandler->addPassThroughAttribute(egp.inColor(), args.fOutputColor);

            // Setup position
            this->setupPosition(vertBuilder, gpArgs, egp.inPosition()->fName);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 gpArgs->fPositionVar,
                                 egp.inPosition()->fName,
                                 egp.localMatrix(),
                                 args.fTransformsIn,
                                 args.fTransformsOut);

            // for outer curve
            fragBuilder->codeAppendf("vec2 scaledOffset = %s*%s.xy;", ellipseOffsets.fsIn(),
                                     ellipseRadii.fsIn());
            fragBuilder->codeAppend("float test = dot(scaledOffset, scaledOffset) - 1.0;");
            fragBuilder->codeAppendf("vec2 grad = 2.0*scaledOffset*%s.xy;", ellipseRadii.fsIn());
            fragBuilder->codeAppend("float grad_dot = dot(grad, grad);");

            // avoid calling inversesqrt on zero.
            fragBuilder->codeAppend("grad_dot = max(grad_dot, 1.0e-4);");
            fragBuilder->codeAppend("float invlen = inversesqrt(grad_dot);");
            fragBuilder->codeAppend("float edgeAlpha = clamp(0.5-test*invlen, 0.0, 1.0);");

            // for inner curve
            if (egp.fStroke) {
                fragBuilder->codeAppendf("scaledOffset = %s*%s.zw;",
                                         ellipseOffsets.fsIn(), ellipseRadii.fsIn());
                fragBuilder->codeAppend("test = dot(scaledOffset, scaledOffset) - 1.0;");
                fragBuilder->codeAppendf("grad = 2.0*scaledOffset*%s.zw;",
                                         ellipseRadii.fsIn());
                fragBuilder->codeAppend("invlen = inversesqrt(dot(grad, grad));");
                fragBuilder->codeAppend("edgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);");
            }

            fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            const EllipseGeometryProcessor& egp = gp.cast<EllipseGeometryProcessor>();
            uint16_t key = egp.fStroke ? 0x1 : 0x0;
            key |= egp.localMatrix().hasPerspective() ? 0x2 : 0x0;
            b->add32(key);
        }

        void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& gp) override {
        }

        void setTransformData(const GrPrimitiveProcessor& primProc,
                              const GrGLSLProgramDataManager& pdman,
                              int index,
                              const SkTArray<const GrCoordTransform*, true>& transforms) override {
            this->setTransformDataHelper<EllipseGeometryProcessor>(primProc, pdman, index,
                                                                   transforms);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;
    };

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override {
        return new GLSLProcessor();
    }

private:
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInEllipseOffset;
    const Attribute* fInEllipseRadii;
    SkMatrix fLocalMatrix;
    bool fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(EllipseGeometryProcessor);

sk_sp<GrGeometryProcessor> EllipseGeometryProcessor::TestCreate(GrProcessorTestData* d) {
    return sk_sp<GrGeometryProcessor>(
        new EllipseGeometryProcessor(d->fRandom->nextBool(), GrTest::TestMatrix(d->fRandom)));
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

enum class DIEllipseStyle { kStroke = 0, kHairline, kFill };

class DIEllipseGeometryProcessor : public GrGeometryProcessor {
public:
    DIEllipseGeometryProcessor(const SkMatrix& viewMatrix, DIEllipseStyle style)
        : fViewMatrix(viewMatrix) {
        this->initClassID<DIEllipseGeometryProcessor>();
        fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                       kHigh_GrSLPrecision));
        fInColor = &this->addVertexAttrib(Attribute("inColor", kVec4ub_GrVertexAttribType));
        fInEllipseOffsets0 = &this->addVertexAttrib(Attribute("inEllipseOffsets0",
                                                              kVec2f_GrVertexAttribType));
        fInEllipseOffsets1 = &this->addVertexAttrib(Attribute("inEllipseOffsets1",
                                                              kVec2f_GrVertexAttribType));
        fStyle = style;
    }


    virtual ~DIEllipseGeometryProcessor() {}

    const char* name() const override { return "DIEllipseEdge"; }

    const Attribute* inPosition() const { return fInPosition; }
    const Attribute* inColor() const { return fInColor; }
    const Attribute* inEllipseOffsets0() const { return fInEllipseOffsets0; }
    const Attribute* inEllipseOffsets1() const { return fInEllipseOffsets1; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }

    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor()
            : fViewMatrix(SkMatrix::InvalidMatrix()) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const DIEllipseGeometryProcessor& diegp = args.fGP.cast<DIEllipseGeometryProcessor>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(diegp);

            GrGLSLVertToFrag offsets0(kVec2f_GrSLType);
            varyingHandler->addVarying("EllipseOffsets0", &offsets0);
            vertBuilder->codeAppendf("%s = %s;", offsets0.vsOut(),
                                     diegp.inEllipseOffsets0()->fName);

            GrGLSLVertToFrag offsets1(kVec2f_GrSLType);
            varyingHandler->addVarying("EllipseOffsets1", &offsets1);
            vertBuilder->codeAppendf("%s = %s;", offsets1.vsOut(),
                                     diegp.inEllipseOffsets1()->fName);

            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
            varyingHandler->addPassThroughAttribute(diegp.inColor(), args.fOutputColor);

            // Setup position
            this->setupPosition(vertBuilder,
                                uniformHandler,
                                gpArgs,
                                diegp.inPosition()->fName,
                                diegp.viewMatrix(),
                                &fViewMatrixUniform);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 gpArgs->fPositionVar,
                                 diegp.inPosition()->fName,
                                 args.fTransformsIn,
                                 args.fTransformsOut);

            SkAssertResult(fragBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            // for outer curve
            fragBuilder->codeAppendf("vec2 scaledOffset = %s.xy;", offsets0.fsIn());
            fragBuilder->codeAppend("float test = dot(scaledOffset, scaledOffset) - 1.0;");
            fragBuilder->codeAppendf("vec2 duvdx = dFdx(%s);", offsets0.fsIn());
            fragBuilder->codeAppendf("vec2 duvdy = dFdy(%s);", offsets0.fsIn());
            fragBuilder->codeAppendf("vec2 grad = vec2(2.0*%s.x*duvdx.x + 2.0*%s.y*duvdx.y,"
                                     "                 2.0*%s.x*duvdy.x + 2.0*%s.y*duvdy.y);",
                                     offsets0.fsIn(), offsets0.fsIn(), offsets0.fsIn(),
                                     offsets0.fsIn());

            fragBuilder->codeAppend("float grad_dot = dot(grad, grad);");
            // avoid calling inversesqrt on zero.
            fragBuilder->codeAppend("grad_dot = max(grad_dot, 1.0e-4);");
            fragBuilder->codeAppend("float invlen = inversesqrt(grad_dot);");
            if (DIEllipseStyle::kHairline == diegp.fStyle) {
                // can probably do this with one step
                fragBuilder->codeAppend("float edgeAlpha = clamp(1.0-test*invlen, 0.0, 1.0);");
                fragBuilder->codeAppend("edgeAlpha *= clamp(1.0+test*invlen, 0.0, 1.0);");
            } else {
                fragBuilder->codeAppend("float edgeAlpha = clamp(0.5-test*invlen, 0.0, 1.0);");
            }

            // for inner curve
            if (DIEllipseStyle::kStroke == diegp.fStyle) {
                fragBuilder->codeAppendf("scaledOffset = %s.xy;", offsets1.fsIn());
                fragBuilder->codeAppend("test = dot(scaledOffset, scaledOffset) - 1.0;");
                fragBuilder->codeAppendf("duvdx = dFdx(%s);", offsets1.fsIn());
                fragBuilder->codeAppendf("duvdy = dFdy(%s);", offsets1.fsIn());
                fragBuilder->codeAppendf("grad = vec2(2.0*%s.x*duvdx.x + 2.0*%s.y*duvdx.y,"
                                         "            2.0*%s.x*duvdy.x + 2.0*%s.y*duvdy.y);",
                                         offsets1.fsIn(), offsets1.fsIn(), offsets1.fsIn(),
                                         offsets1.fsIn());
                fragBuilder->codeAppend("invlen = inversesqrt(dot(grad, grad));");
                fragBuilder->codeAppend("edgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);");
            }

            fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrGLSLCaps&,
                           GrProcessorKeyBuilder* b) {
            const DIEllipseGeometryProcessor& diegp = gp.cast<DIEllipseGeometryProcessor>();
            uint16_t key = static_cast<uint16_t>(diegp.fStyle);
            key |= ComputePosKey(diegp.viewMatrix()) << 10;
            b->add32(key);
        }

        void setData(const GrGLSLProgramDataManager& pdman,
                     const GrPrimitiveProcessor& gp) override {
            const DIEllipseGeometryProcessor& diegp = gp.cast<DIEllipseGeometryProcessor>();

            if (!diegp.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(diegp.viewMatrix())) {
                fViewMatrix = diegp.viewMatrix();
                float viewMatrix[3 * 3];
                GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
                pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
            }
        }

    private:
        SkMatrix fViewMatrix;
        UniformHandle fViewMatrixUniform;

        typedef GrGLSLGeometryProcessor INHERITED;
    };

    void getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrGLSLCaps&) const override {
        return new GLSLProcessor();
    }

private:
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInEllipseOffsets0;
    const Attribute* fInEllipseOffsets1;
    SkMatrix         fViewMatrix;
    DIEllipseStyle   fStyle;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST;

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DIEllipseGeometryProcessor);

sk_sp<GrGeometryProcessor> DIEllipseGeometryProcessor::TestCreate(GrProcessorTestData* d) {
    return sk_sp<GrGeometryProcessor>(
        new DIEllipseGeometryProcessor(GrTest::TestMatrix(d->fRandom),
                                       (DIEllipseStyle)(d->fRandom->nextRangeU(0,2))));
}

///////////////////////////////////////////////////////////////////////////////

GrDrawBatch* GrOvalRenderer::CreateOvalBatch(GrColor color,
                                             const SkMatrix& viewMatrix,
                                             const SkRect& oval,
                                             const SkStrokeRec& stroke,
                                             GrShaderCaps* shaderCaps) {
    // we can draw circles
    if (SkScalarNearlyEqual(oval.width(), oval.height()) && circle_stays_circle(viewMatrix)) {
        return CreateCircleBatch(color, viewMatrix, oval, stroke);
    }

    // if we have shader derivative support, render as device-independent
    if (shaderCaps->shaderDerivativeSupport()) {
        return CreateDIEllipseBatch(color, viewMatrix, oval, stroke);
    }

    // otherwise axis-aligned ellipses only
    if (viewMatrix.rectStaysRect()) {
        return CreateEllipseBatch(color, viewMatrix, oval, stroke);
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

class CircleBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkRect fDevBounds;
        SkScalar fInnerRadius;
        SkScalar fOuterRadius;
        GrColor fColor;
    };

    CircleBatch(const Geometry& geometry, const SkMatrix& viewMatrix, bool stroked)
        : INHERITED(ClassID())
        , fStroked(stroked)
        , fViewMatrixIfUsingLocalCoords(viewMatrix) {
        fGeoData.push_back(geometry);
        this->setBounds(geometry.fDevBounds);
    }
    const char* name() const override { return "CircleBatch"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fGeoData.count(); ++i) {
            string.appendf("Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f],"
                           "InnerRad: %.2f, OuterRad: %.2f\n",
                           fGeoData[i].fColor,
                           fGeoData[i].fDevBounds.fLeft, fGeoData[i].fDevBounds.fTop,
                           fGeoData[i].fDevBounds.fRight, fGeoData[i].fDevBounds.fBottom,
                           fGeoData[i].fInnerRadius,
                           fGeoData[i].fOuterRadius);
        }
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any overrides that affect our GP.
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        if (!overrides.readsLocalCoords()) {
            fViewMatrixIfUsingLocalCoords.reset();
        }
    }

    void onPrepareDraws(Target* target) const override {
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(new CircleGeometryProcessor(fStroked, localMatrix));

        int instanceCount = fGeoData.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(CircleVertex));
        QuadHelper helper;
        CircleVertex* verts = reinterpret_cast<CircleVertex*>(helper.init(target, vertexStride,
                                                                          instanceCount));
        if (!verts) {
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& geom = fGeoData[i];

            GrColor color = geom.fColor;
            SkScalar innerRadius = geom.fInnerRadius;
            SkScalar outerRadius = geom.fOuterRadius;

            const SkRect& bounds = geom.fDevBounds;

            // The inner radius in the vertex data must be specified in normalized space.
            innerRadius = innerRadius / outerRadius;
            verts[0].fPos = SkPoint::Make(bounds.fLeft,  bounds.fTop);
            verts[0].fColor = color;
            verts[0].fOffset = SkPoint::Make(-1, -1);
            verts[0].fOuterRadius = outerRadius;
            verts[0].fInnerRadius = innerRadius;

            verts[1].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
            verts[1].fColor = color;
            verts[1].fOffset = SkPoint::Make(-1, 1);
            verts[1].fOuterRadius = outerRadius;
            verts[1].fInnerRadius = innerRadius;

            verts[2].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
            verts[2].fColor = color;
            verts[2].fOffset = SkPoint::Make(1, 1);
            verts[2].fOuterRadius = outerRadius;
            verts[2].fInnerRadius = innerRadius;

            verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
            verts[3].fColor = color;
            verts[3].fOffset = SkPoint::Make(1, -1);
            verts[3].fOuterRadius = outerRadius;
            verts[3].fInnerRadius = innerRadius;

            verts += kVerticesPerQuad;
        }
        helper.recordDraw(target, gp);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        CircleBatch* that = t->cast<CircleBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (this->fStroked != that->fStroked) {
            return false;
        }

        if (!fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(that->bounds());
        return true;
    }

    bool                         fStroked;
    SkMatrix                     fViewMatrixIfUsingLocalCoords;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

static GrDrawBatch* create_circle_batch(GrColor color,
                                        const SkMatrix& viewMatrix,
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

    CircleBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fInnerRadius = innerRadius;
    geometry.fOuterRadius = outerRadius;
    geometry.fDevBounds = SkRect::MakeLTRB(center.fX - outerRadius, center.fY - outerRadius,
                                           center.fX + outerRadius, center.fY + outerRadius);

    return new CircleBatch(geometry, viewMatrix, isStrokeOnly && innerRadius > 0);
}

GrDrawBatch* GrOvalRenderer::CreateCircleBatch(GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const SkRect& circle,
                                               const SkStrokeRec& stroke) {
    return create_circle_batch(color, viewMatrix, circle, stroke);
}

///////////////////////////////////////////////////////////////////////////////

class EllipseBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkRect fDevBounds;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        GrColor fColor;
    };

    EllipseBatch(const Geometry& geometry, const SkMatrix& viewMatrix, bool stroked)
        : INHERITED(ClassID())
        , fStroked(stroked)
        , fViewMatrixIfUsingLocalCoords(viewMatrix) {
        fGeoData.push_back(geometry);
        this->setBounds(geometry.fDevBounds);
    }

    const char* name() const override { return "EllipseBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any overrides that affect our GP.
        if (!overrides.readsCoverage()) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        }
        if (!overrides.readsLocalCoords()) {
            fViewMatrixIfUsingLocalCoords.reset();
        }
    }

    void onPrepareDraws(Target* target) const override {
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(new EllipseGeometryProcessor(fStroked, localMatrix));

        int instanceCount = fGeoData.count();
        QuadHelper helper;
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(EllipseVertex));
        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(
            helper.init(target, vertexStride, instanceCount));
        if (!verts) {
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& geom = fGeoData[i];

            GrColor color = geom.fColor;
            SkScalar xRadius = geom.fXRadius;
            SkScalar yRadius = geom.fYRadius;

            // Compute the reciprocals of the radii here to save time in the shader
            SkScalar xRadRecip = SkScalarInvert(xRadius);
            SkScalar yRadRecip = SkScalarInvert(yRadius);
            SkScalar xInnerRadRecip = SkScalarInvert(geom.fInnerXRadius);
            SkScalar yInnerRadRecip = SkScalarInvert(geom.fInnerYRadius);

            const SkRect& bounds = geom.fDevBounds;

            // The inner radius in the vertex data must be specified in normalized space.
            verts[0].fPos = SkPoint::Make(bounds.fLeft,  bounds.fTop);
            verts[0].fColor = color;
            verts[0].fOffset = SkPoint::Make(-xRadius, -yRadius);
            verts[0].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts[0].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

            verts[1].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
            verts[1].fColor = color;
            verts[1].fOffset = SkPoint::Make(-xRadius, yRadius);
            verts[1].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts[1].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

            verts[2].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
            verts[2].fColor = color;
            verts[2].fOffset = SkPoint::Make(xRadius, yRadius);
            verts[2].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts[2].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

            verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
            verts[3].fColor = color;
            verts[3].fOffset = SkPoint::Make(xRadius, -yRadius);
            verts[3].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts[3].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

            verts += kVerticesPerQuad;
        }
        helper.recordDraw(target, gp);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        EllipseBatch* that = t->cast<EllipseBatch>();

        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (fStroked != that->fStroked) {
            return false;
        }

        if (!fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(that->bounds());
        return true;
    }


    bool                         fStroked;
    SkMatrix                     fViewMatrixIfUsingLocalCoords;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

static GrDrawBatch* create_ellipse_batch(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& ellipse,
                                         const SkStrokeRec& stroke) {
    SkASSERT(viewMatrix.rectStaysRect());

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
            return nullptr;
        }

        // we don't handle it if curvature of the stroke is less than curvature of the ellipse
        if (scaledStroke.fX*(yRadius*yRadius) < (scaledStroke.fY*scaledStroke.fY)*xRadius ||
            scaledStroke.fY*(xRadius*xRadius) < (scaledStroke.fX*scaledStroke.fX)*yRadius) {
            return nullptr;
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

    EllipseBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fXRadius = xRadius;
    geometry.fYRadius = yRadius;
    geometry.fInnerXRadius = innerXRadius;
    geometry.fInnerYRadius = innerYRadius;
    geometry.fDevBounds = SkRect::MakeLTRB(center.fX - xRadius, center.fY - yRadius,
                                           center.fX + xRadius, center.fY + yRadius);

    return new EllipseBatch(geometry, viewMatrix,
                            isStrokeOnly && innerXRadius > 0 && innerYRadius > 0);
}

GrDrawBatch* GrOvalRenderer::CreateEllipseBatch(GrColor color,
                                                const SkMatrix& viewMatrix,
                                                const SkRect& ellipse,
                                                const SkStrokeRec& stroke) {
    return create_ellipse_batch(color, viewMatrix, ellipse, stroke);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

class DIEllipseBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkMatrix fViewMatrix;
        SkRect fBounds;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        SkScalar fGeoDx;
        SkScalar fGeoDy;
        GrColor fColor;
        DIEllipseStyle fStyle;
    };

    static GrDrawBatch* Create(const Geometry& geometry, const SkRect& bounds) {
        return new DIEllipseBatch(geometry, bounds);
    }

    const char* name() const override { return "DIEllipseBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:

    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any overrides that affect our GP.
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        fUsesLocalCoords = overrides.readsLocalCoords();
    }

    void onPrepareDraws(Target* target) const override {
        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(new DIEllipseGeometryProcessor(this->viewMatrix(),
                                                                            this->style()));

        int instanceCount = fGeoData.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(DIEllipseVertex));
        QuadHelper helper;
        DIEllipseVertex* verts = reinterpret_cast<DIEllipseVertex*>(
            helper.init(target, vertexStride, instanceCount));
        if (!verts) {
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& geom = fGeoData[i];

            GrColor color = geom.fColor;
            SkScalar xRadius = geom.fXRadius;
            SkScalar yRadius = geom.fYRadius;

            const SkRect& bounds = geom.fBounds;

            // This adjusts the "radius" to include the half-pixel border
            SkScalar offsetDx = geom.fGeoDx / xRadius;
            SkScalar offsetDy = geom.fGeoDy / yRadius;

            SkScalar innerRatioX = xRadius / geom.fInnerXRadius;
            SkScalar innerRatioY = yRadius / geom.fInnerYRadius;

            verts[0].fPos = SkPoint::Make(bounds.fLeft, bounds.fTop);
            verts[0].fColor = color;
            verts[0].fOuterOffset = SkPoint::Make(-1.0f - offsetDx, -1.0f - offsetDy);
            verts[0].fInnerOffset = SkPoint::Make(-innerRatioX - offsetDx, -innerRatioY - offsetDy);

            verts[1].fPos = SkPoint::Make(bounds.fLeft,  bounds.fBottom);
            verts[1].fColor = color;
            verts[1].fOuterOffset = SkPoint::Make(-1.0f - offsetDx, 1.0f + offsetDy);
            verts[1].fInnerOffset = SkPoint::Make(-innerRatioX - offsetDx, innerRatioY + offsetDy);

            verts[2].fPos = SkPoint::Make(bounds.fRight, bounds.fBottom);
            verts[2].fColor = color;
            verts[2].fOuterOffset = SkPoint::Make(1.0f + offsetDx, 1.0f + offsetDy);
            verts[2].fInnerOffset = SkPoint::Make(innerRatioX + offsetDx, innerRatioY + offsetDy);

            verts[3].fPos = SkPoint::Make(bounds.fRight, bounds.fTop);
            verts[3].fColor = color;
            verts[3].fOuterOffset = SkPoint::Make(1.0f + offsetDx, -1.0f - offsetDy);
            verts[3].fInnerOffset = SkPoint::Make(innerRatioX + offsetDx, -innerRatioY - offsetDy);

            verts += kVerticesPerQuad;
        }
        helper.recordDraw(target, gp);
    }

    DIEllipseBatch(const Geometry& geometry, const SkRect& bounds) : INHERITED(ClassID()) {
        fGeoData.push_back(geometry);

        this->setBounds(bounds);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        DIEllipseBatch* that = t->cast<DIEllipseBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (this->style() != that->style()) {
            return false;
        }

        // TODO rewrite to allow positioning on CPU
        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(that->bounds());
        return true;
    }

    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    DIEllipseStyle style() const { return fGeoData[0].fStyle; }

    bool                         fUsesLocalCoords;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

static GrDrawBatch* create_diellipse_batch(GrColor color,
                                           const SkMatrix& viewMatrix,
                                           const SkRect& ellipse,
                                           const SkStrokeRec& stroke) {
    SkPoint center = SkPoint::Make(ellipse.centerX(), ellipse.centerY());
    SkScalar xRadius = SkScalarHalf(ellipse.width());
    SkScalar yRadius = SkScalarHalf(ellipse.height());

    SkStrokeRec::Style style = stroke.getStyle();
    DIEllipseStyle dieStyle = (SkStrokeRec::kStroke_Style == style) ?
                                DIEllipseStyle::kStroke :
                                (SkStrokeRec::kHairline_Style == style) ?
                                        DIEllipseStyle::kHairline : DIEllipseStyle::kFill;

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
            return nullptr;
        }

        // we don't handle it if curvature of the stroke is less than curvature of the ellipse
        if (strokeWidth*(yRadius*yRadius) < (strokeWidth*strokeWidth)*xRadius ||
            strokeWidth*(xRadius*xRadius) < (strokeWidth*strokeWidth)*yRadius) {
            return nullptr;
        }

        // set inner radius (if needed)
        if (SkStrokeRec::kStroke_Style == style) {
            innerXRadius = xRadius - strokeWidth;
            innerYRadius = yRadius - strokeWidth;
        }

        xRadius += strokeWidth;
        yRadius += strokeWidth;
    }
    if (DIEllipseStyle::kStroke == dieStyle) {
        dieStyle = (innerXRadius > 0 && innerYRadius > 0) ? DIEllipseStyle ::kStroke :
                                                            DIEllipseStyle ::kFill;
    }

    // This expands the outer rect so that after CTM we end up with a half-pixel border
    SkScalar a = viewMatrix[SkMatrix::kMScaleX];
    SkScalar b = viewMatrix[SkMatrix::kMSkewX];
    SkScalar c = viewMatrix[SkMatrix::kMSkewY];
    SkScalar d = viewMatrix[SkMatrix::kMScaleY];
    SkScalar geoDx = SK_ScalarHalf / SkScalarSqrt(a*a + c*c);
    SkScalar geoDy = SK_ScalarHalf / SkScalarSqrt(b*b + d*d);

    DIEllipseBatch::Geometry geometry;
    geometry.fViewMatrix = viewMatrix;
    geometry.fColor = color;
    geometry.fXRadius = xRadius;
    geometry.fYRadius = yRadius;
    geometry.fInnerXRadius = innerXRadius;
    geometry.fInnerYRadius = innerYRadius;
    geometry.fGeoDx = geoDx;
    geometry.fGeoDy = geoDy;
    geometry.fStyle = dieStyle;
    geometry.fBounds = SkRect::MakeLTRB(center.fX - xRadius - geoDx, center.fY - yRadius - geoDy,
                                        center.fX + xRadius + geoDx, center.fY + yRadius + geoDy);

    SkRect devBounds = geometry.fBounds;
    viewMatrix.mapRect(&devBounds);
    return DIEllipseBatch::Create(geometry, devBounds);
}

GrDrawBatch* GrOvalRenderer::CreateDIEllipseBatch(GrColor color,
                                                  const SkMatrix& viewMatrix,
                                                  const SkRect& ellipse,
                                                  const SkStrokeRec& stroke) {
    return create_diellipse_batch(color, viewMatrix, ellipse, stroke);
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

GR_DECLARE_STATIC_UNIQUE_KEY(gStrokeRRectOnlyIndexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gRRectOnlyIndexBufferKey);
static const GrBuffer* ref_rrect_index_buffer(bool strokeOnly,
                                              GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gStrokeRRectOnlyIndexBufferKey);
    GR_DEFINE_STATIC_UNIQUE_KEY(gRRectOnlyIndexBufferKey);
    if (strokeOnly) {
        return resourceProvider->findOrCreateInstancedIndexBuffer(
            gRRectIndices, kIndicesPerStrokeRRect, kNumRRectsInIndexBuffer, kVertsPerRRect,
            gStrokeRRectOnlyIndexBufferKey);
    } else {
        return resourceProvider->findOrCreateInstancedIndexBuffer(
            gRRectIndices, kIndicesPerRRect, kNumRRectsInIndexBuffer, kVertsPerRRect,
            gRRectOnlyIndexBufferKey);

    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class RRectCircleRendererBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkRect fDevBounds;
        SkScalar fInnerRadius;
        SkScalar fOuterRadius;
        GrColor  fColor;
    };

    RRectCircleRendererBatch(const Geometry& geometry, const SkMatrix& viewMatrix, bool stroked)
        : INHERITED(ClassID())
        , fStroked(stroked)
        , fViewMatrixIfUsingLocalCoords(viewMatrix) {
        fGeoData.push_back(geometry);

        this->setBounds(geometry.fDevBounds);
    }

    const char* name() const override { return "RRectCircleBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any overrides that affect our GP.
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        if (!overrides.readsLocalCoords()) {
            fViewMatrixIfUsingLocalCoords.reset();
        }
    }

    void onPrepareDraws(Target* target) const override {
        // Invert the view matrix as a local matrix (if any other processors require coords).
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(new CircleGeometryProcessor(fStroked, localMatrix));

        int instanceCount = fGeoData.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(CircleVertex));

        // drop out the middle quad if we're stroked
        int indicesPerInstance = fStroked ? kIndicesPerStrokeRRect : kIndicesPerRRect;
        SkAutoTUnref<const GrBuffer> indexBuffer(
            ref_rrect_index_buffer(fStroked, target->resourceProvider()));

        InstancedHelper helper;
        CircleVertex* verts = reinterpret_cast<CircleVertex*>(helper.init(target,
            kTriangles_GrPrimitiveType, vertexStride, indexBuffer, kVertsPerRRect,
            indicesPerInstance, instanceCount));
        if (!verts || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            GrColor color = args.fColor;
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
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(-1, yOuterRadii[i]);
                verts->fOuterRadius = outerRadius;
                verts->fInnerRadius = innerRadius;
                verts++;

                verts->fPos = SkPoint::Make(bounds.fLeft + outerRadius, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(0, yOuterRadii[i]);
                verts->fOuterRadius = outerRadius;
                verts->fInnerRadius = innerRadius;
                verts++;

                verts->fPos = SkPoint::Make(bounds.fRight - outerRadius, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(0, yOuterRadii[i]);
                verts->fOuterRadius = outerRadius;
                verts->fInnerRadius = innerRadius;
                verts++;

                verts->fPos = SkPoint::Make(bounds.fRight, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(1, yOuterRadii[i]);
                verts->fOuterRadius = outerRadius;
                verts->fInnerRadius = innerRadius;
                verts++;
            }
        }

        helper.recordDraw(target, gp);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        RRectCircleRendererBatch* that = t->cast<RRectCircleRendererBatch>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (fStroked != that->fStroked) {
            return false;
        }

        if (!fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(that->bounds());
        return true;
    }

    bool                         fStroked;
    SkMatrix                     fViewMatrixIfUsingLocalCoords;
    SkSTArray<1, Geometry, true> fGeoData;

    typedef GrVertexBatch INHERITED;
};

class RRectEllipseRendererBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    struct Geometry {
        SkRect fDevBounds;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        GrColor fColor;
    };

    RRectEllipseRendererBatch(const Geometry& geometry, const SkMatrix& viewMatrix, bool stroked)
        : INHERITED(ClassID())
        , fStroked(stroked)
        , fViewMatrixIfUsingLocalCoords(viewMatrix) {
        fGeoData.push_back(geometry);
        this->setBounds(geometry.fDevBounds);
    }

    const char* name() const override { return "RRectEllipseRendererBatch"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle overrides that affect our GP.
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        if (!overrides.readsLocalCoords()) {
            fViewMatrixIfUsingLocalCoords.reset();
        }
    }

    void onPrepareDraws(Target* target) const override {
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        SkAutoTUnref<GrGeometryProcessor> gp(new EllipseGeometryProcessor(fStroked, localMatrix));

        int instanceCount = fGeoData.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(EllipseVertex));

        // drop out the middle quad if we're stroked
        int indicesPerInstance = fStroked ? kIndicesPerStrokeRRect : kIndicesPerRRect;
        SkAutoTUnref<const GrBuffer> indexBuffer(
            ref_rrect_index_buffer(fStroked, target->resourceProvider()));

        InstancedHelper helper;
        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(
            helper.init(target, kTriangles_GrPrimitiveType, vertexStride, indexBuffer,
            kVertsPerRRect, indicesPerInstance, instanceCount));
        if (!verts || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            GrColor color = args.fColor;

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
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(xOuterRadius, yOuterOffsets[i]);
                verts->fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
                verts->fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);
                verts++;

                verts->fPos = SkPoint::Make(bounds.fLeft + xOuterRadius, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(SK_ScalarNearlyZero, yOuterOffsets[i]);
                verts->fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
                verts->fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);
                verts++;

                verts->fPos = SkPoint::Make(bounds.fRight - xOuterRadius, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(SK_ScalarNearlyZero, yOuterOffsets[i]);
                verts->fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
                verts->fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);
                verts++;

                verts->fPos = SkPoint::Make(bounds.fRight, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(xOuterRadius, yOuterOffsets[i]);
                verts->fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
                verts->fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);
                verts++;
            }
        }
        helper.recordDraw(target, gp);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        RRectEllipseRendererBatch* that = t->cast<RRectEllipseRendererBatch>();

        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (fStroked != that->fStroked) {
            return false;
        }

        if (!fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(that->bounds());
        return true;
    }

    bool                            fStroked;
    SkMatrix                        fViewMatrixIfUsingLocalCoords;
    SkSTArray<1, Geometry, true>    fGeoData;

    typedef GrVertexBatch INHERITED;
};

static GrDrawBatch* create_rrect_batch(GrColor color,
                                       const SkMatrix& viewMatrix,
                                       const SkRRect& rrect,
                                       const SkStrokeRec& stroke) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(rrect.isSimple());
    SkASSERT(!rrect.isOval());

    // RRect batchs only handle simple, but not too simple, rrects
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
            return nullptr;
        }
    }

    // The way the effect interpolates the offset-to-ellipse/circle-center attribute only works on
    // the interior of the rrect if the radii are >= 0.5. Otherwise, the inner rect of the nine-
    // patch will have fractional coverage. This only matters when the interior is actually filled.
    // We could consider falling back to rect rendering here, since a tiny radius is
    // indistinguishable from a square corner.
    if (!isStrokeOnly && (SK_ScalarHalf > xRadius || SK_ScalarHalf > yRadius)) {
        return nullptr;
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
        geometry.fColor = color;
        geometry.fInnerRadius = innerRadius;
        geometry.fOuterRadius = outerRadius;
        geometry.fDevBounds = bounds;

        return new RRectCircleRendererBatch(geometry, viewMatrix, isStrokeOnly);
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
                return nullptr;
            }

            // we don't handle it if curvature of the stroke is less than curvature of the ellipse
            if (scaledStroke.fX*(yRadius*yRadius) < (scaledStroke.fY*scaledStroke.fY)*xRadius ||
                scaledStroke.fY*(xRadius*xRadius) < (scaledStroke.fX*scaledStroke.fX)*yRadius) {
                return nullptr;
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
        geometry.fColor = color;
        geometry.fXRadius = xRadius;
        geometry.fYRadius = yRadius;
        geometry.fInnerXRadius = innerXRadius;
        geometry.fInnerYRadius = innerYRadius;
        geometry.fDevBounds = bounds;

        return new RRectEllipseRendererBatch(geometry, viewMatrix, isStrokeOnly);
    }
}

GrDrawBatch* GrOvalRenderer::CreateRRectBatch(GrColor color,
                                              const SkMatrix& viewMatrix,
                                              const SkRRect& rrect,
                                              const SkStrokeRec& stroke,
                                              GrShaderCaps* shaderCaps) {
    if (rrect.isOval()) {
        return CreateOvalBatch(color, viewMatrix, rrect.getBounds(), stroke, shaderCaps);
    }

    if (!viewMatrix.rectStaysRect() || !rrect.isSimple()) {
        return nullptr;
    }

    return create_rrect_batch(color, viewMatrix, rrect, stroke);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(CircleBatch) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrColor color = GrRandomColor(random);
    SkRect circle = GrTest::TestSquare(random);
    return create_circle_batch(color, viewMatrix, circle, GrTest::TestStrokeRec(random));
}

DRAW_BATCH_TEST_DEFINE(EllipseBatch) {
    SkMatrix viewMatrix = GrTest::TestMatrixRectStaysRect(random);
    GrColor color = GrRandomColor(random);
    SkRect ellipse = GrTest::TestSquare(random);
    return create_ellipse_batch(color, viewMatrix, ellipse, GrTest::TestStrokeRec(random));
}

DRAW_BATCH_TEST_DEFINE(DIEllipseBatch) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrColor color = GrRandomColor(random);
    SkRect ellipse = GrTest::TestSquare(random);
    return create_diellipse_batch(color, viewMatrix, ellipse, GrTest::TestStrokeRec(random));
}

DRAW_BATCH_TEST_DEFINE(RRectBatch) {
    SkMatrix viewMatrix = GrTest::TestMatrixRectStaysRect(random);
    GrColor color = GrRandomColor(random);
    const SkRRect& rrect = GrTest::TestRRectSimple(random);
    return create_rrect_batch(color, viewMatrix, rrect, GrTest::TestStrokeRec(random));
}

#endif
