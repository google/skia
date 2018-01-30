/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCTriangleShader_DEFINED
#define GrCCTriangleShader_DEFINED

#include "ccpr/GrCCCoverageProcessor.h"

/**
 * Steps 1 & 2: Draw the triangle's conservative raster hull with a coverage of +1, then smooth the
 *              edges by drawing the conservative rasters of all 3 edges and interpolating from
 *              coverage=-1 on the outside to coverage=0 on the inside. The Impl may choose to
 *              implement these steps in either one or two actual render passes.
 */
class GrCCTriangleShader : public GrCCCoverageProcessor::Shader {
    WindHandling onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code,
                                const char* position, const char* coverage,
                                const char* wind) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder*, const char* outputCoverage) const override;

    GrGLSLVarying fCoverageTimesWind;
};

/**
 * Step 3: Touch up the corner pixels. Here we fix the simple distance-to-edge coverage analysis
 *         done previously so that it takes into account the region that is outside both edges at
 *         the same time.
 */
class GrCCTriangleCornerShader : public GrCCCoverageProcessor::Shader {
    void emitSetupCode(GrGLSLVertexGeoBuilder*, const char* pts, const char* repetitionID,
                       const char* wind, GeometryVars*) const override;
    WindHandling onEmitVaryings(GrGLSLVaryingHandler*, GrGLSLVarying::Scope, SkString* code,
                                const char* position, const char* coverage,
                                const char* wind) override;
    void onEmitFragmentCode(GrGLSLPPFragmentBuilder* f, const char* outputCoverage) const override;

    GrShaderVar fAABoxMatrices{"aa_box_matrices", kFloat2x2_GrSLType, 2};
    GrShaderVar fAABoxTranslates{"aa_box_translates", kFloat2_GrSLType, 2};
    GrShaderVar fGeoShaderBisects{"bisects", kFloat2_GrSLType, 2};
    GrGLSLVarying fCornerLocationInAABoxes;
    GrGLSLVarying fBisectInAABoxes;
};

#endif
