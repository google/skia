/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRQuadraticProcessor_DEFINED
#define GrCCPRQuadraticProcessor_DEFINED

#include "ccpr/GrCCPRCoverageProcessor.h"

/**
 * This class renders the coverage of closed quadratic curves using the techniques outlined in
 * "Resolution Independent Curve Rendering using Programmable Graphics Hardware" by Charles Loop and
 * Jim Blinn:
 *
 * https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
 *
 * The provided curves must be monotonic with respect to the vector of their closing edge [P2 - P0].
 * (Use GrCCPRGeometry.)
 */
class GrCCPRQuadraticProcessor : public GrCCPRCoverageProcessor::PrimitiveProcessor {
public:
    GrCCPRQuadraticProcessor()
            : INHERITED(CoverageType::kShader)
            , fCanonicalMatrix("canonical_matrix", kHighFloat3x3_GrSLType, GrShaderVar::kNonArray)
            , fCanonicalDerivatives("canonical_derivatives", kHighFloat2x2_GrSLType,
                                    GrShaderVar::kNonArray)
            , fEdgeDistanceEquation("edge_distance_equation", kHighFloat3_GrSLType,
                                    GrShaderVar::kNonArray)
            , fXYD(kHighFloat3_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        varyingHandler->addVarying("xyd", &fXYD, kHigh_GrSLPrecision);
    }

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const TexelBufferHandle& pointsBuffer, const char* atlasOffset,
                            const char* rtAdjust, GrGPArgs*) const override;
    void emitWind(GrGLSLGeometryBuilder*, const char* rtAdjust, const char* outputWind) const final;
    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjust) const final;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const final;

protected:
    virtual void emitQuadraticGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                                       const char* rtAdjust) const = 0;
    virtual void onEmitPerVertexGeometryCode(SkString* fnBody) const = 0;

    GrShaderVar       fCanonicalMatrix;
    GrShaderVar       fCanonicalDerivatives;
    GrShaderVar       fEdgeDistanceEquation;
    GrGLSLGeoToFrag   fXYD;

    typedef GrCCPRCoverageProcessor::PrimitiveProcessor INHERITED;
};

/**
 * This pass draws a conservative raster hull around the quadratic bezier curve, computes the
 * curve's coverage using the gradient-based AA technique outlined in the Loop/Blinn paper, and
 * uses simple distance-to-edge to subtract out coverage for the flat closing edge [P2 -> P0]. Since
 * the provided curves are monotonic, this will get every pixel right except the two corners.
 */
class GrCCPRQuadraticHullProcessor : public GrCCPRQuadraticProcessor {
public:
    GrCCPRQuadraticHullProcessor()
            : fGradXY(kHighFloat2_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addVarying("grad_xy", &fGradXY, kHigh_GrSLPrecision);
    }

    void emitQuadraticGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                               const char* rtAdjust) const override;
    void onEmitPerVertexGeometryCode(SkString* fnBody) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder* f, const char* outputCoverage) const override;

private:
    GrGLSLGeoToFrag   fGradXY;

    typedef GrCCPRQuadraticProcessor INHERITED;
};

/**
 * This pass fixes the corners of a closed quadratic segment with soft MSAA.
 */
class GrCCPRQuadraticCornerProcessor : public GrCCPRQuadraticProcessor {
public:
    GrCCPRQuadraticCornerProcessor()
            : fEdgeDistanceDerivatives("edge_distance_derivatives", kHighFloat2_GrSLType,
                                       GrShaderVar::kNonArray)
            , fdXYDdx(kHighFloat3_GrSLType)
            , fdXYDdy(kHighFloat3_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addFlatVarying("dXYDdx", &fdXYDdx, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("dXYDdy", &fdXYDdy, kHigh_GrSLPrecision);
    }

    void emitQuadraticGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                               const char* rtAdjust) const override;
    void onEmitPerVertexGeometryCode(SkString* fnBody) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

private:
    GrShaderVar       fEdgeDistanceDerivatives;
    GrGLSLGeoToFrag   fdXYDdx;
    GrGLSLGeoToFrag   fdXYDdy;

    typedef GrCCPRQuadraticProcessor INHERITED;
};

#endif
