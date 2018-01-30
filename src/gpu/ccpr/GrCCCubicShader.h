/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCCubicShader_DEFINED
#define GrCCCubicShader_DEFINED

#include "ccpr/GrCCCoverageProcessor.h"

/**
 * This class renders the coverage of convex closed cubic segments using the techniques outlined in
 * "Resolution Independent Curve Rendering using Programmable Graphics Hardware" by Charles Loop and
 * Jim Blinn:
 *
 * https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
 *
 * The provided curve segments must be convex, monotonic with respect to the vector of their closing
 * edge [P3 - P0], and must not contain or be near any inflection points or loop intersections.
 * (Use GrCCGeometry.)
 */
class GrCCCubicShader : public GrCCCoverageProcessor::Shader {
protected:
    void emitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                       const char* wind, GeometryVars*) const final;

    virtual void onEmitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                                 GeometryVars*) const {}

    WindHandling onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code,
                                const char* position, const char* coverage, const char* wind) final;

    virtual void onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code) = 0;

    GrShaderVar fKLMMatrix{"klm_matrix", kFloat3x3_GrSLType};
    GrShaderVar fEdgeDistanceEquation{"edge_distance_equation", kFloat3_GrSLType};
    GrGLSLVarying fKLMD;
};

class GrCCCubicHullShader : public GrCCCubicShader {
    void onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder*, const char* outputCoverage) const override;

    GrGLSLVarying fGradMatrix;
};

class GrCCCubicCornerShader : public GrCCCubicShader {
    void onEmitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                         GeometryVars*) const override;
    void onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder*, const char* outputCoverage) const override;

    GrGLSLVarying fdKLMDdx;
    GrGLSLVarying fdKLMDdy;
};

#endif
