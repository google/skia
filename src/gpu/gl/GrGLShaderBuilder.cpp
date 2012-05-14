/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLShaderBuilder.h"

namespace {

// number of each input/output type in a single allocation block
static const int sVarsPerBlock = 8;

// except FS outputs where we expect 2 at most.
static const int sMaxFSOutputs = 2;

}

// Architectural assumption: always 2-d input coords.
// Likely to become non-constant and non-static, perhaps even
// varying by stage, if we use 1D textures for gradients!
//const int GrGLShaderBuilder::fCoordDims = 2;

GrGLShaderBuilder::GrGLShaderBuilder()
    : fVSUnis(sVarsPerBlock)
    , fVSAttrs(sVarsPerBlock)
    , fVSOutputs(sVarsPerBlock)
    , fGSInputs(sVarsPerBlock)
    , fGSOutputs(sVarsPerBlock)
    , fFSInputs(sVarsPerBlock)
    , fFSUnis(sVarsPerBlock)
    , fFSOutputs(sMaxFSOutputs)
    , fVaryingDims(0)
    , fUsesGS(false) {

}

void GrGLShaderBuilder::appendVarying(GrSLType type,
                                      const char* name,
                                      const char** vsOutName,
                                      const char** fsInName) {
    fVSOutputs.push_back();
    fVSOutputs.back().setType(type);
    fVSOutputs.back().setTypeModifier(GrGLShaderVar::kOut_TypeModifier);
    fVSOutputs.back().accessName()->printf("v%s", name);
    if (vsOutName) {
        *vsOutName = fVSOutputs.back().getName().c_str();
    }
    // input to FS comes either from VS or GS
    const GrStringBuilder* fsName;
    if (fUsesGS) {
        // if we have a GS take each varying in as an array
        // and output as non-array.
        fGSInputs.push_back();
        fGSInputs.back().setType(type);
        fGSInputs.back().setTypeModifier(GrGLShaderVar::kIn_TypeModifier);
        fGSInputs.back().setUnsizedArray();
        *fGSInputs.back().accessName() = fVSOutputs.back().getName();
        fGSOutputs.push_back();
        fGSOutputs.back().setType(type);
        fGSOutputs.back().setTypeModifier(GrGLShaderVar::kOut_TypeModifier);
        fGSOutputs.back().accessName()->printf("g%s", name);
        fsName = fGSOutputs.back().accessName();
    } else {
        fsName = fVSOutputs.back().accessName();
    }
    fFSInputs.push_back();
    fFSInputs.back().setType(type);
    fFSInputs.back().setTypeModifier(GrGLShaderVar::kIn_TypeModifier);
    fFSInputs.back().setName(*fsName);
    if (fsInName) {
        *fsInName = fsName->c_str();
    }
}


void GrGLShaderBuilder::appendVarying(GrSLType type,
                                      const char* name,
                                      int stageNum,
                                      const char** vsOutName,
                                      const char** fsInName) {
    GrStringBuilder nameWithStage(name);
    nameWithStage.appendS32(stageNum);
    this->appendVarying(type, nameWithStage.c_str(), vsOutName, fsInName);
}
