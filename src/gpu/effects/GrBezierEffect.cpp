/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBezierEffect.h"

#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

struct ConicBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    uint8_t fCoverageScale;
    bool fUsesLocalCoords;
};

class GrGLConicEffect : public GrGLGeometryProcessor {
public:
    GrGLConicEffect(const GrGeometryProcessor&,
                    const GrBatchTracker&);

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrBatchTracker&,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& primProc,
                         const GrBatchTracker& bt) override {
        const GrConicEffect& ce = primProc.cast<GrConicEffect>();
        this->setUniformViewMatrix(pdman, ce.viewMatrix());

        const ConicBatchTracker& local = bt.cast<ConicBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
        if (0xff != local.fCoverageScale && fCoverageScale != local.fCoverageScale) {
            pdman.set1f(fCoverageScaleUniform, GrNormalizeByteToFloat(local.fCoverageScale));
            fCoverageScale = local.fCoverageScale;
        }
    }

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          const GrGLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataHelper<GrConicEffect>(primProc, pdman, index, transforms);
    }

private:
    GrColor fColor;
    uint8_t fCoverageScale;
    GrPrimitiveEdgeType fEdgeType;
    UniformHandle fColorUniform;
    UniformHandle fCoverageScaleUniform;

    typedef GrGLGeometryProcessor INHERITED;
};

