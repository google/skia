/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRCubicShader_DEFINED
#define GrCCPRCubicShader_DEFINED

#include "ccpr/GrCCPRCoverageProcessor.h"

/**
 * This class renders the coverage of convex closed cubic segments using the techniques outlined in
 * "Resolution Independent Curve Rendering using Programmable Graphics Hardware" by Charles Loop and
 * Jim Blinn:
 *
 * https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
 *
 * The provided curve segments must be convex, monotonic with respect to the vector of their closing
 * edge [P3 - P0], and must not contain or be near any inflection points or loop intersections.
 * (Use GrCCPRGeometry.)
 */
class GrCCPRCubicShader : public GrCCPRCoverageProcessor::Shader {
protected:
    void emitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                       const char* wind, GeometryVars*) const final;

    virtual void onEmitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                                 GeometryVars*) const {}

    WindHandling onEmitVaryings(GrGLSLVaryingHandler*, SkString* code, const char* position,
                                const char* coverage, const char* wind) final;

    virtual void onEmitVaryings(GrGLSLVaryingHandler*, SkString* code) = 0;

    GrShaderVar fKLMMatrix{"klm_matrix", kFloat3x3_GrSLType};
    GrShaderVar fEdgeDistanceEquation{"edge_distance_equation", kFloat3_GrSLType};
    GrGLSLVarying fKLMD{kFloat4_GrSLType, GrGLSLVarying::Scope::kGeoToFrag};
};

class GrCCPRCubicHullShader : public GrCCPRCubicShader {
    void onEmitVaryings(GrGLSLVaryingHandler*, SkString* code) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder*, const char* outputCoverage) const override;

    GrGLSLVarying fGradMatrix{kFloat2x2_GrSLType, GrGLSLVarying::Scope::kGeoToFrag};
};

class GrCCPRCubicCornerShader : public GrCCPRCubicShader {
    void onEmitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                         GeometryVars*) const override;
    void onEmitVaryings(GrGLSLVaryingHandler*, SkString* code) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder*, const char* outputCoverage) const override;

    GrGLSLVarying fdKLMDdx{kFloat4_GrSLType, GrGLSLVarying::Scope::kGeoToFrag};
    GrGLSLVarying fdKLMDdy{kFloat4_GrSLType, GrGLSLVarying::Scope::kGeoToFrag};
};

#endif
