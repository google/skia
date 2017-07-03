/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRTriangleProcessor_DEFINED
#define GrCCPRTriangleProcessor_DEFINED

#include "ccpr/GrCCPRCoverageProcessor.h"

class GrCCPRTriangleProcessor : public GrCCPRCoverageProcessor::PrimitiveProcessor {
public:
    enum class GeometryType {
        kHulls,
        kEdges,
        kHullsAndEdges
    };

    GrCCPRTriangleProcessor(CoverageType initialCoverage, GeometryType geometryType)
        : INHERITED(initialCoverage)
        , fGeometryType(geometryType) {}

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const char* rtAdjustName, const TexelBufferHandle& pointsBuffer,
                            GrGPArgs* gpArgs) const override;
    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjustName) const override;

private:
    const GeometryType fGeometryType;

    typedef GrCCPRCoverageProcessor::PrimitiveProcessor INHERITED;
};

/**
 * This class fixes the corner pixels of a triangle. It erases the coverage that was written
 * previously for its hull and edges, and then approximates the true coverage by sampling the
 * triangle.
 */
class GrCCPRTriangleCornerProcessor : public GrCCPRTriangleProcessor {
public:
    GrCCPRTriangleCornerProcessor()
        : INHERITED(CoverageType::kShader, GeometryType::kHulls) // TODO: abolish!
        , fEdgeDistance(kVec3f_GrSLType)
        , fDevCoord(kVec2f_GrSLType)
        , fNeighbors(kVec4f_GrSLType)
        , fEdgeDistances(kMat33f_GrSLType)
        , fCornerIdx(kInt_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addFlatVarying("edge_distance", &fEdgeDistance, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("devcoord", &fDevCoord, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("neighbors", &fNeighbors, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("edge_distances", &fEdgeDistances, kHigh_GrSLPrecision);
        varyingHandler->addFlatVarying("corner_idx", &fCornerIdx, kLow_GrSLPrecision);
    }

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const char* rtAdjustName, const TexelBufferHandle& pointsBuffer,
                            GrGPArgs* gpArgs) const override;
    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjustName) const override;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

private:
    GrGLSLVertToGeo fEdgeDistance;
    GrGLSLVertToGeo fDevCoord;
    GrGLSLGeoToFrag fNeighbors;
    GrGLSLGeoToFrag fEdgeDistances;
    GrGLSLGeoToFrag fCornerIdx;

    typedef GrCCPRTriangleProcessor INHERITED;
};

#endif
