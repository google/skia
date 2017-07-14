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
    v->codeAppendf("ivec3 indices = ivec3(%s.y, %s.x, %s.y + 1);",
                   proc.instanceAttrib(), proc.instanceAttrib(), proc.instanceAttrib());
    v->codeAppend ("highp vec2 self = ");
    v->appendTexelFetch(pointsBuffer, "indices[sk_VertexID]");
    v->codeAppendf(".xy + %s;", atlasOffset);
    gpArgs->fPositionVar.set(kVec2f_GrSLType, "self");
}

void GrCCPRQuadraticProcessor::emitWind(GrGLSLGeometryBuilder* g, const char* rtAdjust,
                                        const char* outputWind) const {
    // We will define bezierpts in onEmitGeometryShader.
    g->codeAppend ("highp float area_times_2 = determinant(mat2(bezierpts[1] - bezierpts[0], "
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
    g->codePrependf("highp mat3x2 bezierpts = mat3x2(sk_in[0].gl_Position.xy, "
                                                    "sk_in[1].gl_Position.xy, "
                                                    "sk_in[2].gl_Position.xy);");

    g->declareGlobal(fCanonicalMatrix);
    g->codeAppendf("%s = mat3(0.0, 0, 1, "
                             "0.5, 0, 1, "
                             "1.0, 1, 1) * "
                        "inverse(mat3(bezierpts[0], 1, "
                                     "bezierpts[1], 1, "
                                     "bezierpts[2], 1));",
                   fCanonicalMatrix.c_str());

    g->declareGlobal(fCanonicalDerivatives);
    g->codeAppendf("%s = mat2(%s) * mat2(%s.x, 0, 0, %s.z);",
                   fCanonicalDerivatives.c_str(), fCanonicalMatrix.c_str(), rtAdjust, rtAdjust);

    this->emitQuadraticGeometry(g, emitVertexFn, wind, rtAdjust);
}

void GrCCPRQuadraticProcessor::emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                                         const char* /*coverage*/,
                                                         const char* /*wind*/) const {
    fnBody->appendf("%s.xy = (%s * vec3(%s, 1)).xy;",
                    fCanonicalCoord.gsOut(), fCanonicalMatrix.c_str(), position);
    fnBody->appendf("%s.zw = vec2(2 * %s.x * %s[0].x - %s[0].y, "
                                 "2 * %s.x * %s[1].x - %s[1].y);",
                    fCanonicalCoord.gsOut(), fCanonicalCoord.gsOut(),
                    fCanonicalDerivatives.c_str(), fCanonicalDerivatives.c_str(),
                    fCanonicalCoord.gsOut(), fCanonicalDerivatives.c_str(),
                    fCanonicalDerivatives.c_str());
}

void GrCCPRQuadraticProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                  const char* outputCoverage) const {
    f->codeAppendf("highp float d = (%s.x * %s.x - %s.y) * inversesqrt(dot(%s.zw, %s.zw));",
                   fCanonicalCoord.fsIn(), fCanonicalCoord.fsIn(), fCanonicalCoord.fsIn(),
                   fCanonicalCoord.fsIn(), fCanonicalCoord.fsIn());
    f->codeAppendf("%s = clamp(0.5 - d, 0, 1);", outputCoverage);
}

void GrCCPRQuadraticHullProcessor::emitQuadraticGeometry(GrGLSLGeometryBuilder* g,
                                                         const char* emitVertexFn,
                                                         const char* wind,
                                                         const char* rtAdjust) const {
    // Find the point on the curve whose tangent is halfway between the tangents at the endpionts.
    // We defined bezierpts in onEmitGeometryShader.
    g->codeAppend ("highp vec2 n = (normalize(bezierpts[0] - bezierpts[1]) + "
                                   "normalize(bezierpts[2] - bezierpts[1]));");
    g->codeAppend ("highp float t = dot(bezierpts[0] - bezierpts[1], n) / "
                                   "dot(bezierpts[2] - 2 * bezierpts[1] + bezierpts[0], n);");
    g->codeAppend ("highp vec2 pt = (1 - t) * (1 - t) * bezierpts[0] + "
                                   "2 * t * (1 - t) * bezierpts[1] + "
                                   "t * t * bezierpts[2];");

    // Clip the triangle by the tangent line at this halfway point.
    g->codeAppend ("highp mat2 v = mat2(bezierpts[0] - bezierpts[1], "
                                       "bezierpts[2] - bezierpts[1]);");
    g->codeAppend ("highp vec2 nv = n * v;");
    g->codeAppend ("highp vec2 d = abs(nv[0]) > 0.1 * max(bloat.x, bloat.y) ? "
                                  "(dot(n, pt - bezierpts[1])) / nv : vec2(0);");

    // Generate a 4-point hull of the curve from the clipped triangle.
    g->codeAppendf("highp mat4x2 quadratic_hull = mat4x2(bezierpts[0], "
                                                        "bezierpts[1] + d[0] * v[0], "
                                                        "bezierpts[1] + d[1] * v[1], "
                                                        "bezierpts[2]);");

    int maxVerts = this->emitHullGeometry(g, emitVertexFn, "quadratic_hull", 4, "sk_InvocationID");

    g->configure(GrGLSLGeometryBuilder::InputType::kTriangles,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 maxVerts, 4);
}

