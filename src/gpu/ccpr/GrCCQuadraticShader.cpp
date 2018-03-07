/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCQuadraticShader.h"

#include "glsl/GrGLSLVertexGeoBuilder.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

using Shader = GrCCCoverageProcessor::Shader;

const char* GrCCQuadraticShader::emitSetupCode(GrGLSLVertexGeoBuilder* s, const char* pts) const {
    s->declareGlobal(fCanonicalMatrix);
    s->codeAppendf("%s = float3x3(0.0, 0, 1, "
                                 "0.5, 0, 1, "
                                 "1.0, 1, 1) * "
                        "inverse(float3x3(%s[0], 1, "
                                         "%s[1], 1, "
                                         "%s[2], 1));",
                   fCanonicalMatrix.c_str(), pts, pts, pts);

    // Find the T value whose tangent is halfway between the tangents at the endpionts.
    s->codeAppendf("float2 tan0 = %s[1] - %s[0];", pts, pts);
    s->codeAppendf("float2 tan1 = %s[2] - %s[1];", pts, pts);
    s->codeAppend ("float2 midnorm = normalize(tan0) - normalize(tan1);");
    s->codeAppend ("float2 T = midnorm * float2x2(tan0 - tan1, tan0);");
    s->codeAppend ("float t = clamp(T.t / T.s, 0, 1);"); // T.s != 0; we cull flat curves on CPU.

    // Clip the bezier triangle by the tangent at our new t value. This is a simple application for
    // De Casteljau's algorithm.
    s->codeAppendf("float4x2 quadratic_hull = float4x2(%s[0], "
                                                      "%s[0] + tan0 * t, "
                                                      "%s[1] + tan1 * t, "
                                                      "%s[2]);", pts, pts, pts, pts);
    return "quadratic_hull";
}

Shader::CoverageHandling GrCCQuadraticShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                                             GrGLSLVarying::Scope scope,
                                                             SkString* code, const char* position,
                                                             const char* coverageTimesWind) {
    fCoords.reset(kFloat4_GrSLType, scope);
    varyingHandler->addVarying("coords", &fCoords);
    code->appendf("%s.xy = (%s * float3(%s, 1)).xy;",
                  OutName(fCoords), fCanonicalMatrix.c_str(), position);
    code->appendf("%s.zw = float2(2 * %s.x, -1) * float2x2(%s);",
                  OutName(fCoords), OutName(fCoords), fCanonicalMatrix.c_str());
    return CoverageHandling::kNotHandled;
}

void GrCCQuadraticShader::onEmitFragmentCode(const GrCCCoverageProcessor& proc,
                                             GrGLSLFPFragmentBuilder* f,
                                             const char* outputCoverage) const {
    f->codeAppendf("float d = (%s.x * %s.x - %s.y) * inversesqrt(dot(%s.zw, %s.zw));",
                   fCoords.fsIn(), fCoords.fsIn(), fCoords.fsIn(), fCoords.fsIn(), fCoords.fsIn());
#ifdef SK_DEBUG
    if (proc.debugVisualizationsEnabled()) {
        f->codeAppendf("d /= %f;", proc.debugBloat());
    }
#endif
    f->codeAppendf("%s = clamp(0.5 - d, 0, 1);", outputCoverage);
}
