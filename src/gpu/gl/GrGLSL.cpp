/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLShaderVar.h"
#include "SkString.h"

GrGLSLGeneration GrGetGLSLGeneration(GrGLBinding binding,
                                   const GrGLInterface* gl) {
    GrGLSLVersion ver = GrGLGetGLSLVersion(gl);
    switch (binding) {
        case kDesktop_GrGLBinding:
            GrAssert(ver >= GR_GLSL_VER(1,10));
            if (ver >= GR_GLSL_VER(1,50)) {
                return k150_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(1,30)) {
                return k130_GrGLSLGeneration;
            } else {
                return k110_GrGLSLGeneration;
            }
        case kES2_GrGLBinding:
            // version 1.00 of ES GLSL based on ver 1.20 of desktop GLSL
            GrAssert(ver >= GR_GL_VER(1,00));
            return k110_GrGLSLGeneration;
        default:
            GrCrash("Unknown GL Binding");
            return k110_GrGLSLGeneration; // suppress warning
    }
}

const char* GrGetGLSLVersionDecl(GrGLBinding binding,
                                   GrGLSLGeneration gen) {
    switch (gen) {
        case k110_GrGLSLGeneration:
            if (kES2_GrGLBinding == binding) {
                // ES2s shader language is based on version 1.20 but is version
                // 1.00 of the ES language.
                return "#version 100\n";
            } else {
                GrAssert(kDesktop_GrGLBinding == binding);
                return "#version 110\n";
            }
        case k130_GrGLSLGeneration:
            GrAssert(kDesktop_GrGLBinding == binding);
            return "#version 130\n";
        case k150_GrGLSLGeneration:
            GrAssert(kDesktop_GrGLBinding == binding);
            return "#version 150\n";
        default:
            GrCrash("Unknown GL version.");
            return ""; // suppress warning
    }
}

bool GrGLSLSetupFSColorOuput(GrGLSLGeneration gen,
                             const char* nameIfDeclared,
                             GrGLShaderVar* var) {
    bool declaredOutput = k110_GrGLSLGeneration != gen;
    var->set(kVec4f_GrSLType,
             GrGLShaderVar::kOut_TypeModifier,
             declaredOutput ? nameIfDeclared : "gl_FragColor");
    return declaredOutput;
}

GrSLType GrSLFloatVectorType (int count) {
    GR_STATIC_ASSERT(kFloat_GrSLType == 1);
    GR_STATIC_ASSERT(kVec2f_GrSLType == 2);
    GR_STATIC_ASSERT(kVec3f_GrSLType == 3);
    GR_STATIC_ASSERT(kVec4f_GrSLType == 4);
    GrAssert(count > 0 && count <= 4);
    return (GrSLType)(count);
}

const char* GrGLSLVectorHomogCoord(int count) {
    static const char* HOMOGS[] = {"ERROR", "", ".y", ".z", ".w"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(HOMOGS));
    return HOMOGS[count];
}

const char* GrGLSLVectorHomogCoord(GrSLType type) {
    return GrGLSLVectorHomogCoord(GrSLTypeToVecLength(type));
}

const char* GrGLSLVectorNonhomogCoords(int count) {
    static const char* NONHOMOGS[] = {"ERROR", "", ".x", ".xy", ".xyz"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(NONHOMOGS));
    return NONHOMOGS[count];
}

const char* GrGLSLVectorNonhomogCoord(GrSLType type) {
    return GrGLSLVectorNonhomogCoords(GrSLTypeToVecLength(type));
}

