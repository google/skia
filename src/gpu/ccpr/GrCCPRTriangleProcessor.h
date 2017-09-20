/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRTriangleProcessor_DEFINED
#define GrCCPRTriangleProcessor_DEFINED

#include "ccpr/GrCCPRCoverageProcessor.h"

/**
 * This class renders the coverage of triangles.
 *
 * Triangles are rendered in three passes:
 *
 * Pass 1: Draw the triangle's conservative raster hull with a coverage of 1. (Conservative raster
 *         is drawn by considering 3 pixel size boxes, one centered at each vertex, and drawing the
 *         convex hull of those boxes.)
 *
 * Pass 2: Smooth the edges that were over-rendered during Pass 1. Draw the conservative raster of
 *         each edge (i.e. convex hull of two pixel-size boxes at the endpoints), interpolating from
 *         coverage=-1 on the outside edge to coverage=0 on the inside edge.
 *
 * Pass 3: Touch up the corner pixels to have the correct coverage.
 */
class GrCCPRTriangleProcessor : public GrCCPRCoverageProcessor::PrimitiveProcessor {
public:
    GrCCPRTriangleProcessor(CoverageType initialCoverage) : INHERITED(initialCoverage) {}

    void onEmitVertexShader(const GrCCPRCoverageProcessor&, GrGLSLVertexBuilder*,
                            const TexelBufferHandle& pointsBuffer, const char* atlasOffset,
                            const char* rtAdjust, GrGPArgs*) const override;
    void emitWind(GrGLSLGeometryBuilder*, const char* rtAdjust, const char* outputWind) const final;

protected:
    void defineInputVertices(GrGLSLGeometryBuilder*) const;

private:
    typedef GrCCPRCoverageProcessor::PrimitiveProcessor INHERITED;
};

class GrCCPRTriangleHullAndEdgeProcessor : public GrCCPRTriangleProcessor {
public:
    enum class GeometryType {
        kHulls,
        kEdges,
        kHullsAndEdges
    };

    GrCCPRTriangleHullAndEdgeProcessor(GeometryType geometryType)
            : INHERITED(GeometryType::kHulls == geometryType ?
                        CoverageType::kOne : CoverageType::kInterpolated)
            , fGeometryType(geometryType) {}

    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjust) const override;

private:
    const GeometryType fGeometryType;

    typedef GrCCPRTriangleProcessor INHERITED;
};

/**
 * This pass fixes the corner pixels of a triangle. It touches up the simple distance-to-edge
 * coverage analysis done previously so that it takes into account the region that is outside both
 * edges at the same time.
 */
class GrCCPRTriangleCornerProcessor : public GrCCPRTriangleProcessor {
public:
    GrCCPRTriangleCornerProcessor()
            : INHERITED(CoverageType::kShader)
            , fAABoxMatrices("aa_box_matrices", kFloat2x2_GrSLType, 2)
            , fAABoxTranslates("aa_box_translates", kFloat2_GrSLType, 2)
            , fGeoShaderBisects("bisects", kFloat2_GrSLType, 2)
            , fCornerLocationInAABoxes(kFloat2x2_GrSLType)
            , fBisectInAABoxes(kFloat2x2_GrSLType) {}

    void resetVaryings(GrGLSLVaryingHandler* varyingHandler) override {
        this->INHERITED::resetVaryings(varyingHandler);
        varyingHandler->addVarying("corner_location_in_aa_boxes", &fCornerLocationInAABoxes);
        varyingHandler->addFlatVarying("bisect_in_aa_boxes", &fBisectInAABoxes);
    }

    void onEmitGeometryShader(GrGLSLGeometryBuilder*, const char* emitVertexFn, const char* wind,
                              const char* rtAdjust) const override;
    void emitPerVertexGeometryCode(SkString* fnBody, const char* position, const char* coverage,
                                   const char* wind) const override;
    void emitShaderCoverage(GrGLSLFragmentBuilder*, const char* outputCoverage) const override;

private:
    GrShaderVar       fAABoxMatrices;
    GrShaderVar       fAABoxTranslates;
    GrShaderVar       fGeoShaderBisects;
    GrGLSLGeoToFrag   fCornerLocationInAABoxes;
    GrGLSLGeoToFrag   fBisectInAABoxes;

    typedef GrCCPRTriangleProcessor INHERITED;
};

#endif