GrGLConicEffect::GrGLConicEffect(const GrGeometryProcessor& processor,
                                 const GrBatchTracker& bt)
    : fColor(GrColor_ILLEGAL), fCoverageScale(0xff) {
    const GrConicEffect& ce = processor.cast<GrConicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLConicEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLGPBuilder* pb = args.fPB;
    GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();
    const GrConicEffect& gp = args.fGP.cast<GrConicEffect>();
    const ConicBatchTracker& local = args.fBT.cast<ConicBatchTracker>();

    // emit attributes
    vsBuilder->emitAttributes(gp);

    GrGLVertToFrag v(kVec4f_GrSLType);
    args.fPB->addVarying("ConicCoeffs", &v);
    vsBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inConicCoeffs()->fName);

    // Setup pass through color
    this->setupColorPassThrough(args.fPB, local.fInputColorType, args.fOutputColor, NULL,
                                &fColorUniform);

    // Setup position
    this->setupPosition(pb, gpArgs, gp.inPosition()->fName, gp.viewMatrix());

    // emit transforms with position
    this->emitTransforms(pb, gpArgs->fPositionVar, gp.inPosition()->fName, gp.localMatrix(),
                         args.fTransformsIn, args.fTransformsOut);

    GrGLFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
    fsBuilder->codeAppend("float edgeAlpha;");

    switch (fEdgeType) {
        case kHairlineAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec3 dklmdx = dFdx(%s.xyz);", v.fsIn());
            fsBuilder->codeAppendf("vec3 dklmdy = dFdy(%s.xyz);", v.fsIn());
            fsBuilder->codeAppendf("float dfdx ="
                                   "2.0 * %s.x * dklmdx.x - %s.y * dklmdx.z - %s.z * dklmdx.y;",
                                   v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppendf("float dfdy ="
                                   "2.0 * %s.x * dklmdy.x - %s.y * dklmdy.z - %s.z * dklmdy.y;",
                                   v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppend("vec2 gF = vec2(dfdx, dfdy);");
            fsBuilder->codeAppend("float gFM = sqrt(dot(gF, gF));");
            fsBuilder->codeAppendf("float func = %s.x*%s.x - %s.y*%s.z;", v.fsIn(), v.fsIn(),
                                   v.fsIn(), v.fsIn());
            fsBuilder->codeAppend("func = abs(func);");
            fsBuilder->codeAppend("edgeAlpha = func / gFM;");
            fsBuilder->codeAppend("edgeAlpha = max(1.0 - edgeAlpha, 0.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec3 dklmdx = dFdx(%s.xyz);", v.fsIn());
            fsBuilder->codeAppendf("vec3 dklmdy = dFdy(%s.xyz);", v.fsIn());
            fsBuilder->codeAppendf("float dfdx ="
                                   "2.0 * %s.x * dklmdx.x - %s.y * dklmdx.z - %s.z * dklmdx.y;",
                                   v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppendf("float dfdy ="
                                   "2.0 * %s.x * dklmdy.x - %s.y * dklmdy.z - %s.z * dklmdy.y;",
                                   v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppend("vec2 gF = vec2(dfdx, dfdy);");
            fsBuilder->codeAppend("float gFM = sqrt(dot(gF, gF));");
            fsBuilder->codeAppendf("float func = %s.x * %s.x - %s.y * %s.z;", v.fsIn(), v.fsIn(),
                                   v.fsIn(), v.fsIn());
            fsBuilder->codeAppend("edgeAlpha = func / gFM;");
            fsBuilder->codeAppend("edgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillBW_GrProcessorEdgeType: {
            fsBuilder->codeAppendf("edgeAlpha = %s.x * %s.x - %s.y * %s.z;", v.fsIn(), v.fsIn(),
                                   v.fsIn(), v.fsIn());
            fsBuilder->codeAppend("edgeAlpha = float(edgeAlpha < 0.0);");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    if (0xff != local.fCoverageScale) {
        const char* coverageScale;
        fCoverageScaleUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                               kFloat_GrSLType,
                                               kDefault_GrSLPrecision,
                                               "Coverage",
                                               &coverageScale);
        fsBuilder->codeAppendf("%s = vec4(%s * edgeAlpha);", args.fOutputCoverage, coverageScale);
    } else {
        fsBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
    }
}

void GrGLConicEffect::GenKey(const GrGeometryProcessor& gp,
                             const GrBatchTracker& bt,
                             const GrGLSLCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrConicEffect& ce = gp.cast<GrConicEffect>();
    const ConicBatchTracker& local = bt.cast<ConicBatchTracker>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    key |= kUniform_GrGPInput == local.fInputColorType ? 0x4 : 0x0;
    key |= 0xff != local.fCoverageScale ? 0x8 : 0x0;
    key |= local.fUsesLocalCoords && ce.localMatrix().hasPerspective() ? 0x10 : 0x0;
    key |= ComputePosKey(ce.viewMatrix()) << 5;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrConicEffect::~GrConicEffect() {}

void GrConicEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                      const GrGLSLCaps& caps,
                                      GrProcessorKeyBuilder* b) const {
    GrGLConicEffect::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor* GrConicEffect::createGLInstance(const GrBatchTracker& bt,
                                                        const GrGLSLCaps&) const {
    return SkNEW_ARGS(GrGLConicEffect, (*this, bt));
}

GrConicEffect::GrConicEffect(GrColor color, const SkMatrix& viewMatrix, uint8_t coverage,
                             GrPrimitiveEdgeType edgeType, const SkMatrix& localMatrix)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fLocalMatrix(viewMatrix)
    , fCoverageScale(coverage)
    , fEdgeType(edgeType) {
    this->initClassID<GrConicEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                   kHigh_GrSLPrecision));
    fInConicCoeffs = &this->addVertexAttrib(Attribute("inConicCoeffs",
                                                        kVec4f_GrVertexAttribType));
}

void GrConicEffect::initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const {
    ConicBatchTracker* local = bt->cast<ConicBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
    local->fCoverageScale = fCoverageScale;
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrConicEffect);

GrGeometryProcessor* GrConicEffect::TestCreate(SkRandom* random,
                                               GrContext*,
                                               const GrDrawTargetCaps& caps,
                                               GrTexture*[]) {
    GrGeometryProcessor* gp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                                                    random->nextULessThan(kGrProcessorEdgeTypeCnt));
        gp = GrConicEffect::Create(GrRandomColor(random), GrTest::TestMatrix(random),
                                   edgeType, caps,
                                   GrTest::TestMatrix(random));
    } while (NULL == gp);
    return gp;
}

//////////////////////////////////////////////////////////////////////////////
// Quad
//////////////////////////////////////////////////////////////////////////////

struct QuadBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    uint8_t fCoverageScale;
    bool fUsesLocalCoords;
};

