/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLNvprProgramBuilder.h"
#include "../GrGpuGL.h"

#define GL_CALL(X) GR_GL_CALL(this->gpu()->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(this->gpu()->glInterface(), R, X)

GrGLNvprProgramBuilder::GrGLNvprProgramBuilder(GrGpuGL* gpu,
                                               const GrOptDrawState& optState)
        : INHERITED(gpu, optState)
        , fSeparableVaryingInfos(kVarsPerBlock) {
}

void GrGLNvprProgramBuilder::emitTransforms(const GrFragmentStage& processorStage,
                                            GrGLProcessor::TransformedCoordsArray* outCoords,
                                            GrGLInstalledFragProc* ifp) {
    const GrFragmentProcessor* effect = processorStage.getProcessor();
    int numTransforms = effect->numTransforms();

    ifp->fTransforms.push_back_n(numTransforms);

    for (int t = 0; t < numTransforms; t++) {
        GrSLType varyingType =
                processorStage.isPerspectiveCoordTransform(t, false) ?
                        kVec3f_GrSLType :
                        kVec2f_GrSLType;

        const char* varyingName = "MatrixCoord";
        SkString suffixedVaryingName;
        if (0 != t) {
            suffixedVaryingName.append(varyingName);
            suffixedVaryingName.appendf("_%i", t);
            varyingName = suffixedVaryingName.c_str();
        }
        GrGLVertToFrag v(varyingType);
        ifp->fTransforms[t].fHandle = this->addSeparableVarying(varyingName, &v);
        ifp->fTransforms[t].fType = varyingType;

        SkNEW_APPEND_TO_TARRAY(outCoords, GrGLProcessor::TransformedCoords,
                               (SkString(v.fsIn()), varyingType));
    }
}

GrGLInstalledFragProc::ShaderVarHandle
GrGLNvprProgramBuilder::addSeparableVarying(const char* name, GrGLVarying* v) {
    this->addVarying(name, v);
    SeparableVaryingInfo& varying = fSeparableVaryingInfos.push_back();
    varying.fVariable = fFS.fInputs.back();
    return GrGLInstalledFragProc::ShaderVarHandle(fSeparableVaryingInfos.count() - 1);
}

void GrGLNvprProgramBuilder::resolveSeparableVaryings(GrGLuint programId) {
    int count = fSeparableVaryingInfos.count();
    for (int i = 0; i < count; ++i) {
        GrGLint location;
        GL_CALL_RET(location,
                    GetProgramResourceLocation(programId,
                                               GR_GL_FRAGMENT_INPUT,
                                               fSeparableVaryingInfos[i].fVariable.c_str()));
        fSeparableVaryingInfos[i].fLocation = location;
    }
}

GrGLProgram* GrGLNvprProgramBuilder::createProgram(GrGLuint programID) {
    // this is just for nvpr es, which has separable varyings that are plugged in after
    // building
    this->resolveSeparableVaryings(programID);
    return SkNEW_ARGS(GrGLNvprProgram, (fGpu, fDesc, fUniformHandles, programID, fUniforms,
                                        fFragmentProcessors.get(), fSeparableVaryingInfos));
}
