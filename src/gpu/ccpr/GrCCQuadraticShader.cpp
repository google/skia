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

void GrCCQuadraticShader::emitSetupCode(GrGLSLVertexGeoBuilder* s, const char* pts,
                                        const char* wind, const char** tighterHull) const {
    s->declareGlobal(fQCoordMatrix);
    s->codeAppendf("%s = float2x2(1, 1, .5, 0) * inverse(float2x2(%s[2] - %s[0], %s[1] - %s[0]));",
                   fQCoordMatrix.c_str(), pts, pts, pts, pts);

    s->declareGlobal(fQCoord0);
    s->codeAppendf("%s = %s[0];", fQCoord0.c_str(), pts);

    s->declareGlobal(fEdgeDistanceEquation);
    s->codeAppendf("float2 edgept0 = %s[%s > 0 ? 2 : 0];", pts, wind);
    s->codeAppendf("float2 edgept1 = %s[%s > 0 ? 0 : 2];", pts, wind);
    Shader::EmitEdgeDistanceEquation(s, "edgept0", "edgept1", fEdgeDistanceEquation.c_str());

    if (tighterHull) {
        // Find the T value whose tangent is halfway between the tangents at the endpionts.
        s->codeAppendf("float2 tan0 = %s[1] - %s[0];", pts, pts);
        s->codeAppendf("float2 tan1 = %s[2] - %s[1];", pts, pts);
        s->codeAppend ("float2 midnorm = normalize(tan0) - normalize(tan1);");
        s->codeAppend ("float2 T = midnorm * float2x2(tan0 - tan1, tan0);");
        s->codeAppend ("float t = clamp(T.t / T.s, 0, 1);"); // T.s!=0; we cull flat curves on CPU.

        // Clip the bezier triangle by the tangent at our new t value. This is a simple application
        // for De Casteljau's algorithm.
        s->codeAppendf("float4x2 quadratic_hull = float4x2(%s[0], "
                                                          "%s[0] + tan0 * t, "
                                                          "%s[1] + tan1 * t, "
                                                          "%s[2]);", pts, pts, pts, pts);
        *tighterHull = "quadratic_hull";
    }
}

void GrCCQuadraticShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                         GrGLSLVarying::Scope scope, SkString* code,
                                         const char* position, const char* coverage,
                                         const char* attenuatedCoverage) {
    fCoord.reset(kFloat4_GrSLType, scope);
    varyingHandler->addVarying("coord", &fCoord);
    code->appendf("%s.xy = %s * (%s - %s);", // Quadratic coords.
                  OutName(fCoord), fQCoordMatrix.c_str(), position, fQCoord0.c_str());
    code->appendf("%s.zw = 2*bloat * float2(2 * %s.x, -1) * %s;", // Gradient.
                  OutName(fCoord), OutName(fCoord), fQCoordMatrix.c_str());

    // Coverages need full precision since distance to the opposite edge can be large.
    fCoverages.reset(attenuatedCoverage ? kFloat4_GrSLType : kFloat2_GrSLType, scope);
    varyingHandler->addVarying("coverages", &fCoverages);
    code->appendf("%s.x = dot(%s, float3(%s, 1));", // Distance to flat edge opposite the curve.
                  OutName(fCoverages), fEdgeDistanceEquation.c_str(), position);
    code->appendf("%s.y = %s;", OutName(fCoverages), coverage); // Wind.
    if (attenuatedCoverage) {
        code->appendf("%s.zw = %s;", // Attenuated corner coverage.
                      OutName(fCoverages), attenuatedCoverage);
    }
}

void GrCCQuadraticShader::onEmitFragmentCode(GrGLSLFPFragmentBuilder* f,
                                             const char* outputCoverage) const {
    f->codeAppendf("float x = %s.x, y = %s.y;", fCoord.fsIn(), fCoord.fsIn());
    f->codeAppend ("float f = x*x - y;");
    f->codeAppendf("float2 grad = %s.zw;", fCoord.fsIn());
    f->codeAppendf("%s = clamp(0.5 - f * inversesqrt(dot(grad, grad)), 0, 1);", outputCoverage);

    f->codeAppendf("half d = min(%s.x, 0);", fCoverages.fsIn()); // Flat edge opposite the curve.
    f->codeAppendf("half wind = %s.y;", fCoverages.fsIn());
    f->codeAppendf("%s = (%s + d) * wind;", outputCoverage, outputCoverage);

    if (kFloat4_GrSLType == fCoverages.type()) {
        f->codeAppendf("%s = %s.z * %s.w + %s;", // Attenuated corner coverage.
                       outputCoverage, fCoverages.fsIn(), fCoverages.fsIn(), outputCoverage);
    }
}
