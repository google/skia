/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLContext.h"
#include "GrGLUtil.h"
#include "SkString.h"
#include "../private/GrGLSL.h"

bool GrGLGetGLSLGeneration(const GrGLInterface* gl, GrGLSLGeneration* generation) {
    SkASSERT(generation);
    GrGLSLVersion ver = GrGLGetGLSLVersion(gl);
    if (GR_GLSL_INVALID_VER == ver) {
        return false;
    }
    switch (gl->fStandard) {
        case kGL_GrGLStandard:
            SkASSERT(ver >= GR_GLSL_VER(1,10));
            if (ver >= GR_GLSL_VER(4,20)) {
                *generation = k420_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(4,00)) {
                *generation = k400_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(3,30)) {
                *generation = k330_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(1,50)) {
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
            SkASSERT(ver >= GR_GL_VER(1,00));
            if (ver >= GR_GLSL_VER(3,20)) {
                *generation = k320es_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(3,10)) {
                *generation = k310es_GrGLSLGeneration;
            } else if (ver >= GR_GLSL_VER(3,00)) {
                *generation = k330_GrGLSLGeneration;
            } else {
                *generation = k110_GrGLSLGeneration;
            }
            return true;
        default:
            SkFAIL("Unknown GL Standard");
            return false;
    }
}
