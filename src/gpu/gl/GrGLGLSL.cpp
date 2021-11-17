/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLGLSL.h"
#include "src/gpu/gl/GrGLUtil.h"

bool GrGLGetGLSLGeneration(const GrGLDriverInfo& info, SkSL::GLSLGeneration* generation) {
    SkASSERT(generation);
    // Workaround for a bug on some Adreno 308 devices with Android 9. The driver reports a GL
    // version of 3.0, and a GLSL version of 3.1. If we use version 310 shaders, the driver reports
    // that it's not supported. To keep things simple, we pin the GLSL version to the GL version.
    // Note that GLSL versions have an extra digit on their minor level, so we have to scale up
    // the GL version's minor revision to get a comparable GLSL version. This logic can easily
    // create invalid GLSL versions (older GL didn't keep the versions in sync), but the checks
    // below will further pin the GLSL generation correctly.
    // https://github.com/flutter/flutter/issues/36130
    uint32_t glMajor = GR_GL_MAJOR_VER(info.fVersion),
             glMinor = GR_GL_MINOR_VER(info.fVersion);
    GrGLSLVersion ver = std::min(info.fGLSLVersion, GR_GLSL_VER(glMajor, 10 * glMinor));
    if (info.fGLSLVersion == GR_GLSL_INVALID_VER) {
        return false;
    }

    if (GR_IS_GR_GL(info.fStandard)) {
        SkASSERT(ver >= GR_GLSL_VER(1,10));
        if (ver >= GR_GLSL_VER(4,20)) {
            *generation = SkSL::GLSLGeneration::k420;
        } else if (ver >= GR_GLSL_VER(4,00)) {
            *generation = SkSL::GLSLGeneration::k400;
        } else if (ver >= GR_GLSL_VER(3,30)) {
            *generation = SkSL::GLSLGeneration::k330;
        } else if (ver >= GR_GLSL_VER(1,50)) {
            *generation = SkSL::GLSLGeneration::k150;
        } else if (ver >= GR_GLSL_VER(1,40)) {
            *generation = SkSL::GLSLGeneration::k140;
        } else if (ver >= GR_GLSL_VER(1,30)) {
            *generation = SkSL::GLSLGeneration::k130;
        } else {
            *generation = SkSL::GLSLGeneration::k110;
        }
        return true;
    } else if (GR_IS_GR_GL_ES(info.fStandard)) {
        SkASSERT(ver >= GR_GL_VER(1,00));
        if (ver >= GR_GLSL_VER(3,20)) {
            *generation = SkSL::GLSLGeneration::k320es;
        } else if (ver >= GR_GLSL_VER(3,10)) {
            *generation = SkSL::GLSLGeneration::k310es;
        } else if (ver >= GR_GLSL_VER(3,00)) {
            *generation = SkSL::GLSLGeneration::k330;
        } else {
            *generation = SkSL::GLSLGeneration::k110;
        }
        return true;
    } else if (GR_IS_GR_WEBGL(info.fStandard)) {
        SkASSERT(ver >= GR_GL_VER(1,0));
        if (ver >= GR_GLSL_VER(2,0)) {
            *generation = SkSL::GLSLGeneration::k330;  // ES 3.0
        } else {
            *generation = SkSL::GLSLGeneration::k110;
        }
        return true;
    }
    SK_ABORT("Unknown GL Standard");
}
