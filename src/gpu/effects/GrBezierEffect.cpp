/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBezierEffect.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLUtil.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

class GrGLConicEffect : public GrGLSLGeometryProcessor {
public:
    GrGLConicEffect(const GrGeometryProcessor&);

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder*);

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const GrConicEffect& ce = primProc.cast<GrConicEffect>();

        if (!ce.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(ce.viewMatrix())) {
            fViewMatrix = ce.viewMatrix();
            float viewMatrix[3 * 3];
            GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
            pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
        }

        if (ce.color() != fColor) {
            float c[4];
            GrColorToRGBAFloat(ce.color(), c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = ce.color();
        }

        if (ce.coverageScale() != 0xff && ce.coverageScale() != fCoverageScale) {
            pdman.set1f(fCoverageScaleUniform, GrNormalizeByteToFloat(ce.coverageScale()));
            fCoverageScale = ce.coverageScale();
        }
    }

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          const GrGLSLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataHelper<GrConicEffect>(primProc, pdman, index, transforms);
    }

private:
    SkMatrix fViewMatrix;
    GrColor fColor;
    uint8_t fCoverageScale;
    GrPrimitiveEdgeType fEdgeType;
    UniformHandle fColorUniform;
    UniformHandle fCoverageScaleUniform;
    UniformHandle fViewMatrixUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLConicEffect::GrGLConicEffect(const GrGeometryProcessor& processor)
    : fViewMatrix(SkMatrix::InvalidMatrix()), fColor(GrColor_ILLEGAL), fCoverageScale(0xff) {
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

    GrGLSLVertToFrag v(kVec4f_GrSLType);
    varyingHandler->addVarying("ConicCoeffs", &v, kHigh_GrSLPrecision);
    vertBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inConicCoeffs()->fName);

    GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    if (!gp.colorIgnored()) {
        this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);
    }

    // Setup position
    this->setupPosition(vertBuilder,
                        uniformHandler,
                        gpArgs,
                        gp.inPosition()->fName,
                        gp.viewMatrix(),
                        &fViewMatrixUniform);

    // emit transforms with position
    this->emitTransforms(vertBuilder,
                         varyingHandler,
                         uniformHandler,
                         gpArgs->fPositionVar,
                         gp.inPosition()->fName,
                         gp.localMatrix(),
                         args.fTransformsIn,
                         args.fTransformsOut);

    // TODO: this precision check should actually be a check on the number of bits
    // high and medium provide and the selection of the lowest level that suffices.
    // Additionally we should assert that the upstream code only lets us get here if
    // either high or medium provides the required number of bits.
    GrSLPrecision precision = kHigh_GrSLPrecision;
    const GrShaderCaps::PrecisionInfo& highP = args.fGLSLCaps->getFloatShaderPrecisionInfo(
                                                             kFragment_GrShaderType,
                                                             kHigh_GrSLPrecision);
    if (!highP.supported()) {
        precision = kMedium_GrSLPrecision;
    }

    GrGLSLShaderVar edgeAlpha("edgeAlpha", kFloat_GrSLType, 0, precision);
    GrGLSLShaderVar dklmdx("dklmdx", kVec3f_GrSLType, 0, precision);
    GrGLSLShaderVar dklmdy("dklmdy", kVec3f_GrSLType, 0, precision);
    GrGLSLShaderVar dfdx("dfdx", kFloat_GrSLType, 0, precision);
    GrGLSLShaderVar dfdy("dfdy", kFloat_GrSLType, 0, precision);
    GrGLSLShaderVar gF("gF", kVec2f_GrSLType, 0, precision);
    GrGLSLShaderVar gFM("gFM", kFloat_GrSLType, 0, precision);
    GrGLSLShaderVar func("func", kFloat_GrSLType, 0, precision);

    fragBuilder->declAppend(edgeAlpha);
    fragBuilder->declAppend(dklmdx);
    fragBuilder->declAppend(dklmdy);
    fragBuilder->declAppend(dfdx);
    fragBuilder->declAppend(dfdy);
    fragBuilder->declAppend(gF);
    fragBuilder->declAppend(gFM);
    fragBuilder->declAppend(func);

    switch (fEdgeType) {
        case kHairlineAA_GrProcessorEdgeType: {
            SkAssertResult(fragBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
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
            fragBuilder->codeAppendf("%s = vec2(%s, %s);", gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fragBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                                     gFM.c_str(), gF.c_str(), gF.c_str());
            fragBuilder->codeAppendf("%s = %s.x*%s.x - %s.y*%s.z;",
                                     func.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = abs(%s);", func.c_str(), func.c_str());
            fragBuilder->codeAppendf("%s = %s / %s;",
                                     edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fragBuilder->codeAppendf("%s = max(1.0 - %s, 0.0);",
                                     edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillAA_GrProcessorEdgeType: {
            SkAssertResult(fragBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
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
            fragBuilder->codeAppendf("%s = vec2(%s, %s);", gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fragBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                                     gFM.c_str(), gF.c_str(), gF.c_str());
            fragBuilder->codeAppendf("%s = %s.x * %s.x - %s.y * %s.z;",
                                     func.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = %s / %s;",
                                     edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fragBuilder->codeAppendf("%s = clamp(1.0 - %s, 0.0, 1.0);",
                                     edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillBW_GrProcessorEdgeType: {
            fragBuilder->codeAppendf("%s = %s.x * %s.x - %s.y * %s.z;",
                                     edgeAlpha.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = float(%s < 0.0);",
                                     edgeAlpha.c_str(), edgeAlpha.c_str());
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    // TODO should we really be doing this?
    if (gp.coverageScale() != 0xff) {
        const char* coverageScale;
        fCoverageScaleUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                           kFloat_GrSLType,
                                                           kHigh_GrSLPrecision,
                                                           "Coverage",
                                                           &coverageScale);
        fragBuilder->codeAppendf("%s = vec4(%s * %s);",
                                 args.fOutputCoverage, coverageScale, edgeAlpha.c_str());
    } else {
        fragBuilder->codeAppendf("%s = vec4(%s);", args.fOutputCoverage, edgeAlpha.c_str());
    }
}

void GrGLConicEffect::GenKey(const GrGeometryProcessor& gp,
                             const GrGLSLCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrConicEffect& ce = gp.cast<GrConicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    key |= GrColor_ILLEGAL != ce.color() ? 0x4 : 0x0;
    key |= 0xff != ce.coverageScale() ? 0x8 : 0x0;
    key |= ce.usesLocalCoords() && ce.localMatrix().hasPerspective() ? 0x10 : 0x0;
    key |= ComputePosKey(ce.viewMatrix()) << 5;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrConicEffect::~GrConicEffect() {}

void GrConicEffect::getGLSLProcessorKey(const GrGLSLCaps& caps,
                                        GrProcessorKeyBuilder* b) const {
    GrGLConicEffect::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrConicEffect::createGLSLInstance(const GrGLSLCaps&) const {
    return new GrGLConicEffect(*this);
}

GrConicEffect::GrConicEffect(GrColor color, const SkMatrix& viewMatrix, uint8_t coverage,
                             GrPrimitiveEdgeType edgeType, const SkMatrix& localMatrix,
                             bool usesLocalCoords)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fLocalMatrix(viewMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fCoverageScale(coverage)
    , fEdgeType(edgeType) {
    this->initClassID<GrConicEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                   kHigh_GrSLPrecision));
    fInConicCoeffs = &this->addVertexAttrib(Attribute("inConicCoeffs",
                                                      kVec4f_GrVertexAttribType));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrConicEffect);

sk_sp<GrGeometryProcessor> GrConicEffect::TestCreate(GrProcessorTestData* d) {
    sk_sp<GrGeometryProcessor> gp;
    do {
        GrPrimitiveEdgeType edgeType =
                static_cast<GrPrimitiveEdgeType>(
                        d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt));
        gp = GrConicEffect::Make(GrRandomColor(d->fRandom), GrTest::TestMatrix(d->fRandom),
                                 edgeType, *d->fCaps,
                                 GrTest::TestMatrix(d->fRandom), d->fRandom->nextBool());
    } while (nullptr == gp);
    return gp;
}

//////////////////////////////////////////////////////////////////////////////
// Quad
//////////////////////////////////////////////////////////////////////////////

class GrGLQuadEffect : public GrGLSLGeometryProcessor {
public:
    GrGLQuadEffect(const GrGeometryProcessor&);

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder*);

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const GrQuadEffect& qe = primProc.cast<GrQuadEffect>();

        if (!qe.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(qe.viewMatrix())) {
            fViewMatrix = qe.viewMatrix();
            float viewMatrix[3 * 3];
            GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
            pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
        }

        if (qe.color() != fColor) {
            float c[4];
            GrColorToRGBAFloat(qe.color(), c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = qe.color();
        }

        if (qe.coverageScale() != 0xff && qe.coverageScale() != fCoverageScale) {
            pdman.set1f(fCoverageScaleUniform, GrNormalizeByteToFloat(qe.coverageScale()));
            fCoverageScale = qe.coverageScale();
        }
    }

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          const GrGLSLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataHelper<GrQuadEffect>(primProc, pdman, index, transforms);
    }

private:
    SkMatrix fViewMatrix;
    GrColor fColor;
    uint8_t fCoverageScale;
    GrPrimitiveEdgeType fEdgeType;
    UniformHandle fColorUniform;
    UniformHandle fCoverageScaleUniform;
    UniformHandle fViewMatrixUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLQuadEffect::GrGLQuadEffect(const GrGeometryProcessor& processor)
    : fViewMatrix(SkMatrix::InvalidMatrix()), fColor(GrColor_ILLEGAL), fCoverageScale(0xff) {
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

    GrGLSLVertToFrag v(kVec4f_GrSLType);
    varyingHandler->addVarying("HairQuadEdge", &v);
    vertBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inHairQuadEdge()->fName);

    GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    if (!gp.colorIgnored()) {
        this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);
    }

    // Setup position
    this->setupPosition(vertBuilder,
                        uniformHandler,
                        gpArgs,
                        gp.inPosition()->fName,
                        gp.viewMatrix(),
                        &fViewMatrixUniform);

    // emit transforms with position
    this->emitTransforms(vertBuilder,
                         varyingHandler,
                         uniformHandler,
                         gpArgs->fPositionVar,
                         gp.inPosition()->fName,
                         gp.localMatrix(),
                         args.fTransformsIn,
                         args.fTransformsOut);

    fragBuilder->codeAppendf("float edgeAlpha;");

    switch (fEdgeType) {
        case kHairlineAA_GrProcessorEdgeType: {
            SkAssertResult(fragBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fragBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", v.fsIn());
            fragBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", v.fsIn());
            fragBuilder->codeAppendf("vec2 gF = vec2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                     "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                     v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);",
                                     v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppend("edgeAlpha = sqrt(edgeAlpha * edgeAlpha / dot(gF, gF));");
            fragBuilder->codeAppend("edgeAlpha = max(1.0 - edgeAlpha, 0.0);");
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillAA_GrProcessorEdgeType: {
            SkAssertResult(fragBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fragBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", v.fsIn());
            fragBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", v.fsIn());
            fragBuilder->codeAppendf("vec2 gF = vec2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                     "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                     v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);",
                                     v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppend("edgeAlpha = edgeAlpha / sqrt(dot(gF, gF));");
            fragBuilder->codeAppend("edgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);");
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillBW_GrProcessorEdgeType: {
            fragBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);",
                                     v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppend("edgeAlpha = float(edgeAlpha < 0.0);");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    if (0xff != gp.coverageScale()) {
        const char* coverageScale;
        fCoverageScaleUniform = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                           kFloat_GrSLType,
                                                           kDefault_GrSLPrecision,
                                                           "Coverage",
                                                           &coverageScale);
        fragBuilder->codeAppendf("%s = vec4(%s * edgeAlpha);", args.fOutputCoverage, coverageScale);
    } else {
        fragBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
    }
}

void GrGLQuadEffect::GenKey(const GrGeometryProcessor& gp,
                            const GrGLSLCaps&,
                            GrProcessorKeyBuilder* b) {
    const GrQuadEffect& ce = gp.cast<GrQuadEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    key |= ce.color() != GrColor_ILLEGAL ? 0x4 : 0x0;
    key |= ce.coverageScale() != 0xff ? 0x8 : 0x0;
    key |= ce.usesLocalCoords() && ce.localMatrix().hasPerspective() ? 0x10 : 0x0;
    key |= ComputePosKey(ce.viewMatrix()) << 5;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrQuadEffect::~GrQuadEffect() {}

void GrQuadEffect::getGLSLProcessorKey(const GrGLSLCaps& caps,
                                       GrProcessorKeyBuilder* b) const {
    GrGLQuadEffect::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrQuadEffect::createGLSLInstance(const GrGLSLCaps&) const {
    return new GrGLQuadEffect(*this);
}

GrQuadEffect::GrQuadEffect(GrColor color, const SkMatrix& viewMatrix, uint8_t coverage,
                           GrPrimitiveEdgeType edgeType, const SkMatrix& localMatrix,
                           bool usesLocalCoords)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fLocalMatrix(localMatrix)
    , fUsesLocalCoords(usesLocalCoords)
    , fCoverageScale(coverage)
    , fEdgeType(edgeType) {
    this->initClassID<GrQuadEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                   kHigh_GrSLPrecision));
    fInHairQuadEdge = &this->addVertexAttrib(Attribute("inHairQuadEdge",
                                                        kVec4f_GrVertexAttribType));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrQuadEffect);

sk_sp<GrGeometryProcessor> GrQuadEffect::TestCreate(GrProcessorTestData* d) {
    sk_sp<GrGeometryProcessor> gp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt));
        gp = GrQuadEffect::Make(GrRandomColor(d->fRandom),
                                GrTest::TestMatrix(d->fRandom),
                                edgeType, *d->fCaps,
                                GrTest::TestMatrix(d->fRandom),
                                d->fRandom->nextBool());
    } while (nullptr == gp);
    return gp;
}

//////////////////////////////////////////////////////////////////////////////
// Cubic
//////////////////////////////////////////////////////////////////////////////

class GrGLCubicEffect : public GrGLSLGeometryProcessor {
public:
    GrGLCubicEffect(const GrGeometryProcessor&);

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder*);

    void setData(const GrGLSLProgramDataManager& pdman,
                 const GrPrimitiveProcessor& primProc) override {
        const GrCubicEffect& ce = primProc.cast<GrCubicEffect>();

        if (!ce.viewMatrix().isIdentity() && !fViewMatrix.cheapEqualTo(ce.viewMatrix())) {
            fViewMatrix = ce.viewMatrix();
            float viewMatrix[3 * 3];
            GrGLSLGetMatrix<3>(viewMatrix, fViewMatrix);
            pdman.setMatrix3f(fViewMatrixUniform, viewMatrix);
        }

        if (ce.color() != fColor) {
            float c[4];
            GrColorToRGBAFloat(ce.color(), c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = ce.color();
        }
    }

private:
    SkMatrix fViewMatrix;
    GrColor fColor;
    GrPrimitiveEdgeType fEdgeType;
    UniformHandle fColorUniform;
    UniformHandle fViewMatrixUniform;

    typedef GrGLSLGeometryProcessor INHERITED;
};

GrGLCubicEffect::GrGLCubicEffect(const GrGeometryProcessor& processor)
    : fViewMatrix(SkMatrix::InvalidMatrix()), fColor(GrColor_ILLEGAL) {
    const GrCubicEffect& ce = processor.cast<GrCubicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLCubicEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLSLVertexBuilder* vertBuilder = args.fVertBuilder;
    const GrCubicEffect& gp = args.fGP.cast<GrCubicEffect>();
    GrGLSLVaryingHandler* varyingHandler = args.fVaryingHandler;
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

    // emit attributes
    varyingHandler->emitAttributes(gp);

    GrGLSLVertToFrag v(kVec4f_GrSLType);
    varyingHandler->addVarying("CubicCoeffs", &v, kHigh_GrSLPrecision);
    vertBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inCubicCoeffs()->fName);

    GrGLSLPPFragmentBuilder* fragBuilder = args.fFragBuilder;
    // Setup pass through color
    if (!gp.colorIgnored()) {
        this->setupUniformColor(fragBuilder, uniformHandler, args.fOutputColor, &fColorUniform);
    }

    // Setup position
    this->setupPosition(vertBuilder,
                        uniformHandler,
                        gpArgs,
                        gp.inPosition()->fName,
                        gp.viewMatrix(),
                        &fViewMatrixUniform);

    // emit transforms with position
    this->emitTransforms(vertBuilder,
                         varyingHandler,
                         uniformHandler,
                         gpArgs->fPositionVar,
                         gp.inPosition()->fName,
                         args.fTransformsIn,
                         args.fTransformsOut);


    GrGLSLShaderVar edgeAlpha("edgeAlpha", kFloat_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLSLShaderVar dklmdx("dklmdx", kVec3f_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLSLShaderVar dklmdy("dklmdy", kVec3f_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLSLShaderVar dfdx("dfdx", kFloat_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLSLShaderVar dfdy("dfdy", kFloat_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLSLShaderVar gF("gF", kVec2f_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLSLShaderVar gFM("gFM", kFloat_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLSLShaderVar func("func", kFloat_GrSLType, 0, kHigh_GrSLPrecision);

    fragBuilder->declAppend(edgeAlpha);
    fragBuilder->declAppend(dklmdx);
    fragBuilder->declAppend(dklmdy);
    fragBuilder->declAppend(dfdx);
    fragBuilder->declAppend(dfdy);
    fragBuilder->declAppend(gF);
    fragBuilder->declAppend(gFM);
    fragBuilder->declAppend(func);

    switch (fEdgeType) {
        case kHairlineAA_GrProcessorEdgeType: {
            SkAssertResult(fragBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fragBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), v.fsIn());
            fragBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), v.fsIn());
            fragBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                     dfdx.c_str(), v.fsIn(), v.fsIn(), dklmdx.c_str(), v.fsIn(),
                                     dklmdx.c_str(), v.fsIn(), dklmdx.c_str());
            fragBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                     dfdy.c_str(), v.fsIn(), v.fsIn(), dklmdy.c_str(), v.fsIn(),
                                     dklmdy.c_str(), v.fsIn(), dklmdy.c_str());
            fragBuilder->codeAppendf("%s = vec2(%s, %s);", gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fragBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                                     gFM.c_str(), gF.c_str(), gF.c_str());
            fragBuilder->codeAppendf("%s = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                     func.c_str(), v.fsIn(), v.fsIn(),
                                     v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = abs(%s);", func.c_str(), func.c_str());
            fragBuilder->codeAppendf("%s = %s / %s;",
                                     edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fragBuilder->codeAppendf("%s = max(1.0 - %s, 0.0);",
                                     edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppendf("%s = %s * %s * (3.0 - 2.0 * %s);",
            //                        edgeAlpha.c_str(), edgeAlpha.c_str(), edgeAlpha.c_str(),
            //                        edgeAlpha.c_str());
            break;
        }
        case kFillAA_GrProcessorEdgeType: {
            SkAssertResult(fragBuilder->enableFeature(
                    GrGLSLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fragBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), v.fsIn());
            fragBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), v.fsIn());
            fragBuilder->codeAppendf("%s ="
                                     "3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                     dfdx.c_str(), v.fsIn(), v.fsIn(), dklmdx.c_str(), v.fsIn(),
                                     dklmdx.c_str(), v.fsIn(), dklmdx.c_str());
            fragBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                     dfdy.c_str(), v.fsIn(), v.fsIn(), dklmdy.c_str(), v.fsIn(),
                                     dklmdy.c_str(), v.fsIn(), dklmdy.c_str());
            fragBuilder->codeAppendf("%s = vec2(%s, %s);", gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fragBuilder->codeAppendf("%s = sqrt(dot(%s, %s));",
                                     gFM.c_str(), gF.c_str(), gF.c_str());
            fragBuilder->codeAppendf("%s = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                     func.c_str(),
                                     v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = %s / %s;",
                                     edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fragBuilder->codeAppendf("%s = clamp(1.0 - %s, 0.0, 1.0);",
                                     edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fragBuilder->codeAppendf("%s = %s * %s * (3.0 - 2.0 * %s);",
            //                        edgeAlpha.c_str(), edgeAlpha.c_str(), edgeAlpha.c_str(),
            //                        edgeAlpha.c_str());
            break;
        }
        case kFillBW_GrProcessorEdgeType: {
            fragBuilder->codeAppendf("%s = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                     edgeAlpha.c_str(), v.fsIn(), v.fsIn(),
                                     v.fsIn(), v.fsIn(), v.fsIn());
            fragBuilder->codeAppendf("%s = float(%s < 0.0);", edgeAlpha.c_str(), edgeAlpha.c_str());
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }


    fragBuilder->codeAppendf("%s = vec4(%s);", args.fOutputCoverage, edgeAlpha.c_str());
}

void GrGLCubicEffect::GenKey(const GrGeometryProcessor& gp,
                             const GrGLSLCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrCubicEffect& ce = gp.cast<GrCubicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    key |= ce.color() != GrColor_ILLEGAL ? 0x4 : 0x8;
    key |= ComputePosKey(ce.viewMatrix()) << 5;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrCubicEffect::~GrCubicEffect() {}

void GrCubicEffect::getGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GrGLCubicEffect::GenKey(*this, caps, b);
}

GrGLSLPrimitiveProcessor* GrCubicEffect::createGLSLInstance(const GrGLSLCaps&) const {
    return new GrGLCubicEffect(*this);
}

GrCubicEffect::GrCubicEffect(GrColor color, const SkMatrix& viewMatrix,
                             GrPrimitiveEdgeType edgeType)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fEdgeType(edgeType) {
    this->initClassID<GrCubicEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                   kHigh_GrSLPrecision));
    fInCubicCoeffs = &this->addVertexAttrib(Attribute("inCubicCoeffs",
                                                        kVec4f_GrVertexAttribType));
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrCubicEffect);

sk_sp<GrGeometryProcessor> GrCubicEffect::TestCreate(GrProcessorTestData* d) {
    sk_sp<GrGeometryProcessor> gp;
    do {
        GrPrimitiveEdgeType edgeType =
                static_cast<GrPrimitiveEdgeType>(
                        d->fRandom->nextULessThan(kGrProcessorEdgeTypeCnt));
        gp = GrCubicEffect::Make(GrRandomColor(d->fRandom),
                                 GrTest::TestMatrix(d->fRandom), edgeType, *d->fCaps);
    } while (nullptr == gp);
    return gp;
}
