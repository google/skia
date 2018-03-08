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
                                    const char* /*repetitionID*/, const char* /*wind*/,
                                    GeometryVars*) const {
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

    // Orient the KLM matrix so L & M are both positive on the side of the curve we wish to fill.
    s->codeAppendf("float2 orientation = sign(float3(midpoint, 1) * float2x3(%s[1], %s[2]));",
                   fKLMMatrix.c_str(), fKLMMatrix.c_str());
    s->codeAppendf("%s *= float3x3(orientation[0] * orientation[1], 0, 0, "
                                  "0, orientation[0], 0, "
                                  "0, 0, orientation[1]);", fKLMMatrix.c_str());
}

void GrCCCubicShader::onEmitVaryings(GrGLSLVaryingHandler* varyingHandler,
                                     GrGLSLVarying::Scope scope, SkString* code,
                                     const char* position, const char* inputCoverage,
                                     const char* wind) {
    code->appendf("float3 klm = float3(%s, 1) * %s;", position, fKLMMatrix.c_str());

    fKLMW.reset(kFloat4_GrSLType, scope);
    varyingHandler->addVarying("klmw", &fKLMW);
    code->appendf("%s.xyz = klm;", OutName(fKLMW));
    code->appendf("%s.w = %s * %s;", OutName(fKLMW), inputCoverage, wind);

    fGradMatrix.reset(kFloat2x2_GrSLType, scope);
    varyingHandler->addVarying("grad_matrix", &fGradMatrix);
    code->appendf("%s[0] = 3 * klm[0] * %s[0].xy;", OutName(fGradMatrix), fKLMMatrix.c_str());
    code->appendf("%s[1] = -klm[1] * %s[2].xy - klm[2] * %s[1].xy;",
                    OutName(fGradMatrix), fKLMMatrix.c_str(), fKLMMatrix.c_str());
}

void GrCCCubicShader::onEmitFragmentCode(const GrCCCoverageProcessor& proc,
                                         GrGLSLFPFragmentBuilder* f,
                                         const char* outputCoverage) const {
    f->codeAppendf("float k = %s.x, l = %s.y, m = %s.z;",
                   fKLMW.fsIn(), fKLMW.fsIn(), fKLMW.fsIn());
    f->codeAppend ("float f = k*k*k - l*m;");
    f->codeAppendf("float2 grad_f = %s * float2(k, 1);", fGradMatrix.fsIn());
    f->codeAppend ("float d = f * inversesqrt(dot(grad_f, grad_f));");
#ifdef SK_DEBUG
    if (proc.debugVisualizationsEnabled()) {
        f->codeAppendf("d /= %f;", proc.debugBloat());
    }
#endif
    f->codeAppendf("%s = clamp(0.5 - d, 0, 1) * %s.w;", outputCoverage, fKLMW.fsIn());
}
