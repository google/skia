/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLShaderVar.h"
#include "SkString.h"

bool GrGetGLSLGeneration(const GrGLInterface* gl, GrGLSLGeneration* generation) {
    SkASSERT(generation);
    GrGLSLVersion ver = GrGLGetGLSLVersion(gl);
    if (GR_GLSL_INVALID_VER == ver) {
        return false;
    }
    switch (gl->fStandard) {
        case kGL_GrGLStandard:
            SkASSERT(ver >= GR_GLSL_VER(1,10));
            if (ver >= GR_GLSL_VER(1,50)) {
                *generation = k150_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(1,40)) {
                *generation = k140_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(1,30)) {
                *generation = k130_GrGLSLGeneration;
            } else {
                *generation = k110_GrGLSLGeneration;
            }
            return true;
        case kGLES_GrGLStandard:
            // version 1.00 of ES GLSL based on ver 1.20 of desktop GLSL
            SkASSERT(ver >= GR_GL_VER(1,00));
            *generation = k110_GrGLSLGeneration;
            return true;
        default:
            SkFAIL("Unknown GL Standard");
            return false;
    }
}

const char* GrGetGLSLVersionDecl(const GrGLContextInfo& info) {
    switch (info.glslGeneration()) {
        case k110_GrGLSLGeneration:
            if (kGLES_GrGLStandard == info.standard()) {
                // ES2s shader language is based on version 1.20 but is version
                // 1.00 of the ES language.
                return "#version 100\n";
            } else {
                SkASSERT(kGL_GrGLStandard == info.standard());
                return "#version 110\n";
            }
        case k130_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == info.standard());
            return "#version 130\n";
        case k140_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == info.standard());
            return "#version 140\n";
        case k150_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == info.standard());
            if (info.caps()->isCoreProfile()) {
                return "#version 150\n";
            } else {
                return "#version 150 compatibility\n";
            }
        default:
            SkFAIL("Unknown GL version.");
            return ""; // suppress warning
    }
}

namespace {
    void append_tabs(SkString* outAppend, int tabCnt) {
        static const char kTabs[] = "\t\t\t\t\t\t\t\t";
        while (tabCnt) {
            int cnt = SkTMin((int)SK_ARRAY_COUNT(kTabs), tabCnt);
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
