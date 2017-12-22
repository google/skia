/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRQuadraticShader_DEFINED
#define GrCCPRQuadraticShader_DEFINED

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
class GrCCPRQuadraticShader : public GrCCPRCoverageProcessor::Shader {
protected:
    void emitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                       const char* wind, GeometryVars*) const final;

    virtual void onEmitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                                 GeometryVars*) const = 0;

    WindHandling onEmitVaryings(GrGLSLVaryingHandler*, SkString* code, const char* position,
                                const char* coverage, const char* wind) final;

    virtual void onEmitVaryings(GrGLSLVaryingHandler*, SkString* code) = 0;

    const GrShaderVar fCanonicalMatrix{"canonical_matrix", kFloat3x3_GrSLType};
    const GrShaderVar fEdgeDistanceEquation{"edge_distance_equation", kFloat3_GrSLType};
    GrGLSLVarying fXYD{kFloat3_GrSLType, GrGLSLVarying::Scope::kGeoToFrag};
};

/**
 * This pass draws a conservative raster hull around the quadratic bezier curve, computes the
 * curve's coverage using the gradient-based AA technique outlined in the Loop/Blinn paper, and
 * uses simple distance-to-edge to subtract out coverage for the flat closing edge [P2 -> P0]. Since
 * the provided curves are monotonic, this will get every pixel right except the two corners.
 */
class GrCCPRQuadraticHullShader : public GrCCPRQuadraticShader {
    void onEmitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                         GeometryVars*) const override;
    void onEmitVaryings(GrGLSLVaryingHandler*, SkString* code) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder*, const char* outputCoverage) const override;

    GrGLSLVarying fGrad{kFloat2_GrSLType, GrGLSLVarying::Scope::kGeoToFrag};
};

/**
 * This pass fixes the corners of a closed quadratic segment with soft MSAA.
 */
class GrCCPRQuadraticCornerShader : public GrCCPRQuadraticShader {
    void onEmitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                         GeometryVars*) const override;
    void onEmitVaryings(GrGLSLVaryingHandler*, SkString* code) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder*, const char* outputCoverage) const override;

    GrGLSLVarying fdXYDdx{kFloat3_GrSLType, GrGLSLVarying::Scope::kGeoToFrag};
    GrGLSLVarying fdXYDdy{kFloat3_GrSLType, GrGLSLVarying::Scope::kGeoToFrag};
};

#endif
