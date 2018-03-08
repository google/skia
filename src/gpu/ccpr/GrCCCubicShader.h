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
                       const char* wind, GeometryVars*) const override;

    void onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code,
                        const char* position, const char* inputCoverage, const char* wind) override;

    void onEmitFragmentCode(const GrCCCoverageProcessor&, GrGLSLFPFragmentBuilder*,
                            const char* outputCoverage) const override;

    GrShaderVar fKLMMatrix{"klm_matrix", kFloat3x3_GrSLType};
    GrGLSLVarying fKLMW;
    GrGLSLVarying fGradMatrix;
};

#endif
