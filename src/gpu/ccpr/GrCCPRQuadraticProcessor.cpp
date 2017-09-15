/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRQuadraticProcessor.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryShaderBuilder.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

void GrCCPRQuadraticProcessor::onEmitVertexShader(const GrCCPRCoverageProcessor& proc,
                                                  GrGLSLVertexBuilder* v,
                                                  const TexelBufferHandle& pointsBuffer,
                                                  const char* atlasOffset, const char* rtAdjust,
                                                  GrGPArgs* gpArgs) const {
    v->codeAppend ("highfloat2 self = ");
    v->appendTexelFetch(pointsBuffer,
                        SkStringPrintf("%s.x + sk_VertexID", proc.instanceAttrib()).c_str());
    v->codeAppendf(".xy + %s;", atlasOffset);
    gpArgs->fPositionVar.set(kHighFloat2_GrSLType, "self");
}

void GrCCPRQuadraticProcessor::emitWind(GrGLSLGeometryBuilder* g, const char* rtAdjust,
                                        const char* outputWind) const {
    // We will define bezierpts in onEmitGeometryShader.
    g->codeAppend ("highfloat area_times_2 = "
                                         "determinant(highfloat2x2(bezierpts[1] - bezierpts[0], "
                                                                  "bezierpts[2] - bezierpts[0]));");
    // Drop curves that are nearly flat, in favor of the higher quality triangle antialiasing.
    g->codeAppendf("if (2 * abs(area_times_2) < length((bezierpts[2] - bezierpts[0]) * %s.zx)) {",
                   rtAdjust);
#ifndef SK_BUILD_FOR_MAC
    g->codeAppend (    "return;");
#else
    // Returning from this geometry shader makes Mac very unhappy. Instead we make wind 0.
    g->codeAppend (    "area_times_2 = 0;");
#endif
    g->codeAppend ("}");
    g->codeAppendf("%s = sign(area_times_2);", outputWind);
}

void GrCCPRQuadraticProcessor::onEmitGeometryShader(GrGLSLGeometryBuilder* g,
                                                    const char* emitVertexFn, const char* wind,
                                                    const char* rtAdjust) const {
    // Prepend bezierpts at the start of the shader.
    g->codePrependf("highfloat3x2 bezierpts = highfloat3x2(sk_in[0].gl_Position.xy, "
                                                          "sk_in[1].gl_Position.xy, "
                                                          "sk_in[2].gl_Position.xy);");

    g->declareGlobal(fCanonicalMatrix);
    g->codeAppendf("%s = highfloat3x3(0.0, 0, 1, "
                                     "0.5, 0, 1, "
                                     "1.0, 1, 1) * "
                        "inverse(highfloat3x3(bezierpts[0], 1, "
                                             "bezierpts[1], 1, "
                                             "bezierpts[2], 1));",
                   fCanonicalMatrix.c_str());

    g->declareGlobal(fCanonicalDerivatives);
    g->codeAppendf("%s = highfloat2x2(%s) * highfloat2x2(%s.x, 0, 0, %s.z);",
                   fCanonicalDerivatives.c_str(), fCanonicalMatrix.c_str(), rtAdjust, rtAdjust);

    g->declareGlobal(fEdgeDistanceEquation);
    g->codeAppendf("highfloat2 edgept0 = bezierpts[%s > 0 ? 2 : 0];", wind);
    g->codeAppendf("highfloat2 edgept1 = bezierpts[%s > 0 ? 0 : 2];", wind);
    this->emitEdgeDistanceEquation(g, "edgept0", "edgept1", fEdgeDistanceEquation.c_str());

    this->emitQuadraticGeometry(g, emitVertexFn, rtAdjust);
}

void GrCCPRQuadraticProcessor::emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                                         const char* /*coverage*/,
                                                         const char* /*wind*/) const {
    fnBody->appendf("%s.xy = (%s * highfloat3(%s, 1)).xy;",
                    fXYD.gsOut(), fCanonicalMatrix.c_str(), position);
    fnBody->appendf("%s.z = dot(%s.xy, %s) + %s.z;",
                    fXYD.gsOut(), fEdgeDistanceEquation.c_str(), position,
                    fEdgeDistanceEquation.c_str());
    this->onEmitPerVertexGeometryCode(fnBody);
}

