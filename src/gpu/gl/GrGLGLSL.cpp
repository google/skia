/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGLSL.h"
#include "GrGLShaderVar.h"
#include "SkString.h"

bool GrGLGetGLSLGeneration(const GrGLInterface* gl, GrGLSLGeneration* generation) {
    SkASSERT(generation);
    GrGLSLVersion ver = GrGLGetGLSLVersion(gl);
    if (GR_GLSL_INVALID_VER == ver) {
        return false;
    }
    switch (gl->fStandard) {
        case kGL_GrGLStandard:
            SkASSERT(ver >= GR_GLSL_VER(1,10));
            if (ver >= GR_GLSL_VER(3,30)) {
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
            if (ver >= GR_GLSL_VER(3,1)) {
                *generation = k310es_GrGLSLGeneration;
            }
            else if (ver >= GR_GLSL_VER(3,0)) {
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

const char* GrGLGetGLSLVersionDecl(const GrGLContextInfo& info) {
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
        case k330_GrGLSLGeneration:
            if (kGLES_GrGLStandard == info.standard()) {
                return "#version 300 es\n";
            } else {
                SkASSERT(kGL_GrGLStandard == info.standard());
                if (info.caps()->isCoreProfile()) {
                    return "#version 330\n";
                } else {
                    return "#version 330 compatibility\n";
                }
            }
        case k310es_GrGLSLGeneration:
            SkASSERT(kGLES_GrGLStandard == info.standard());
            return "#version 310 es\n";
    }
    return "<no version>";
}

void GrGLAppendGLSLDefaultFloatPrecisionDeclaration(GrSLPrecision p, GrGLStandard s, SkString* out) {
    // Desktop GLSL has added precision qualifiers but they don't do anything.
    if (kGLES_GrGLStandard == s) {
        switch (p) {
            case kHigh_GrSLPrecision:
                out->append("precision highp float;\n");
                break;
            case kMedium_GrSLPrecision:
                out->append("precision mediump float;\n");
                break;
            case kLow_GrSLPrecision:
                out->append("precision lowp float;\n");
                break;
            default:
                SkFAIL("Unknown precision value.");
        }
    }
}
