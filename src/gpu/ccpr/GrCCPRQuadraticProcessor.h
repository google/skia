/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRQuadraticProcessor_DEFINED
#define GrCCPRQuadraticProcessor_DEFINED

#include "ccpr/GrCCPRCoverageProcessor.h"

class GrCCPRQuadraticProcessor : public GrCCPRCoverageProcessor::PrimitiveProcessor {
public:
    GrCCPRQuadraticProcessor()
        : INHERITED(CoverageType::kShader)
        , fCanonicalMatrix("canonical_matrix", kMat33f_GrSLType, GrShaderVar::kNonArray,
                           kHigh_GrSLPrecision)
        , fCanonicalDerivatives("canonical_derivatives", kMat22f_GrSLType, GrShaderVar::kNonArray,
                                kHigh_GrSLPrecision)
        , fCanonicalCoord(kVec4f_GrSLType) {} 

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        varyingHandler->addVarying("canonical_coord", &fCanonicalCoord, kHigh_GrSLPrecision);
    }

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const char* rtAdjustName, const TexelBufferHandle& pointsBuffer,
                            GrGPArgs* gpArgs) const override;
    void emitWind(GrGLSLGeometryBuilder*, const char* windName,
                  const char* rtAdjustName) const final;
    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjustName) const final;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder* f, const char* outputCoverage) const override;

protected:
    virtual void emitQuadraticGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                                       const char* wind, const char* rtAdjustName) const = 0;

    GrShaderVar       fCanonicalMatrix;
    GrShaderVar       fCanonicalDerivatives;
    GrGLSLGeoToFrag   fCanonicalCoord;

    typedef GrCCPRCoverageProcessor::PrimitiveProcessor INHERITED;
};

class GrCCPRQuadraticHullProcessor : public GrCCPRQuadraticProcessor {
public:
    void emitQuadraticGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                               const char* wind, const char* rtAdjustName) const override;

private:
    typedef GrCCPRQuadraticProcessor INHERITED;
};

/**
 * This class fixes the inner edge of a quadratic triangle as follows:
 *
 *   1) Erase what the previous hull shader estimated for the coverage.
 *   2) Replace coverage with distance to the triangle's inner edge.
 *   3) Use pseudo msaa to subtract out the remaining pixel coverage that is still inside the inner
 *      edge, but outside the curve.
 */
class GrCCPRQuadraticSharedEdgeProcessor : public GrCCPRQuadraticProcessor {
public:
    GrCCPRQuadraticSharedEdgeProcessor()
        : fXYD("xyd", kMat33f_GrSLType, GrShaderVar::kNonArray, kHigh_GrSLPrecision)
        , fEdgeDistanceDerivatives("edge_distance_derivatives", kVec2f_GrSLType,
                                   GrShaderVar::kNonArray, kHigh_GrSLPrecision)
        , fFragCanonicalDerivatives(kMat22f_GrSLType)
        , fEdgeDistance(kVec3f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addFlatVarying("canonical_derivatives", &fFragCanonicalDerivatives,
                                       kHigh_GrSLPrecision);
        varyingHandler->addVarying("edge_distance", &fEdgeDistance, kHigh_GrSLPrecision);
    }

    void emitQuadraticGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                               const char* wind, const char* rtAdjustName) const override;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

private:
    GrShaderVar       fXYD;
    GrShaderVar       fEdgeDistanceDerivatives;
    GrGLSLGeoToFrag   fFragCanonicalDerivatives;
    GrGLSLGeoToFrag   fEdgeDistance;

    typedef GrCCPRQuadraticProcessor INHERITED;
};

#endif
