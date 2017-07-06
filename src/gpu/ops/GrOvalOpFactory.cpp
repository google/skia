/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOvalOpFactory.h"
#include "GrDrawOpTest.h"
#include "GrGeometryProcessor.h"
#include "GrOpFlushState.h"
#include "GrProcessor.h"
#include "GrResourceProvider.h"
#include "GrShaderCaps.h"
#include "GrStyle.h"
#include "SkRRect.h"
#include "SkStrokeRec.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLUtil.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"
#include "ops/GrMeshDrawOp.h"
#include "ops/GrSimpleMeshDrawOpHelper.h"

namespace {

struct EllipseVertex {
    SkPoint fPos;
    GrColor fColor;
    SkPoint fOffset;
    SkPoint fOuterRadii;
    SkPoint fInnerRadii;
};

struct DIEllipseVertex {
    SkPoint fPos;
    GrColor fColor;
    SkPoint fOuterOffset;
    SkPoint fInnerOffset;
};

static inline bool circle_stays_circle(const SkMatrix& m) { return m.isSimilarity(); }
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
 * Additional clip planes are supported for rendering circular arcs. The additional planes are
 * either intersected or unioned together. Up to three planes are supported (an initial plane,
 * a plane intersected with the initial plane, and a plane unioned with the first two). Only two
 * are useful for any given arc, but having all three in one instance allows combining different
 * types of arcs.
 */

class CircleGeometryProcessor : public GrGeometryProcessor {
public:
    CircleGeometryProcessor(bool stroke, bool clipPlane, bool isectPlane, bool unionPlane,
                            const SkMatrix& localMatrix)
            : fLocalMatrix(localMatrix) {
        this->initClassID<CircleGeometryProcessor>();
        fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                             kHigh_GrSLPrecision);
        fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
        fInCircleEdge = &this->addVertexAttrib("inCircleEdge", kVec4f_GrVertexAttribType,
                                               kHigh_GrSLPrecision);
        if (clipPlane) {
            fInClipPlane = &this->addVertexAttrib("inClipPlane", kVec3f_GrVertexAttribType);
        } else {
            fInClipPlane = nullptr;
        }
        if (isectPlane) {
            fInIsectPlane = &this->addVertexAttrib("inIsectPlane", kVec3f_GrVertexAttribType);
        } else {
            fInIsectPlane = nullptr;
        }
        if (unionPlane) {
            fInUnionPlane = &this->addVertexAttrib("inUnionPlane", kVec3f_GrVertexAttribType);
        } else {
            fInUnionPlane = nullptr;
        }
        fStroke = stroke;
    }

    ~CircleGeometryProcessor() override {}

    const char* name() const override { return "CircleEdge"; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor();
    }

private:
    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor() {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const CircleGeometryProcessor& cgp = args.fGP.cast<CircleGeometryProcessor>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;

            // emit attributes
            varyingHandler->emitAttributes(cgp);
            fragBuilder->codeAppend("highp vec4 circleEdge;");
            varyingHandler->addPassThroughAttribute(cgp.fInCircleEdge, "circleEdge",
                                                    kHigh_GrSLPrecision);
            if (cgp.fInClipPlane) {
                fragBuilder->codeAppend("vec3 clipPlane;");
                varyingHandler->addPassThroughAttribute(cgp.fInClipPlane, "clipPlane");
            }
            if (cgp.fInIsectPlane) {
                SkASSERT(cgp.fInClipPlane);
                fragBuilder->codeAppend("vec3 isectPlane;");
                varyingHandler->addPassThroughAttribute(cgp.fInIsectPlane, "isectPlane");
            }
            if (cgp.fInUnionPlane) {
                SkASSERT(cgp.fInClipPlane);
                fragBuilder->codeAppend("vec3 unionPlane;");
                varyingHandler->addPassThroughAttribute(cgp.fInUnionPlane, "unionPlane");
            }

            // setup pass through color
            varyingHandler->addPassThroughAttribute(cgp.fInColor, args.fOutputColor);

            // Setup position
            this->setupPosition(vertBuilder, gpArgs, cgp.fInPosition->fName);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 gpArgs->fPositionVar,
                                 cgp.fInPosition->fName,
                                 cgp.fLocalMatrix,
                                 args.fFPCoordTransformHandler);

            fragBuilder->codeAppend("highp float d = length(circleEdge.xy);");
            fragBuilder->codeAppend("float distanceToOuterEdge = circleEdge.z * (1.0 - d);");
            fragBuilder->codeAppend("float edgeAlpha = clamp(distanceToOuterEdge, 0.0, 1.0);");
            if (cgp.fStroke) {
                fragBuilder->codeAppend(
                        "float distanceToInnerEdge = circleEdge.z * (d - circleEdge.w);");
                fragBuilder->codeAppend("float innerAlpha = clamp(distanceToInnerEdge, 0.0, 1.0);");
                fragBuilder->codeAppend("edgeAlpha *= innerAlpha;");
            }

            if (cgp.fInClipPlane) {
                fragBuilder->codeAppend(
                        "float clip = clamp(circleEdge.z * dot(circleEdge.xy, clipPlane.xy) + "
                        "clipPlane.z, 0.0, 1.0);");
                if (cgp.fInIsectPlane) {
                    fragBuilder->codeAppend(
                            "clip *= clamp(circleEdge.z * dot(circleEdge.xy, isectPlane.xy) + "
                            "isectPlane.z, 0.0, 1.0);");
                }
                if (cgp.fInUnionPlane) {
                    fragBuilder->codeAppend(
                            "clip += (1.0 - clip)*clamp(circleEdge.z * dot(circleEdge.xy, "
                            "unionPlane.xy) + unionPlane.z, 0.0, 1.0);");
                }
                fragBuilder->codeAppend("edgeAlpha *= clip;");
            }
            fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrShaderCaps&,
                           GrProcessorKeyBuilder* b) {
            const CircleGeometryProcessor& cgp = gp.cast<CircleGeometryProcessor>();
            uint16_t key;
            key = cgp.fStroke ? 0x01 : 0x0;
            key |= cgp.fLocalMatrix.hasPerspective() ? 0x02 : 0x0;
            key |= cgp.fInClipPlane ? 0x04 : 0x0;
            key |= cgp.fInIsectPlane ? 0x08 : 0x0;
            key |= cgp.fInUnionPlane ? 0x10 : 0x0;
            b->add32(key);
        }

        void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                     FPCoordTransformIter&& transformIter) override {
            this->setTransformDataHelper(primProc.cast<CircleGeometryProcessor>().fLocalMatrix,
                                         pdman, &transformIter);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;
    };

    SkMatrix fLocalMatrix;
    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInCircleEdge;
    const Attribute* fInClipPlane;
    const Attribute* fInIsectPlane;
    const Attribute* fInUnionPlane;
    bool fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(CircleGeometryProcessor);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> CircleGeometryProcessor::TestCreate(GrProcessorTestData* d) {
    return sk_sp<GrGeometryProcessor>(new CircleGeometryProcessor(
            d->fRandom->nextBool(), d->fRandom->nextBool(), d->fRandom->nextBool(),
            d->fRandom->nextBool(), GrTest::TestMatrix(d->fRandom)));
}
#endif

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
    EllipseGeometryProcessor(bool stroke, const SkMatrix& localMatrix) : fLocalMatrix(localMatrix) {
        this->initClassID<EllipseGeometryProcessor>();
        fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType);
        fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
        fInEllipseOffset = &this->addVertexAttrib("inEllipseOffset", kVec2f_GrVertexAttribType);
        fInEllipseRadii = &this->addVertexAttrib("inEllipseRadii", kVec4f_GrVertexAttribType);
        fStroke = stroke;
    }

    ~EllipseGeometryProcessor() override {}

    const char* name() const override { return "EllipseEdge"; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor();
    }

private:
    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor() {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const EllipseGeometryProcessor& egp = args.fGP.cast<EllipseGeometryProcessor>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(egp);

            GrGLSLVertToFrag ellipseOffsets(kVec2f_GrSLType);
            varyingHandler->addVarying("EllipseOffsets", &ellipseOffsets);
            vertBuilder->codeAppendf("%s = %s;", ellipseOffsets.vsOut(),
                                     egp.fInEllipseOffset->fName);

            GrGLSLVertToFrag ellipseRadii(kVec4f_GrSLType);
            varyingHandler->addVarying("EllipseRadii", &ellipseRadii);
            vertBuilder->codeAppendf("%s = %s;", ellipseRadii.vsOut(), egp.fInEllipseRadii->fName);

            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
            // setup pass through color
            varyingHandler->addPassThroughAttribute(egp.fInColor, args.fOutputColor);

            // Setup position
            this->setupPosition(vertBuilder, gpArgs, egp.fInPosition->fName);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 gpArgs->fPositionVar,
                                 egp.fInPosition->fName,
                                 egp.fLocalMatrix,
                                 args.fFPCoordTransformHandler);

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
                fragBuilder->codeAppendf("scaledOffset = %s*%s.zw;", ellipseOffsets.fsIn(),
                                         ellipseRadii.fsIn());
                fragBuilder->codeAppend("test = dot(scaledOffset, scaledOffset) - 1.0;");
                fragBuilder->codeAppendf("grad = 2.0*scaledOffset*%s.zw;", ellipseRadii.fsIn());
                fragBuilder->codeAppend("invlen = inversesqrt(dot(grad, grad));");
                fragBuilder->codeAppend("edgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);");
            }

            fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrShaderCaps&,
                           GrProcessorKeyBuilder* b) {
            const EllipseGeometryProcessor& egp = gp.cast<EllipseGeometryProcessor>();
            uint16_t key = egp.fStroke ? 0x1 : 0x0;
            key |= egp.fLocalMatrix.hasPerspective() ? 0x2 : 0x0;
            b->add32(key);
        }

        void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                     FPCoordTransformIter&& transformIter) override {
            const EllipseGeometryProcessor& egp = primProc.cast<EllipseGeometryProcessor>();
            this->setTransformDataHelper(egp.fLocalMatrix, pdman, &transformIter);
        }

    private:
        typedef GrGLSLGeometryProcessor INHERITED;
    };

    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInEllipseOffset;
    const Attribute* fInEllipseRadii;
    SkMatrix fLocalMatrix;
    bool fStroke;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(EllipseGeometryProcessor);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> EllipseGeometryProcessor::TestCreate(GrProcessorTestData* d) {
    return sk_sp<GrGeometryProcessor>(
            new EllipseGeometryProcessor(d->fRandom->nextBool(), GrTest::TestMatrix(d->fRandom)));
}
#endif

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
        fInPosition = &this->addVertexAttrib("inPosition", kVec2f_GrVertexAttribType,
                                             kHigh_GrSLPrecision);
        fInColor = &this->addVertexAttrib("inColor", kVec4ub_GrVertexAttribType);
        fInEllipseOffsets0 = &this->addVertexAttrib("inEllipseOffsets0", kVec2f_GrVertexAttribType);
        fInEllipseOffsets1 = &this->addVertexAttrib("inEllipseOffsets1", kVec2f_GrVertexAttribType);
        fStyle = style;
    }

    ~DIEllipseGeometryProcessor() override {}

    const char* name() const override { return "DIEllipseEdge"; }

    void getGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        GLSLProcessor::GenKey(*this, caps, b);
    }

    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const override {
        return new GLSLProcessor();
    }

