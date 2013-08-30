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

bool GrGLSLSetupFSColorOuput(GrGLSLGeneration gen, const char* nameIfDeclared, GrGLShaderVar* var) {
    bool declaredOutput = k110_GrGLSLGeneration != gen;
    var->set(kVec4f_GrSLType,
             GrGLShaderVar::kOut_TypeModifier,
             declaredOutput ? nameIfDeclared : "gl_FragColor");
    return declaredOutput;
}

const char* GrGLSLVectorHomogCoord(int count) {
    static const char* HOMOGS[] = {"ERROR", "", ".y", ".z", ".w"};
    SkASSERT(count >= 1 && count < (int)GR_ARRAY_COUNT(HOMOGS));
    return HOMOGS[count];
}

const char* GrGLSLVectorHomogCoord(GrSLType type) {
    return GrGLSLVectorHomogCoord(GrSLTypeToVecLength(type));
}

const char* GrGLSLVectorNonhomogCoords(int count) {
    static const char* NONHOMOGS[] = {"ERROR", "", ".x", ".xy", ".xyz"};
    SkASSERT(count >= 1 && count < (int)GR_ARRAY_COUNT(NONHOMOGS));
    return NONHOMOGS[count];
}

const char* GrGLSLVectorNonhomogCoords(GrSLType type) {
    return GrGLSLVectorNonhomogCoords(GrSLTypeToVecLength(type));
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

GrSLConstantVec GrGLSLMulVarBy4f(SkString* outAppend,
                                 int tabCnt,
                                 const char* vec4VarName,
                                 const char* mulFactor,
                                 GrSLConstantVec mulFactorDefault) {
    bool haveFactor = NULL != mulFactor && '\0' != *mulFactor;

    SkASSERT(NULL != outAppend);
    SkASSERT(NULL != vec4VarName);
    SkASSERT(kNone_GrSLConstantVec != mulFactorDefault || haveFactor);

    if (!haveFactor) {
        if (kOnes_GrSLConstantVec == mulFactorDefault) {
            return kNone_GrSLConstantVec;
        } else {
            SkASSERT(kZeros_GrSLConstantVec == mulFactorDefault);
            append_tabs(outAppend, tabCnt);
            outAppend->appendf("%s = vec4(0, 0, 0, 0);\n", vec4VarName);
            return kZeros_GrSLConstantVec;
        }
    }
    append_tabs(outAppend, tabCnt);
    outAppend->appendf("%s *= %s;\n", vec4VarName, mulFactor);
    return kNone_GrSLConstantVec;
}

GrSLConstantVec GrGLSLGetComponent4f(SkString* outAppend,
                                     const char* expr,
                                     GrColorComponentFlags component,
                                     GrSLConstantVec defaultExpr,
                                     bool omitIfConst) {
    if (NULL == expr || '\0' == *expr) {
        SkASSERT(defaultExpr != kNone_GrSLConstantVec);
        if (!omitIfConst) {
            if (kOnes_GrSLConstantVec == defaultExpr) {
                outAppend->append("1.0");
            } else {
                SkASSERT(kZeros_GrSLConstantVec == defaultExpr);
                outAppend->append("0.0");
            }
        }
        return defaultExpr;
    } else {
        outAppend->appendf("(%s).%c", expr, GrColorComponentFlagToChar(component));
        return kNone_GrSLConstantVec;
    }
}