class GrGLQuadEffect : public GrGLGeometryProcessor {
public:
    GrGLQuadEffect(const GrGeometryProcessor&,
                   const GrBatchTracker&);

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrBatchTracker&,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& primProc,
                         const GrBatchTracker& bt) override {
        const GrQuadEffect& qe = primProc.cast<GrQuadEffect>();
        this->setUniformViewMatrix(pdman, qe.viewMatrix());

        const QuadBatchTracker& local = bt.cast<QuadBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
        if (0xff != local.fCoverageScale && local.fCoverageScale != fCoverageScale) {
            pdman.set1f(fCoverageScaleUniform, GrNormalizeByteToFloat(local.fCoverageScale));
            fCoverageScale = local.fCoverageScale;
        }
    }

    void setTransformData(const GrPrimitiveProcessor& primProc,
                          const GrGLProgramDataManager& pdman,
                          int index,
                          const SkTArray<const GrCoordTransform*, true>& transforms) override {
        this->setTransformDataHelper<GrQuadEffect>(primProc, pdman, index, transforms);
    }

private:
    GrColor fColor;
    uint8_t fCoverageScale;
    GrPrimitiveEdgeType fEdgeType;
    UniformHandle fColorUniform;
    UniformHandle fCoverageScaleUniform;

    typedef GrGLGeometryProcessor INHERITED;
};

GrGLQuadEffect::GrGLQuadEffect(const GrGeometryProcessor& processor,
                               const GrBatchTracker& bt)
    : fColor(GrColor_ILLEGAL), fCoverageScale(0xff) {
    const GrQuadEffect& ce = processor.cast<GrQuadEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLQuadEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLGPBuilder* pb = args.fPB;
    GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();
    const GrQuadEffect& gp = args.fGP.cast<GrQuadEffect>();
    const QuadBatchTracker& local = args.fBT.cast<QuadBatchTracker>();

    // emit attributes
    vsBuilder->emitAttributes(gp);

    GrGLVertToFrag v(kVec4f_GrSLType);
    args.fPB->addVarying("HairQuadEdge", &v);
    vsBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inHairQuadEdge()->fName);

    // Setup pass through color
    this->setupColorPassThrough(args.fPB, local.fInputColorType, args.fOutputColor, NULL,
                                &fColorUniform);

    // Setup position
    this->setupPosition(pb, gpArgs, gp.inPosition()->fName, gp.viewMatrix());

    // emit transforms with position
    this->emitTransforms(pb, gpArgs->fPositionVar, gp.inPosition()->fName, gp.localMatrix(),
                         args.fTransformsIn, args.fTransformsOut);

    GrGLFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
    fsBuilder->codeAppendf("float edgeAlpha;");

    switch (fEdgeType) {
        case kHairlineAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", v.fsIn());
            fsBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", v.fsIn());
            fsBuilder->codeAppendf("vec2 gF = vec2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                   "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                   v.fsIn(), v.fsIn());
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppend("edgeAlpha = sqrt(edgeAlpha * edgeAlpha / dot(gF, gF));");
            fsBuilder->codeAppend("edgeAlpha = max(1.0 - edgeAlpha, 0.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", v.fsIn());
            fsBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", v.fsIn());
            fsBuilder->codeAppendf("vec2 gF = vec2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                   "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                   v.fsIn(), v.fsIn());
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppend("edgeAlpha = edgeAlpha / sqrt(dot(gF, gF));");
            fsBuilder->codeAppend("edgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillBW_GrProcessorEdgeType: {
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppend("edgeAlpha = float(edgeAlpha < 0.0);");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    if (0xff != local.fCoverageScale) {
        const char* coverageScale;
        fCoverageScaleUniform = pb->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                          kFloat_GrSLType,
                                          kDefault_GrSLPrecision,
                                          "Coverage",
                                          &coverageScale);
        fsBuilder->codeAppendf("%s = vec4(%s * edgeAlpha);", args.fOutputCoverage, coverageScale);
    } else {
        fsBuilder->codeAppendf("%s = vec4(edgeAlpha);", args.fOutputCoverage);
    }
}

