/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRCubicShader.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"

void GrCCPRCubicShader::appendInputPointFetch(const GrCCPRCoverageProcessor& proc,
                                              GrGLSLShaderBuilder* s,
                                              const TexelBufferHandle& pointsBuffer,
                                              const char* pointId) const {
    s->appendTexelFetch(pointsBuffer,
                        SkStringPrintf("%s.x + %s", proc.instanceAttrib(), pointId).c_str());
}

void GrCCPRCubicShader::emitWind(GrGLSLShaderBuilder* s, const char* pts,
                                 const char* outputWind) const {

    s->codeAppendf("float area_times_2 = determinant(float3x3(1, %s[0], "
                                                             "1, %s[2], "
                                                             "0, %s[3] - %s[1]));",
                                                             pts, pts, pts, pts);
    // Drop curves that are nearly flat. The KLM  math becomes unstable in this case.
    s->codeAppendf("if (2 * abs(area_times_2) < length(%s[3] - %s[0])) {", pts, pts);
#ifndef SK_BUILD_FOR_MAC
    s->codeAppend (    "return;");
#else
    // Returning from this geometry shader makes Mac very unhappy. Instead we make wind 0.
    s->codeAppend (    "area_times_2 = 0;");
#endif
    s->codeAppend ("}");
    s->codeAppendf("%s = sign(area_times_2);", outputWind);
}

void GrCCPRCubicShader::emitSetupCode(GrGLSLShaderBuilder* s, const char* pts,
                                      const char* segmentId, const char* wind,
                                      GeometryVars* vars) const {
    // Evaluate the cubic at T=.5 for an mid-ish point.
    s->codeAppendf("float2 midpoint = %s * float4(.125, .375, .375, .125);", pts);

    // Find the cubic's power basis coefficients.
    s->codeAppendf("float2x4 C = float4x4(-1,  3, -3,  1, "
                                         " 3, -6,  3,  0, "
                                         "-3,  3,  0,  0, "
                                         " 1,  0,  0,  0) * transpose(%s);", pts);

    // Find the cubic's inflection function.
    s->codeAppend ("float D3 = +determinant(float2x2(C[0].yz, C[1].yz));");
    s->codeAppend ("float D2 = -determinant(float2x2(C[0].xz, C[1].xz));");
    s->codeAppend ("float D1 = +determinant(float2x2(C));");

    // Calculate the KLM matrix.
    s->declareGlobal(fKLMMatrix);
    s->codeAppend ("float4 K, L, M;");
    s->codeAppend ("float2 l, m;");
    s->codeAppend ("float discr = 3*D2*D2 - 4*D1*D3;");
    if (CubicType::kSerpentine == fCubicType) {
        // This math also works out for the "cusp" and "cusp at infinity" cases.
        s->codeAppend ("float q = sqrt(max(3*discr, 0));");
        s->codeAppend ("q = 3*D2 + (D2 >= 0 ? q : -q);");
        s->codeAppend ("l.ts = normalize(float2(q, 6*D1));");
        s->codeAppend ("m.ts = discr <= 0 ? l.ts : normalize(float2(2*D3, q));");
        s->codeAppend ("K = float4(0, l.s * m.s, -l.t * m.s - m.t * l.s, l.t * m.t);");
        s->codeAppend ("L = float4(-1,3,-3,1) * l.ssst * l.sstt * l.sttt;");
        s->codeAppend ("M = float4(-1,3,-3,1) * m.ssst * m.sstt * m.sttt;");
    } else {
        s->codeAppend ("float q = sqrt(max(-discr, 0));");
        s->codeAppend ("q = D2 + (D2 >= 0 ? q : -q);");
        s->codeAppend ("l.ts = normalize(float2(q, 2*D1));");
        s->codeAppend ("m.ts = discr >= 0 ? l.ts : normalize(float2(2 * (D2*D2 - D3*D1), D1*q));");
        s->codeAppend ("float4 lxm = float4(l.s * m.s, l.s * m.t, l.t * m.s, l.t * m.t);");
        s->codeAppend ("K = float4(0, lxm.x, -lxm.y - lxm.z, lxm.w);");
        s->codeAppend ("L = float4(-1,1,-1,1) * l.sstt * (lxm.xyzw + float4(0, 2*lxm.zy, 0));");
        s->codeAppend ("M = float4(-1,1,-1,1) * m.sstt * (lxm.xzyw + float4(0, 2*lxm.yz, 0));");
    }
    s->codeAppend ("short middlerow = abs(D2) > abs(D1) ? 2 : 1;");
    s->codeAppend ("float3x3 CI = inverse(float3x3(C[0][0], C[0][middlerow], C[0][3], "
                                                  "C[1][0], C[1][middlerow], C[1][3], "
                                                  "      0,               0,       1));");
    s->codeAppendf("%s = CI * float3x3(K[0], K[middlerow], K[3], "
                                      "L[0], L[middlerow], L[3], "
                                      "M[0], M[middlerow], M[3]);", fKLMMatrix.c_str());

    // Orient the KLM matrix so we fill the correct side of the curve.
    s->codeAppendf("float2 orientation = sign(float3(midpoint, 1) * float2x3(%s[1], %s[2]));",
                   fKLMMatrix.c_str(), fKLMMatrix.c_str());
    s->codeAppendf("%s *= float3x3(orientation[0] * orientation[1], 0, 0, "
                                  "0, orientation[0], 0, "
                                  "0, 0, orientation[1]);", fKLMMatrix.c_str());

    // TODO: remove in followup CL.
    s->declareGlobal(fKLMDerivatives);
    s->codeAppendf("%s[0] = %s[0].xy;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str());
    s->codeAppendf("%s[1] = %s[1].xy;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str());
    s->codeAppendf("%s[2] = %s[2].xy;",
                   fKLMDerivatives.c_str(), fKLMMatrix.c_str());

    // Determine the amount of additional coverage to subtract out for the flat edge (P3 -> P0).
    s->declareGlobal(fEdgeDistanceEquation);
    s->codeAppendf("short edgeidx0 = %s > 0 ? 3 : 0;", wind);
    s->codeAppendf("float2 edgept0 = %s[edgeidx0];", pts);
    s->codeAppendf("float2 edgept1 = %s[3 - edgeidx0];", pts);
    Shader::EmitEdgeDistanceEquation(s, "edgept0", "edgept1", fEdgeDistanceEquation.c_str());

    this->onEmitSetupCode(s, pts, segmentId, vars);
}

GrCCPRCubicShader::WindHandling
GrCCPRCubicShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler, SkString* code,
                                  const char* position, const char* /*coverage*/,
                                  const char* /*wind*/) {
    varyingHandler->addVarying("klmd", &fKLMD);
    code->appendf("float3 klm = float3(%s, 1) * %s;", position, fKLMMatrix.c_str());
    code->appendf("float d = dot(float3(%s, 1), %s);", position, fEdgeDistanceEquation.c_str());
    code->appendf("%s = float4(klm, d);", fKLMD.gsOut());

    this->onEmitVaryings(varyingHandler, code);
    return WindHandling::kNotHandled;
}

void GrCCPRCubicHullShader::onEmitSetupCode(GrGLSLShaderBuilder* s, const char* /*pts*/,
                                            const char* /*wedgeId*/, GeometryVars* vars) const {
    // "midpoint" was just defined by the base class.
    vars->fHullVars.fAlternateMidpoint = "midpoint";
}

void GrCCPRCubicHullShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler, SkString* code) {
    // "klm" was just defined by the base class.
    varyingHandler->addVarying("grad_matrix", &fGradMatrix);
    code->appendf("%s[0] = 3 * klm[0] * %s[0];", fGradMatrix.gsOut(), fKLMDerivatives.c_str());
    code->appendf("%s[1] = -klm[1] * %s[2].xy - klm[2] * %s[1].xy;",
                    fGradMatrix.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str());
}

