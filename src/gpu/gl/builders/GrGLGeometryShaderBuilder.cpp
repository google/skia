/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGeometryShaderBuilder.h"
#include "GrGLProgramBuilder.h"
#include "../GrGLGpu.h"

GrGLGeometryBuilder::GrGLGeometryBuilder(GrGLProgramBuilder* program)
    : INHERITED(program) {

}

void GrGLGeometryBuilder::addVarying(const char* name, GrGLVarying* v) {
    // if we have a GS take each varying in as an array
    // and output as non-array.
    if (v->vsVarying()) {
        fInputs.push_back();
        fInputs.back().setType(v->fType);
        fInputs.back().setTypeModifier(GrGLShaderVar::kVaryingIn_TypeModifier);
        fInputs.back().setUnsizedArray();
        *fInputs.back().accessName() = v->fVsOut;
        v->fGsIn = v->fVsOut;
    }

    if (v->fsVarying()) {
        fOutputs.push_back();
        fOutputs.back().setType(v->fType);
        fOutputs.back().setTypeModifier(GrGLShaderVar::kVaryingOut_TypeModifier);
        fProgramBuilder->nameVariable(fOutputs.back().accessName(), 'g', name);
        v->fGsOut = fOutputs.back().getName().c_str();
    }
}

bool GrGLGeometryBuilder::compileAndAttachShaders(GrGLuint programId,
        SkTDArray<GrGLuint>* shaderIds) {
    SkFAIL("Geometry shaders are not currently supported");
    return false;
}
