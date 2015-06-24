/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLSL.h"
#include "SkString.h"

bool GrGLSLSupportsNamedFragmentShaderOutputs(GrGLSLGeneration gen) {
    switch (gen) {
        case k110_GrGLSLGeneration:
            return false;
        case k130_GrGLSLGeneration:
        case k140_GrGLSLGeneration:
        case k150_GrGLSLGeneration:
        case k330_GrGLSLGeneration:
        case k310es_GrGLSLGeneration:
            return true;
    }
    return false;
}

void GrGLSLMulVarBy4f(SkString* outAppend, const char* vec4VarName, const GrGLSLExpr4& mulFactor) {
    if (mulFactor.isOnes()) {
        *outAppend = SkString();
    }

    if (mulFactor.isZeros()) {
        outAppend->appendf("%s = vec4(0);", vec4VarName);
    } else {
        outAppend->appendf("%s *= %s;", vec4VarName, mulFactor.c_str());
    }
}
