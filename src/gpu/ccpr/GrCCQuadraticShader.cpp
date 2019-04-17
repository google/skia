/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCQuadraticShader.h"

#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

void GrCCQuadraticShader::emitSetupCode(
        GrGLSLVertexGeoBuilder* s, const char* pts, const char** outHull4) const {
    s->declareGlobal(fQCoordMatrix);
    s->codeAppendf("%s = float2x2(1, 1, .5, 0) * inverse(float2x2(%s[2] - %s[0], %s[1] - %s[0]));",
                   fQCoordMatrix.c_str(), pts, pts, pts, pts);

    s->declareGlobal(fQCoord0);
    s->codeAppendf("%s = %s[0];", fQCoord0.c_str(), pts);

    if (outHull4) {
        // Clip the bezier triangle by the tangent line at maximum height. Quadratics have the nice
        // property that maximum height always occurs at T=.5. This is a simple application for
        // De Casteljau's algorithm.
        s->codeAppend ("float2 quadratic_hull[4];");
        s->codeAppendf("quadratic_hull[0] = %s[0];", pts);
        s->codeAppendf("quadratic_hull[1] = (%s[0] + %s[1]) * .5;", pts, pts);
        s->codeAppendf("quadratic_hull[2] = (%s[1] + %s[2]) * .5;", pts, pts);
        s->codeAppendf("quadratic_hull[3] = %s[2];", pts);
        *outHull4 = "quadratic_hull";
    }
}

void GrCCQuadraticShader::onEmitVaryings(
        GrGLSLVaryingHandler* varyingHandler, GrGLSLVarying::Scope scope, SkString* code,
        const char* position, const char* coverage, const char* cornerCoverage, const char* wind) {
    fCoord_fGrad.reset(kFloat4_GrSLType, scope);
    varyingHandler->addVarying("coord_and_grad", &fCoord_fGrad);
    code->appendf("%s.xy = %s * (%s - %s);",  // Quadratic coords.
                  OutName(fCoord_fGrad), fQCoordMatrix.c_str(), position, fQCoord0.c_str());
    code->appendf("%s.zw = 2*bloat * float2(2 * %s.x, -1) * %s;",  // Gradient.
                  OutName(fCoord_fGrad), OutName(fCoord_fGrad), fQCoordMatrix.c_str());

    // Coverages need full precision since distance to the opposite edge can be large.
    fEdge_fWind_fCorner.reset(cornerCoverage ? kFloat4_GrSLType : kFloat2_GrSLType, scope);
    varyingHandler->addVarying("edge_and_wind_and_corner", &fEdge_fWind_fCorner);
    code->appendf("%s.x = %s;", OutName(fEdge_fWind_fCorner), coverage);
    code->appendf("%s.y = %s;", OutName(fEdge_fWind_fCorner), wind);

    if (cornerCoverage) {
        SkASSERT(coverage);
        code->appendf("half hull_coverage;");
        this->calcHullCoverage(code, OutName(fCoord_fGrad), coverage, "hull_coverage");
        code->appendf("%s.zw = half2(hull_coverage, 1) * %s;",
                      OutName(fEdge_fWind_fCorner), cornerCoverage);
    }
}

void GrCCQuadraticShader::onEmitFragmentCode(
        GrGLSLFPFragmentBuilder* f, const char* outputCoverage) const {
    this->calcHullCoverage(&AccessCodeString(f), fCoord_fGrad.fsIn(),
                           SkStringPrintf("%s.x", fEdge_fWind_fCorner.fsIn()).c_str(),
                           outputCoverage);
    f->codeAppendf("%s *= half(%s.y);", outputCoverage, fEdge_fWind_fCorner.fsIn());  // Wind.

    if (kFloat4_GrSLType == fEdge_fWind_fCorner.type()) {
        f->codeAppendf("%s = half(%s.z * %s.w) + %s;",  // Attenuated corner coverage.
                       outputCoverage, fEdge_fWind_fCorner.fsIn(), fEdge_fWind_fCorner.fsIn(),
                       outputCoverage);
    }
}

void GrCCQuadraticShader::calcHullCoverage(SkString* code, const char* coordAndGrad,
                                           const char* edge, const char* outputCoverage) const {
    code->appendf("float x = %s.x, y = %s.y;", coordAndGrad, coordAndGrad);
    code->appendf("float2 grad = %s.zw;", coordAndGrad);
    code->append ("float f = x*x - y;");
    code->append ("float fwidth = abs(grad.x) + abs(grad.y);");
    code->appendf("float curve_coverage = min(0.5 - f/fwidth, 1);");
    // Flat edge opposite the curve.
    code->appendf("float edge_coverage = min(%s, 0);", edge);
    // Total hull coverage.
    code->appendf("%s = max(half(curve_coverage + edge_coverage), 0);", outputCoverage);
}
