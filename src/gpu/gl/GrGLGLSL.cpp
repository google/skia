/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLGLSL.h"
#include "src/gpu/gl/GrGLUtil.h"

bool GrGLGetGLSLGeneration(const GrGLInterface* gl, GrGLSLGeneration* generation) {
    SkASSERT(generation);
    GrGLSLVersion ver = GrGLGetGLSLVersion(gl);
    if (GR_GLSL_INVALID_VER == ver) {
        return false;
    }

    // Workaround for a bug on some Adreno 308 devices with Android 9. The driver reports a GL
    // version of 3.0, and a GLSL version of 3.1. If we use version 310 shaders, the driver reports
    // that it's not supported. To keep things simple, we pin the GLSL version to the GL version.
    // https://github.com/flutter/flutter/issues/36130
    GrGLVersion glVer = GrGLGetVersion(gl);
    ver = SkTMin(ver, glVer);

    if (GR_IS_GR_GL(gl->fStandard)) {
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
    } else if (GR_IS_GR_GL_ES(gl->fStandard) || GR_IS_GR_WEBGL(gl->fStandard)) {
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
    }
    SK_ABORT("Unknown GL Standard");
}
