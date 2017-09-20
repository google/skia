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
    v->codeAppend ("float2 self = ");
    v->appendTexelFetch(pointsBuffer,
                        SkStringPrintf("%s.x + sk_VertexID", proc.instanceAttrib()).c_str());
    v->codeAppendf(".xy + %s;", atlasOffset);
    gpArgs->fPositionVar.set(kFloat2_GrSLType, "self");
}

void GrCCPRCubicProcessor::emitWind(GrGLSLGeometryBuilder* g, const char* rtAdjust,
                                    const char* outputWind) const {
    // We will define bezierpts in onEmitGeometryShader.
    g->codeAppend ("float area_times_2 = "
                                      "determinant(float3x3(1, bezierpts[0], "
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
    g->codePrependf("float4x2 bezierpts = float4x2(sk_in[0].gl_Position.xy, "
                                                          "sk_in[1].gl_Position.xy, "
                                                          "sk_in[2].gl_Position.xy, "
                                                          "sk_in[3].gl_Position.xy);");

    // Evaluate the cubic at T=.5 for an mid-ish point.
    g->codeAppendf("float2 midpoint = bezierpts * float4(.125, .375, .375, .125);");

    // Find the cubic's power basis coefficients.
    g->codeAppend ("float2x4 C = float4x4(-1,  3, -3,  1, "
                                                 " 3, -6,  3,  0, "
                                                 "-3,  3,  0,  0, "
                                                 " 1,  0,  0,  0) * transpose(bezierpts);");

    // Find the cubic's inflection function.
    g->codeAppend ("float D3 = +determinant(float2x2(C[0].yz, C[1].yz));");
    g->codeAppend ("float D2 = -determinant(float2x2(C[0].xz, C[1].xz));");
    g->codeAppend ("float D1 = +determinant(float2x2(C));");

    // Calculate the KLM matrix.
    g->declareGlobal(fKLMMatrix);
    g->codeAppend ("float4 K, L, M;");
    g->codeAppend ("float2 l, m;");
    g->codeAppend ("float discr = 3*D2*D2 - 4*D1*D3;");
    if (CubicType::kSerpentine == fCubicType) {
        // This math also works out for the "cusp" and "cusp at infinity" cases.
        g->codeAppend ("float q = 3*D2 + sign(D2) * sqrt(max(3*discr, 0));");
        g->codeAppend ("l.ts = normalize(float2(q, 6*D1));");
        g->codeAppend ("m.ts = discr <= 0 ? l.ts : normalize(float2(2*D3, q));");
        g->codeAppend ("K = float4(0, l.s * m.s, -l.t * m.s - m.t * l.s, l.t * m.t);");
        g->codeAppend ("L = float4(-1,3,-3,1) * l.ssst * l.sstt * l.sttt;");
        g->codeAppend ("M = float4(-1,3,-3,1) * m.ssst * m.sstt * m.sttt;");
    } else {
        g->codeAppend ("float q = D2 + sign(D2) * sqrt(max(-discr, 0));");
        g->codeAppend ("l.ts = normalize(float2(q, 2*D1));");
        g->codeAppend ("m.ts = discr >= 0 ? l.ts : normalize(float2(2 * (D2*D2 - D3*D1), D1*q));");
        g->codeAppend ("float4 lxm = float4(l.s * m.s, l.s * m.t, l.t * m.s, l.t * m.t);");
        g->codeAppend ("K = float4(0, lxm.x, -lxm.y - lxm.z, lxm.w);");
        g->codeAppend ("L = float4(-1,1,-1,1) * l.sstt * (lxm.xyzw + float4(0, 2*lxm.zy, 0));");
        g->codeAppend ("M = float4(-1,1,-1,1) * m.sstt * (lxm.xzyw + float4(0, 2*lxm.yz, 0));");
    }
    g->codeAppend ("short middlerow = abs(D2) > abs(D1) ? 2 : 1;");
    g->codeAppend ("float3x3 CI = inverse(float3x3(C[0][0], C[0][middlerow], C[0][3], "
                                                          "C[1][0], C[1][middlerow], C[1][3], "
                                                          "      0,               0,       1));");
    g->codeAppendf("%s = CI * float3x3(K[0], K[middlerow], K[3], "
                                          "L[0], L[middlerow], L[3], "
                                          "M[0], M[middlerow], M[3]);", fKLMMatrix.c_str());

    // Orient the KLM matrix so we fill the correct side of the curve.
    g->codeAppendf("half2 orientation = sign(half3(midpoint, 1) * half2x3(%s[1], %s[2]));",
                   fKLMMatrix.c_str(), fKLMMatrix.c_str());
    g->codeAppendf("%s *= float3x3(orientation[0] * orientation[1], 0, 0, "
                                      "0, orientation[0], 0, "
                                      "0, 0, orientation[1]);", fKLMMatrix.c_str());

    g->declareGlobal(fKLMDerivatives);
    g->codeAppendf("%s[0] = %s[0].xy * %s.xz;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str(), rtAdjust);
    g->codeAppendf("%s[1] = %s[1].xy * %s.xz;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str(), rtAdjust);
    g->codeAppendf("%s[2] = %s[2].xy * %s.xz;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str(), rtAdjust);

    // Determine the amount of additional coverage to subtract out for the flat edge (P3 -> P0).
    g->declareGlobal(fEdgeDistanceEquation);
    g->codeAppendf("short edgeidx0 = %s > 0 ? 3 : 0;", wind);
    g->codeAppendf("float2 edgept0 = bezierpts[edgeidx0];");
    g->codeAppendf("float2 edgept1 = bezierpts[3 - edgeidx0];");
    this->emitEdgeDistanceEquation(g, "edgept0", "edgept1", fEdgeDistanceEquation.c_str());

    this->emitCubicGeometry(g, emitVertexFn, wind, rtAdjust);
}

void GrCCPRCubicProcessor::emitPerVertexGeometryCode(SkString* fnBody, const char* position,
                                                     const char* /*coverage*/,
                                                     const char* /*wind*/) const {
    fnBody->appendf("float3 klm = float3(%s, 1) * %s;", position, fKLMMatrix.c_str());
    fnBody->appendf("float d = dot(float3(%s, 1), %s);",
                    position, fEdgeDistanceEquation.c_str());
    fnBody->appendf("%s = float4(klm, d);", fKLMD.gsOut());
    this->onEmitPerVertexGeometryCode(fnBody);
}

void GrCCPRCubicHullProcessor::emitCubicGeometry(GrGLSLGeometryBuilder* g, const char* emitVertexFn,
                                                 const char* wind, const char* rtAdjust) const {
    // FIXME: we should clip this geometry at the tip of the curve.
    int maxVertices = this->emitHullGeometry(g, emitVertexFn, "bezierpts", 4, "sk_InvocationID",
                                             "midpoint");

    g->configure(GrGLSLGeometryBuilder::InputType::kLinesAdjacency,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip,
                 maxVertices, 4);
}

void GrCCPRCubicHullProcessor::onEmitPerVertexGeometryCode(SkString* fnBody) const {
    // "klm" was just defined by the base class.
    fnBody->appendf("%s[0] = 3 * klm[0] * %s[0];", fGradMatrix.gsOut(), fKLMDerivatives.c_str());
    fnBody->appendf("%s[1] = -klm[1] * %s[2].xy - klm[2] * %s[1].xy;",
                    fGradMatrix.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str());
}

void GrCCPRCubicHullProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                  const char* outputCoverage) const {
    f->codeAppendf("float k = %s.x, l = %s.y, m = %s.z, d = %s.w;",
                   fKLMD.fsIn(), fKLMD.fsIn(), fKLMD.fsIn(), fKLMD.fsIn());
    f->codeAppend ("float f = k*k*k - l*m;");
    f->codeAppendf("float2 grad_f = %s * float2(k, 1);", fGradMatrix.fsIn());
    f->codeAppendf("%s = clamp(0.5 - f * inversesqrt(dot(grad_f, grad_f)), 0, 1);", outputCoverage);
    f->codeAppendf("%s += min(d, 0);", outputCoverage); // Flat closing edge.
}

void GrCCPRCubicCornerProcessor::emitCubicGeometry(GrGLSLGeometryBuilder* g,
                                                   const char* emitVertexFn, const char* wind,
                                                   const char* rtAdjust) const {
    // We defined bezierpts in onEmitGeometryShader.
    g->declareGlobal(fEdgeDistanceDerivatives);
    g->codeAppendf("%s = %s.xy * %s.xz;",
                   fEdgeDistanceDerivatives.c_str(), fEdgeDistanceEquation.c_str(), rtAdjust);

    g->codeAppendf("float2 corner = bezierpts[sk_InvocationID * 3];");
    int numVertices = this->emitCornerGeometry(g, emitVertexFn, "corner");

    g->configure(GrGLSLGeometryBuilder::InputType::kLinesAdjacency,
                 GrGLSLGeometryBuilder::OutputType::kTriangleStrip, numVertices, 2);
}

void GrCCPRCubicCornerProcessor::onEmitPerVertexGeometryCode(SkString* fnBody) const {
    fnBody->appendf("%s = float4(%s[0].x, %s[1].x, %s[2].x, %s.x);",
                    fdKLMDdx.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str(),
                    fKLMDerivatives.c_str(), fEdgeDistanceDerivatives.c_str());
    fnBody->appendf("%s = float4(%s[0].y, %s[1].y, %s[2].y, %s.y);",
                    fdKLMDdy.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str(),
                    fKLMDerivatives.c_str(), fEdgeDistanceDerivatives.c_str());

    // Otherwise, fEdgeDistances = fEdgeDistances * sign(wind * rtAdjust.x * rdAdjust.z).
    GR_STATIC_ASSERT(kTopLeft_GrSurfaceOrigin == GrCCPRCoverageProcessor::kAtlasOrigin);
}

void GrCCPRCubicCornerProcessor::emitShaderCoverage(GrGLSLFragmentBuilder* f,
                                                    const char* outputCoverage) const {
    f->codeAppendf("float2x4 grad_klmd = float2x4(%s, %s);",
                   fdKLMDdx.fsIn(), fdKLMDdy.fsIn());

    // Erase what the previous hull shader wrote. We don't worry about the two corners falling on
    // the same pixel because those cases should have been weeded out by this point.
    f->codeAppendf("float k = %s.x, l = %s.y, m = %s.z, d = %s.w;",
                   fKLMD.fsIn(), fKLMD.fsIn(), fKLMD.fsIn(), fKLMD.fsIn());
    f->codeAppend ("float f = k*k*k - l*m;");
    f->codeAppend ("float2 grad_f = float3(3*k*k, -m, -l) * float2x3(grad_klmd);");
    f->codeAppendf("%s = -clamp(0.5 - f * inversesqrt(dot(grad_f, grad_f)), 0, 1);",
                   outputCoverage);
    f->codeAppendf("%s -= d;", outputCoverage);

    // Use software msaa to estimate actual coverage at the corner pixels.
    const int sampleCount = this->defineSoftSampleLocations(f, "samples");
    f->codeAppendf("float4 klmd_center = float4(%s.xyz, %s.w + 0.5);",
                   fKLMD.fsIn(), fKLMD.fsIn());
    f->codeAppendf("for (int i = 0; i < %i; ++i) {", sampleCount);
    f->codeAppend (    "float4 klmd = grad_klmd * samples[i] + klmd_center;");
    f->codeAppend (    "half f = klmd.y * klmd.z - klmd.x * klmd.x * klmd.x;");
    f->codeAppendf(    "%s += all(greaterThan(half4(f, klmd.y, klmd.z, klmd.w), "
                                             "half4(0))) ? %f : 0;",
                       outputCoverage, 1.0 / sampleCount);
    f->codeAppend ("}");
}
