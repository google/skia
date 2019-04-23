/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCCubicShader_DEFINED
#define GrCCCubicShader_DEFINED

#include "src/gpu/ccpr/GrCCCoverageProcessor.h"

/**
 * This class renders the coverage of convex closed cubic segments using the techniques outlined in
 * "Resolution Independent Curve Rendering using Programmable Graphics Hardware" by Charles Loop and
 * Jim Blinn:
 *
 * https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
 *
 * The provided curve segments must be convex, monotonic with respect to the vector of their closing
 * edge [P3 - P0], and must not contain or be near any inflection points or loop intersections.
 * (Use GrCCGeometry::cubicTo().)
 */
class GrCCCubicShader : public GrCCCoverageProcessor::Shader {
public:
    void emitSetupCode(
            GrGLSLVertexGeoBuilder*, const char* pts, const char** outHull4) const override;

    void onEmitVaryings(
            GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code, const char* position,
            const char* coverage, const char* cornerCoverage, const char* wind) override;

    void onEmitFragmentCode(GrGLSLFPFragmentBuilder*, const char* outputCoverage) const override;

private:
    void calcHullCoverage(SkString* code, const char* klmAndEdge, const char* gradMatrix,
                          const char* outputCoverage) const;

    const GrShaderVar fKLMMatrix{"klm_matrix", kFloat3x3_GrSLType};
    GrGLSLVarying fKLM_fEdge;
    GrGLSLVarying fGradMatrix;
    GrGLSLVarying fCornerCoverage;
};

#endif