void GrCCPRQuadraticSharedEdgeProcessor::emitQuadraticGeometry(GrGLSLGeometryBuilder* g,
                                                               const char* emitVertexFn,
                                                               const char* wind,
                                                               const char* rtAdjust) const {
    // We defined bezierpts in onEmitGeometryShader.
    g->codeAppendf("int leftidx = %s > 0 ? 2 : 0;", wind);
    g->codeAppendf("highp vec2 left = bezierpts[leftidx];");
    g->codeAppendf("highp vec2 right = bezierpts[2 - leftidx];");
    this->emitEdgeDistanceEquation(g, "left", "right", "highp vec3 edge_distance_equation");

    g->declareGlobal(fEdgeDistanceDerivatives);
    g->codeAppendf("%s = edge_distance_equation.xy * %s.xz;",
                   fEdgeDistanceDerivatives.c_str(), rtAdjust);

    int maxVertices = this->emitEdgeGeometry(g, emitVertexFn, "left", "right",
                                             "edge_distance_equation");

    g->configure(GrGLSLGeometryBuilder::InputType::kTriangles,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip, maxVertices, 1);
}

void GrCCPRQuadraticSharedEdgeProcessor::emitPerVertexGeometryCode(SkString* fnBody,
                                                                   const char* position,
                                                                   const char* coverage,
                                                                   const char* wind) const {
    this->INHERITED::emitPerVertexGeometryCode(fnBody, position, coverage, wind);
    fnBody->appendf("%s = %s;", fFragCanonicalDerivatives.gsOut(), fCanonicalDerivatives.c_str());
    fnBody->appendf("%s.x = %s + 0.5;", fEdgeDistance.gsOut(), coverage); // outer=-.5, inner=+.5.
    fnBody->appendf("%s.yz = %s;", fEdgeDistance.gsOut(), fEdgeDistanceDerivatives.c_str());
}

void GrCCPRQuadraticSharedEdgeProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                            const char* outputCoverage) const {
    // Erase what the previous hull shader wrote and replace with edge coverage.
    this->INHERITED::emitShaderCoverage(f, outputCoverage);
    f->codeAppendf("%s = %s.x + 0.5 - %s;",
                   outputCoverage, fEdgeDistance.fsIn(), outputCoverage);

    // Use software msaa to subtract out the remaining pixel coverage that is still inside the
    // shared edge, but outside the curve.
    int sampleCount = this->defineSoftSampleLocations(f, "samples");

    f->codeAppendf("highp mat2x3 grad_xyd = mat2x3(%s[0],%s.y, %s[1],%s.z);",
                   fFragCanonicalDerivatives.fsIn(), fEdgeDistance.fsIn(),
                   fFragCanonicalDerivatives.fsIn(), fEdgeDistance.fsIn());
    f->codeAppendf("highp vec3 center_xyd = vec3(%s.xy, %s.x);",
                   fCanonicalCoord.fsIn(), fEdgeDistance.fsIn());

    f->codeAppendf("for (int i = 0; i < %i; ++i) {", sampleCount);
    f->codeAppend (    "highp vec3 xyd = grad_xyd * samples[i] + center_xyd;");
    f->codeAppend (    "lowp float f = xyd.x * xyd.x - xyd.y;"); // f > 0 -> outside curve.
    f->codeAppend (    "bvec2 outside_curve_inside_edge = greaterThan(vec2(f, xyd.z), vec2(0));");
    f->codeAppendf(    "%s -= all(outside_curve_inside_edge) ? %f : 0;",
                       outputCoverage, 1.0 / sampleCount);
    f->codeAppendf("}");
}
