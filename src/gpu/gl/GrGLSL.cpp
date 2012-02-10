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

const char* GrGetGLSLVarPrecisionDeclType(GrGLBinding binding) {
    if (kES2_GrGLBinding == binding) {
        return "mediump";
    } else {
        return " ";
    }
}

const char* GrGetGLSLShaderPrecisionDecl(GrGLBinding binding) {
    if (kES2_GrGLBinding == binding) {
        return "precision mediump float;\n";
    } else {
        return "";
    }
}

bool GrGLSLSetupFSColorOuput(GrGLSLGeneration gen,
                             const char* nameIfDeclared,
                             GrGLShaderVar* var) {
    bool declaredOutput = k110_GrGLSLGeneration != gen;
    var->set(GrGLShaderVar::kVec4f_Type,
             GrGLShaderVar::kOut_TypeModifier,
             declaredOutput ? nameIfDeclared : "gl_FragColor");
    return declaredOutput;
}
