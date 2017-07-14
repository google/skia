/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRCubicProcessor.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLGeometryShaderBuilder.h"
#include "glsl/GrGLSLVertexShaderBuilder.h"

void GrCCPRCubicProcessor::onEmitVertexShader(const GrCCPRCoverageProcessor& proc,
                                              GrGLSLVertexBuilder* v,
                                              const TexelBufferHandle& pointsBuffer,
                                              const char* atlasOffset, const char* rtAdjust,
                                              GrGPArgs* gpArgs) const {
    float inset = 1 - kAABloatRadius;
#ifdef SK_DEBUG
    if (proc.debugVisualizations()) {
        inset *= GrCCPRCoverageProcessor::kDebugBloat;
    }
#endif

    // Fetch all 4 cubic bezier points.
    v->codeAppendf("ivec4 indices = ivec4(%s.y, %s.x, %s.x + 1, %s.y + 1);",
                   proc.instanceAttrib(), proc.instanceAttrib(), proc.instanceAttrib(),
                   proc.instanceAttrib());
    v->codeAppend ("highp mat4x2 bezierpts = mat4x2(");
    v->appendTexelFetch(pointsBuffer, "indices[sk_VertexID]");
    v->codeAppend (".xy, ");
    v->appendTexelFetch(pointsBuffer, "indices[(sk_VertexID + 1) % 4]");
    v->codeAppend (".xy, ");
    v->appendTexelFetch(pointsBuffer, "indices[(sk_VertexID + 2) % 4]");
    v->codeAppend (".xy, ");
    v->appendTexelFetch(pointsBuffer, "indices[(sk_VertexID + 3) % 4]");
    v->codeAppend (".xy);");

    // Find the corner of the inset geometry that corresponds to this bezier vertex (bezierpts[0]).
    v->codeAppend ("highp mat2 N = mat2(bezierpts[3].y - bezierpts[0].y, "
                                       "bezierpts[0].x - bezierpts[3].x, "
                                       "bezierpts[1].y - bezierpts[0].y, "
                                       "bezierpts[0].x - bezierpts[1].x);");
    v->codeAppend ("highp mat2 P = mat2(bezierpts[3], bezierpts[1]);");
    v->codeAppend ("if (abs(determinant(N)) < 2) {"); // Area of [pts[3], pts[0], pts[1]] < 1px.
                       // The inset corner doesn't exist because we are effectively colinear with
                       // both neighbor vertices. Just duplicate a neighbor's inset corner.
    v->codeAppend (    "int smallidx = (dot(N[0], N[0]) > dot(N[1], N[1])) ? 1 : 0;");
    v->codeAppend (    "N[smallidx] = vec2(bezierpts[2].y - bezierpts[3 - smallidx * 2].y, "
                                          "bezierpts[3 - smallidx * 2].x - bezierpts[2].x);");
    v->codeAppend (    "P[smallidx] = bezierpts[2];");
    v->codeAppend ("}");
    v->codeAppend ("N[0] *= sign(dot(N[0], P[1] - P[0]));");
    v->codeAppend ("N[1] *= sign(dot(N[1], P[0] - P[1]));");

    v->codeAppendf("highp vec2 K = vec2(dot(N[0], P[0] + %f * sign(N[0])), "
                                       "dot(N[1], P[1] + %f * sign(N[1])));", inset, inset);
    v->codeAppendf("%s.xy = K * inverse(N) + %s;", fInset.vsOut(), atlasOffset);
    v->codeAppendf("%s.xy = %s.xy * %s.xz + %s.yw;",
                   fInset.vsOut(), fInset.vsOut(), rtAdjust, rtAdjust);

    // The z component tells the gemetry shader how "sharp" this corner is.
    v->codeAppendf("%s.z = determinant(N) * sign(%s.x) * sign(%s.z);",
                   fInset.vsOut(), rtAdjust, rtAdjust);

    // Fetch one of the t,s klm root values for the geometry shader.
    v->codeAppendf("%s = ", fTS.vsOut());
    v->appendTexelFetch(pointsBuffer,
                        SkStringPrintf("%s.x + 2 + sk_VertexID/2", proc.instanceAttrib()).c_str());
    v->codeAppend ("[sk_VertexID % 2];");

    // Emit the vertex position.
    v->codeAppendf("highp vec2 self = bezierpts[0] + %s;", atlasOffset);
    gpArgs->fPositionVar.set(kVec2f_GrSLType, "self");
}

