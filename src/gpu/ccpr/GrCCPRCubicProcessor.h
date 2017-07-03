/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRCubicProcessor_DEFINED
#define GrCCPRCubicProcessor_DEFINED

#include "ccpr/GrCCPRCoverageProcessor.h"

class GrGLSLGeometryBuilder;

class GrCCPRCubicProcessor : public GrCCPRCoverageProcessor::PrimitiveProcessor {
public:
    enum class Type {
        kSerpenitne,
        kLoop
    };

    GrCCPRCubicProcessor(Type type)
        : INHERITED(CoverageType::kShader)
        , fType(type)
        , fInset(kVec3f_GrSLType)
        , fTS(kFloat_GrSLType)
        , fKLMMatrix("klm_matrix", kMat33f_GrSLType, GrShaderVar::kNonArray, kHigh_GrSLPrecision)
        , fKLMDerivatives("dklm", kVec2f_GrSLType, 3, kHigh_GrSLPrecision) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        varyingHandler->addVarying("insets", &fInset, kHigh_GrSLPrecision);
        varyingHandler->addVarying("ts", &fTS, kHigh_GrSLPrecision);
    }

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const char* rtAdjustName, const TexelBufferHandle& pointsBuffer,
                            GrGPArgs* gpArgs) const override;
    void emitWind(GrGLSLGeometryBuilder*, const char* windName,
                  const char* rtAdjustName) const final;
    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjustName) const final;

protected:
    virtual void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                                   const char* wind, const char* rtAdjustName) const = 0;

    const Type        fType;
    GrGLSLVertToGeo   fInset;
    GrGLSLVertToGeo   fTS;
    GrShaderVar       fKLMMatrix;
    GrShaderVar       fKLMDerivatives;

    typedef GrCCPRCoverageProcessor::PrimitiveProcessor INHERITED;
};

class GrCCPRCubicInsetProcessor : public GrCCPRCubicProcessor {
public:
    GrCCPRCubicInsetProcessor(Type type)
        : INHERITED(type)
        , fKLM(kVec3f_GrSLType)
        , fGradMatrix(kMat22f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addVarying("klm", &fKLM, kHigh_GrSLPrecision);
        varyingHandler->addVarying("grad_matrix", &fGradMatrix, kHigh_GrSLPrecision);
    }

    void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                           const char* wind, const char* rtAdjustName) const override;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                   const char* coverage, const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

protected:
    GrGLSLGeoToFrag   fKLM;
    GrGLSLGeoToFrag   fGradMatrix;

    typedef GrCCPRCubicProcessor INHERITED;
};

class GrCCPRCubicBorderProcessor : public GrCCPRCubicProcessor {
public:
    GrCCPRCubicBorderProcessor(Type type)
        : INHERITED(type)
        , fEdgeDistanceEquation("edge_distance_equation", kVec3f_GrSLType, GrShaderVar::kNonArray,
                                kHigh_GrSLPrecision)
        , fEdgeDistanceDerivatives("edge_distance_derivatives", kVec2f_GrSLType,
                                    GrShaderVar::kNonArray, kHigh_GrSLPrecision)
        , fEdgeSpaceTransform("edge_space_transform", kVec4f_GrSLType, GrShaderVar::kNonArray,
                              kHigh_GrSLPrecision)
        , fKLMD(kVec4f_GrSLType)
        , fdKLMDdx(kVec4f_GrSLType)
        , fdKLMDdy(kVec4f_GrSLType)
        , fEdgeSpaceCoord(kVec2f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addVarying("klmd", &fKLMD, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("dklmddx", &fdKLMDdx, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("dklmddy", &fdKLMDdy, kHigh_GrSLPrecision);
        varyingHandler->addVarying("edge_space_coord", &fEdgeSpaceCoord, kHigh_GrSLPrecision);
    }

    void emitCubicGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                           const char* wind, const char* rtAdjustName) const override;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                   const char* coverage, const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

protected:
    GrShaderVar        fEdgeDistanceEquation;
    GrShaderVar        fEdgeDistanceDerivatives;
    GrShaderVar        fEdgeSpaceTransform;
    GrGLSLGeoToFrag    fKLMD;
    GrGLSLGeoToFrag    fdKLMDdx;
    GrGLSLGeoToFrag    fdKLMDdy;
    GrGLSLGeoToFrag    fEdgeSpaceCoord;

    typedef GrCCPRCubicProcessor INHERITED;
};

#endif