void GrCCPRCubicHullShader::onEmitFragmentCode(GrGLSLPPFragmentBuilder* f,
                                               const char* outputCoverage) const {
    f->codeAppendf("float k = %s.x, l = %s.y, m = %s.z, d = %s.w;",
                   fKLMD.fsIn(), fKLMD.fsIn(), fKLMD.fsIn(), fKLMD.fsIn());
    f->codeAppend ("float f = k*k*k - l*m;");
    f->codeAppendf("float2 grad_f = %s * float2(k, 1);", fGradMatrix.fsIn());
    f->codeAppendf("%s = clamp(0.5 - f * inversesqrt(dot(grad_f, grad_f)), 0, 1);", outputCoverage);
    f->codeAppendf("%s += min(d, 0);", outputCoverage); // Flat closing edge.
}

void GrCCPRCubicCornerShader::onEmitSetupCode(GrGLSLShaderBuilder* s, const char* pts,
                                              const char* cornerId, GeometryVars* vars) const {
    // TODO: remove in followup CL.
    s->declareGlobal(fEdgeDistanceDerivatives);
    s->codeAppendf("%s = %s.xy;",
                   fEdgeDistanceDerivatives.c_str(), fEdgeDistanceEquation.c_str());

    s->codeAppendf("float2 corner = %s[%s * 3];", pts, cornerId);
    vars->fCornerVars.fPoint = "corner";
}

void GrCCPRCubicCornerShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler, SkString* code) {
    varyingHandler->addFlatVarying("dklmddx", &fdKLMDdx);
    code->appendf("%s = float4(%s[0].x, %s[1].x, %s[2].x, %s.x);",
                    fdKLMDdx.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str(),
                    fKLMDerivatives.c_str(), fEdgeDistanceDerivatives.c_str());

    varyingHandler->addFlatVarying("dklmddy", &fdKLMDdy);
    code->appendf("%s = float4(%s[0].y, %s[1].y, %s[2].y, %s.y);",
                    fdKLMDdy.gsOut(), fKLMDerivatives.c_str(), fKLMDerivatives.c_str(),
                    fKLMDerivatives.c_str(), fEdgeDistanceDerivatives.c_str());
}

void GrCCPRCubicCornerShader::onEmitFragmentCode(GrGLSLPPFragmentBuilder* f,
                                                 const char* outputCoverage) const {
    f->codeAppendf("float2x4 grad_klmd = float2x4(%s, %s);", fdKLMDdx.fsIn(), fdKLMDdy.fsIn());

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
    const int sampleCount = Shader::DefineSoftSampleLocations(f, "samples");
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