GrSLConstantVec GrGLSLModulate4f(SkString* outAppend,
                                 const char* in0,
                                 const char* in1,
                                 GrSLConstantVec default0,
                                 GrSLConstantVec default1) {
    GrAssert(NULL != outAppend);

    bool has0 = NULL != in0 && '\0' != *in0;
    bool has1 = NULL != in1 && '\0' != *in1;

    GrAssert(has0 || kNone_GrSLConstantVec != default0);
    GrAssert(has1 || kNone_GrSLConstantVec != default1);

    if (!has0 && !has1) {
        GrAssert(kZeros_GrSLConstantVec == default0 || kOnes_GrSLConstantVec == default0);
        GrAssert(kZeros_GrSLConstantVec == default1 || kOnes_GrSLConstantVec == default1);
        if (kZeros_GrSLConstantVec == default0 || kZeros_GrSLConstantVec == default1) {
            outAppend->append(GrGLSLZerosVecf(4));
            return kZeros_GrSLConstantVec;
        } else {
            // both inputs are ones vectors
            outAppend->append(GrGLSLOnesVecf(4));
            return kOnes_GrSLConstantVec;
        }
    } else if (!has0) {
        GrAssert(kZeros_GrSLConstantVec == default0 || kOnes_GrSLConstantVec == default0);
        if (kZeros_GrSLConstantVec == default0) {
            outAppend->append(GrGLSLZerosVecf(4));
            return kZeros_GrSLConstantVec;
        } else {
            outAppend->appendf("vec4(%s)", in1);
            return kNone_GrSLConstantVec;
        }
    } else if (!has1) {
        GrAssert(kZeros_GrSLConstantVec == default1 || kOnes_GrSLConstantVec == default1);
        if (kZeros_GrSLConstantVec == default1) {
            outAppend->append(GrGLSLZerosVecf(4));
            return kZeros_GrSLConstantVec;
        } else {
            outAppend->appendf("vec4(%s)", in0);
            return kNone_GrSLConstantVec;
        }
    } else {
        outAppend->appendf("vec4(%s * %s)", in0, in1);
        return kNone_GrSLConstantVec;
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

GrSLConstantVec GrGLSLMulVarBy4f(SkString* outAppend,
                                 int tabCnt,
                                 const char* vec4VarName,
                                 const char* mulFactor,
                                 GrSLConstantVec mulFactorDefault) {
    bool haveFactor = NULL != mulFactor && '\0' != *mulFactor;

    GrAssert(NULL != outAppend);
    GrAssert(NULL != vec4VarName);
    GrAssert(kNone_GrSLConstantVec != mulFactorDefault || haveFactor);

    if (!haveFactor) {
        if (kOnes_GrSLConstantVec == mulFactorDefault) {
            return kNone_GrSLConstantVec;
        } else {
            GrAssert(kZeros_GrSLConstantVec == mulFactorDefault);
            append_tabs(outAppend, tabCnt);
            outAppend->appendf("%s = vec4(0, 0, 0, 0);\n", vec4VarName);
            return kZeros_GrSLConstantVec;
        }
    }
    append_tabs(outAppend, tabCnt);
    outAppend->appendf("%s *= %s;\n", vec4VarName, mulFactor);
    return kNone_GrSLConstantVec;
}

GrSLConstantVec GrGLSLAdd4f(SkString* outAppend,
                            const char* in0,
                            const char* in1,
                            GrSLConstantVec default0,
                            GrSLConstantVec default1) {
    GrAssert(NULL != outAppend);

    bool has0 = NULL != in0 && '\0' != *in0;
    bool has1 = NULL != in1 && '\0' != *in1;

    if (!has0 && !has1) {
        GrAssert(kZeros_GrSLConstantVec == default0);
        GrAssert(kZeros_GrSLConstantVec == default1);
        outAppend->append(GrGLSLZerosVecf(4));
        return kZeros_GrSLConstantVec;
    } else if (!has0) {
        GrAssert(kZeros_GrSLConstantVec == default0);
        outAppend->appendf("vec4(%s)", in1);
        return kNone_GrSLConstantVec;
    } else if (!has1) {
        GrAssert(kZeros_GrSLConstantVec == default1);
        outAppend->appendf("vec4(%s)", in0);
        return kNone_GrSLConstantVec;
    } else {
        outAppend->appendf("(vec4(%s) + vec4(%s))", in0, in1);
        return kNone_GrSLConstantVec;
    }
}
