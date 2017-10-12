/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRQuadraticShader.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"

void GrCCPRQuadraticShader::appendInputPointFetch(const GrCCPRCoverageProcessor& proc,
                                                  GrGLSLShaderBuilder* s,
                                                  const TexelBufferHandle& pointsBuffer,
                                                  const char* pointId) const {
    s->appendTexelFetch(pointsBuffer,
                        SkStringPrintf("%s.x + %s", proc.instanceAttrib(), pointId).c_str());
}

void GrCCPRQuadraticShader::emitWind(GrGLSLShaderBuilder* s, const char* pts, const char* rtAdjust,
                                     const char* outputWind) const {
    s->codeAppendf("float area_times_2 = determinant(float2x2(%s[1] - %s[0], %s[2] - %s[0]));",
                                                     pts, pts, pts, pts);
    // Drop curves that are nearly flat, in favor of the higher quality triangle antialiasing.
    s->codeAppendf("if (2 * abs(area_times_2) < length((%s[2] - %s[0]) * %s.zx)) {",
                   pts, pts, rtAdjust);
#ifndef SK_BUILD_FOR_MAC
    s->codeAppend (    "return;");
#else
    // Returning from this geometry shader makes Mac very unhappy. Instead we make wind 0.
    s->codeAppend (    "area_times_2 = 0;");
#endif
    s->codeAppend ("}");
    s->codeAppendf("%s = sign(area_times_2);", outputWind);
}

void GrCCPRQuadraticShader::emitSetupCode(GrGLSLShaderBuilder* s, const char* pts,
                                          const char* segmentId, const char* bloat,
                                          const char* wind, const char* rtAdjust,
                                          GeometryVars* vars) const {
    s->declareGlobal(fCanonicalMatrix);
    s->codeAppendf("%s = float3x3(0.0, 0, 1, "
                                 "0.5, 0, 1, "
                                 "1.0, 1, 1) * "
                        "inverse(float3x3(%s[0], 1, "
                                         "%s[1], 1, "
                                         "%s[2], 1));",
                   fCanonicalMatrix.c_str(), pts, pts, pts);

    s->declareGlobal(fCanonicalDerivatives);
    s->codeAppendf("%s = float2x2(%s) * float2x2(%s.x, 0, 0, %s.z);",
                   fCanonicalDerivatives.c_str(), fCanonicalMatrix.c_str(), rtAdjust, rtAdjust);

    s->declareGlobal(fEdgeDistanceEquation);
    s->codeAppendf("float2 edgept0 = %s[%s > 0 ? 2 : 0];", pts, wind);
    s->codeAppendf("float2 edgept1 = %s[%s > 0 ? 0 : 2];", pts, wind);
    Shader::EmitEdgeDistanceEquation(s, "edgept0", "edgept1", fEdgeDistanceEquation.c_str());

    this->onEmitSetupCode(s, pts, segmentId, rtAdjust, vars);
}

GrCCPRQuadraticShader::WindHandling
GrCCPRQuadraticShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler, SkString* code,
                                      const char* position, const char* /*coverage*/,
                                      const char* /*wind*/) {
    varyingHandler->addVarying("xyd", &fXYD, kHigh_GrSLPrecision);
    code->appendf("%s.xy = (%s * float3(%s, 1)).xy;",
                  fXYD.gsOut(), fCanonicalMatrix.c_str(), position);
    code->appendf("%s.z = dot(%s.xy, %s) + %s.z;",
                  fXYD.gsOut(), fEdgeDistanceEquation.c_str(), position,
                  fEdgeDistanceEquation.c_str());

    this->onEmitVaryings(varyingHandler, code);
    return WindHandling::kNotHandled;
}

void GrCCPRQuadraticHullShader::onEmitSetupCode(GrGLSLShaderBuilder* s, const char* pts,
                                                const char* /*wedgeId*/, const char* /*rtAdjust*/,
                                                GeometryVars* vars) const {
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
    vars->fHullVars.fAlternatePoints = "quadratic_hull";
}

void GrCCPRQuadraticHullShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                               SkString* code) {
    varyingHandler->addVarying("grad", &fGrad, kHigh_GrSLPrecision);
    code->appendf("%s = float2(2 * %s.x, -1) * %s;",
                  fGrad.gsOut(), fXYD.gsOut(), fCanonicalDerivatives.c_str());
}

void GrCCPRQuadraticHullShader::onEmitFragmentCode(GrGLSLPPFragmentBuilder* f,
                                                   const char* outputCoverage) const {
    f->codeAppendf("float d = (%s.x * %s.x - %s.y) * inversesqrt(dot(%s, %s));",
                   fXYD.fsIn(), fXYD.fsIn(), fXYD.fsIn(), fGrad.fsIn(), fGrad.fsIn());
    f->codeAppendf("%s = clamp(0.5 - d, 0, 1);", outputCoverage);
    f->codeAppendf("%s += min(%s.z, 0);", outputCoverage, fXYD.fsIn()); // Flat closing edge.
}

void GrCCPRQuadraticCornerShader::onEmitSetupCode(GrGLSLShaderBuilder* s, const char* pts,
                                                  const char* cornerId, const char* rtAdjust,
                                                  GeometryVars* vars) const {
    s->declareGlobal(fEdgeDistanceDerivatives);
    s->codeAppendf("%s = %s.xy * %s.xz;",
                   fEdgeDistanceDerivatives.c_str(), fEdgeDistanceEquation.c_str(), rtAdjust);

    s->codeAppendf("float2 corner = %s[%s * 2];", pts, cornerId);
    vars->fCornerVars.fPoint = "corner";
}

void GrCCPRQuadraticCornerShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                                 SkString* code) {
    varyingHandler->addFlatVarying("dXYDdx", &fdXYDdx, kHigh_GrSLPrecision);
    code->appendf("%s = float3(%s[0].x, %s[0].y, %s.x);",
                  fdXYDdx.gsOut(), fCanonicalDerivatives.c_str(), fCanonicalDerivatives.c_str(),
                  fEdgeDistanceDerivatives.c_str());

    varyingHandler->addFlatVarying("dXYDdy", &fdXYDdy, kHigh_GrSLPrecision);
    code->appendf("%s = float3(%s[1].x, %s[1].y, %s.y);",
                  fdXYDdy.gsOut(), fCanonicalDerivatives.c_str(), fCanonicalDerivatives.c_str(),
                  fEdgeDistanceDerivatives.c_str());
}

void GrCCPRQuadraticCornerShader::onEmitFragmentCode(GrGLSLPPFragmentBuilder* f,
                                                     const char* outputCoverage) const {
    f->codeAppendf("float x = %s.x, y = %s.y, d = %s.z;",
                   fXYD.fsIn(), fXYD.fsIn(), fXYD.fsIn());
    f->codeAppendf("float2x3 grad_xyd = float2x3(%s, %s);", fdXYDdx.fsIn(), fdXYDdy.fsIn());

    // Erase what the previous hull shader wrote. We don't worry about the two corners falling on
    // the same pixel because those cases should have been weeded out by this point.
    f->codeAppend ("float f = x*x - y;");
    f->codeAppend ("float2 grad_f = float2(2*x, -1) * float2x2(grad_xyd);");
    f->codeAppendf("%s = -(0.5 - f * inversesqrt(dot(grad_f, grad_f)));", outputCoverage);
    f->codeAppendf("%s -= d;", outputCoverage);

    // Use software msaa to approximate coverage at the corner pixels.
    int sampleCount = Shader::DefineSoftSampleLocations(f, "samples");
    f->codeAppendf("float3 xyd_center = float3(%s.xy, %s.z + 0.5);", fXYD.fsIn(), fXYD.fsIn());
    f->codeAppendf("for (int i = 0; i < %i; ++i) {", sampleCount);
    f->codeAppend (    "float3 xyd = grad_xyd * samples[i] + xyd_center;");
    f->codeAppend (    "half f = xyd.y - xyd.x * xyd.x;"); // f > 0 -> inside curve.
    f->codeAppendf(    "%s += all(greaterThan(float2(f,xyd.z), float2(0))) ? %f : 0;",
                       outputCoverage, 1.0 / sampleCount);
    f->codeAppendf("}");
}
