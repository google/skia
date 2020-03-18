/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/effects/GrBezierEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

class GrGLConicEffect : public GrGLSLGeometryProcessor {
public:
    GrGLConicEffect(const GrGeometryProcessor&);

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder*);

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const GrConicEffect& ce = primProc.cast<GrConicEffect>();

        if (!ce.viewMatrix().isIdentity() &&
            !SkMatrixPriv::CheapEqual(fViewMatrix, ce.viewMatrix()))
        {
            fViewMatrix = ce.viewMatrix();
            pdman.setSkMatrix(fViewMatrixUniform, fViewMatrix);
        }

        if (ce.color() != fColor) {
            pdman.set4fv(fColorUniform, 1, ce.color().vec());
            fColor = ce.color();
        }

        if (ce.coverageScale() != 0xff && ce.coverageScale() != fCoverageScale) {
            pdman.set1f(fCoverageScaleUniform, GrNormalizeByteToFloat(ce.coverageScale()));
            fCoverageScale = ce.coverageScale();
        }
        this->setTransformDataHelper(ce.localMatrix(), pdman, transformRange);
    }

private:
    SkMatrix fViewMatrix;
    SkPMColor4f fColor;
    uint8_t fCoverageScale;
    GrClipEdgeType fEdgeType;
    UniformHandle fColorUniform;
    UniformHandle fCoverageScaleUniform;
    UniformHandle fViewMatrixUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLConicEffect::GrGLConicEffect(const GrGeometryProcessor& processor)
    : fViewMatrix(SkMatrix::InvalidMatrix()), fColor(SK_PMColor4fILLEGAL), fCoverageScale(0xff) {
    const GrConicEffect& ce = processor.cast<GrConicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLConicEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    const GrConicEffect& gp = args.fGP.cast<GrConicEffect>();
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    // emit attributes
    varyingHandler->emitAttributes(gp);

    GrGLSLVarying v(kFloat4_GrSLType);
    varyingHandler->addVarying("ConicCoeffs", &v);
    vertBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inConicCoeffs().name());

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);

    // Setup position
    this->writeOutputPosition(vertBuilder,
                              uniformHandler,
                              gpArgs,
                              gp.inPosition().name(),
                              gp.viewMatrix(),
                              &fViewMatrixUniform);

    // emit transforms with position
    this->emitTransforms(vertBuilder,
                         varyingHandler,
                         uniformHandler,
                         gp.inPosition().asShaderVar(),
                         gp.localMatrix(),
                         args.fFPCoordTransformHandler);

    // TODO: we should check on the number of bits float and half provide and use the smallest one
    // that suffices. Additionally we should assert that the upstream code only lets us get here if
    // either float or half provides the required number of bits.

    GrShaderVar edgeAlpha("edgeAlpha", kHalf_GrSLType, 0);
    GrShaderVar dklmdx("dklmdx", kFloat3_GrSLType, 0);
    GrShaderVar dklmdy("dklmdy", kFloat3_GrSLType, 0);
    GrShaderVar dfdx("dfdx", kFloat_GrSLType, 0);
    GrShaderVar dfdy("dfdy", kFloat_GrSLType, 0);
    GrShaderVar gF("gF", kFloat2_GrSLType, 0);
    GrShaderVar gFM("gFM", kFloat_GrSLType, 0);
    GrShaderVar func("func", kFloat_GrSLType, 0);

    fragBuilder->declAppend(edgeAlpha);
    fragBuilder->declAppend(dklmdx);
    fragBuilder->declAppend(dklmdy);
    fragBuilder->declAppend(dfdx);
    fragBuilder->declAppend(dfdy);
    fragBuilder->declAppend(gF);
    fragBuilder->declAppend(gFM);
    fragBuilder->declAppend(func);

    switch (fEdgeType) {
        case GrClipEdgeType::kHairlineAA: {
            fragBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), v.fsIn());
            fragBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), v.fsIn());
            fragBuilder->codeAppendf("%s = 2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                     dfdx.c_str(),
                                     v.fsIn(), dklmdx.c_str(),
                                     v.fsIn(), dklmdx.c_str(),
                                     v.fsIn(), dklmdx.c_str());
            fragBuilder->codeAppendf("%s = 2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                     dfdy.c_str(),
                                     v.fsIn(), dklmdy.c_str(),
                                     v.fsIn(), dklmdy.c_str(),
                                     v.fsIn(), dklmdy.c_str());
            fragBuilder->codeAppendf("%s = float2(%s, %s);", gF.c_str(), dfdx.c_str(),
                                     dfdy.c_str());
            fragBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                                     gFM.c_str(), gF.c_str(), gF.c_str());
            fragBuilder->codeAppendf("%s = %s.x*%s.x - %s.y*%s.z;",
                                     func.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = abs(%s);", func.c_str(), func.c_str());
            fragBuilder->codeAppendf("%s = half(%s / %s);",
                                     edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fragBuilder->codeAppendf("%s = max(1.0 - %s, 0.0);",
                                     edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case GrClipEdgeType::kFillAA: {
            fragBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), v.fsIn());
            fragBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), v.fsIn());
            fragBuilder->codeAppendf("%s ="
                                     "2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                     dfdx.c_str(),
                                     v.fsIn(), dklmdx.c_str(),
                                     v.fsIn(), dklmdx.c_str(),
                                     v.fsIn(), dklmdx.c_str());
            fragBuilder->codeAppendf("%s ="
                                     "2.0 * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                     dfdy.c_str(),
                                     v.fsIn(), dklmdy.c_str(),
                                     v.fsIn(), dklmdy.c_str(),
                                     v.fsIn(), dklmdy.c_str());
            fragBuilder->codeAppendf("%s = float2(%s, %s);", gF.c_str(), dfdx.c_str(),
                                     dfdy.c_str());
            fragBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                                     gFM.c_str(), gF.c_str(), gF.c_str());
            fragBuilder->codeAppendf("%s = %s.x * %s.x - %s.y * %s.z;",
                                     func.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = half(%s / %s);",
                                     edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fragBuilder->codeAppendf("%s = saturate(0.5 - %s);",
                                     edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case GrClipEdgeType::kFillBW: {
            fragBuilder->codeAppendf("%s = half(%s.x * %s.x - %s.y * %s.z);",
                                     edgeAlpha.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = half(%s < 0.0);",
                                     edgeAlpha.c_str(), edgeAlpha.c_str());
            break;
        }
        default:
            SK_ABORT("Shouldn't get here");
    }

    // TODO should we really be doing this?
    if (gp.coverageScale() != 0xff) {
        const char* coverageScale;
        fCoverageScaleUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                           kFloat_GrSLType,
                                                           "Coverage",
                                                           &coverageScale);
        fragBuilder->codeAppendf("%s = half4(half(%s) * %s);",
                                 args.fOutputCoverage, coverageScale, edgeAlpha.c_str());
    } else {
        fragBuilder->codeAppendf("%s = half4(%s);", args.fOutputCoverage, edgeAlpha.c_str());
    }
}

