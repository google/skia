/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBezierEffect.h"

#include "gl/builders/GrGLFullProgramBuilder.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLSL.h"
#include "gl/GrGLGeometryProcessor.h"
#include "GrTBackendProcessorFactory.h"

class GrGLConicEffect : public GrGLGeometryProcessor {
public:
    GrGLConicEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {}

private:
    GrPrimitiveEdgeType fEdgeType;

    typedef GrGLGeometryProcessor INHERITED;
};

GrGLConicEffect::GrGLConicEffect(const GrBackendProcessorFactory& factory,
                                 const GrProcessor& effect)
    : INHERITED (factory) {
    const GrConicEffect& ce = effect.cast<GrConicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLConicEffect::emitCode(GrGLFullProgramBuilder* builder,
                               const GrGeometryProcessor& geometryProcessor,
                               const GrProcessorKey& key,
                               const char* outputColor,
                               const char* inputColor,
                               const TransformedCoordsArray&,
                               const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    builder->addVarying(kVec4f_GrSLType, "ConicCoeffs",
                              &vsName, &fsName);

    const GrShaderVar& inConicCoeffs = geometryProcessor.cast<GrConicEffect>().inConicCoeffs();
    GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
    vsBuilder->codeAppendf("%s = %s;", vsName, inConicCoeffs.c_str());

    GrGLProcessorFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    fsBuilder->codeAppend("float edgeAlpha;");

    switch (fEdgeType) {
        case kHairlineAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec3 dklmdx = dFdx(%s.xyz);", fsName);
            fsBuilder->codeAppendf("vec3 dklmdy = dFdy(%s.xyz);", fsName);
            fsBuilder->codeAppendf("float dfdx ="
                                   "2.0 * %s.x * dklmdx.x - %s.y * dklmdx.z - %s.z * dklmdx.y;",
                                   fsName, fsName, fsName);
            fsBuilder->codeAppendf("float dfdy ="
                                   "2.0 * %s.x * dklmdy.x - %s.y * dklmdy.z - %s.z * dklmdy.y;",
                                   fsName, fsName, fsName);
            fsBuilder->codeAppend("vec2 gF = vec2(dfdx, dfdy);");
            fsBuilder->codeAppend("float gFM = sqrt(dot(gF, gF));");
            fsBuilder->codeAppendf("float func = %s.x*%s.x - %s.y*%s.z;", fsName, fsName,
                                   fsName, fsName);
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
            fsBuilder->codeAppendf("vec3 dklmdx = dFdx(%s.xyz);", fsName);
            fsBuilder->codeAppendf("vec3 dklmdy = dFdy(%s.xyz);", fsName);
            fsBuilder->codeAppendf("float dfdx ="
                                   "2.0 * %s.x * dklmdx.x - %s.y * dklmdx.z - %s.z * dklmdx.y;",
                                   fsName, fsName, fsName);
            fsBuilder->codeAppendf("float dfdy ="
                                   "2.0 * %s.x * dklmdy.x - %s.y * dklmdy.z - %s.z * dklmdy.y;",
                                   fsName, fsName, fsName);
            fsBuilder->codeAppend("vec2 gF = vec2(dfdx, dfdy);");
            fsBuilder->codeAppend("float gFM = sqrt(dot(gF, gF));");
            fsBuilder->codeAppendf("float func = %s.x * %s.x - %s.y * %s.z;", fsName, fsName,
                                   fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = func / gFM;");
            fsBuilder->codeAppend("edgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillBW_GrProcessorEdgeType: {
            fsBuilder->codeAppendf("edgeAlpha = %s.x * %s.x - %s.y * %s.z;", fsName, fsName,
                                   fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = float(edgeAlpha < 0.0);");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    fsBuilder->codeAppendf("%s = %s;", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
}

void GrGLConicEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrConicEffect& ce = processor.cast<GrConicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrConicEffect::~GrConicEffect() {}

const GrBackendGeometryProcessorFactory& GrConicEffect::getFactory() const {
    return GrTBackendGeometryProcessorFactory<GrConicEffect>::getInstance();
}

GrConicEffect::GrConicEffect(GrPrimitiveEdgeType edgeType)
    : fEdgeType(edgeType)
    , fInConicCoeffs(this->addVertexAttrib(GrShaderVar("inConicCoeffs",
                                                       kVec4f_GrSLType,
                                                       GrShaderVar::kAttribute_TypeModifier))) {
}

bool GrConicEffect::onIsEqual(const GrProcessor& other) const {
    const GrConicEffect& ce = other.cast<GrConicEffect>();
    return (ce.fEdgeType == fEdgeType);
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
        gp = GrConicEffect::Create(edgeType, caps);
    } while (NULL == gp);
    return gp;
}

//////////////////////////////////////////////////////////////////////////////
// Quad
//////////////////////////////////////////////////////////////////////////////

class GrGLQuadEffect : public GrGLGeometryProcessor {
public:
    GrGLQuadEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {}

private:
    GrPrimitiveEdgeType fEdgeType;

    typedef GrGLGeometryProcessor INHERITED;
};

GrGLQuadEffect::GrGLQuadEffect(const GrBackendProcessorFactory& factory,
                                 const GrProcessor& effect)
    : INHERITED (factory) {
    const GrQuadEffect& ce = effect.cast<GrQuadEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLQuadEffect::emitCode(GrGLFullProgramBuilder* builder,
                              const GrGeometryProcessor& geometryProcessor,
                              const GrProcessorKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray&,
                              const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;
    builder->addVarying(kVec4f_GrSLType, "HairQuadEdge", &vsName, &fsName);

    GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
    const GrShaderVar& inHairQuadEdge = geometryProcessor.cast<GrQuadEffect>().inHairQuadEdge();
    vsBuilder->codeAppendf("%s = %s;", vsName, inHairQuadEdge.c_str());

    GrGLProcessorFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
    fsBuilder->codeAppendf("float edgeAlpha;");

    switch (fEdgeType) {
        case kHairlineAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", fsName);
            fsBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", fsName);
            fsBuilder->codeAppendf("vec2 gF = vec2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                   "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                   fsName, fsName);
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", fsName, fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = sqrt(edgeAlpha * edgeAlpha / dot(gF, gF));");
            fsBuilder->codeAppend("edgeAlpha = max(1.0 - edgeAlpha, 0.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillAA_GrProcessorEdgeType: {
            SkAssertResult(fsBuilder->enableFeature(
                    GrGLFragmentShaderBuilder::kStandardDerivatives_GLSLFeature));
            fsBuilder->codeAppendf("vec2 duvdx = dFdx(%s.xy);", fsName);
            fsBuilder->codeAppendf("vec2 duvdy = dFdy(%s.xy);", fsName);
            fsBuilder->codeAppendf("vec2 gF = vec2(2.0 * %s.x * duvdx.x - duvdx.y,"
                                   "               2.0 * %s.x * duvdy.x - duvdy.y);",
                                   fsName, fsName);
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", fsName, fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = edgeAlpha / sqrt(dot(gF, gF));");
            fsBuilder->codeAppend("edgeAlpha = clamp(1.0 - edgeAlpha, 0.0, 1.0);");
            // Add line below for smooth cubic ramp
            // fsBuilder->codeAppend("edgeAlpha = edgeAlpha*edgeAlpha*(3.0-2.0*edgeAlpha);");
            break;
        }
        case kFillBW_GrProcessorEdgeType: {
            fsBuilder->codeAppendf("edgeAlpha = (%s.x * %s.x - %s.y);", fsName, fsName, fsName);
            fsBuilder->codeAppend("edgeAlpha = float(edgeAlpha < 0.0);");
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    fsBuilder->codeAppendf("%s = %s;", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1("edgeAlpha")).c_str());
}

void GrGLQuadEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
                            GrProcessorKeyBuilder* b) {
    const GrQuadEffect& ce = processor.cast<GrQuadEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrQuadEffect::~GrQuadEffect() {}

const GrBackendGeometryProcessorFactory& GrQuadEffect::getFactory() const {
    return GrTBackendGeometryProcessorFactory<GrQuadEffect>::getInstance();
}

GrQuadEffect::GrQuadEffect(GrPrimitiveEdgeType edgeType)
    : fEdgeType(edgeType)
    , fInHairQuadEdge(this->addVertexAttrib(GrShaderVar("inCubicCoeffs",
                                                        kVec4f_GrSLType,
                                                        GrShaderVar::kAttribute_TypeModifier))) {
}

bool GrQuadEffect::onIsEqual(const GrProcessor& other) const {
    const GrQuadEffect& ce = other.cast<GrQuadEffect>();
    return (ce.fEdgeType == fEdgeType);
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
        gp = GrQuadEffect::Create(edgeType, caps);
    } while (NULL == gp);
    return gp;
}

//////////////////////////////////////////////////////////////////////////////
// Cubic
//////////////////////////////////////////////////////////////////////////////

class GrGLCubicEffect : public GrGLGeometryProcessor {
public:
    GrGLCubicEffect(const GrBackendProcessorFactory&, const GrProcessor&);

    virtual void emitCode(GrGLFullProgramBuilder* builder,
                          const GrGeometryProcessor& geometryProcessor,
                          const GrProcessorKey& key,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static inline void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*);

    virtual void setData(const GrGLProgramDataManager&, const GrProcessor&) SK_OVERRIDE {}

private:
    GrPrimitiveEdgeType fEdgeType;

    typedef GrGLGeometryProcessor INHERITED;
};

GrGLCubicEffect::GrGLCubicEffect(const GrBackendProcessorFactory& factory,
                                 const GrProcessor& processor)
    : INHERITED (factory) {
    const GrCubicEffect& ce = processor.cast<GrCubicEffect>();
    fEdgeType = ce.getEdgeType();
}

void GrGLCubicEffect::emitCode(GrGLFullProgramBuilder* builder,
                               const GrGeometryProcessor& geometryProcessor,
                               const GrProcessorKey& key,
                               const char* outputColor,
                               const char* inputColor,
                               const TransformedCoordsArray&,
                               const TextureSamplerArray& samplers) {
    const char *vsName, *fsName;

    builder->addVarying(kVec4f_GrSLType, "CubicCoeffs",
                              &vsName, &fsName, GrGLShaderVar::kHigh_Precision);

    GrGLVertexShaderBuilder* vsBuilder = builder->getVertexShaderBuilder();
    const GrShaderVar& inCubicCoeffs = geometryProcessor.cast<GrCubicEffect>().inCubicCoeffs();
    vsBuilder->codeAppendf("%s = %s;", vsName, inCubicCoeffs.c_str());

    GrGLProcessorFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();

    GrGLShaderVar edgeAlpha("edgeAlpha", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar dklmdx("dklmdx", kVec3f_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar dklmdy("dklmdy", kVec3f_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar dfdx("dfdx", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar dfdy("dfdy", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar gF("gF", kVec2f_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar gFM("gFM", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);
    GrGLShaderVar func("func", kFloat_GrSLType, 0, GrGLShaderVar::kHigh_Precision);

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
            fsBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), fsName);
            fsBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), fsName);
            fsBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdx.c_str(), fsName, fsName, dklmdx.c_str(), fsName,
                                   dklmdx.c_str(), fsName, dklmdx.c_str());
            fsBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdy.c_str(), fsName, fsName, dklmdy.c_str(), fsName,
                                   dklmdy.c_str(), fsName, dklmdy.c_str());
            fsBuilder->codeAppendf("%s = vec2(%s, %s);", gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fsBuilder->codeAppendf("%s = sqrt(dot(%s, %s));", gFM.c_str(), gF.c_str(), gF.c_str());
            fsBuilder->codeAppendf("%s = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                   func.c_str(), fsName, fsName, fsName, fsName, fsName);
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
            fsBuilder->codeAppendf("%s = dFdx(%s.xyz);", dklmdx.c_str(), fsName);
            fsBuilder->codeAppendf("%s = dFdy(%s.xyz);", dklmdy.c_str(), fsName);
            fsBuilder->codeAppendf("%s ="
                                   "3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdx.c_str(), fsName, fsName, dklmdx.c_str(), fsName,
                                   dklmdx.c_str(), fsName, dklmdx.c_str());
            fsBuilder->codeAppendf("%s = 3.0 * %s.x * %s.x * %s.x - %s.y * %s.z - %s.z * %s.y;",
                                   dfdy.c_str(), fsName, fsName, dklmdy.c_str(), fsName,
                                   dklmdy.c_str(), fsName, dklmdy.c_str());
            fsBuilder->codeAppendf("%s = vec2(%s, %s);", gF.c_str(), dfdx.c_str(), dfdy.c_str());
            fsBuilder->codeAppendf("%s = sqrt(dot(%s, %s));", gFM.c_str(), gF.c_str(), gF.c_str());
            fsBuilder->codeAppendf("%s = %s.x * %s.x * %s.x - %s.y * %s.z;",
                                   func.c_str(), fsName, fsName, fsName, fsName, fsName);
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
                                   edgeAlpha.c_str(), fsName, fsName, fsName, fsName, fsName);
            fsBuilder->codeAppendf("%s = float(%s < 0.0);", edgeAlpha.c_str(), edgeAlpha.c_str());
            break;
        }
        default:
            SkFAIL("Shouldn't get here");
    }

    fsBuilder->codeAppendf("%s = %s;", outputColor,
                           (GrGLSLExpr4(inputColor) * GrGLSLExpr1(edgeAlpha.c_str())).c_str());
}

void GrGLCubicEffect::GenKey(const GrProcessor& processor, const GrGLCaps&,
                             GrProcessorKeyBuilder* b) {
    const GrCubicEffect& ce = processor.cast<GrCubicEffect>();
    uint32_t key = ce.isAntiAliased() ? (ce.isFilled() ? 0x0 : 0x1) : 0x2;
    b->add32(key);
}

//////////////////////////////////////////////////////////////////////////////

GrCubicEffect::~GrCubicEffect() {}

const GrBackendGeometryProcessorFactory& GrCubicEffect::getFactory() const {
    return GrTBackendGeometryProcessorFactory<GrCubicEffect>::getInstance();
}

GrCubicEffect::GrCubicEffect(GrPrimitiveEdgeType edgeType)
    : fEdgeType(edgeType)
    , fInCubicCoeffs(this->addVertexAttrib(GrShaderVar("inCubicCoeffs",
                                                       kVec4f_GrSLType,
                                                       GrShaderVar::kAttribute_TypeModifier))) {
}

bool GrCubicEffect::onIsEqual(const GrProcessor& other) const {
    const GrCubicEffect& ce = other.cast<GrCubicEffect>();
    return (ce.fEdgeType == fEdgeType);
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
        gp = GrCubicEffect::Create(edgeType, caps);
    } while (NULL == gp);
    return gp;
}