private:
    class GLSLProcessor : public GrGLSLGeometryProcessor {
    public:
        GLSLProcessor() : fViewMatrix(SkMatrix::InvalidMatrix()) {}

        void onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) override {
            const DIEllipseGeometryProcessor& diegp = args.fGP.cast<DIEllipseGeometryProcessor>();
            GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
            GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            // emit attributes
            varyingHandler->emitAttributes(diegp);

            GrGLSLVertToFrag offsets0(kVec2f_GrSLType);
            varyingHandler->addVarying("EllipseOffsets0", &offsets0);
            vertBuilder->codeAppendf("%s = %s;", offsets0.vsOut(), diegp.fInEllipseOffsets0->fName);

            GrGLSLVertToFrag offsets1(kVec2f_GrSLType);
            varyingHandler->addVarying("EllipseOffsets1", &offsets1);
            vertBuilder->codeAppendf("%s = %s;", offsets1.vsOut(), diegp.fInEllipseOffsets1->fName);

            GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
            varyingHandler->addPassThroughAttribute(diegp.fInColor, args.fOutputColor);

            // Setup position
            this->setupPosition(vertBuilder,
                                uniformHandler,
                                gpArgs,
                                diegp.fInPosition->fName,
                                diegp.fViewMatrix,
                                &fViewMatrixUniform);

            // emit transforms
            this->emitTransforms(vertBuilder,
                                 varyingHandler,
                                 uniformHandler,
                                 gpArgs->fPositionVar,
                                 diegp.fInPosition->fName,
                                 args.fFPCoordTransformHandler);

            // for outer curve
            fragBuilder->codeAppendf("vec2 scaledOffset = %s.xy;", offsets0.fsIn());
            fragBuilder->codeAppend("float test = dot(scaledOffset, scaledOffset) - 1.0;");
            fragBuilder->codeAppendf("vec2 duvdx = dFdx(%s);", offsets0.fsIn());
            fragBuilder->codeAppendf("vec2 duvdy = dFdy(%s);", offsets0.fsIn());
            fragBuilder->codeAppendf(
                    "vec2 grad = vec2(2.0*%s.x*duvdx.x + 2.0*%s.y*duvdx.y,"
                    "                 2.0*%s.x*duvdy.x + 2.0*%s.y*duvdy.y);",
                    offsets0.fsIn(), offsets0.fsIn(), offsets0.fsIn(), offsets0.fsIn());

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
                fragBuilder->codeAppendf(
                        "grad = vec2(2.0*%s.x*duvdx.x + 2.0*%s.y*duvdx.y,"
                        "            2.0*%s.x*duvdy.x + 2.0*%s.y*duvdy.y);",
                        offsets1.fsIn(), offsets1.fsIn(), offsets1.fsIn(), offsets1.fsIn());
                fragBuilder->codeAppend("invlen = inversesqrt(dot(grad, grad));");
                fragBuilder->codeAppend("edgeAlpha *= clamp(0.5+test*invlen, 0.0, 1.0);");
            }

            fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
        }

        static void GenKey(const GrGeometryProcessor& gp,
                           const GrShaderCaps&,
                           GrProcessorKeyBuilder* b) {
            const DIEllipseGeometryProcessor& diegp = gp.cast<DIEllipseGeometryProcessor>();
            uint16_t key = static_cast<uint16_t>(diegp.fStyle);
            key |= ComputePosKey(diegp.fViewMatrix) << 10;
            b->add32(key);
        }

        void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& gp,
                     FPCoordTransformIter&& transformIter) override {
            const DIEllipseGeometryProcessor& diegp = gp.cast<DIEllipseGeometryProcessor>();

            if (!diegp.fViewMatrix.isIdentity() && !fViewMatrix.cheapEqualTo(diegp.fViewMatrix)) {
                fViewMatrix = diegp.fViewMatrix;
                float viewMatrix[3 * 3];
                GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
                pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
            }
            this->setTransformDataHelper(SkMatrix::I(), pdman, &transformIter);
        }

    private:
        SkMatrix fViewMatrix;
        UniformHandle fViewMatrixUniform;

        typedef GrGLSLGeometryProcessor INHERITED;
    };

    const Attribute* fInPosition;
    const Attribute* fInColor;
    const Attribute* fInEllipseOffsets0;
    const Attribute* fInEllipseOffsets1;
    SkMatrix fViewMatrix;
    DIEllipseStyle fStyle;

    GR_DECLARE_GEOMETRY_PROCESSOR_TEST

    typedef GrGeometryProcessor INHERITED;
};

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(DIEllipseGeometryProcessor);

#if GR_TEST_UTILS
sk_sp<GrGeometryProcessor> DIEllipseGeometryProcessor::TestCreate(GrProcessorTestData* d) {
    return sk_sp<GrGeometryProcessor>(new DIEllipseGeometryProcessor(
            GrTest::TestMatrix(d->fRandom), (DIEllipseStyle)(d->fRandom->nextRangeU(0, 2))));
}
#endif

///////////////////////////////////////////////////////////////////////////////

// We have two possible cases for geometry for a circle:

// In the case of a normal fill, we draw geometry for the circle as an octagon.
static const uint16_t gFillCircleIndices[] = {
        // enter the octagon
        // clang-format off
        0, 1, 8, 1, 2, 8,
        2, 3, 8, 3, 4, 8,
        4, 5, 8, 5, 6, 8,
        6, 7, 8, 7, 0, 8
        // clang-format on
};

// For stroked circles, we use two nested octagons.
static const uint16_t gStrokeCircleIndices[] = {
        // enter the octagon
        // clang-format off
        0, 1,  9, 0, 9,   8,
        1, 2, 10, 1, 10,  9,
        2, 3, 11, 2, 11, 10,
        3, 4, 12, 3, 12, 11,
        4, 5, 13, 4, 13, 12,
        5, 6, 14, 5, 14, 13,
        6, 7, 15, 6, 15, 14,
        7, 0,  8, 7,  8, 15,
        // clang-format on
};


static const int kIndicesPerFillCircle = SK_ARRAY_COUNT(gFillCircleIndices);
static const int kIndicesPerStrokeCircle = SK_ARRAY_COUNT(gStrokeCircleIndices);
static const int kVertsPerStrokeCircle = 16;
static const int kVertsPerFillCircle = 9;

static int circle_type_to_vert_count(bool stroked) {
    return stroked ? kVertsPerStrokeCircle : kVertsPerFillCircle;
}

static int circle_type_to_index_count(bool stroked) {
    return stroked ? kIndicesPerStrokeCircle : kIndicesPerFillCircle;
}

static const uint16_t* circle_type_to_indices(bool stroked) {
    return stroked ? gStrokeCircleIndices : gFillCircleIndices;
}

///////////////////////////////////////////////////////////////////////////////

class CircleOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    /** Optional extra params to render a partial arc rather than a full circle. */
    struct ArcParams {
        SkScalar fStartAngleRadians;
        SkScalar fSweepAngleRadians;
        bool fUseCenter;
    };

    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          SkPoint center, SkScalar radius, const GrStyle& style,
                                          const ArcParams* arcParams = nullptr) {
        SkASSERT(circle_stays_circle(viewMatrix));
        if (style.hasPathEffect()) {
            return nullptr;
        }
        const SkStrokeRec& stroke = style.strokeRec();
        SkStrokeRec::Style recStyle = stroke.getStyle();
        if (arcParams) {
            // Arc support depends on the style.
            switch (recStyle) {
                case SkStrokeRec::kStrokeAndFill_Style:
                    // This produces a strange result that this op doesn't implement.
                    return nullptr;
                case SkStrokeRec::kFill_Style:
                    // This supports all fills.
                    break;
                case SkStrokeRec::kStroke_Style:  // fall through
                case SkStrokeRec::kHairline_Style:
                    // Strokes that don't use the center point are supported with butt cap.
                    if (arcParams->fUseCenter || stroke.getCap() != SkPaint::kButt_Cap) {
                        return nullptr;
                    }
                    break;
            }
        }
        return Helper::FactoryHelper<CircleOp>(std::move(paint), viewMatrix, center, radius, style,
                                               arcParams);
    }

    CircleOp(const Helper::MakeArgs& helperArgs, GrColor color, const SkMatrix& viewMatrix,
             SkPoint center, SkScalar radius, const GrStyle& style, const ArcParams* arcParams)
            : GrMeshDrawOp(ClassID()), fHelper(helperArgs, GrAAType::kCoverage) {
        const SkStrokeRec& stroke = style.strokeRec();
        SkStrokeRec::Style recStyle = stroke.getStyle();

        viewMatrix.mapPoints(&center, 1);
        radius = viewMatrix.mapRadius(radius);
        SkScalar strokeWidth = viewMatrix.mapRadius(stroke.getWidth());

        bool isStrokeOnly =
                SkStrokeRec::kStroke_Style == recStyle || SkStrokeRec::kHairline_Style == recStyle;
        bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == recStyle;

        SkScalar innerRadius = -SK_ScalarHalf;
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

        // The radii are outset for two reasons. First, it allows the shader to simply perform
        // simpler computation because the computed alpha is zero, rather than 50%, at the radius.
        // Second, the outer radius is used to compute the verts of the bounding box that is
        // rendered and the outset ensures the box will cover all partially covered by the circle.
        outerRadius += SK_ScalarHalf;
        innerRadius -= SK_ScalarHalf;
        bool stroked = isStrokeOnly && innerRadius > 0.0f;
        fViewMatrixIfUsingLocalCoords = viewMatrix;

        // This makes every point fully inside the intersection plane.
        static constexpr SkScalar kUnusedIsectPlane[] = {0.f, 0.f, 1.f};
        // This makes every point fully outside the union plane.
        static constexpr SkScalar kUnusedUnionPlane[] = {0.f, 0.f, 0.f};
        SkRect devBounds = SkRect::MakeLTRB(center.fX - outerRadius, center.fY - outerRadius,
                                            center.fX + outerRadius, center.fY + outerRadius);
        if (arcParams) {
            // The shader operates in a space where the circle is translated to be centered at the
            // origin. Here we compute points on the unit circle at the starting and ending angles.
            SkPoint startPoint, stopPoint;
            startPoint.fY = SkScalarSinCos(arcParams->fStartAngleRadians, &startPoint.fX);
            SkScalar endAngle = arcParams->fStartAngleRadians + arcParams->fSweepAngleRadians;
            stopPoint.fY = SkScalarSinCos(endAngle, &stopPoint.fX);

            // Adjust the start and end points based on the view matrix (to handle rotated arcs)
            startPoint = viewMatrix.mapVector(startPoint.fX, startPoint.fY);
            stopPoint = viewMatrix.mapVector(stopPoint.fX, stopPoint.fY);
            startPoint.normalize();
            stopPoint.normalize();

            // If the matrix included scale (on one axis) we need to swap our start and end points
            if ((viewMatrix.getScaleX() < 0) != (viewMatrix.getScaleY() < 0)) {
                SkTSwap(startPoint, stopPoint);
            }

            // Like a fill without useCenter, butt-cap stroke can be implemented by clipping against
            // radial lines. However, in both cases we have to be careful about the half-circle.
            // case. In that case the two radial lines are equal and so that edge gets clipped
            // twice. Since the shared edge goes through the center we fall back on the useCenter
            // case.
            bool useCenter =
                    (arcParams->fUseCenter || isStrokeOnly) &&
                    !SkScalarNearlyEqual(SkScalarAbs(arcParams->fSweepAngleRadians), SK_ScalarPI);
            if (useCenter) {
                SkVector norm0 = {startPoint.fY, -startPoint.fX};
                SkVector norm1 = {stopPoint.fY, -stopPoint.fX};
                if (arcParams->fSweepAngleRadians > 0) {
                    norm0.negate();
                } else {
                    norm1.negate();
                }
                fClipPlane = true;
                if (SkScalarAbs(arcParams->fSweepAngleRadians) > SK_ScalarPI) {
                    fCircles.emplace_back(Circle{
                            color,
                            innerRadius,
                            outerRadius,
                            {norm0.fX, norm0.fY, 0.5f},
                            {kUnusedIsectPlane[0], kUnusedIsectPlane[1], kUnusedIsectPlane[2]},
                            {norm1.fX, norm1.fY, 0.5f},
                            devBounds,
                            stroked});
                    fClipPlaneIsect = false;
                    fClipPlaneUnion = true;
                } else {
                    fCircles.emplace_back(Circle{
                            color,
                            innerRadius,
                            outerRadius,
                            {norm0.fX, norm0.fY, 0.5f},
                            {norm1.fX, norm1.fY, 0.5f},
                            {kUnusedUnionPlane[0], kUnusedUnionPlane[1], kUnusedUnionPlane[2]},
                            devBounds,
                            stroked});
                    fClipPlaneIsect = true;
                    fClipPlaneUnion = false;
                }
            } else {
                // We clip to a secant of the original circle.
                startPoint.scale(radius);
                stopPoint.scale(radius);
                SkVector norm = {startPoint.fY - stopPoint.fY, stopPoint.fX - startPoint.fX};
                norm.normalize();
                if (arcParams->fSweepAngleRadians > 0) {
                    norm.negate();
                }
                SkScalar d = -norm.dot(startPoint) + 0.5f;

                fCircles.emplace_back(
                        Circle{color,
                               innerRadius,
                               outerRadius,
                               {norm.fX, norm.fY, d},
                               {kUnusedIsectPlane[0], kUnusedIsectPlane[1], kUnusedIsectPlane[2]},
                               {kUnusedUnionPlane[0], kUnusedUnionPlane[1], kUnusedUnionPlane[2]},
                               devBounds,
                               stroked});
                fClipPlane = true;
                fClipPlaneIsect = false;
                fClipPlaneUnion = false;
            }
        } else {
            fCircles.emplace_back(
                    Circle{color,
                           innerRadius,
                           outerRadius,
                           {kUnusedIsectPlane[0], kUnusedIsectPlane[1], kUnusedIsectPlane[2]},
                           {kUnusedIsectPlane[0], kUnusedIsectPlane[1], kUnusedIsectPlane[2]},
                           {kUnusedUnionPlane[0], kUnusedUnionPlane[1], kUnusedUnionPlane[2]},
                           devBounds,
                           stroked});
            fClipPlane = false;
            fClipPlaneIsect = false;
            fClipPlaneUnion = false;
        }
        // Use the original radius and stroke radius for the bounds so that it does not include the
        // AA bloat.
        radius += halfWidth;
        this->setBounds(
                {center.fX - radius, center.fY - radius, center.fX + radius, center.fY + radius},
                HasAABloat::kYes, IsZeroArea::kNo);
        fVertCount = circle_type_to_vert_count(stroked);
        fIndexCount = circle_type_to_index_count(stroked);
        fAllFill = !stroked;
    }

    const char* name() const override { return "CircleOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fCircles.count(); ++i) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f],"
                    "InnerRad: %.2f, OuterRad: %.2f\n",
                    fCircles[i].fColor, fCircles[i].fDevBounds.fLeft, fCircles[i].fDevBounds.fTop,
                    fCircles[i].fDevBounds.fRight, fCircles[i].fDevBounds.fBottom,
                    fCircles[i].fInnerRadius, fCircles[i].fOuterRadius);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = &fCircles.front().fColor;
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kSingleChannel,
                                            color);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