void GrCCPRQuadraticHullProcessor::emitQuadraticGeometry(GrGLSLGeometryBuilder* g,
                                                         const char* emitVertexFn,
                                                         const char* /*rtAdjust*/) const {
    // Find the t value whose tangent is halfway between the tangents at the endpionts.
    // (We defined bezierpts in onEmitGeometryShader.)
    g->codeAppend ("highfloat2 tan0 = bezierpts[1] - bezierpts[0];");
    g->codeAppend ("highfloat2 tan1 = bezierpts[2] - bezierpts[1];");
    g->codeAppend ("highfloat2 midnorm = normalize(tan0) - normalize(tan1);");
    g->codeAppend ("highfloat2 T = midnorm * highfloat2x2(tan0 - tan1, tan0);");
    g->codeAppend ("highfloat t = clamp(T.t / T.s, 0, 1);"); // T.s=0 is weeded out by this point.

    // Clip the bezier triangle by the tangent at our new t value. This is a simple application for
    // De Casteljau's algorithm.
    g->codeAppendf("highfloat4x2 quadratic_hull = highfloat4x2(bezierpts[0], "
                                                              "bezierpts[0] + tan0 * t, "
                                                              "bezierpts[1] + tan1 * t, "
                                                              "bezierpts[2]);");

    int maxVerts = this->emitHullGeometry(g, emitVertexFn, "quadratic_hull", 4, "sk_InvocationID");

    g->configure(GrGLSLGeometryBuilder::InputType::kTriangles,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 maxVerts, 4);
}

void GrCCPRQuadraticHullProcessor::onEmitPerVertexGeometryCode(SkString* fnBody) const {
    fnBody->appendf("%s = highfloat2(2 * %s.x, -1) * %s;",
                    fGradXY.gsOut(), fXYD.gsOut(), fCanonicalDerivatives.c_str());
}

void GrCCPRQuadraticHullProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                      const char* outputCoverage) const {
    f->codeAppendf("highfloat d = (%s.x * %s.x - %s.y) * inversesqrt(dot(%s, %s));",
                   fXYD.fsIn(), fXYD.fsIn(), fXYD.fsIn(), fGradXY.fsIn(), fGradXY.fsIn());
    f->codeAppendf("%s = clamp(0.5 - d, 0, 1);", outputCoverage);
    f->codeAppendf("%s += min(%s.z, 0);", outputCoverage, fXYD.fsIn()); // Flat closing edge.
}

void GrCCPRQuadraticCornerProcessor::emitQuadraticGeometry(GrGLSLGeometryBuilder* g,
                                                           const char* emitVertexFn,
                                                           const char* rtAdjust) const {
    g->declareGlobal(fEdgeDistanceDerivatives);
    g->codeAppendf("%s = %s.xy * %s.xz;",
                   fEdgeDistanceDerivatives.c_str(), fEdgeDistanceEquation.c_str(), rtAdjust);

    g->codeAppendf("highfloat2 corner = bezierpts[sk_InvocationID * 2];");
    int numVertices = this->emitCornerGeometry(g, emitVertexFn, "corner");

    g->configure(GrGLSLGeometryBuilder::InputType::kTriangles,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip, numVertices, 2);
}

void GrCCPRQuadraticCornerProcessor::onEmitPerVertexGeometryCode(SkString* fnBody) const {
    fnBody->appendf("%s = highfloat3(%s[0].x, %s[0].y, %s.x);",
                    fdXYDdx.gsOut(), fCanonicalDerivatives.c_str(), fCanonicalDerivatives.c_str(),
                    fEdgeDistanceDerivatives.c_str());
    fnBody->appendf("%s = highfloat3(%s[1].x, %s[1].y, %s.y);",
                    fdXYDdy.gsOut(), fCanonicalDerivatives.c_str(), fCanonicalDerivatives.c_str(),
                    fEdgeDistanceDerivatives.c_str());
}

void GrCCPRQuadraticCornerProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                        const char* outputCoverage) const {
    f->codeAppendf("highfloat x = %s.x, y = %s.y, d = %s.z;",
                   fXYD.fsIn(), fXYD.fsIn(), fXYD.fsIn());
    f->codeAppendf("highfloat2x3 grad_xyd = highfloat2x3(%s, %s);", fdXYDdx.fsIn(), fdXYDdy.fsIn());

    // Erase what the previous hull shader wrote. We don't worry about the two corners falling on
    // the same pixel because those cases should have been weeded out by this point.
    f->codeAppend ("highfloat f = x*x - y;");
    f->codeAppend ("highfloat2 grad_f = highfloat2(2*x, -1) * highfloat2x2(grad_xyd);");
    f->codeAppendf("%s = -(0.5 - f * inversesqrt(dot(grad_f, grad_f)));", outputCoverage);
    f->codeAppendf("%s -= d;", outputCoverage);

    // Use software msaa to approximate coverage at the corner pixels.
    int sampleCount = this->defineSoftSampleLocations(f, "samples");
    f->codeAppendf("highfloat3 xyd_center = highfloat3(%s.xy, %s.z + 0.5);",
                   fXYD.fsIn(), fXYD.fsIn());
    f->codeAppendf("for (int i = 0; i < %i; ++i) {", sampleCount);
    f->codeAppend (    "highfloat3 xyd = grad_xyd * samples[i] + xyd_center;");
    f->codeAppend (    "half f = xyd.y - xyd.x * xyd.x;"); // f > 0 -> inside curve.
    f->codeAppendf(    "%s += all(greaterThan(highfloat2(f,xyd.z), highfloat2(0))) ? %f : 0;",
                       outputCoverage, 1.0 / sampleCount);
    f->codeAppendf("}");
}