void GrGLConicEffect::GenKey(const GrGeometryProcessor& gp,
                             const GrShaderCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrConicEffect& ce = gp.cast<GrConicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    key |= 0xff != ce.coverageScale() ? 0x8 : 0x0;
    key |= ce.usesLocalCoords() && ce.localMatrix().hasPerspective() ? 0x10 : 0x0;
    key |= ComputePosKey(ce.viewMatrix()) << 5;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

constexpr GrPrimitiveProcessor::Attribute GrConicEffect::kAttributes[];

GrConicEffect::~GrConicEffect() {}

void GrConicEffect::getGLSLProcessorKey(const GrShaderCaps& caps,
                                        GrProcessorKeyBuilder* b) const {
    GrGLConicEffect::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrConicEffect::createGLSLInstance(const GrShaderCaps&) const {
    return new GrGLConicEffect(*this);
}

GrConicEffect::GrConicEffect(const SkPMColor4f& color, const SkMatrix& viewMatrix, uint8_t coverage,
                             GrClipEdgeType edgeType, const SkMatrix& localMatrix,
                             bool usesLocalCoords)
        : INHERITED(kGrConicEffect_ClassID)
        , fColor(color)
        , fViewMatrix(viewMatrix)
        , fLocalMatrix(viewMatrix)
        , fUsesLocalCoords(usesLocalCoords)
        , fCoverageScale(coverage)
        , fEdgeType(edgeType) {
    this->setVertexAttributes(kAttributes, SK_ARRAY_COUNT(kAttributes));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrConicEffect);

#if GR_TEST_UTILS
GrGeometryProcessor* GrConicEffect::TestCreate(GrProcessorTestData* d) {
    GrGeometryProcessor* gp;
    do {
        GrClipEdgeType edgeType =
                static_cast<GrClipEdgeType>(
                        d->fRandom->nextULessThan(kGrClipEdgeTypeCnt));
        gp = GrConicEffect::Make(d->allocator(),
                                 SkPMColor4f::FromBytes_RGBA(GrRandomColor(d->fRandom)),
                                 GrTest::TestMatrix(d->fRandom), edgeType, *d->caps(),
                                 GrTest::TestMatrix(d->fRandom), d->fRandom->nextBool());
    } while (nullptr == gp);
    return gp;
}
#endif

//////////////////////////////////////////////////////////////////////////////
// Quad
//////////////////////////////////////////////////////////////////////////////

class GrGLQuadEffect : public GrGLSLGeometryProcessor {
public:
    GrGLQuadEffect(const GrGeometryProcessor&);

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrShaderCaps&,
                              GrProcessorKeyBuilder*);

    void setData(const GrGLSLProgramDataManager& pdman, const GrPrimitiveProcessor& primProc,
                 const CoordTransformRange& transformRange) override {
        const GrQuadEffect& qe = primProc.cast<GrQuadEffect>();

        if (!qe.viewMatrix().isIdentity() &&
            !SkMatrixPriv::CheapEqual(fViewMatrix, qe.viewMatrix()))
        {
            fViewMatrix = qe.viewMatrix();
            pdman.setSkMatrix(fViewMatrixUniform, fViewMatrix);
        }

        if (qe.color() != fColor) {
            pdman.set4fv(fColorUniform, 1, qe.color().vec());
            fColor = qe.color();
        }

        if (qe.coverageScale() != 0xff && qe.coverageScale() != fCoverageScale) {
            pdman.set1f(fCoverageScaleUniform, GrNormalizeByteToFloat(qe.coverageScale()));
            fCoverageScale = qe.coverageScale();
        }
        this->setTransformDataHelper(qe.localMatrix(), pdman, transformRange);
    }

private:
    SkMatrix fViewMatrix;
    SkPMColor4f fColor;
    uint8_t fCoverageScale;
    GrClipEdgeType fEdgeType;
    UniformHandle fColorUniform;
    UniformHandle fCoverageScaleUniform;
    UniformHandle fViewMatrixUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLQuadEffect::GrGLQuadEffect(const GrGeometryProcessor& processor)
    : fViewMatrix(SkMatrix::InvalidMatrix()), fColor(SK_PMColor4fILLEGAL), fCoverageScale(0xff) {
    const GrQuadEffect& ce = processor.cast<GrQuadEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLQuadEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    const GrQuadEffect& gp = args.fGP.cast<GrQuadEffect>();
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    // emit attributes
    varyingHandler->emitAttributes(gp);

    GrGLSLVarying v(kHalf4_GrSLType);
    varyingHandler->addVarying("HairQuadEdge", &v);
    vertBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inHairQuadEdge().name());

    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);

    // Setup position
    this->writeOutputPosition(vertBuilder,
                              uniformHandler,
                              gpArgs,
                              gp.inPosition().name(),
                              gp.viewMatrix(),
                              &fViewMatrixUniform);

    // emit transforms with position
    this->emitTransforms(vertBuilder,
                         varyingHandler,
                         uniformHandler,
                         gp.inPosition().asShaderVar(),
                         gp.localMatrix(),
                         args.fFPCoordTransformHandler);

    fragBuilder->codeAppendf("half edgeAlpha;");

    switch (fEdgeType) {
        case GrClipEdgeType::kHairlineAA: {
            fragBuilder->codeAppendf("half2 duvdx = half2(dFdx(%s.xy));", v.fsIn());
            fragBuilder->codeAppendf("half2 duvdy = half2(dFdy(%s.xy));", v.fsIn());
            fragBuilder->codeAppendf("half2 gF = half2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                     "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                     v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = half(%s.x * %s.x - %s.y);",
                                     v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppend("edgeAlpha = sqrt(edgeAlpha * edgeAlpha / dot(gF, gF));");
            fragBuilder->codeAppend("edgeAlpha = max(1.0 - edgeAlpha, 0.0);");
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case GrClipEdgeType::kFillAA: {
            fragBuilder->codeAppendf("half2 duvdx = half2(dFdx(%s.xy));", v.fsIn());
            fragBuilder->codeAppendf("half2 duvdy = half2(dFdy(%s.xy));", v.fsIn());
            fragBuilder->codeAppendf("half2 gF = half2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                     "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                     v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = half(%s.x * %s.x - %s.y);",
                                     v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppend("edgeAlpha = edgeAlpha / sqrt(dot(gF, gF));");
            fragBuilder->codeAppend("edgeAlpha = saturate(0.5 - edgeAlpha);");
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case GrClipEdgeType::kFillBW: {
            fragBuilder->codeAppendf("edgeAlpha = half(%s.x * %s.x - %s.y);",
                                     v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppend("edgeAlpha = half(edgeAlpha < 0.0);");
            break;
        }
        default:
            SK_ABORT("Shouldn't get here");
    }

    if (0xff != gp.coverageScale()) {
        const char* coverageScale;
        fCoverageScaleUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                           kHalf_GrSLType,
                                                           "Coverage",
                                                           &coverageScale);
        fragBuilder->codeAppendf("%s = half4(%s * edgeAlpha);", args.fOutputCoverage,
                                 coverageScale);
    } else {
        fragBuilder->codeAppendf("%s = half4(edgeAlpha);", args.fOutputCoverage);
    }
}

void GrGLQuadEffect::GenKey(const GrGeometryProcessor& gp,
                            const GrShaderCaps&,
                            GrProcessorKeyBuilder* b) {
    const GrQuadEffect& ce = gp.cast<GrQuadEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    key |= ce.coverageScale() != 0xff ? 0x8 : 0x0;
    key |= ce.usesLocalCoords() && ce.localMatrix().hasPerspective() ? 0x10 : 0x0;
    key |= ComputePosKey(ce.viewMatrix()) << 5;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

constexpr GrPrimitiveProcessor::Attribute GrQuadEffect::kAttributes[];

GrQuadEffect::~GrQuadEffect() {}

void GrQuadEffect::getGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const {
    GrGLQuadEffect::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrQuadEffect::createGLSLInstance(const GrShaderCaps&) const {
    return new GrGLQuadEffect(*this);
}

GrQuadEffect::GrQuadEffect(const SkPMColor4f& color, const SkMatrix& viewMatrix, uint8_t coverage,
                           GrClipEdgeType edgeType, const SkMatrix& localMatrix,
                           bool usesLocalCoords)
    : INHERITED(kGrQuadEffect_ClassID)
    , fColor(color)
    , fViewMatrix(viewMatrix)
    , fLocalMatrix(localMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fCoverageScale(coverage)
    , fEdgeType(edgeType) {
    this->setVertexAttributes(kAttributes, SK_ARRAY_COUNT(kAttributes));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrQuadEffect);

#if GR_TEST_UTILS
GrGeometryProcessor* GrQuadEffect::TestCreate(GrProcessorTestData* d) {
    GrGeometryProcessor* gp;
    do {
        GrClipEdgeType edgeType = static_cast<GrClipEdgeType>(
                d->fRandom->nextULessThan(kGrClipEdgeTypeCnt));
        gp = GrQuadEffect::Make(d->allocator(),
                                SkPMColor4f::FromBytes_RGBA(GrRandomColor(d->fRandom)),
                                GrTest::TestMatrix(d->fRandom), edgeType, *d->caps(),
                                GrTest::TestMatrix(d->fRandom), d->fRandom->nextBool());
    } while (nullptr == gp);
    return gp;
}
#endif