private:
    void onPrepareDraws(Target* target) const override {
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        sk_sp<GrGeometryProcessor> gp(new CircleGeometryProcessor(
                !fAllFill, fClipPlane, fClipPlaneIsect, fClipPlaneUnion, localMatrix));

        struct CircleVertex {
            SkPoint fPos;
            GrColor fColor;
            SkPoint fOffset;
            SkScalar fOuterRadius;
            SkScalar fInnerRadius;
            // These planes may or may not be present in the vertex buffer.
            SkScalar fHalfPlanes[3][3];
        };

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride ==
                 sizeof(CircleVertex) - (fClipPlane ? 0 : 3 * sizeof(SkScalar)) -
                         (fClipPlaneIsect ? 0 : 3 * sizeof(SkScalar)) -
                         (fClipPlaneUnion ? 0 : 3 * sizeof(SkScalar)));

        const GrBuffer* vertexBuffer;
        int firstVertex;
        char* vertices = (char*)target->makeVertexSpace(vertexStride, fVertCount, &vertexBuffer,
                                                        &firstVertex);
        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        const GrBuffer* indexBuffer = nullptr;
        int firstIndex = 0;
        uint16_t* indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }

        int currStartVertex = 0;
        for (const auto& circle : fCircles) {
            SkScalar innerRadius = circle.fInnerRadius;
            SkScalar outerRadius = circle.fOuterRadius;
            GrColor color = circle.fColor;
            const SkRect& bounds = circle.fDevBounds;

            CircleVertex* v0 = reinterpret_cast<CircleVertex*>(vertices + 0 * vertexStride);
            CircleVertex* v1 = reinterpret_cast<CircleVertex*>(vertices + 1 * vertexStride);
            CircleVertex* v2 = reinterpret_cast<CircleVertex*>(vertices + 2 * vertexStride);
            CircleVertex* v3 = reinterpret_cast<CircleVertex*>(vertices + 3 * vertexStride);
            CircleVertex* v4 = reinterpret_cast<CircleVertex*>(vertices + 4 * vertexStride);
            CircleVertex* v5 = reinterpret_cast<CircleVertex*>(vertices + 5 * vertexStride);
            CircleVertex* v6 = reinterpret_cast<CircleVertex*>(vertices + 6 * vertexStride);
            CircleVertex* v7 = reinterpret_cast<CircleVertex*>(vertices + 7 * vertexStride);

            // The inner radius in the vertex data must be specified in normalized space.
            innerRadius = innerRadius / outerRadius;

            SkPoint center = SkPoint::Make(bounds.centerX(), bounds.centerY());
            SkScalar halfWidth = 0.5f * bounds.width();
            SkScalar octOffset = 0.41421356237f;  // sqrt(2) - 1

            v0->fPos = center + SkPoint::Make(-octOffset * halfWidth, -halfWidth);
            v0->fColor = color;
            v0->fOffset = SkPoint::Make(-octOffset, -1);
            v0->fOuterRadius = outerRadius;
            v0->fInnerRadius = innerRadius;

            v1->fPos = center + SkPoint::Make(octOffset * halfWidth, -halfWidth);
            v1->fColor = color;
            v1->fOffset = SkPoint::Make(octOffset, -1);
            v1->fOuterRadius = outerRadius;
            v1->fInnerRadius = innerRadius;

            v2->fPos = center + SkPoint::Make(halfWidth, -octOffset * halfWidth);
            v2->fColor = color;
            v2->fOffset = SkPoint::Make(1, -octOffset);
            v2->fOuterRadius = outerRadius;
            v2->fInnerRadius = innerRadius;

            v3->fPos = center + SkPoint::Make(halfWidth, octOffset * halfWidth);
            v3->fColor = color;
            v3->fOffset = SkPoint::Make(1, octOffset);
            v3->fOuterRadius = outerRadius;
            v3->fInnerRadius = innerRadius;

            v4->fPos = center + SkPoint::Make(octOffset * halfWidth, halfWidth);
            v4->fColor = color;
            v4->fOffset = SkPoint::Make(octOffset, 1);
            v4->fOuterRadius = outerRadius;
            v4->fInnerRadius = innerRadius;

            v5->fPos = center + SkPoint::Make(-octOffset * halfWidth, halfWidth);
            v5->fColor = color;
            v5->fOffset = SkPoint::Make(-octOffset, 1);
            v5->fOuterRadius = outerRadius;
            v5->fInnerRadius = innerRadius;

            v6->fPos = center + SkPoint::Make(-halfWidth, octOffset * halfWidth);
            v6->fColor = color;
            v6->fOffset = SkPoint::Make(-1, octOffset);
            v6->fOuterRadius = outerRadius;
            v6->fInnerRadius = innerRadius;

            v7->fPos = center + SkPoint::Make(-halfWidth, -octOffset * halfWidth);
            v7->fColor = color;
            v7->fOffset = SkPoint::Make(-1, -octOffset);
            v7->fOuterRadius = outerRadius;
            v7->fInnerRadius = innerRadius;

            if (fClipPlane) {
                memcpy(v0->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                memcpy(v1->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                memcpy(v2->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                memcpy(v3->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                memcpy(v4->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                memcpy(v5->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                memcpy(v6->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                memcpy(v7->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
            }
            int unionIdx = 1;
            if (fClipPlaneIsect) {
                memcpy(v0->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                memcpy(v1->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                memcpy(v2->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                memcpy(v3->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                memcpy(v4->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                memcpy(v5->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                memcpy(v6->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                memcpy(v7->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                unionIdx = 2;
            }
            if (fClipPlaneUnion) {
                memcpy(v0->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                memcpy(v1->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                memcpy(v2->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                memcpy(v3->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                memcpy(v4->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                memcpy(v5->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                memcpy(v6->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                memcpy(v7->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
            }

            if (circle.fStroked) {
                // compute the inner ring
                CircleVertex* v0 = reinterpret_cast<CircleVertex*>(vertices + 8 * vertexStride);
                CircleVertex* v1 = reinterpret_cast<CircleVertex*>(vertices + 9 * vertexStride);
                CircleVertex* v2 = reinterpret_cast<CircleVertex*>(vertices + 10 * vertexStride);
                CircleVertex* v3 = reinterpret_cast<CircleVertex*>(vertices + 11 * vertexStride);
                CircleVertex* v4 = reinterpret_cast<CircleVertex*>(vertices + 12 * vertexStride);
                CircleVertex* v5 = reinterpret_cast<CircleVertex*>(vertices + 13 * vertexStride);
                CircleVertex* v6 = reinterpret_cast<CircleVertex*>(vertices + 14 * vertexStride);
                CircleVertex* v7 = reinterpret_cast<CircleVertex*>(vertices + 15 * vertexStride);

                // cosine and sine of pi/8
                SkScalar c = 0.923579533f;
                SkScalar s = 0.382683432f;
                SkScalar r = circle.fInnerRadius;

                v0->fPos = center + SkPoint::Make(-s * r, -c * r);
                v0->fColor = color;
                v0->fOffset = SkPoint::Make(-s * innerRadius, -c * innerRadius);
                v0->fOuterRadius = outerRadius;
                v0->fInnerRadius = innerRadius;

                v1->fPos = center + SkPoint::Make(s * r, -c * r);
                v1->fColor = color;
                v1->fOffset = SkPoint::Make(s * innerRadius, -c * innerRadius);
                v1->fOuterRadius = outerRadius;
                v1->fInnerRadius = innerRadius;

                v2->fPos = center + SkPoint::Make(c * r, -s * r);
                v2->fColor = color;
                v2->fOffset = SkPoint::Make(c * innerRadius, -s * innerRadius);
                v2->fOuterRadius = outerRadius;
                v2->fInnerRadius = innerRadius;

                v3->fPos = center + SkPoint::Make(c * r, s * r);
                v3->fColor = color;
                v3->fOffset = SkPoint::Make(c * innerRadius, s * innerRadius);
                v3->fOuterRadius = outerRadius;
                v3->fInnerRadius = innerRadius;

                v4->fPos = center + SkPoint::Make(s * r, c * r);
                v4->fColor = color;
                v4->fOffset = SkPoint::Make(s * innerRadius, c * innerRadius);
                v4->fOuterRadius = outerRadius;
                v4->fInnerRadius = innerRadius;

                v5->fPos = center + SkPoint::Make(-s * r, c * r);
                v5->fColor = color;
                v5->fOffset = SkPoint::Make(-s * innerRadius, c * innerRadius);
                v5->fOuterRadius = outerRadius;
                v5->fInnerRadius = innerRadius;

                v6->fPos = center + SkPoint::Make(-c * r, s * r);
                v6->fColor = color;
                v6->fOffset = SkPoint::Make(-c * innerRadius, s * innerRadius);
                v6->fOuterRadius = outerRadius;
                v6->fInnerRadius = innerRadius;

                v7->fPos = center + SkPoint::Make(-c * r, -s * r);
                v7->fColor = color;
                v7->fOffset = SkPoint::Make(-c * innerRadius, -s * innerRadius);
                v7->fOuterRadius = outerRadius;
                v7->fInnerRadius = innerRadius;

                if (fClipPlane) {
                    memcpy(v0->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                    memcpy(v1->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                    memcpy(v2->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                    memcpy(v3->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                    memcpy(v4->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                    memcpy(v5->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                    memcpy(v6->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                    memcpy(v7->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                }
                int unionIdx = 1;
                if (fClipPlaneIsect) {
                    memcpy(v0->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    memcpy(v1->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    memcpy(v2->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    memcpy(v3->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    memcpy(v4->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    memcpy(v5->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    memcpy(v6->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    memcpy(v7->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    unionIdx = 2;
                }
                if (fClipPlaneUnion) {
                    memcpy(v0->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                    memcpy(v1->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                    memcpy(v2->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                    memcpy(v3->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                    memcpy(v4->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                    memcpy(v5->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                    memcpy(v6->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                    memcpy(v7->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                }
            } else {
                // filled
                CircleVertex* v8 = reinterpret_cast<CircleVertex*>(vertices + 8 * vertexStride);
                v8->fPos = center;
                v8->fColor = color;
                v8->fOffset = SkPoint::Make(0, 0);
                v8->fOuterRadius = outerRadius;
                v8->fInnerRadius = innerRadius;
                if (fClipPlane) {
                    memcpy(v8->fHalfPlanes[0], circle.fClipPlane, 3 * sizeof(SkScalar));
                }
                int unionIdx = 1;
                if (fClipPlaneIsect) {
                    memcpy(v8->fHalfPlanes[1], circle.fIsectPlane, 3 * sizeof(SkScalar));
                    unionIdx = 2;
                }
                if (fClipPlaneUnion) {
                    memcpy(v8->fHalfPlanes[unionIdx], circle.fUnionPlane, 3 * sizeof(SkScalar));
                }
            }

            const uint16_t* primIndices = circle_type_to_indices(circle.fStroked);
            const int primIndexCount = circle_type_to_index_count(circle.fStroked);
            for (int i = 0; i < primIndexCount; ++i) {
                *indices++ = primIndices[i] + currStartVertex;
            }

            currStartVertex += circle_type_to_vert_count(circle.fStroked);
            vertices += circle_type_to_vert_count(circle.fStroked) * vertexStride;
        }

        GrMesh mesh(GrPrimitiveType::kTriangles);
        mesh.setIndexed(indexBuffer, fIndexCount, firstIndex, 0, fVertCount - 1);
        mesh.setVertexData(vertexBuffer, firstVertex);
        target->draw(gp.get(),  fHelper.makePipeline(target), mesh);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        CircleOp* that = t->cast<CircleOp>();

        // can only represent 65535 unique vertices with 16-bit indices
        if (fVertCount + that->fVertCount > 65536) {
            return false;
        }

        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        if (fHelper.usesLocalCoords() &&
            !fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        // Because we've set up the ops that don't use the planes with noop values
        // we can just accumulate used planes by later ops.
        fClipPlane |= that->fClipPlane;
        fClipPlaneIsect |= that->fClipPlaneIsect;
        fClipPlaneUnion |= that->fClipPlaneUnion;

        fCircles.push_back_n(that->fCircles.count(), that->fCircles.begin());
        this->joinBounds(*that);
        fVertCount += that->fVertCount;
        fIndexCount += that->fIndexCount;
        fAllFill = fAllFill && that->fAllFill;
        return true;
    }

    struct Circle {
        GrColor fColor;
        SkScalar fInnerRadius;
        SkScalar fOuterRadius;
        SkScalar fClipPlane[3];
        SkScalar fIsectPlane[3];
        SkScalar fUnionPlane[3];
        SkRect fDevBounds;
        bool fStroked;
    };

    SkMatrix fViewMatrixIfUsingLocalCoords;
    Helper fHelper;
    SkSTArray<1, Circle, true> fCircles;
    int fVertCount;
    int fIndexCount;
    bool fAllFill;
    bool fClipPlane;
    bool fClipPlaneIsect;
    bool fClipPlaneUnion;

    typedef GrMeshDrawOp INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class EllipseOp : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

    struct DeviceSpaceParams {
        SkPoint fCenter;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
    };

public:
    DEFINE_OP_CLASS_ID
    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRect& ellipse, const SkStrokeRec& stroke) {
        DeviceSpaceParams params;
        // do any matrix crunching before we reset the draw state for device coords
        params.fCenter = SkPoint::Make(ellipse.centerX(), ellipse.centerY());
        viewMatrix.mapPoints(&params.fCenter, 1);
        SkScalar ellipseXRadius = SkScalarHalf(ellipse.width());
        SkScalar ellipseYRadius = SkScalarHalf(ellipse.height());
        params.fXRadius = SkScalarAbs(viewMatrix[SkMatrix::kMScaleX] * ellipseXRadius +
                                      viewMatrix[SkMatrix::kMSkewX] * ellipseYRadius);
        params.fYRadius = SkScalarAbs(viewMatrix[SkMatrix::kMSkewY] * ellipseXRadius +
                                      viewMatrix[SkMatrix::kMScaleY] * ellipseYRadius);

        // do (potentially) anisotropic mapping of stroke
        SkVector scaledStroke;
        SkScalar strokeWidth = stroke.getWidth();
        scaledStroke.fX = SkScalarAbs(
                strokeWidth * (viewMatrix[SkMatrix::kMScaleX] + viewMatrix[SkMatrix::kMSkewY]));
        scaledStroke.fY = SkScalarAbs(
                strokeWidth * (viewMatrix[SkMatrix::kMSkewX] + viewMatrix[SkMatrix::kMScaleY]));

        SkStrokeRec::Style style = stroke.getStyle();
        bool isStrokeOnly =
                SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style;
        bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == style;

        params.fInnerXRadius = 0;
        params.fInnerYRadius = 0;
        if (hasStroke) {
            if (SkScalarNearlyZero(scaledStroke.length())) {
                scaledStroke.set(SK_ScalarHalf, SK_ScalarHalf);
            } else {
                scaledStroke.scale(SK_ScalarHalf);
            }

            // we only handle thick strokes for near-circular ellipses
            if (scaledStroke.length() > SK_ScalarHalf &&
                (0.5f * params.fXRadius > params.fYRadius ||
                 0.5f * params.fYRadius > params.fXRadius)) {
                return nullptr;
            }

            // we don't handle it if curvature of the stroke is less than curvature of the ellipse
            if (scaledStroke.fX * (params.fXRadius * params.fYRadius) <
                        (scaledStroke.fY * scaledStroke.fY) * params.fXRadius ||
                scaledStroke.fY * (params.fXRadius * params.fXRadius) <
                        (scaledStroke.fX * scaledStroke.fX) * params.fYRadius) {
                return nullptr;
            }

            // this is legit only if scale & translation (which should be the case at the moment)
            if (isStrokeOnly) {
                params.fInnerXRadius = params.fXRadius - scaledStroke.fX;
                params.fInnerYRadius = params.fYRadius - scaledStroke.fY;
            }

            params.fXRadius += scaledStroke.fX;
            params.fYRadius += scaledStroke.fY;
        }
        return Helper::FactoryHelper<EllipseOp>(std::move(paint), viewMatrix, params, stroke);
    }

    EllipseOp(const Helper::MakeArgs& helperArgs, GrColor color, const SkMatrix& viewMatrix,
              const DeviceSpaceParams& params, const SkStrokeRec& stroke)
            : INHERITED(ClassID()), fHelper(helperArgs, GrAAType::kCoverage) {
        SkStrokeRec::Style style = stroke.getStyle();
        bool isStrokeOnly =
                SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style;

        fEllipses.emplace_back(Ellipse{color, params.fXRadius, params.fYRadius,
                                       params.fInnerXRadius, params.fInnerYRadius,
                                       SkRect::MakeLTRB(params.fCenter.fX - params.fXRadius,
                                                        params.fCenter.fY - params.fYRadius,
                                                        params.fCenter.fX + params.fXRadius,
                                                        params.fCenter.fY + params.fYRadius)});

        this->setBounds(fEllipses.back().fDevBounds, HasAABloat::kYes, IsZeroArea::kNo);

        // Outset bounds to include half-pixel width antialiasing.
        fEllipses[0].fDevBounds.outset(SK_ScalarHalf, SK_ScalarHalf);

        fStroked = isStrokeOnly && params.fInnerXRadius > 0 && params.fInnerYRadius > 0;
        fViewMatrixIfUsingLocalCoords = viewMatrix;
    }

    const char* name() const override { return "EllipseOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Stroked: %d\n", fStroked);
        for (const auto& geo : fEllipses) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "XRad: %.2f, YRad: %.2f, InnerXRad: %.2f, InnerYRad: %.2f\n",
                    geo.fColor, geo.fDevBounds.fLeft, geo.fDevBounds.fTop, geo.fDevBounds.fRight,
                    geo.fDevBounds.fBottom, geo.fXRadius, geo.fYRadius, geo.fInnerXRadius,
                    geo.fInnerYRadius);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = &fEllipses.front().fColor;
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kSingleChannel,
                                            color);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

private:
    void onPrepareDraws(Target* target) const override {
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        sk_sp<GrGeometryProcessor> gp(new EllipseGeometryProcessor(fStroked, localMatrix));

        QuadHelper helper;
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(EllipseVertex));
        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(
                helper.init(target, vertexStride, fEllipses.count()));
        if (!verts) {
            return;
        }

        for (const auto& ellipse : fEllipses) {
            GrColor color = ellipse.fColor;
            SkScalar xRadius = ellipse.fXRadius;
            SkScalar yRadius = ellipse.fYRadius;

            // Compute the reciprocals of the radii here to save time in the shader
            SkScalar xRadRecip = SkScalarInvert(xRadius);
            SkScalar yRadRecip = SkScalarInvert(yRadius);
            SkScalar xInnerRadRecip = SkScalarInvert(ellipse.fInnerXRadius);
            SkScalar yInnerRadRecip = SkScalarInvert(ellipse.fInnerYRadius);

            // fOffsets are expanded from xyRadii to include the half-pixel antialiasing width.
            SkScalar xMaxOffset = xRadius + SK_ScalarHalf;
            SkScalar yMaxOffset = yRadius + SK_ScalarHalf;

            // The inner radius in the vertex data must be specified in normalized space.
            verts[0].fPos = SkPoint::Make(ellipse.fDevBounds.fLeft, ellipse.fDevBounds.fTop);
            verts[0].fColor = color;
            verts[0].fOffset = SkPoint::Make(-xMaxOffset, -yMaxOffset);
            verts[0].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts[0].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

            verts[1].fPos = SkPoint::Make(ellipse.fDevBounds.fLeft, ellipse.fDevBounds.fBottom);
            verts[1].fColor = color;
            verts[1].fOffset = SkPoint::Make(-xMaxOffset, yMaxOffset);
            verts[1].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts[1].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

            verts[2].fPos = SkPoint::Make(ellipse.fDevBounds.fRight, ellipse.fDevBounds.fBottom);
            verts[2].fColor = color;
            verts[2].fOffset = SkPoint::Make(xMaxOffset, yMaxOffset);
            verts[2].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts[2].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

            verts[3].fPos = SkPoint::Make(ellipse.fDevBounds.fRight, ellipse.fDevBounds.fTop);
            verts[3].fColor = color;
            verts[3].fOffset = SkPoint::Make(xMaxOffset, -yMaxOffset);
            verts[3].fOuterRadii = SkPoint::Make(xRadRecip, yRadRecip);
            verts[3].fInnerRadii = SkPoint::Make(xInnerRadRecip, yInnerRadRecip);

            verts += kVerticesPerQuad;
        }
        helper.recordDraw(target, gp.get(), fHelper.makePipeline(target));
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        EllipseOp* that = t->cast<EllipseOp>();

        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        if (fStroked != that->fStroked) {
            return false;
        }

        if (fHelper.usesLocalCoords() &&
            !fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fEllipses.push_back_n(that->fEllipses.count(), that->fEllipses.begin());
        this->joinBounds(*that);
        return true;
    }

    struct Ellipse {
        GrColor fColor;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        SkRect fDevBounds;
    };

    SkMatrix fViewMatrixIfUsingLocalCoords;
    Helper fHelper;
    bool fStroked;
    SkSTArray<1, Ellipse, true> fEllipses;

    typedef GrMeshDrawOp INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

class DIEllipseOp : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

    struct DeviceSpaceParams {
        SkPoint fCenter;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        DIEllipseStyle fStyle;
    };

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRect& ellipse, const SkStrokeRec& stroke) {
        DeviceSpaceParams params;
        params.fCenter = SkPoint::Make(ellipse.centerX(), ellipse.centerY());
        params.fXRadius = SkScalarHalf(ellipse.width());
        params.fYRadius = SkScalarHalf(ellipse.height());

        SkStrokeRec::Style style = stroke.getStyle();
        params.fStyle = (SkStrokeRec::kStroke_Style == style)
                                ? DIEllipseStyle::kStroke
                                : (SkStrokeRec::kHairline_Style == style)
                                          ? DIEllipseStyle::kHairline
                                          : DIEllipseStyle::kFill;

        params.fInnerXRadius = 0;
        params.fInnerYRadius = 0;
        if (SkStrokeRec::kFill_Style != style && SkStrokeRec::kHairline_Style != style) {
            SkScalar strokeWidth = stroke.getWidth();

            if (SkScalarNearlyZero(strokeWidth)) {
                strokeWidth = SK_ScalarHalf;
            } else {
                strokeWidth *= SK_ScalarHalf;
            }

            // we only handle thick strokes for near-circular ellipses
            if (strokeWidth > SK_ScalarHalf &&
                (SK_ScalarHalf * params.fXRadius > params.fYRadius ||
                 SK_ScalarHalf * params.fYRadius > params.fXRadius)) {
                return nullptr;
            }

            // we don't handle it if curvature of the stroke is less than curvature of the ellipse
            if (strokeWidth * (params.fYRadius * params.fYRadius) <
                (strokeWidth * strokeWidth) * params.fXRadius) {
                return nullptr;
            }
            if (strokeWidth * (params.fXRadius * params.fXRadius) <
                (strokeWidth * strokeWidth) * params.fYRadius) {
                return nullptr;
            }

            // set inner radius (if needed)
            if (SkStrokeRec::kStroke_Style == style) {
                params.fInnerXRadius = params.fXRadius - strokeWidth;
                params.fInnerYRadius = params.fYRadius - strokeWidth;
            }

            params.fXRadius += strokeWidth;
            params.fYRadius += strokeWidth;
        }
        if (DIEllipseStyle::kStroke == params.fStyle &&
            (params.fInnerXRadius <= 0 || params.fInnerYRadius <= 0)) {
            params.fStyle = DIEllipseStyle::kFill;
        }
        return Helper::FactoryHelper<DIEllipseOp>(std::move(paint), params, viewMatrix);
    }

    DIEllipseOp(Helper::MakeArgs& helperArgs, GrColor color, const DeviceSpaceParams& params,
                const SkMatrix& viewMatrix)
            : INHERITED(ClassID()), fHelper(helperArgs, GrAAType::kCoverage) {
        // This expands the outer rect so that after CTM we end up with a half-pixel border
        SkScalar a = viewMatrix[SkMatrix::kMScaleX];
        SkScalar b = viewMatrix[SkMatrix::kMSkewX];
        SkScalar c = viewMatrix[SkMatrix::kMSkewY];
        SkScalar d = viewMatrix[SkMatrix::kMScaleY];
        SkScalar geoDx = SK_ScalarHalf / SkScalarSqrt(a * a + c * c);
        SkScalar geoDy = SK_ScalarHalf / SkScalarSqrt(b * b + d * d);

        fEllipses.emplace_back(
                Ellipse{viewMatrix, color, params.fXRadius, params.fYRadius, params.fInnerXRadius,
                        params.fInnerYRadius, geoDx, geoDy, params.fStyle,
                        SkRect::MakeLTRB(params.fCenter.fX - params.fXRadius - geoDx,
                                         params.fCenter.fY - params.fYRadius - geoDy,
                                         params.fCenter.fX + params.fXRadius + geoDx,
                                         params.fCenter.fY + params.fYRadius + geoDy)});
        this->setTransformedBounds(fEllipses[0].fBounds, viewMatrix, HasAABloat::kYes,
                                   IsZeroArea::kNo);
    }

    const char* name() const override { return "DIEllipseOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (const auto& geo : fEllipses) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], XRad: %.2f, "
                    "YRad: %.2f, InnerXRad: %.2f, InnerYRad: %.2f, GeoDX: %.2f, "
                    "GeoDY: %.2f\n",
                    geo.fColor, geo.fBounds.fLeft, geo.fBounds.fTop, geo.fBounds.fRight,
                    geo.fBounds.fBottom, geo.fXRadius, geo.fYRadius, geo.fInnerXRadius,
                    geo.fInnerYRadius, geo.fGeoDx, geo.fGeoDy);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = &fEllipses.front().fColor;
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kSingleChannel,
                                            color);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

private:
    void onPrepareDraws(Target* target) const override {
        // Setup geometry processor
        sk_sp<GrGeometryProcessor> gp(
                new DIEllipseGeometryProcessor(this->viewMatrix(), this->style()));

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(DIEllipseVertex));
        QuadHelper helper;
        DIEllipseVertex* verts = reinterpret_cast<DIEllipseVertex*>(
                helper.init(target, vertexStride, fEllipses.count()));
        if (!verts) {
            return;
        }

        for (const auto& ellipse : fEllipses) {
            GrColor color = ellipse.fColor;
            SkScalar xRadius = ellipse.fXRadius;
            SkScalar yRadius = ellipse.fYRadius;

            const SkRect& bounds = ellipse.fBounds;

            // This adjusts the "radius" to include the half-pixel border
            SkScalar offsetDx = ellipse.fGeoDx / xRadius;
            SkScalar offsetDy = ellipse.fGeoDy / yRadius;

            SkScalar innerRatioX = xRadius / ellipse.fInnerXRadius;
            SkScalar innerRatioY = yRadius / ellipse.fInnerYRadius;

            verts[0].fPos = SkPoint::Make(bounds.fLeft, bounds.fTop);
            verts[0].fColor = color;
            verts[0].fOuterOffset = SkPoint::Make(-1.0f - offsetDx, -1.0f - offsetDy);
            verts[0].fInnerOffset = SkPoint::Make(-innerRatioX - offsetDx, -innerRatioY - offsetDy);

            verts[1].fPos = SkPoint::Make(bounds.fLeft, bounds.fBottom);
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
        helper.recordDraw(target, gp.get(), fHelper.makePipeline(target));
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        DIEllipseOp* that = t->cast<DIEllipseOp>();
        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        if (this->style() != that->style()) {
            return false;
        }

        // TODO rewrite to allow positioning on CPU
        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        fEllipses.push_back_n(that->fEllipses.count(), that->fEllipses.begin());
        this->joinBounds(*that);
        return true;
    }

    const SkMatrix& viewMatrix() const { return fEllipses[0].fViewMatrix; }
    DIEllipseStyle style() const { return fEllipses[0].fStyle; }

    struct Ellipse {
        SkMatrix fViewMatrix;
        GrColor fColor;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        SkScalar fGeoDx;
        SkScalar fGeoDy;
        DIEllipseStyle fStyle;
        SkRect fBounds;
    };

    Helper fHelper;
    SkSTArray<1, Ellipse, true> fEllipses;

    typedef GrMeshDrawOp INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

// We have three possible cases for geometry for a roundrect.
//
// In the case of a normal fill or a stroke, we draw the roundrect as a 9-patch:
//    ____________
//   |_|________|_|
//   | |        | |
//   | |        | |
//   | |        | |
//   |_|________|_|
//   |_|________|_|
//
// For strokes, we don't draw the center quad.
//
// For circular roundrects, in the case where the stroke width is greater than twice
// the corner radius (overstroke), we add additional geometry to mark out the rectangle
// in the center. The shared vertices are duplicated so we can set a different outer radius
// for the fill calculation.
//    ____________
//   |_|________|_|
//   | |\ ____ /| |
//   | | |    | | |
//   | | |____| | |
//   |_|/______\|_|
//   |_|________|_|
//
// We don't draw the center quad from the fill rect in this case.
//
// For filled rrects that need to provide a distance vector we resuse the overstroke
// geometry but make the inner rect degenerate (either a point or a horizontal or
// vertical line).

static const uint16_t gOverstrokeRRectIndices[] = {
        // clang-format off
        // overstroke quads
        // we place this at the beginning so that we can skip these indices when rendering normally
        16, 17, 19, 16, 19, 18,
        19, 17, 23, 19, 23, 21,
        21, 23, 22, 21, 22, 20,
        22, 16, 18, 22, 18, 20,

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
        // we place this at the end so that we can ignore these indices when not rendering as filled
        5, 6, 10, 5, 10, 9,
        // clang-format on
};

// fill and standard stroke indices skip the overstroke "ring"
static const uint16_t* gStandardRRectIndices = gOverstrokeRRectIndices + 6 * 4;

// overstroke count is arraysize minus the center indices
static const int kIndicesPerOverstrokeRRect = SK_ARRAY_COUNT(gOverstrokeRRectIndices) - 6;
// fill count skips overstroke indices and includes center
static const int kIndicesPerFillRRect = kIndicesPerOverstrokeRRect - 6 * 4 + 6;
// stroke count is fill count minus center indices
static const int kIndicesPerStrokeRRect = kIndicesPerFillRRect - 6;
static const int kVertsPerStandardRRect = 16;
static const int kVertsPerOverstrokeRRect = 24;

enum RRectType {
    kFill_RRectType,
    kStroke_RRectType,
    kOverstroke_RRectType,
};

static int rrect_type_to_vert_count(RRectType type) {
    switch (type) {
        case kFill_RRectType:
        case kStroke_RRectType:
            return kVertsPerStandardRRect;
        case kOverstroke_RRectType:
            return kVertsPerOverstrokeRRect;
    }
    SkFAIL("Invalid type");
    return 0;
}

static int rrect_type_to_index_count(RRectType type) {
    switch (type) {
        case kFill_RRectType:
            return kIndicesPerFillRRect;
        case kStroke_RRectType:
            return kIndicesPerStrokeRRect;
        case kOverstroke_RRectType:
            return kIndicesPerOverstrokeRRect;
    }
    SkFAIL("Invalid type");
    return 0;
}

static const uint16_t* rrect_type_to_indices(RRectType type) {
    switch (type) {
        case kFill_RRectType:
        case kStroke_RRectType:
            return gStandardRRectIndices;
        case kOverstroke_RRectType:
            return gOverstrokeRRectIndices;
    }
    SkFAIL("Invalid type");
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// For distance computations in the interior of filled rrects we:
//
//   add a interior degenerate (point or line) rect
//   each vertex of that rect gets -outerRad as its radius
//      this makes the computation of the distance to the outer edge be negative
//      negative values are caught and then handled differently in the GP's onEmitCode
//   each vertex is also given the normalized x & y distance from the interior rect's edge
//      the GP takes the min of those depths +1 to get the normalized distance to the outer edge

class CircularRRectOp : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    // A devStrokeWidth <= 0 indicates a fill only. If devStrokeWidth > 0 then strokeOnly indicates
    // whether the rrect is only stroked or stroked and filled.
    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRect& devRect, float devRadius,
                                          float devStrokeWidth, bool strokeOnly) {
        return Helper::FactoryHelper<CircularRRectOp>(std::move(paint), viewMatrix, devRect,
                                                      devRadius, devStrokeWidth, strokeOnly);
    }
    CircularRRectOp(Helper::MakeArgs& helperArgs, GrColor color, const SkMatrix& viewMatrix,
                    const SkRect& devRect, float devRadius, float devStrokeWidth, bool strokeOnly)
            : INHERITED(ClassID())
            , fViewMatrixIfUsingLocalCoords(viewMatrix)
            , fHelper(helperArgs, GrAAType::kCoverage) {
        SkRect bounds = devRect;
        SkASSERT(!(devStrokeWidth <= 0 && strokeOnly));
        SkScalar innerRadius = 0.0f;
        SkScalar outerRadius = devRadius;
        SkScalar halfWidth = 0;
        RRectType type = kFill_RRectType;
        if (devStrokeWidth > 0) {
            if (SkScalarNearlyZero(devStrokeWidth)) {
                halfWidth = SK_ScalarHalf;
            } else {
                halfWidth = SkScalarHalf(devStrokeWidth);
            }

            if (strokeOnly) {
                // Outset stroke by 1/4 pixel
                devStrokeWidth += 0.25f;
                // If stroke is greater than width or height, this is still a fill
                // Otherwise we compute stroke params
                if (devStrokeWidth <= devRect.width() && devStrokeWidth <= devRect.height()) {
                    innerRadius = devRadius - halfWidth;
                    type = (innerRadius >= 0) ? kStroke_RRectType : kOverstroke_RRectType;
                }
            }
            outerRadius += halfWidth;
            bounds.outset(halfWidth, halfWidth);
        }

        // The radii are outset for two reasons. First, it allows the shader to simply perform
        // simpler computation because the computed alpha is zero, rather than 50%, at the radius.
        // Second, the outer radius is used to compute the verts of the bounding box that is
        // rendered and the outset ensures the box will cover all partially covered by the rrect
        // corners.
        outerRadius += SK_ScalarHalf;
        innerRadius -= SK_ScalarHalf;

        this->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kNo);

        // Expand the rect for aa to generate correct vertices.
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);

        fRRects.emplace_back(RRect{color, innerRadius, outerRadius, bounds, type});
        fVertCount = rrect_type_to_vert_count(type);
        fIndexCount = rrect_type_to_index_count(type);
        fAllFill = (kFill_RRectType == type);
    }

    const char* name() const override { return "CircularRRectOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fRRects.count(); ++i) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f],"
                    "InnerRad: %.2f, OuterRad: %.2f\n",
                    fRRects[i].fColor, fRRects[i].fDevBounds.fLeft, fRRects[i].fDevBounds.fTop,
                    fRRects[i].fDevBounds.fRight, fRRects[i].fDevBounds.fBottom,
                    fRRects[i].fInnerRadius, fRRects[i].fOuterRadius);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = &fRRects.front().fColor;
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kSingleChannel,
                                            color);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

private:
    struct CircleVertex {
        SkPoint fPos;
        GrColor fColor;
        SkPoint fOffset;
        SkScalar fOuterRadius;
        SkScalar fInnerRadius;
        // No half plane, we don't use it here.
    };

    static void FillInOverstrokeVerts(CircleVertex** verts, const SkRect& bounds, SkScalar smInset,
                                      SkScalar bigInset, SkScalar xOffset, SkScalar outerRadius,
                                      SkScalar innerRadius, GrColor color) {
        SkASSERT(smInset < bigInset);

        // TL
        (*verts)->fPos = SkPoint::Make(bounds.fLeft + smInset, bounds.fTop + smInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(xOffset, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fInnerRadius = innerRadius;
        (*verts)++;

        // TR
        (*verts)->fPos = SkPoint::Make(bounds.fRight - smInset, bounds.fTop + smInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(xOffset, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fInnerRadius = innerRadius;
        (*verts)++;

        (*verts)->fPos = SkPoint::Make(bounds.fLeft + bigInset, bounds.fTop + bigInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fInnerRadius = innerRadius;
        (*verts)++;

        (*verts)->fPos = SkPoint::Make(bounds.fRight - bigInset, bounds.fTop + bigInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fInnerRadius = innerRadius;
        (*verts)++;

        (*verts)->fPos = SkPoint::Make(bounds.fLeft + bigInset, bounds.fBottom - bigInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fInnerRadius = innerRadius;
        (*verts)++;

        (*verts)->fPos = SkPoint::Make(bounds.fRight - bigInset, bounds.fBottom - bigInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fInnerRadius = innerRadius;
        (*verts)++;

        // BL
        (*verts)->fPos = SkPoint::Make(bounds.fLeft + smInset, bounds.fBottom - smInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(xOffset, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fInnerRadius = innerRadius;
        (*verts)++;

        // BR
        (*verts)->fPos = SkPoint::Make(bounds.fRight - smInset, bounds.fBottom - smInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(xOffset, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fInnerRadius = innerRadius;
        (*verts)++;
    }

    void onPrepareDraws(Target* target) const override {
        // Invert the view matrix as a local matrix (if any other processors require coords).
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        sk_sp<GrGeometryProcessor> gp(
                new CircleGeometryProcessor(!fAllFill, false, false, false, localMatrix));

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(sizeof(CircleVertex) == vertexStride);

        const GrBuffer* vertexBuffer;
        int firstVertex;

        CircleVertex* verts = (CircleVertex*)target->makeVertexSpace(vertexStride, fVertCount,
                                                                     &vertexBuffer, &firstVertex);
        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        const GrBuffer* indexBuffer = nullptr;
        int firstIndex = 0;
        uint16_t* indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }

        int currStartVertex = 0;
        for (const auto& rrect : fRRects) {
            GrColor color = rrect.fColor;
            SkScalar outerRadius = rrect.fOuterRadius;
            const SkRect& bounds = rrect.fDevBounds;

            SkScalar yCoords[4] = {bounds.fTop, bounds.fTop + outerRadius,
                                   bounds.fBottom - outerRadius, bounds.fBottom};

            SkScalar yOuterRadii[4] = {-1, 0, 0, 1};
            // The inner radius in the vertex data must be specified in normalized space.
            // For fills, specifying -1/outerRadius guarantees an alpha of 1.0 at the inner radius.
            SkScalar innerRadius = rrect.fType != kFill_RRectType
                                           ? rrect.fInnerRadius / rrect.fOuterRadius
                                           : -1.0f / rrect.fOuterRadius;
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
            // Add the additional vertices for overstroked rrects.
            // Effectively this is an additional stroked rrect, with its
            // outer radius = outerRadius - innerRadius, and inner radius = 0.
            // This will give us correct AA in the center and the correct
            // distance to the outer edge.
            //
            // Also, the outer offset is a constant vector pointing to the right, which
            // guarantees that the distance value along the outer rectangle is constant.
            if (kOverstroke_RRectType == rrect.fType) {
                SkASSERT(rrect.fInnerRadius <= 0.0f);

                SkScalar overstrokeOuterRadius = outerRadius - rrect.fInnerRadius;
                // this is the normalized distance from the outer rectangle of this
                // geometry to the outer edge
                SkScalar maxOffset = -rrect.fInnerRadius / overstrokeOuterRadius;

                FillInOverstrokeVerts(&verts, bounds, outerRadius, overstrokeOuterRadius, maxOffset,
                                      overstrokeOuterRadius, 0.0f, rrect.fColor);
            }

            const uint16_t* primIndices = rrect_type_to_indices(rrect.fType);
            const int primIndexCount = rrect_type_to_index_count(rrect.fType);
            for (int i = 0; i < primIndexCount; ++i) {
                *indices++ = primIndices[i] + currStartVertex;
            }

            currStartVertex += rrect_type_to_vert_count(rrect.fType);
        }

        GrMesh mesh(GrPrimitiveType::kTriangles);
        mesh.setIndexed(indexBuffer, fIndexCount, firstIndex, 0, fVertCount - 1);
        mesh.setVertexData(vertexBuffer, firstVertex);
        target->draw(gp.get(), fHelper.makePipeline(target), mesh);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        CircularRRectOp* that = t->cast<CircularRRectOp>();

        // can only represent 65535 unique vertices with 16-bit indices
        if (fVertCount + that->fVertCount > 65536) {
            return false;
        }

        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        if (fHelper.usesLocalCoords() &&
            !fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fRRects.push_back_n(that->fRRects.count(), that->fRRects.begin());
        this->joinBounds(*that);
        fVertCount += that->fVertCount;
        fIndexCount += that->fIndexCount;
        fAllFill = fAllFill && that->fAllFill;
        return true;
    }

    struct RRect {
        GrColor fColor;
        SkScalar fInnerRadius;
        SkScalar fOuterRadius;
        SkRect fDevBounds;
        RRectType fType;
    };

    SkMatrix fViewMatrixIfUsingLocalCoords;
    Helper fHelper;
    int fVertCount;
    int fIndexCount;
    bool fAllFill;
    SkSTArray<1, RRect, true> fRRects;

    typedef GrMeshDrawOp INHERITED;
};

static const int kNumRRectsInIndexBuffer = 256;

GR_DECLARE_STATIC_UNIQUE_KEY(gStrokeRRectOnlyIndexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gRRectOnlyIndexBufferKey);
static const GrBuffer* ref_rrect_index_buffer(RRectType type,
                                              GrResourceProvider* resourceProvider) {
    GR_DEFINE_STATIC_UNIQUE_KEY(gStrokeRRectOnlyIndexBufferKey);
    GR_DEFINE_STATIC_UNIQUE_KEY(gRRectOnlyIndexBufferKey);
    switch (type) {
        case kFill_RRectType:
            return resourceProvider->findOrCreatePatternedIndexBuffer(
                    gStandardRRectIndices, kIndicesPerFillRRect, kNumRRectsInIndexBuffer,
                    kVertsPerStandardRRect, gRRectOnlyIndexBufferKey);
        case kStroke_RRectType:
            return resourceProvider->findOrCreatePatternedIndexBuffer(
                    gStandardRRectIndices, kIndicesPerStrokeRRect, kNumRRectsInIndexBuffer,
                    kVertsPerStandardRRect, gStrokeRRectOnlyIndexBufferKey);
        default:
            SkASSERT(false);
            return nullptr;
    };
}

class EllipticalRRectOp : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    // If devStrokeWidths values are <= 0 indicates then fill only. Otherwise, strokeOnly indicates
    // whether the rrect is only stroked or stroked and filled.
    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRect& devRect, float devXRadius, float devYRadius,
                                          SkVector devStrokeWidths, bool strokeOnly) {
        SkASSERT(devXRadius > 0.5);
        SkASSERT(devYRadius > 0.5);
        SkASSERT((devStrokeWidths.fX > 0) == (devStrokeWidths.fY > 0));
        SkASSERT(!(strokeOnly && devStrokeWidths.fX <= 0));
        if (devStrokeWidths.fX > 0) {
            if (SkScalarNearlyZero(devStrokeWidths.length())) {
                devStrokeWidths.set(SK_ScalarHalf, SK_ScalarHalf);
            } else {
                devStrokeWidths.scale(SK_ScalarHalf);
            }

            // we only handle thick strokes for near-circular ellipses
            if (devStrokeWidths.length() > SK_ScalarHalf &&
                (SK_ScalarHalf * devXRadius > devYRadius ||
                 SK_ScalarHalf * devYRadius > devXRadius)) {
                return nullptr;
            }

            // we don't handle it if curvature of the stroke is less than curvature of the ellipse
            if (devStrokeWidths.fX * (devYRadius * devYRadius) <
                (devStrokeWidths.fY * devStrokeWidths.fY) * devXRadius) {
                return nullptr;
            }
            if (devStrokeWidths.fY * (devXRadius * devXRadius) <
                (devStrokeWidths.fX * devStrokeWidths.fX) * devYRadius) {
                return nullptr;
            }
        }
        return Helper::FactoryHelper<EllipticalRRectOp>(std::move(paint), viewMatrix, devRect,
                                                        devXRadius, devYRadius, devStrokeWidths,
                                                        strokeOnly);
    }

    EllipticalRRectOp(Helper::MakeArgs helperArgs, GrColor color, const SkMatrix& viewMatrix,
                      const SkRect& devRect, float devXRadius, float devYRadius,
                      SkVector devStrokeHalfWidths, bool strokeOnly)
            : INHERITED(ClassID()), fHelper(helperArgs, GrAAType::kCoverage) {
        SkScalar innerXRadius = 0.0f;
        SkScalar innerYRadius = 0.0f;
        SkRect bounds = devRect;
        bool stroked = false;
        if (devStrokeHalfWidths.fX > 0) {
            // this is legit only if scale & translation (which should be the case at the moment)
            if (strokeOnly) {
                innerXRadius = devXRadius - devStrokeHalfWidths.fX;
                innerYRadius = devYRadius - devStrokeHalfWidths.fY;
                stroked = (innerXRadius >= 0 && innerYRadius >= 0);
            }

            devXRadius += devStrokeHalfWidths.fX;
            devYRadius += devStrokeHalfWidths.fY;
            bounds.outset(devStrokeHalfWidths.fX, devStrokeHalfWidths.fY);
        }

        fStroked = stroked;
        fViewMatrixIfUsingLocalCoords = viewMatrix;
        this->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kNo);
        // Expand the rect for aa in order to generate the correct vertices.
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);
        fRRects.emplace_back(
                RRect{color, devXRadius, devYRadius, innerXRadius, innerYRadius, bounds});
    }

    const char* name() const override { return "EllipticalRRectOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("Stroked: %d\n", fStroked);
        for (const auto& geo : fRRects) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "XRad: %.2f, YRad: %.2f, InnerXRad: %.2f, InnerYRad: %.2f\n",
                    geo.fColor, geo.fDevBounds.fLeft, geo.fDevBounds.fTop, geo.fDevBounds.fRight,
                    geo.fDevBounds.fBottom, geo.fXRadius, geo.fYRadius, geo.fInnerXRadius,
                    geo.fInnerYRadius);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        GrColor* color = &fRRects.front().fColor;
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kSingleChannel,
                                            color);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

private:
    void onPrepareDraws(Target* target) const override {
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        sk_sp<GrGeometryProcessor> gp(new EllipseGeometryProcessor(fStroked, localMatrix));

        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(EllipseVertex));

        // drop out the middle quad if we're stroked
        int indicesPerInstance = fStroked ? kIndicesPerStrokeRRect : kIndicesPerFillRRect;
        sk_sp<const GrBuffer> indexBuffer(ref_rrect_index_buffer(
                fStroked ? kStroke_RRectType : kFill_RRectType, target->resourceProvider()));

        PatternHelper helper(GrPrimitiveType::kTriangles);
        EllipseVertex* verts = reinterpret_cast<EllipseVertex*>(
                helper.init(target, vertexStride, indexBuffer.get(), kVertsPerStandardRRect,
                            indicesPerInstance, fRRects.count()));
        if (!verts || !indexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (const auto& rrect : fRRects) {
            GrColor color = rrect.fColor;
            // Compute the reciprocals of the radii here to save time in the shader
            SkScalar xRadRecip = SkScalarInvert(rrect.fXRadius);
            SkScalar yRadRecip = SkScalarInvert(rrect.fYRadius);
            SkScalar xInnerRadRecip = SkScalarInvert(rrect.fInnerXRadius);
            SkScalar yInnerRadRecip = SkScalarInvert(rrect.fInnerYRadius);

            // Extend the radii out half a pixel to antialias.
            SkScalar xOuterRadius = rrect.fXRadius + SK_ScalarHalf;
            SkScalar yOuterRadius = rrect.fYRadius + SK_ScalarHalf;

            const SkRect& bounds = rrect.fDevBounds;

            SkScalar yCoords[4] = {bounds.fTop, bounds.fTop + yOuterRadius,
                                   bounds.fBottom - yOuterRadius, bounds.fBottom};
            SkScalar yOuterOffsets[4] = {yOuterRadius,
                                         SK_ScalarNearlyZero,  // we're using inversesqrt() in
                                                               // shader, so can't be exactly 0
                                         SK_ScalarNearlyZero, yOuterRadius};

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
        helper.recordDraw(target, gp.get(), fHelper.makePipeline(target));
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        EllipticalRRectOp* that = t->cast<EllipticalRRectOp>();

        if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
            return false;
        }

        if (fStroked != that->fStroked) {
            return false;
        }

        if (fHelper.usesLocalCoords() &&
            !fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fRRects.push_back_n(that->fRRects.count(), that->fRRects.begin());
        this->joinBounds(*that);
        return true;
    }

    struct RRect {
        GrColor fColor;
        SkScalar fXRadius;
        SkScalar fYRadius;
        SkScalar fInnerXRadius;
        SkScalar fInnerYRadius;
        SkRect fDevBounds;
    };

    SkMatrix fViewMatrixIfUsingLocalCoords;
    Helper fHelper;
    bool fStroked;
    SkSTArray<1, RRect, true> fRRects;

    typedef GrMeshDrawOp INHERITED;
};

static std::unique_ptr<GrDrawOp> make_rrect_op(GrPaint&& paint,
                                               const SkMatrix& viewMatrix,
                                               const SkRRect& rrect,
                                               const SkStrokeRec& stroke) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(rrect.isSimple());
    SkASSERT(!rrect.isOval());

    // RRect ops only handle simple, but not too simple, rrects.
    // Do any matrix crunching before we reset the draw state for device coords.
    const SkRect& rrectBounds = rrect.getBounds();
    SkRect bounds;
    viewMatrix.mapRect(&bounds, rrectBounds);

    SkVector radii = rrect.getSimpleRadii();
    SkScalar xRadius = SkScalarAbs(viewMatrix[SkMatrix::kMScaleX] * radii.fX +
                                   viewMatrix[SkMatrix::kMSkewY] * radii.fY);
    SkScalar yRadius = SkScalarAbs(viewMatrix[SkMatrix::kMSkewX] * radii.fX +
                                   viewMatrix[SkMatrix::kMScaleY] * radii.fY);

    SkStrokeRec::Style style = stroke.getStyle();

    // Do (potentially) anisotropic mapping of stroke. Use -1s to indicate fill-only draws.
    SkVector scaledStroke = {-1, -1};
    SkScalar strokeWidth = stroke.getWidth();

    bool isStrokeOnly =
            SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style;
    bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == style;

    bool isCircular = (xRadius == yRadius);
    if (hasStroke) {
        if (SkStrokeRec::kHairline_Style == style) {
            scaledStroke.set(1, 1);
        } else {
            scaledStroke.fX = SkScalarAbs(
                    strokeWidth * (viewMatrix[SkMatrix::kMScaleX] + viewMatrix[SkMatrix::kMSkewY]));
            scaledStroke.fY = SkScalarAbs(
                    strokeWidth * (viewMatrix[SkMatrix::kMSkewX] + viewMatrix[SkMatrix::kMScaleY]));
        }

        isCircular = isCircular && scaledStroke.fX == scaledStroke.fY;
        // for non-circular rrects, if half of strokewidth is greater than radius,
        // we don't handle that right now
        if (!isCircular && (SK_ScalarHalf * scaledStroke.fX > xRadius ||
                            SK_ScalarHalf * scaledStroke.fY > yRadius)) {
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
    if (isCircular) {
        return CircularRRectOp::Make(std::move(paint), viewMatrix, bounds, xRadius, scaledStroke.fX,
                                     isStrokeOnly);
        // otherwise we use the ellipse renderer
    } else {
        return EllipticalRRectOp::Make(std::move(paint), viewMatrix, bounds, xRadius, yRadius,
                                       scaledStroke, isStrokeOnly);
    }
}

std::unique_ptr<GrDrawOp> GrOvalOpFactory::MakeRRectOp(GrPaint&& paint,
                                                       const SkMatrix& viewMatrix,
                                                       const SkRRect& rrect,
                                                       const SkStrokeRec& stroke,
                                                       const GrShaderCaps* shaderCaps) {
    if (rrect.isOval()) {
        return MakeOvalOp(std::move(paint), viewMatrix, rrect.getBounds(), stroke, shaderCaps);
    }

    if (!viewMatrix.rectStaysRect() || !rrect.isSimple()) {
        return nullptr;
    }

    return make_rrect_op(std::move(paint), viewMatrix, rrect, stroke);
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrDrawOp> GrOvalOpFactory::MakeOvalOp(GrPaint&& paint,
                                                      const SkMatrix& viewMatrix,
                                                      const SkRect& oval,
                                                      const SkStrokeRec& stroke,
                                                      const GrShaderCaps* shaderCaps) {
    // we can draw circles
    SkScalar width = oval.width();
    if (width > SK_ScalarNearlyZero && SkScalarNearlyEqual(width, oval.height()) &&
        circle_stays_circle(viewMatrix)) {
        SkPoint center = {oval.centerX(), oval.centerY()};
        return CircleOp::Make(std::move(paint), viewMatrix, center, width / 2.f,
                              GrStyle(stroke, nullptr));
    }

    // prefer the device space ellipse op for batchability
    if (viewMatrix.rectStaysRect()) {
        return EllipseOp::Make(std::move(paint), viewMatrix, oval, stroke);
    }

    // Otherwise, if we have shader derivative support, render as device-independent
    if (shaderCaps->shaderDerivativeSupport()) {
        return DIEllipseOp::Make(std::move(paint), viewMatrix, oval, stroke);
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrDrawOp> GrOvalOpFactory::MakeArcOp(GrPaint&& paint, const SkMatrix& viewMatrix,
                                                     const SkRect& oval, SkScalar startAngle,
                                                     SkScalar sweepAngle, bool useCenter,
                                                     const GrStyle& style,
                                                     const GrShaderCaps* shaderCaps) {
    SkASSERT(!oval.isEmpty());
    SkASSERT(sweepAngle);
    SkScalar width = oval.width();
    if (SkScalarAbs(sweepAngle) >= 360.f) {
        return nullptr;
    }
    if (!SkScalarNearlyEqual(width, oval.height()) || !circle_stays_circle(viewMatrix)) {
        return nullptr;
    }
    SkPoint center = {oval.centerX(), oval.centerY()};
    CircleOp::ArcParams arcParams = {SkDegreesToRadians(startAngle), SkDegreesToRadians(sweepAngle),
                                     useCenter};
    return CircleOp::Make(std::move(paint), viewMatrix, center, width / 2.f, style, &arcParams);
}

///////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(CircleOp) {
    do {
        SkScalar rotate = random->nextSScalar1() * 360.f;
        SkScalar translateX = random->nextSScalar1() * 1000.f;
        SkScalar translateY = random->nextSScalar1() * 1000.f;
        SkScalar scale = random->nextSScalar1() * 100.f;
        SkMatrix viewMatrix;
        viewMatrix.setRotate(rotate);
        viewMatrix.postTranslate(translateX, translateY);
        viewMatrix.postScale(scale, scale);
        SkRect circle = GrTest::TestSquare(random);
        SkPoint center = {circle.centerX(), circle.centerY()};
        SkScalar radius = circle.width() / 2.f;
        SkStrokeRec stroke = GrTest::TestStrokeRec(random);
        CircleOp::ArcParams arcParamsTmp;
        const CircleOp::ArcParams* arcParams = nullptr;
        if (random->nextBool()) {
            arcParamsTmp.fStartAngleRadians = random->nextSScalar1() * SK_ScalarPI * 2;
            arcParamsTmp.fSweepAngleRadians = random->nextSScalar1() * SK_ScalarPI * 2 - .01f;
            arcParamsTmp.fUseCenter = random->nextBool();
            arcParams = &arcParamsTmp;
        }
        std::unique_ptr<GrDrawOp> op = CircleOp::Make(std::move(paint), viewMatrix, center, radius,
                                                      GrStyle(stroke, nullptr), arcParams);
        if (op) {
            return op;
        }
    } while (true);
}

GR_DRAW_OP_TEST_DEFINE(EllipseOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixRectStaysRect(random);
    SkRect ellipse = GrTest::TestSquare(random);
    return EllipseOp::Make(std::move(paint), viewMatrix, ellipse, GrTest::TestStrokeRec(random));
}

GR_DRAW_OP_TEST_DEFINE(DIEllipseOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkRect ellipse = GrTest::TestSquare(random);
    return DIEllipseOp::Make(std::move(paint), viewMatrix, ellipse, GrTest::TestStrokeRec(random));
}

GR_DRAW_OP_TEST_DEFINE(RRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixRectStaysRect(random);
    const SkRRect& rrect = GrTest::TestRRectSimple(random);
    return make_rrect_op(std::move(paint), viewMatrix, rrect, GrTest::TestStrokeRec(random));
}

#endif
