/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLShaderVar.h"
#include "SkString.h"

GrGLSLGeneration GrGetGLSLGeneration(GrGLBinding binding, const GrGLInterface* gl) {
    GrGLSLVersion ver = GrGLGetGLSLVersion(gl);
    switch (binding) {
        case kDesktop_GrGLBinding:
            SkASSERT(ver >= GR_GLSL_VER(1,10));
            if (ver >= GR_GLSL_VER(1,50)) {
                return k150_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(1,40)) {
                return k140_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(1,30)) {
                return k130_GrGLSLGeneration;
            } else {
                return k110_GrGLSLGeneration;
            }
        case kES_GrGLBinding:
            // version 1.00 of ES GLSL based on ver 1.20 of desktop GLSL
            SkASSERT(ver >= GR_GL_VER(1,00));
            return k110_GrGLSLGeneration;
        default:
            GrCrash("Unknown GL Binding");
            return k110_GrGLSLGeneration; // suppress warning
    }
}

const char* GrGetGLSLVersionDecl(const GrGLContextInfo& info) {
    switch (info.glslGeneration()) {
        case k110_GrGLSLGeneration:
            if (kES_GrGLBinding == info.binding()) {
                // ES2s shader language is based on version 1.20 but is version
                // 1.00 of the ES language.
                return "#version 100\n";
            } else {
                SkASSERT(kDesktop_GrGLBinding == info.binding());
                return "#version 110\n";
            }
        case k130_GrGLSLGeneration:
            SkASSERT(kDesktop_GrGLBinding == info.binding());
            return "#version 130\n";
        case k140_GrGLSLGeneration:
            SkASSERT(kDesktop_GrGLBinding == info.binding());
            return "#version 140\n";
        case k150_GrGLSLGeneration:
            SkASSERT(kDesktop_GrGLBinding == info.binding());
            if (info.caps()->isCoreProfile()) {
                return "#version 150\n";
            } else {
                return "#version 150 compatibility\n";
            }
        default:
            GrCrash("Unknown GL version.");
            return ""; // suppress warning
    }
}

namespace {
    void append_tabs(SkString* outAppend, int tabCnt) {
        static const char kTabs[] = "\t\t\t\t\t\t\t\t";
        while (tabCnt) {
            int cnt = GrMin((int)GR_ARRAY_COUNT(kTabs), tabCnt);
            outAppend->append(kTabs, cnt);
            tabCnt -= cnt;
        }
    }
}

void GrGLSLMulVarBy4f(SkString* outAppend,
                      unsigned tabCnt,
                      const char* vec4VarName,
                      const GrGLSLExpr4& mulFactor) {
    if (mulFactor.isOnes()) {
        *outAppend = SkString();
    }

    append_tabs(outAppend, tabCnt);

    if (mulFactor.isZeros()) {
        outAppend->appendf("%s = vec4(0);\n", vec4VarName);
    } else {
        outAppend->appendf("%s *= %s;\n", vec4VarName, mulFactor.c_str());
    }
}
