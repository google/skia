/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "GrGLShaderVar.h"

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
    GR_STATIC_ASSERT(kFloat_GrSLType == 0);
    GR_STATIC_ASSERT(kVec2f_GrSLType == 1);
    GR_STATIC_ASSERT(kVec3f_GrSLType == 2);
    GR_STATIC_ASSERT(kVec4f_GrSLType == 3);
    GrAssert(count > 0 && count <= 4);
    return (GrSLType)(count - 1);
}

const char* GrGLSLVectorHomogCoord(int count) {
    static const char* HOMOGS[] = {"ERROR", "", ".y", ".z", ".w"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(HOMOGS));
    return HOMOGS[count];
}

const char* GrGLSLVectorNonhomogCoords(int count) {
    static const char* NONHOMOGS[] = {"ERROR", "", ".x", ".xy", ".xyz"};
    GrAssert(count >= 1 && count < (int)GR_ARRAY_COUNT(NONHOMOGS));
    return NONHOMOGS[count];
}

