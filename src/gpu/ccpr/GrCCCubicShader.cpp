/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCCubicShader.h"

#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

using Shader = GrCCCoverageProcessor::Shader;

void GrCCCubicShader::emitSetupCode(
        GrGLSLVertexGeoBuilder* s, const char* pts, const char** /*outHull4*/) const {
    // Find the cubic's power basis coefficients.
    s->codeAppendf("float2x4 C = float4x4(-1,  3, -3,  1, "
                                         " 3, -6,  3,  0, "
                                         "-3,  3,  0,  0, "
                                         " 1,  0,  0,  0) * transpose(%s);", pts);

    // Find the cubic's inflection function.
    s->codeAppend ("float D3 = +determinant(float2x2(C[0].yz, C[1].yz));");
    s->codeAppend ("float D2 = -determinant(float2x2(C[0].xz, C[1].xz));");
    s->codeAppend ("float D1 = +determinant(float2x2(C));");

    // Shift the exponents in D so the largest magnitude falls somewhere in 1..2. This protects us
    // from overflow while solving for roots and KLM functionals.
    s->codeAppend ("float Dmax = max(max(abs(D1), abs(D2)), abs(D3));");
    s->codeAppend ("float norm;");
    if (s->getProgramBuilder()->shaderCaps()->fpManipulationSupport()) {
        s->codeAppend ("int exp;");
        s->codeAppend ("frexp(Dmax, exp);");
        s->codeAppend ("norm = ldexp(1, 1 - exp);");
    } else {
        s->codeAppend ("norm = 1/Dmax;"); // Dmax will not be 0 because we cull line cubics on CPU.
    }
    s->codeAppend ("D3 *= norm;");
    s->codeAppend ("D2 *= norm;");
    s->codeAppend ("D1 *= norm;");

    // Calculate the KLM matrix.
    s->declareGlobal(fKLMMatrix);
    s->codeAppend ("float discr = 3*D2*D2 - 4*D1*D3;");
    s->codeAppend ("float x = discr >= 0 ? 3 : 1;");
    s->codeAppend ("float q = sqrt(x * abs(discr));");
    s->codeAppend ("q = x*D2 + (D2 >= 0 ? q : -q);");

    s->codeAppend ("float2 l, m;");
    s->codeAppend ("l.ts = float2(q, 2*x * D1);");
    s->codeAppend ("m.ts = float2(2, q) * (discr >= 0 ? float2(D3, 1) "
                                                     ": float2(D2*D2 - D3*D1, D1));");

    s->codeAppend ("float4 K;");
    s->codeAppend ("float4 lm = l.sstt * m.stst;");
    s->codeAppend ("K = float4(0, lm.x, -lm.y - lm.z, lm.w);");

    s->codeAppend ("float4 L, M;");
    s->codeAppend ("lm.yz += 2*lm.zy;");
    s->codeAppend ("L = float4(-1,x,-x,1) * l.sstt * (discr >= 0 ? l.ssst * l.sttt : lm);");
    s->codeAppend ("M = float4(-1,x,-x,1) * m.sstt * (discr >= 0 ? m.ssst * m.sttt : lm.xzyw);");

    s->codeAppend ("int middlerow = abs(D2) > abs(D1) ? 2 : 1;");
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

void GrCCCubicShader::onEmitVaryings(
        GrGLSLVaryingHandler* varyingHandler, GrGLSLVarying::Scope scope, SkString* code,
        const char* position, const char* coverage, const char* cornerCoverage, const char* wind) {
    code->appendf("float3 klm = float3(%s, 1) * %s;", position, fKLMMatrix.c_str());
    if (coverage) {
        fKLM_fEdge.reset(kFloat4_GrSLType, scope);
        varyingHandler->addVarying("klm_and_edge", &fKLM_fEdge);
        // Give L&M both the same sign as wind, in order to pass this value to the fragment shader.
        // (Cubics are pre-chopped such that L&M do not change sign within any individual segment.)
        code->appendf("%s.xyz = klm * float3(1, %s, %s);", OutName(fKLM_fEdge), wind, wind);
        // Flat edge opposite the curve.
        code->appendf("%s.w = %s;", OutName(fKLM_fEdge), coverage);
    } else {
        fKLM_fEdge.reset(kFloat3_GrSLType, scope);
        varyingHandler->addVarying("klm", &fKLM_fEdge);
        code->appendf("%s = klm;", OutName(fKLM_fEdge));
    }

    fGradMatrix.reset(kFloat4_GrSLType, scope);
    varyingHandler->addVarying("grad_matrix", &fGradMatrix);
    code->appendf("%s.xy = 2*bloat * 3 * klm[0] * %s[0].xy;",
                  OutName(fGradMatrix), fKLMMatrix.c_str());
    code->appendf("%s.zw = -2*bloat * (klm[1] * %s[2].xy + klm[2] * %s[1].xy);",
                    OutName(fGradMatrix), fKLMMatrix.c_str(), fKLMMatrix.c_str());

    if (cornerCoverage) {
        SkASSERT(coverage);
        code->appendf("half hull_coverage; {");
        this->calcHullCoverage(code, OutName(fKLM_fEdge), OutName(fGradMatrix), "hull_coverage");
        code->appendf("}");
        fCornerCoverage.reset(kHalf2_GrSLType, scope);
        varyingHandler->addVarying("corner_coverage", &fCornerCoverage);
        code->appendf("%s = half2(hull_coverage, 1) * %s;",
                      OutName(fCornerCoverage), cornerCoverage);
    }
}

void GrCCCubicShader::emitFragmentCoverageCode(
        GrGLSLFPFragmentBuilder* f, const char* outputCoverage) const {
    this->calcHullCoverage(
            &AccessCodeString(f), fKLM_fEdge.fsIn(), fGradMatrix.fsIn(), outputCoverage);

    // Wind is the sign of both L and/or M. Take the sign of whichever has the larger magnitude.
    // (In reality, either would be fine because we chop cubics with more than a half pixel of
    // padding around the L & M lines, so neither should approach zero.)
    f->codeAppend ("half wind = sign(half(l + m));");
    f->codeAppendf("%s *= wind;", outputCoverage);

    if (fCornerCoverage.fsIn()) {
        f->codeAppendf("%s = %s.x * %s.y + %s;", // Attenuated corner coverage.
                       outputCoverage, fCornerCoverage.fsIn(), fCornerCoverage.fsIn(),
                       outputCoverage);
    }
}

void GrCCCubicShader::calcHullCoverage(SkString* code, const char* klmAndEdge,
                                       const char* gradMatrix, const char* outputCoverage) const {
    code->appendf("float k = %s.x, l = %s.y, m = %s.z;", klmAndEdge, klmAndEdge, klmAndEdge);
    code->append ("float f = k*k*k - l*m;");
    code->appendf("float2 grad = %s.xy * k + %s.zw;", gradMatrix, gradMatrix);
    code->append ("float fwidth = abs(grad.x) + abs(grad.y);");
    code->appendf("float curve_coverage = min(0.5 - f/fwidth, 1);");
     // Flat edge opposite the curve.
    code->appendf("float edge_coverage = min(%s.w, 0);", klmAndEdge);
    // Total hull coverage.
    code->appendf("%s = max(half(curve_coverage + edge_coverage), 0);", outputCoverage);
}

void GrCCCubicShader::emitSampleMaskCode(GrGLSLFPFragmentBuilder* f) const {
    f->codeAppendf("float k = %s.x, l = %s.y, m = %s.z;",
                   fKLM_fEdge.fsIn(), fKLM_fEdge.fsIn(), fKLM_fEdge.fsIn());
    f->codeAppendf("float f = k*k*k - l*m;");
    f->codeAppendf("float2x2 grad_matrix = float2x2(%s);", fGradMatrix.fsIn());
    f->codeAppendf("float2 grad = grad_matrix * float2(k, 1);");
    f->applyFnToMultisampleMask("f", "grad", GrGLSLFPFragmentBuilder::ScopeFlags::kTopLevel);
}
