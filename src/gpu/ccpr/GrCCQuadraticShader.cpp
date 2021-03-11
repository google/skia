/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCQuadraticShader.h"

#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

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
}

void GrCCQuadraticShader::emitSampleMaskCode(GrGLSLFPFragmentBuilder* f) const {
    f->codeAppendf("float x = %s.x, y = %s.y;", fCoord_fGrad.fsIn(), fCoord_fGrad.fsIn());
    f->codeAppendf("float f = x*x - y;");
    f->codeAppendf("float2 grad = %s.zw;", fCoord_fGrad.fsIn());
    f->applyFnToMultisampleMask("f", "grad", GrGLSLFPFragmentBuilder::ScopeFlags::kTopLevel);
}
