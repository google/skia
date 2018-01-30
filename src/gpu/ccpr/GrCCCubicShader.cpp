/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCCubicShader.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

using Shader = GrCCCoverageProcessor::Shader;

void GrCCCubicShader::emitSetupCode(GrGLSLVertexGeoBuilder* s, const char* pts,
                                    const char* repetitionID, const char* wind,
                                    GeometryVars* vars) const {
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
    s->codeAppend ("float discr = 3*D2*D2 - 4*D1*D3;");
    s->codeAppend ("float x = discr >= 0 ? 3 : 1;");
    s->codeAppend ("float q = sqrt(x * abs(discr));");
    s->codeAppend ("q = x*D2 + (D2 >= 0 ? q : -q);");

    s->codeAppend ("float2 l, m;");
    s->codeAppend ("l.ts = normalize(float2(q, 2*x * D1));");
    s->codeAppend ("m.ts = normalize(float2(2, q) * (discr >= 0 ? float2(D3, 1) "
                                                               ": float2(D2*D2 - D3*D1, D1)));");

    s->codeAppend ("float4 K;");
    s->codeAppend ("float4 lm = l.sstt * m.stst;");
    s->codeAppend ("K = float4(0, lm.x, -lm.y - lm.z, lm.w);");

    s->codeAppend ("float4 L, M;");
    s->codeAppend ("lm.yz += 2*lm.zy;");
    s->codeAppend ("L = float4(-1,x,-x,1) * l.sstt * (discr >= 0 ? l.ssst * l.sttt : lm);");
    s->codeAppend ("M = float4(-1,x,-x,1) * m.sstt * (discr >= 0 ? m.ssst * m.sttt : lm.xzyw);");

    s->codeAppend ("short middlerow = abs(D2) > abs(D1) ? 2 : 1;");
    s->codeAppend ("float3x3 CI = inverse(float3x3(C[0][0], C[0][middlerow], C[0][3], "
                                                  "C[1][0], C[1][middlerow], C[1][3], "
                                                  "      0,               0,       1));");
    s->codeAppendf("%s = CI * float3x3(K[0], K[middlerow], K[3], "
                                      "L[0], L[middlerow], L[3], "
                                      "M[0], M[middlerow], M[3]);", fKLMMatrix.c_str());

    // Evaluate the cubic at T=.5 for a mid-ish point.
    s->codeAppendf("float2 midpoint = %s * float4(.125, .375, .375, .125);", pts);

    // Orient the KLM matrix so we fill the correct side of the curve.
    s->codeAppendf("float2 orientation = sign(float3(midpoint, 1) * float2x3(%s[1], %s[2]));",
                   fKLMMatrix.c_str(), fKLMMatrix.c_str());
    s->codeAppendf("%s *= float3x3(orientation[0] * orientation[1], 0, 0, "
                                  "0, orientation[0], 0, "
                                  "0, 0, orientation[1]);", fKLMMatrix.c_str());

    // Determine the amount of additional coverage to subtract out for the flat edge (P3 -> P0).
    s->declareGlobal(fEdgeDistanceEquation);
    s->codeAppendf("short edgeidx0 = %s > 0 ? 3 : 0;", wind);
    s->codeAppendf("float2 edgept0 = %s[edgeidx0];", pts);
    s->codeAppendf("float2 edgept1 = %s[3 - edgeidx0];", pts);
    Shader::EmitEdgeDistanceEquation(s, "edgept0", "edgept1", fEdgeDistanceEquation.c_str());

    this->onEmitSetupCode(s, pts, repetitionID, vars);
}

Shader::WindHandling GrCCCubicShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                                     GrGLSLVarying::Scope scope,
                                                     SkString* code, const char* position,
                                                     const char* coverage, const char* /*wind*/) {
    SkASSERT(!coverage);

    fKLMD.reset(kFloat4_GrSLType, scope);
    varyingHandler->addVarying("klmd", &fKLMD);
    code->appendf("float3 klm = float3(%s, 1) * %s;", position, fKLMMatrix.c_str());
    code->appendf("float d = dot(float3(%s, 1), %s);", position, fEdgeDistanceEquation.c_str());
    code->appendf("%s = float4(klm, d);", OutName(fKLMD));

    this->onEmitVaryings(varyingHandler, scope, code);
    return WindHandling::kNotHandled;
}

void GrCCCubicHullShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                         GrGLSLVarying::Scope scope, SkString* code) {
    fGradMatrix.reset(kFloat2x2_GrSLType, scope);
    varyingHandler->addVarying("grad_matrix", &fGradMatrix);
    // "klm" was just defined by the base class.
    code->appendf("%s[0] = 3 * klm[0] * %s[0].xy;", OutName(fGradMatrix), fKLMMatrix.c_str());
    code->appendf("%s[1] = -klm[1] * %s[2].xy - klm[2] * %s[1].xy;",
                    OutName(fGradMatrix), fKLMMatrix.c_str(), fKLMMatrix.c_str());
}

void GrCCCubicHullShader::onEmitFragmentCode(GrGLSLPPFragmentBuilder* f,
                                             const char* outputCoverage) const {
    f->codeAppendf("float k = %s.x, l = %s.y, m = %s.z, d = %s.w;",
                   fKLMD.fsIn(), fKLMD.fsIn(), fKLMD.fsIn(), fKLMD.fsIn());
    f->codeAppend ("float f = k*k*k - l*m;");
    f->codeAppendf("float2 grad_f = %s * float2(k, 1);", fGradMatrix.fsIn());
    f->codeAppendf("%s = clamp(0.5 - f * inversesqrt(dot(grad_f, grad_f)), 0, 1);", outputCoverage);
    f->codeAppendf("%s += min(d, 0);", outputCoverage); // Flat closing edge.
}

void GrCCCubicCornerShader::onEmitSetupCode(GrGLSLVertexGeoBuilder* s, const char* pts,
                                            const char* repetitionID, GeometryVars* vars) const {
    s->codeAppendf("float2 corner = %s[%s * 3];", pts, repetitionID);
    vars->fCornerVars.fPoint = "corner";
}

void GrCCCubicCornerShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                           GrGLSLVarying::Scope scope, SkString* code) {
    fdKLMDdx.reset(kFloat4_GrSLType, scope);
    varyingHandler->addFlatVarying("dklmddx", &fdKLMDdx);
    code->appendf("%s = float4(%s[0].x, %s[1].x, %s[2].x, %s.x);",
                  OutName(fdKLMDdx), fKLMMatrix.c_str(), fKLMMatrix.c_str(),
                  fKLMMatrix.c_str(), fEdgeDistanceEquation.c_str());

    fdKLMDdy.reset(kFloat4_GrSLType, scope);
    varyingHandler->addFlatVarying("dklmddy", &fdKLMDdy);
    code->appendf("%s = float4(%s[0].y, %s[1].y, %s[2].y, %s.y);",
                  OutName(fdKLMDdy), fKLMMatrix.c_str(), fKLMMatrix.c_str(),
                  fKLMMatrix.c_str(), fEdgeDistanceEquation.c_str());
}

void GrCCCubicCornerShader::onEmitFragmentCode(GrGLSLPPFragmentBuilder* f,
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