void GrGLQuadEffect::GenKey(const GrGeometryProcessor& gp,
                            const GrBatchTracker& bt,
                            const GrGLSLCaps&,
                            GrProcessorKeyBuilder* b) {
    const GrQuadEffect& ce = gp.cast<GrQuadEffect>();
    const QuadBatchTracker& local = bt.cast<QuadBatchTracker>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    key |= kUniform_GrGPInput == local.fInputColorType ? 0x4 : 0x0;
    key |= 0xff != local.fCoverageScale ? 0x8 : 0x0;
    key |= local.fUsesLocalCoords && ce.localMatrix().hasPerspective() ? 0x10 : 0x0;
    key |= ComputePosKey(ce.viewMatrix()) << 5;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrQuadEffect::~GrQuadEffect() {}

void GrQuadEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                     const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const {
    GrGLQuadEffect::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor* GrQuadEffect::createGLInstance(const GrBatchTracker& bt,
                                                       const GrGLSLCaps&) const {
    return SkNEW_ARGS(GrGLQuadEffect, (*this, bt));
}

GrQuadEffect::GrQuadEffect(GrColor color, const SkMatrix& viewMatrix, uint8_t coverage,
                           GrPrimitiveEdgeType edgeType, const SkMatrix& localMatrix)
    : fColor(color)
    , fViewMatrix(viewMatrix)
    , fLocalMatrix(localMatrix)
    , fCoverageScale(coverage)
    , fEdgeType(edgeType) {
    this->initClassID<GrQuadEffect>();
    fInPosition = &this->addVertexAttrib(Attribute("inPosition", kVec2f_GrVertexAttribType,
                                                   kHigh_GrSLPrecision));
    fInHairQuadEdge = &this->addVertexAttrib(Attribute("inHairQuadEdge",
                                                        kVec4f_GrVertexAttribType));
}

void GrQuadEffect::initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const {
    QuadBatchTracker* local = bt->cast<QuadBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
    local->fCoverageScale = fCoverageScale;
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrQuadEffect);

GrGeometryProcessor* GrQuadEffect::TestCreate(SkRandom* random,
                                              GrContext*,
                                              const GrDrawTargetCaps& caps,
                                              GrTexture*[]) {
    GrGeometryProcessor* gp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                random->nextULessThan(kGrProcessorEdgeTypeCnt));
        gp = GrQuadEffect::Create(GrRandomColor(random),
                                  GrTest::TestMatrix(random),
                                  edgeType, caps,
                                  GrTest::TestMatrix(random));
    } while (NULL == gp);
    return gp;
}

//////////////////////////////////////////////////////////////////////////////
// Cubic
//////////////////////////////////////////////////////////////////////////////

struct CubicBatchTracker {
    GrGPInput fInputColorType;
    GrColor fColor;
    bool fUsesLocalCoords;
};

class GrGLCubicEffect : public GrGLGeometryProcessor {
public:
    GrGLCubicEffect(const GrGeometryProcessor&,
                    const GrBatchTracker&);

    void onEmitCode(EmitArgs&, GrGPArgs*) override;

    static inline void GenKey(const GrGeometryProcessor&,
                              const GrBatchTracker&,
                              const GrGLSLCaps&,
                              GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager& pdman,
                         const GrPrimitiveProcessor& primProc,
                         const GrBatchTracker& bt) override {
        const GrCubicEffect& ce = primProc.cast<GrCubicEffect>();
        this->setUniformViewMatrix(pdman, ce.viewMatrix());

        const CubicBatchTracker& local = bt.cast<CubicBatchTracker>();
        if (kUniform_GrGPInput == local.fInputColorType && local.fColor != fColor) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(local.fColor, c);
            pdman.set4fv(fColorUniform, 1, c);
            fColor = local.fColor;
        }
    }

