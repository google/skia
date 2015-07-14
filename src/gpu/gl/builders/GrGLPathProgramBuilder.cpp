/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPathProgramBuilder.h"
#include "gl/GrGLGpu.h"
#include "gl/GrGLPathProgram.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

GrGLPathProgramBuilder::GrGLPathProgramBuilder(GrGLGpu* gpu, const DrawArgs& args)
    : INHERITED(gpu, args)
    , fSeparableVaryingInfos(kVarsPerBlock) {
}

GrGLProgram* GrGLPathProgramBuilder::createProgram(GrGLuint programID) {
    return SkNEW_ARGS(GrGLPathProgram, (fGpu, this->desc(), fUniformHandles, programID,
                                        fUniforms,
                                        fSeparableVaryingInfos,
                                        fGeometryProcessor,
                                        fXferProcessor, fFragmentProcessors.get(),
                                        &fSamplerUniforms));
}

GrGLProgramBuilder::SeparableVaryingHandle GrGLPathProgramBuilder::addSeparableVarying(
    const char* name, GrGLVertToFrag* v, GrSLPrecision fsPrecision) {
    this->addVarying(name, v, fsPrecision);
    SeparableVaryingInfo& varyingInfo = fSeparableVaryingInfos.push_back();
    varyingInfo.fVariable = this->getFragmentShaderBuilder()->fInputs.back();
    varyingInfo.fLocation = fSeparableVaryingInfos.count() - 1;
    return SeparableVaryingHandle::CreateFromSeparableVaryingIndex(varyingInfo.fLocation);
}

void GrGLPathProgramBuilder::bindProgramResourceLocations(GrGLuint programID) {
    this->INHERITED::bindProgramResourceLocations(programID);
    if (!fGpu->glPathRendering()->shouldBindFragmentInputs()) {
        return;
    }
    int count = fSeparableVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GL_CALL(BindFragmentInputLocation(programID,
                                          i,
                                          fSeparableVaryingInfos[i].fVariable.c_str()));
        fSeparableVaryingInfos[i].fLocation = i;
    }
}

void GrGLPathProgramBuilder::resolveProgramResourceLocations(GrGLuint programID) {
    this->INHERITED::resolveProgramResourceLocations(programID);
    if (fGpu->glPathRendering()->shouldBindFragmentInputs()) {
        return;
    }
    int count = fSeparableVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GrGLint location;
        GL_CALL_RET(location,
                    GetProgramResourceLocation(programID,
                                               GR_GL_FRAGMENT_INPUT,
                                               fSeparableVaryingInfos[i].fVariable.c_str()));
        fSeparableVaryingInfos[i].fLocation = location;
    }
}
