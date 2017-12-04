/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRTriangleShader_DEFINED
#define GrCCPRTriangleShader_DEFINED

#include "ccpr/GrCCPRCoverageProcessor.h"

/**
 * This class renders the coverage of triangles. Triangles are rendered in three passes, as
 * described below.
 */
class GrCCPRTriangleShader : public GrCCPRCoverageProcessor::Shader {
public:
    int getNumInputPoints() const final { return 3; }
    int getNumSegments() const final { return 3; } // 3 wedges, 3 edges, 3 corners.
    void emitWind(GrGLSLShaderBuilder* s, const char* pts, const char* outputWind) const final;
};

/**
 * Pass 1: Draw the triangle's conservative raster hull with a coverage of 1. (Conservative raster
 *         is drawn by considering 3 pixel size boxes, one centered at each vertex, and drawing the
 *         convex hull of those boxes.)
 */
class GrCCPRTriangleHullShader : public GrCCPRTriangleShader {
    GeometryType getGeometryType() const override { return GeometryType::kHull; }

    WindHandling onEmitVaryings(GrGLSLVaryingHandler*, SkString* code, const char* position,
                                const char* coverage, const char* wind) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder* f, const char* outputCoverage) const override;
};

/**
 * Pass 2: Smooth the edges that were over-rendered during Pass 1. Draw the conservative raster of
 *         each edge (i.e. convex hull of two pixel-size boxes at the endpoints), interpolating from
 *         coverage=-1 on the outside edge to coverage=0 on the inside edge.
 */
class GrCCPRTriangleEdgeShader : public GrCCPRTriangleShader {
    GeometryType getGeometryType() const override { return GeometryType::kEdges; }

    WindHandling onEmitVaryings(GrGLSLVaryingHandler*, SkString* code, const char* position,
                                const char* coverage, const char* wind) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder*, const char* outputCoverage) const override;

    GrGLSLGeoToFrag fCoverageTimesWind{kHalf_GrSLType};
};

/**
 * Pass 3: Touch up the corner pixels. Here we fix the simple distance-to-edge coverage analysis
 *         done previously so that it takes into account the region that is outside both edges at
 *         the same time.
 */
class GrCCPRTriangleCornerShader : public GrCCPRTriangleShader {
    GeometryType getGeometryType() const override { return GeometryType::kCorners; }

    void emitSetupCode(GrGLSLShaderBuilder*, const char* pts, const char* cornerId,
                       const char* wind, GeometryVars*) const override;
    WindHandling onEmitVaryings(GrGLSLVaryingHandler*, SkString* code, const char* position,
                                const char* coverage, const char* wind) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder* f, const char* outputCoverage) const override;

    GrShaderVar       fAABoxMatrices{"aa_box_matrices", kFloat2x2_GrSLType, 2};
    GrShaderVar       fAABoxTranslates{"aa_box_translates", kFloat2_GrSLType, 2};
    GrShaderVar       fGeoShaderBisects{"bisects", kFloat2_GrSLType, 2};
    GrGLSLGeoToFrag   fCornerLocationInAABoxes{kFloat2x2_GrSLType};
    GrGLSLGeoToFrag   fBisectInAABoxes{kFloat2x2_GrSLType};
};

#endif