private:
    GrColor fColor;
    GrPrimitiveEdgeType fEdgeType;
    UniformHandle fColorUniform;

    typedef GrGLGeometryProcessor INHERITED;
};

GrGLCubicEffect::GrGLCubicEffect(const GrGeometryProcessor& processor,
                                 const GrBatchTracker&)
    : fColor(GrColor_ILLEGAL) {
    const GrCubicEffect& ce = processor.cast<GrCubicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLCubicEffect::onEmitCode(EmitArgs& args, GrGPArgs* gpArgs) {
    GrGLVertexBuilder* vsBuilder = args.fPB->getVertexShaderBuilder();
    const GrCubicEffect& gp = args.fGP.cast<GrCubicEffect>();
    const CubicBatchTracker& local = args.fBT.cast<CubicBatchTracker>();

    // emit attributes
    vsBuilder->emitAttributes(gp);

    GrGLVertToFrag v(kVec4f_GrSLType);
    args.fPB->addVarying("CubicCoeffs", &v, kHigh_GrSLPrecision);
    vsBuilder->codeAppendf("%s = %s;", v.vsOut(), gp.inCubicCoeffs()->fName);

    // Setup pass through color
    this->setupColorPassThrough(args.fPB, local.fInputColorType, args.fOutputColor, NULL,
                                &fColorUniform);

    // Setup position
    this->setupPosition(args.fPB, gpArgs, gp.inPosition()->fName, gp.viewMatrix());

    // emit transforms with position
    this->emitTransforms(args.fPB, gpArgs->fPositionVar, gp.inPosition()->fName, args.fTransformsIn,
                         args.fTransformsOut);

    GrGLFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();

    GrGLShaderVar edgeAlpha("edgeAlpha", kFloat_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLShaderVar dklmdx("dklmdx", kVec3f_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLShaderVar dklmdy("dklmdy", kVec3f_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLShaderVar dfdx("dfdx", kFloat_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLShaderVar dfdy("dfdy", kFloat_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLShaderVar gF("gF", kVec2f_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLShaderVar gFM("gFM", kFloat_GrSLType, 0, kHigh_GrSLPrecision);
    GrGLShaderVar func("func", kFloat_GrSLType, 0, kHigh_GrSLPrecision);

    fsBuilder->declAppend(edgeAlpha);
    fsBuilder->declAppend(dklmdx);
    fsBuilder->declAppend(dklmdy);
    fsBuilder->declAppend(dfdx);
    fsBuilder->declAppend(dfdy);
    fsBuilder->declAppend(gF);
    fsBuilder->declAppend(gFM);
    fsBuilder->declAppend(func);

    switch (fEdgeType) {
        case kHairlineAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), v.fsIn());
            fsBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), v.fsIn());
            fsBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdx.c_str(), v.fsIn(), v.fsIn(), dklmdx.c_str(), v.fsIn(),
                                   dklmdx.c_str(), v.fsIn(), dklmdx.c_str());
            fsBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdy.c_str(), v.fsIn(), v.fsIn(), dklmdy.c_str(), v.fsIn(),
                                   dklmdy.c_str(), v.fsIn(), dklmdy.c_str());
            fsBuilder->codeAppendf("%s = vec2(%s, %s);", gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fsBuilder->codeAppendf("%s = sqrt(dot(%s, %s));", gFM.c_str(), gF.c_str(), gF.c_str());
            fsBuilder->codeAppendf("%s = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                   func.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppendf("%s = abs(%s);", func.c_str(), func.c_str());
            fsBuilder->codeAppendf("%s = %s / %s;",
                                   edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fsBuilder->codeAppendf("%s = max(1.0 - %s, 0.0);",
                                   edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppendf("%s = %s * %s * (3.0 - 2.0 * %s);",
            //                        edgeAlpha.c_str(), edgeAlpha.c_str(), edgeAlpha.c_str(),
            //                        edgeAlpha.c_str());
            break;
        }
        case kFillAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), v.fsIn());
            fsBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), v.fsIn());
            fsBuilder->codeAppendf("%s ="
                                   "3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdx.c_str(), v.fsIn(), v.fsIn(), dklmdx.c_str(), v.fsIn(),
                                   dklmdx.c_str(), v.fsIn(), dklmdx.c_str());
            fsBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdy.c_str(), v.fsIn(), v.fsIn(), dklmdy.c_str(), v.fsIn(),
                                   dklmdy.c_str(), v.fsIn(), dklmdy.c_str());
            fsBuilder->codeAppendf("%s = vec2(%s, %s);", gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fsBuilder->codeAppendf("%s = sqrt(dot(%s, %s));", gFM.c_str(), gF.c_str(), gF.c_str());
            fsBuilder->codeAppendf("%s = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                   func.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppendf("%s = %s / %s;",
                                   edgeAlpha.c_str(), func.c_str(), gFM.c_str());
            fsBuilder->codeAppendf("%s = clamp(1.0 - %s, 0.0, 1.0);",
                                   edgeAlpha.c_str(), edgeAlpha.c_str());
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppendf("%s = %s * %s * (3.0 - 2.0 * %s);",
            //                        edgeAlpha.c_str(), edgeAlpha.c_str(), edgeAlpha.c_str(),
            //                        edgeAlpha.c_str());
            break;
        }
        case kFillBW_GrProcessorEdgeType: {
            fsBuilder->codeAppendf("%s = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                   edgeAlpha.c_str(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn(), v.fsIn());
            fsBuilder->codeAppendf("%s = float(%s < 0.0);", edgeAlpha.c_str(), edgeAlpha.c_str());
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }


    fsBuilder->codeAppendf("%s = vec4(%s);", args.fOutputCoverage, edgeAlpha.c_str());
}

void GrGLCubicEffect::GenKey(const GrGeometryProcessor& gp,
                             const GrBatchTracker& bt,
                             const GrGLSLCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrCubicEffect& ce = gp.cast<GrCubicEffect>();
    const CubicBatchTracker& local = bt.cast<CubicBatchTracker>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    key |= kUniform_GrGPInput == local.fInputColorType ? 0x4 : 0x8;
    key |= ComputePosKey(ce.viewMatrix()) << 5;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrCubicEffect::~GrCubicEffect() {}

void GrCubicEffect::getGLProcessorKey(const GrBatchTracker& bt,
                                      const GrGLSLCaps& caps,
                                      GrProcessorKeyBuilder* b) const {
    GrGLCubicEffect::GenKey(*this, bt, caps, b);
}

GrGLPrimitiveProcessor* GrCubicEffect::createGLInstance(const GrBatchTracker& bt,
                                                        const GrGLSLCaps&) const {
    return SkNEW_ARGS(GrGLCubicEffect, (*this, bt));
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

void GrCubicEffect::initBatchTracker(GrBatchTracker* bt, const GrPipelineInfo& init) const {
    CubicBatchTracker* local = bt->cast<CubicBatchTracker>();
    local->fInputColorType = GetColorInputType(&local->fColor, this->color(), init, false);
    local->fUsesLocalCoords = init.fUsesLocalCoords;
}

//////////////////////////////////////////////////////////////////////////////

GR_DEFINE_GEOMETRY_PROCESSOR_TEST(GrCubicEffect);

GrGeometryProcessor* GrCubicEffect::TestCreate(SkRandom* random,
                                               GrContext*,
                                               const GrDrawTargetCaps& caps,
                                               GrTexture*[]) {
    GrGeometryProcessor* gp;
    do {
        GrPrimitiveEdgeType edgeType = static_cast<GrPrimitiveEdgeType>(
                                                    random->nextULessThan(kGrProcessorEdgeTypeCnt));
        gp = GrCubicEffect::Create(GrRandomColor(random),
                                   GrTest::TestMatrix(random), edgeType, caps);
    } while (NULL == gp);
    return gp;
}

