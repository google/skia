/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCTriangleShader_DEFINED
#define GrCCTriangleShader_DEFINED

#include "ccpr/GrCCCoverageProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

/**
 * This class renders AA triangles. It relies on the coverage processor to set up the geometry and
 * provide coverage values at each vertex, then simply interpolates these values in the fragment
 * shader.
 */
class GrCCTriangleShader : public GrCCCoverageProcessor::Shader {
    void onEmitVaryings(GrGLSLVaryingHandler* varyingHandler, GrGLSLVarying::Scope scope,
                        SkString* code, const char* position, const char* coverage,
                        const char* attenuatedCoverage, const char* wind) override {
        if (!attenuatedCoverage) {
            fCoverageTimesWind.reset(kHalf_GrSLType, scope);
            varyingHandler->addVarying("coverage_times_wind", &fCoverageTimesWind);
            code->appendf("%s = %s * %s;", OutName(fCoverageTimesWind), coverage, wind);
        } else {
            fCoverageTimesWind.reset(kHalf3_GrSLType, scope);
            varyingHandler->addVarying("coverage_times_wind", &fCoverageTimesWind);
            code->appendf("%s = half3(%s, %s);",
                          OutName(fCoverageTimesWind), attenuatedCoverage, coverage);
            code->appendf("%s.yz *= %s;", OutName(fCoverageTimesWind), wind);
        }
    }

    void onEmitFragmentCode(GrGLSLFPFragmentBuilder* f, const char* outputCoverage) const override {
        if (kHalf_GrSLType == fCoverageTimesWind.type()) {
            f->codeAppendf("%s = %s;", outputCoverage, fCoverageTimesWind.fsIn());
        } else {
            f->codeAppendf("%s = %s.x * %s.y + %s.z;",
                           outputCoverage, fCoverageTimesWind.fsIn(), fCoverageTimesWind.fsIn(),
                           fCoverageTimesWind.fsIn());
        }
    }

    GrGLSLVarying fCoverageTimesWind;
};

#endif
