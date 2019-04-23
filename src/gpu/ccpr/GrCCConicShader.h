/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCConicShader_DEFINED
#define GrCCConicShader_DEFINED

#include "src/gpu/ccpr/GrCCCoverageProcessor.h"

/**
 * This class renders the coverage of closed conic curves using the techniques outlined in
 * "Resolution Independent Curve Rendering using Programmable Graphics Hardware" by Charles Loop and
 * Jim Blinn:
 *
 * https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
 *
 * The provided curves must be monotonic with respect to the vector of their closing edge [P2 - P0].
 * (Use GrCCGeometry::conicTo().)
 */
class GrCCConicShader : public GrCCCoverageProcessor::Shader {
public:
    bool calculatesOwnEdgeCoverage() const override { return true; }

    void emitSetupCode(
            GrGLSLVertexGeoBuilder*, const char* pts, const char** outHull4) const override;

    void onEmitVaryings(
            GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code, const char* position,
            const char* coverage, const char* cornerCoverage, const char* wind) override;

    void onEmitFragmentCode(GrGLSLFPFragmentBuilder*, const char* outputCoverage) const override;

private:
    void calcHullCoverage(SkString* code, const char* klm, const char* grad,
                          const char* outputCoverage) const;

    const GrShaderVar fKLMMatrix{"klm_matrix", kFloat3x3_GrSLType};
    const GrShaderVar fControlPoint{"control_point", kFloat2_GrSLType};
    GrGLSLVarying fKLM_fWind;
    GrGLSLVarying fGrad_fCorner;
};

#endif
