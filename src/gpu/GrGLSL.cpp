/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"

GrGLSLGeneration GetGLSLGeneration(GrGLBinding binding,
                                   const GrGLInterface* gl) {
    GrGLSLVersion ver = GrGLGetGLSLVersion(gl);
    switch (binding) {
        case kDesktop_GrGLBinding:
            GrAssert(ver >= GR_GLSL_VER(1,10));
            if (ver >= GR_GLSL_VER(1,50)) {
                return k150_GLSLGeneration;
            } else if (ver >= GR_GLSL_VER(1,30)) {
                return k130_GLSLGeneration;
            } else {
                return k110_GLSLGeneration;
            }
        case kES2_GrGLBinding:
            // version 1.00 of ES GLSL based on ver 1.20 of desktop GLSL
            GrAssert(ver >= GR_GL_VER(1,00));
            return k110_GLSLGeneration;
        default:
            GrCrash("Unknown GL Binding");
            return k110_GLSLGeneration; // suppress warning
    }
}