void GrCCPRCubicProcessor::emitWind(GrGLSLGeometryBuilder* g, const char* rtAdjust,
                                    const char* outputWind) const {
    // We will define bezierpts in onEmitGeometryShader.
    g->codeAppend ("highp float area_times_2 = determinant(mat3(1, bezierpts[0], "
                                                               "1, bezierpts[2], "
                                                               "0, bezierpts[3] - bezierpts[1]));");
    // Drop curves that are nearly flat. The KLM  math becomes unstable in this case.
    g->codeAppendf("if (2 * abs(area_times_2) < length((bezierpts[3] - bezierpts[0]) * %s.zx)) {",
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

void GrCCPRCubicProcessor::onEmitGeometryShader(GrGLSLGeometryBuilder* g, const char* emitVertexFn,
                                                const char* wind, const char* rtAdjust) const {
    // Prepend bezierpts at the start of the shader.
    g->codePrependf("highp mat4x2 bezierpts = mat4x2(sk_in[0].gl_Position.xy, "
                                                    "sk_in[1].gl_Position.xy, "
                                                    "sk_in[2].gl_Position.xy, "
                                                    "sk_in[3].gl_Position.xy);");

    // Evaluate the cubic at t=.5 for an approximate midpoint.
    g->codeAppendf("highp vec2 midpoint = bezierpts * vec4(.125, .375, .375, .125);");

    // Finish finding the inset geometry we started in the vertex shader. The z component tells us
    // how "sharp" an inset corner is. And the vertex shader already skips one corner if it is
    // colinear with its neighbors. So at this point, if a corner is flat, it means the inset
    // geometry is all empty (it should never be non-convex because the curve gets chopped into
    // convex segments ahead of time).
    g->codeAppendf("bool isempty = "
                       "any(lessThan(vec4(%s[0].z, %s[1].z, %s[2].z, %s[3].z) * %s, vec4(2)));",
                   fInset.gsIn(), fInset.gsIn(), fInset.gsIn(), fInset.gsIn(), wind);
    g->codeAppendf("highp vec2 inset[4];");
    g->codeAppend ("for (int i = 0; i < 4; ++i) {");
    g->codeAppendf(    "inset[i] = isempty ? midpoint : %s[i].xy;", fInset.gsIn());
    g->codeAppend ("}");

    // We determine crossover and/or degeneracy by how many inset edges run the opposite direction
    // of their corresponding bezier edge. If there is one backwards edge, the inset geometry is
    // actually triangle with a vertex at the crossover point. If there are >1 backwards edges, the
    // inset geometry doesn't exist (i.e. the bezier quadrilateral isn't large enough) and we
    // degenerate to the midpoint.
    g->codeAppend ("lowp float backwards[4];");
    g->codeAppend ("lowp int numbackwards = 0;");
    g->codeAppend ("for (int i = 0; i < 4; ++i) {");
    g->codeAppend (    "lowp int j = (i + 1) % 4;");
    g->codeAppendf(    "highp vec2 inner = inset[j] - inset[i];");
    g->codeAppendf(    "highp vec2 outer = sk_in[j].gl_Position.xy - sk_in[i].gl_Position.xy;");
    g->codeAppendf(    "backwards[i] = sign(dot(outer, inner));");
    g->codeAppendf(    "numbackwards += backwards[i] < 0 ? 1 : 0;");
    g->codeAppend ("}");

    // Find the crossover point. If there actually isn't one, this math is meaningless and will get
    // dropped on the floor later.
    g->codeAppend ("lowp int x = (backwards[0] != backwards[2]) ? 1 : 0;");
    g->codeAppend ("lowp int x3 = (x + 3) % 4;");
    g->codeAppend ("highp mat2 X = mat2(inset[x].y - inset[x+1].y, "
                                       "inset[x+1].x - inset[x].x, "
                                       "inset[x+2].y - inset[x3].y, "
                                       "inset[x3].x - inset[x+2].x);");
    g->codeAppend ("highp vec2 KK = vec2(dot(X[0], inset[x]), dot(X[1], inset[x+2]));");
    g->codeAppend ("highp vec2 crossoverpoint = KK * inverse(X);");

    // Determine what point backwards edges should collapse into. If there is one backwards edge,
    // it should collapse to the crossover point. If >1, they should all collapse to the midpoint.
    g->codeAppend ("highp vec2 collapsepoint = numbackwards == 1 ? crossoverpoint : midpoint;");

    // Collapse backwards egdes to the "collapse" point.
    g->codeAppend ("for (int i = 0; i < 4; ++i) {");
    g->codeAppend (    "if (backwards[i] < 0) {");
    g->codeAppend (        "inset[i] = inset[(i + 1) % 4] = collapsepoint;");
    g->codeAppend (    "}");
    g->codeAppend ("}");

    // Calculate the KLM matrix.
    g->declareGlobal(fKLMMatrix);
    g->codeAppend ("highp vec4 K, L, M;");
    if (Type::kSerpentine == fType) {
        g->codeAppend ("highp vec2 l,m;");
        g->codeAppendf("l.ts = vec2(%s[0], %s[1]);", fTS.gsIn(), fTS.gsIn());
        g->codeAppendf("m.ts = vec2(%s[2], %s[3]);", fTS.gsIn(), fTS.gsIn());
        g->codeAppend ("K = vec4(0, l.s * m.s, -l.t * m.s - m.t * l.s, l.t * m.t);");
        g->codeAppend ("L = vec4(-1,3,-3,1) * l.ssst * l.sstt * l.sttt;");
        g->codeAppend ("M = vec4(-1,3,-3,1) * m.ssst * m.sstt * m.sttt;");

    } else {
        g->codeAppend ("highp vec2 d,e;");
        g->codeAppendf("d.ts = vec2(%s[0], %s[1]);", fTS.gsIn(), fTS.gsIn());
        g->codeAppendf("e.ts = vec2(%s[2], %s[3]);", fTS.gsIn(), fTS.gsIn());
        g->codeAppend ("highp vec4 dxe = vec4(d.s * e.s, d.s * e.t, d.t * e.s, d.t * e.t);");
        g->codeAppend ("K = vec4(0, dxe.x, -dxe.y - dxe.z, dxe.w);");
        g->codeAppend ("L = vec4(-1,1,-1,1) * d.sstt * (dxe.xyzw + vec4(0, 2*dxe.zy, 0));");
        g->codeAppend ("M = vec4(-1,1,-1,1) * e.sstt * (dxe.xzyw + vec4(0, 2*dxe.yz, 0));");
    }

    g->codeAppend ("highp mat2x4 C = mat4(-1,  3, -3,  1, "
                                         " 3, -6,  3,  0, "
                                         "-3,  3,  0,  0, "
                                         " 1,  0,  0,  0) * transpose(bezierpts);");

    g->codeAppend ("highp vec2 absdet = abs(C[0].xx * C[1].zy - C[1].xx * C[0].zy);");
    g->codeAppend ("lowp int middlerow = absdet[0] > absdet[1] ? 2 : 1;");

    g->codeAppend ("highp mat3 CI = inverse(mat3(C[0][0], C[0][middlerow], C[0][3], "
                                                "C[1][0], C[1][middlerow], C[1][3], "
                                                "      0,               0,       1));");
    g->codeAppendf("%s = CI * mat3(K[0], K[middlerow], K[3], "
                                  "L[0], L[middlerow], L[3], "
                                  "M[0], M[middlerow], M[3]);", fKLMMatrix.c_str());

    // Orient the KLM matrix so we fill the correct side of the curve.
    g->codeAppendf("lowp vec2 orientation = sign(vec3(midpoint, 1) * mat2x3(%s[1], %s[2]));",
                   fKLMMatrix.c_str(), fKLMMatrix.c_str());
    g->codeAppendf("%s *= mat3(orientation[0] * orientation[1], 0, 0, "
                              "0, orientation[0], 0, "
                              "0, 0, orientation[1]);", fKLMMatrix.c_str());

    g->declareGlobal(fKLMDerivatives);
    g->codeAppendf("%s[0] = %s[0].xy * %s.xz;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str(), rtAdjust);
    g->codeAppendf("%s[1] = %s[1].xy * %s.xz;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str(), rtAdjust);
    g->codeAppendf("%s[2] = %s[2].xy * %s.xz;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str(), rtAdjust);

    this->emitCubicGeometry(g, emitVertexFn, wind, rtAdjust);
}

void GrCCPRCubicInsetProcessor::emitCubicGeometry(GrGLSLGeometryBuilder* g,
                                                  const char* emitVertexFn, const char* wind,
                                                  const char* rtAdjust) const {
    // FIXME: we should clip this geometry at the tip of the curve.
    g->codeAppendf("%s(inset[0], 1);", emitVertexFn);
    g->codeAppendf("%s(inset[1], 1);", emitVertexFn);
    g->codeAppendf("%s(inset[3], 1);", emitVertexFn);
    g->codeAppendf("%s(inset[2], 1);", emitVertexFn);
    g->codeAppend ("EndPrimitive();");

    g->configure(GrGLSLGeometryBuilder::InputType::kLinesAdjacency,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 4, 1);
}

void GrCCPRCubicInsetProcessor::emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                                          const char* /*coverage*/,
                                                          const char* /*wind*/) const {
    fnBody->appendf("highp vec3 klm = vec3(%s, 1) * %s;", position, fKLMMatrix.c_str());
    fnBody->appendf("%s = klm;", fKLM.gsOut());
    fnBody->appendf("%s[0] = 3 * klm[0] * %s[0];", fGradMatrix.gsOut(), fKLMDerivatives.c_str());
    fnBody->appendf("%s[1] = -klm[1] * %s[2].xy - klm[2] * %s[1].xy;",
                    fGradMatrix.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str());
}

void GrCCPRCubicInsetProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                   const char* outputCoverage) const {
    f->codeAppendf("highp float k = %s.x, l = %s.y, m = %s.z;",
                   fKLM.fsIn(), fKLM.fsIn(), fKLM.fsIn());
    f->codeAppend ("highp float f = k*k*k - l*m;");
    f->codeAppendf("highp vec2 grad = %s * vec2(k, 1);", fGradMatrix.fsIn());
    f->codeAppend ("highp float d = f * inversesqrt(dot(grad, grad));");
    f->codeAppendf("%s = clamp(0.5 - d, 0, 1);", outputCoverage);
}

void GrCCPRCubicBorderProcessor::emitCubicGeometry(GrGLSLGeometryBuilder* g,
                                                   const char* emitVertexFn, const char* wind,
                                                   const char* rtAdjust) const {
    // We defined bezierpts in onEmitGeometryShader.
    g->declareGlobal(fEdgeDistanceEquation);
    g->codeAppendf("int edgeidx0 = %s > 0 ? 3 : 0;", wind);
    g->codeAppendf("highp vec2 edgept0 = bezierpts[edgeidx0];");
    g->codeAppendf("highp vec2 edgept1 = bezierpts[3 - edgeidx0];");
    this->emitEdgeDistanceEquation(g, "edgept0", "edgept1", fEdgeDistanceEquation.c_str());
    g->codeAppendf("%s.z += 0.5;", fEdgeDistanceEquation.c_str()); // outer = -.5, inner = .5

    g->declareGlobal(fEdgeDistanceDerivatives);
    g->codeAppendf("%s = %s.xy * %s.xz;",
                   fEdgeDistanceDerivatives.c_str(), fEdgeDistanceEquation.c_str(), rtAdjust);

    g->declareGlobal(fEdgeSpaceTransform);
    g->codeAppend ("highp vec4 edgebbox = vec4(min(bezierpts[0], bezierpts[3]) - bloat, "
                                              "max(bezierpts[0], bezierpts[3]) + bloat);");
    g->codeAppendf("%s.xy = 2 / vec2(edgebbox.zw - edgebbox.xy);", fEdgeSpaceTransform.c_str());
    g->codeAppendf("%s.zw = -1 - %s.xy * edgebbox.xy;",
                   fEdgeSpaceTransform.c_str(), fEdgeSpaceTransform.c_str());

    int maxVertices = this->emitHullGeometry(g, emitVertexFn, "bezierpts", 4, "sk_InvocationID",
                                             "inset");

    g->configure(GrGLSLGeometryBuilder::InputType::kLinesAdjacency,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 maxVertices, 4);
}

void GrCCPRCubicBorderProcessor::emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                                           const char* /*coverage*/,
                                                           const char* /*wind*/) const {
    fnBody->appendf("highp vec3 klm = vec3(%s, 1) * %s;", position, fKLMMatrix.c_str());
    fnBody->appendf("highp float d = dot(vec3(%s, 1), %s);",
                    position, fEdgeDistanceEquation.c_str());
    fnBody->appendf("%s = vec4(klm, d);", fKLMD.gsOut());
    fnBody->appendf("%s = vec4(%s[0].x, %s[1].x, %s[2].x, %s.x);",
                    fdKLMDdx.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str(),
                    fKLMDerivatives.c_str(), fEdgeDistanceDerivatives.c_str());
    fnBody->appendf("%s = vec4(%s[0].y, %s[1].y, %s[2].y, %s.y);",
                    fdKLMDdy.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str(),
                    fKLMDerivatives.c_str(), fEdgeDistanceDerivatives.c_str());
    fnBody->appendf("%s = position * %s.xy + %s.zw;", fEdgeSpaceCoord.gsOut(),
                    fEdgeSpaceTransform.c_str(), fEdgeSpaceTransform.c_str());

    // Otherwise, fEdgeDistances = fEdgeDistances * sign(wind * rtAdjust.x * rdAdjust.z).
    GR_STATIC_ASSERT(kTopLeft_GrSurfaceOrigin == GrCCPRCoverageProcessor::kAtlasOrigin);
}

void GrCCPRCubicBorderProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                    const char* outputCoverage) const {
    // Use software msaa to determine coverage.
    const int sampleCount = this->defineSoftSampleLocations(f, "samples");

    // Along the shared edge, we start with distance-to-edge coverage, then subtract out the
    // remaining pixel coverage that is still inside the shared edge, but outside the curve.
    // Outside the shared edege, we just use standard msaa to count samples inside the curve.
    f->codeAppendf("bool use_edge = all(lessThan(abs(%s), vec2(1)));", fEdgeSpaceCoord.fsIn());
    f->codeAppendf("%s = (use_edge ? clamp(%s.w + 0.5, 0, 1) : 0) * %i;",
                   outputCoverage, fKLMD.fsIn(), sampleCount);

    f->codeAppendf("highp mat2x4 grad_klmd = mat2x4(%s, %s);", fdKLMDdx.fsIn(), fdKLMDdy.fsIn());

    f->codeAppendf("for (int i = 0; i < %i; ++i) {", sampleCount);
    f->codeAppendf(    "highp vec4 klmd = grad_klmd * samples[i] + %s;", fKLMD.fsIn());
    f->codeAppend (    "lowp float f = klmd.y * klmd.z - klmd.x * klmd.x * klmd.x;");
    // A sample is inside our cubic sub-section if it is inside the implicit AND L & M are both
    // positive. This works because the sections get chopped at the K/L and K/M intersections.
    f->codeAppend (    "bvec4 inside = greaterThan(vec4(f,klmd.yzw), vec4(0));");
    f->codeAppend (    "lowp float in_curve = all(inside.xyz) ? 1 : 0;");
    f->codeAppend (    "lowp float in_edge = inside.w ? 1 : 0;");
    f->codeAppendf(    "%s += use_edge ? in_edge * (in_curve - 1) : in_curve;", outputCoverage);
    f->codeAppend ("}");

    f->codeAppendf("%s *= %f;", outputCoverage, 1.0 / sampleCount);
}
