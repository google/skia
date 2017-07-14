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
 * The curves are rendered in two passes:
 *
 * Pass 1: Draw a conservative raster hull around the quadratic bezier points, and compute the
 *         curve's coverage using the gradient-based AA technique outlined in the Loop/Blinn paper.
 *
 * Pass 2: Touch up and antialias the flat edge from P2 back to P0.
 */
class GrCCPRQuadraticProcessor : public GrCCPRCoverageProcessor::PrimitiveProcessor {
public:
    GrCCPRQuadraticProcessor()
            : INHERITED(CoverageType::kShader)
            , fCanonicalMatrix("canonical_matrix", kMat33f_GrSLType, GrShaderVar::kNonArray,
                               kHigh_GrSLPrecision)
            , fCanonicalDerivatives("canonical_derivatives", kMat22f_GrSLType,
                                    GrShaderVar::kNonArray, kHigh_GrSLPrecision)
            , fCanonicalCoord(kVec4f_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        varyingHandler->addVarying("canonical_coord", &fCanonicalCoord, kHigh_GrSLPrecision);
    }

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const TexelBufferHandle& pointsBuffer, const char* atlasOffset,
                            const char* rtAdjust, GrGPArgs*) const override;
    void emitWind(GrGLSLGeometryBuilder*, const char* rtAdjust, const char* outputWind) const final;
    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjust) const final;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder* f, const char* outputCoverage) const override;

protected:
    virtual void emitQuadraticGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                                       const char* wind, const char* rtAdjust) const = 0;

    GrShaderVar       fCanonicalMatrix;
    GrShaderVar       fCanonicalDerivatives;
    GrGLSLGeoToFrag   fCanonicalCoord;

    typedef GrCCPRCoverageProcessor::PrimitiveProcessor INHERITED;
};

class GrCCPRQuadraticHullProcessor : public GrCCPRQuadraticProcessor {
public:
    void emitQuadraticGeometry(GrGLSLGeometryBuilder*, const char* emitVertexFn,
                               const char* wind, const char* rtAdjust) const override;

private:
    typedef GrCCPRQuadraticProcessor INHERITED;
};

/**
 * This pass touches up the flat edge (P2 -> P0) of a closed quadratic segment as follows:
 *
 *   1) Erase what the previous hull shader estimated for coverage.
 *   2) Replace coverage with distance to the curve's flat edge (this is necessary when the edge
 *      is shared and must create a "water-tight" seam).
 *   3) Use pseudo MSAA to subtract out the remaining pixel coverage that is still inside the flat
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
                               const char* wind, const char* rtAdjust) const override;
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
