/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShaderCaps.h"
#include "SkString.h"
#include "../private/GrGLSL.h"

bool GrGLSLSupportsNamedFragmentShaderOutputs(GrGLSLGeneration gen) {
    switch (gen) {
        case k110_GrGLSLGeneration:
            return false;
        case k130_GrGLSLGeneration:
        case k140_GrGLSLGeneration:
        case k150_GrGLSLGeneration:
        case k330_GrGLSLGeneration:
        case k400_GrGLSLGeneration:
        case k420_GrGLSLGeneration:
        case k310es_GrGLSLGeneration:
        case k320es_GrGLSLGeneration:
            return true;
    }
    return false;
}

void GrGLSLAppendDefaultFloatPrecisionDeclaration(GrSLPrecision p,
                                                  const GrShaderCaps& shaderCaps,
                                                  SkString* out) {
    if (shaderCaps.usesPrecisionModifiers()) {
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
