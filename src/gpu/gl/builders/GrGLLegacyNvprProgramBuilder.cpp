/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLLegacyNvprProgramBuilder.h"
#include "../GrGLGpu.h"

GrGLLegacyNvprProgramBuilder::GrGLLegacyNvprProgramBuilder(GrGLGpu* gpu,
                                                           const GrOptDrawState& optState)
    : INHERITED(gpu, optState)
    , fTexCoordSetCnt(0) {
}

int GrGLLegacyNvprProgramBuilder::addTexCoordSets(int count) {
    int firstFreeCoordSet = fTexCoordSetCnt;
    fTexCoordSetCnt += count;
    SkASSERT(gpu()->glCaps().maxFixedFunctionTextureCoords() >= fTexCoordSetCnt);
    return firstFreeCoordSet;
}

void GrGLLegacyNvprProgramBuilder::emitTransforms(const GrPendingFragmentStage& processorStage,
                                            GrGLProcessor::TransformedCoordsArray* outCoords,
                                            GrGLInstalledFragProc* ifp) {
    int numTransforms = processorStage.getProcessor()->numTransforms();
    int texCoordIndex = this->addTexCoordSets(numTransforms);

    // Use the first uniform location as the texcoord index.  This may seem a bit hacky but it
    // allows us to use one program effects object for all of our programs which really simplifies
    // the code overall
    ifp->fTransforms.push_back_n(1);
    ifp->fTransforms[0].fHandle = GrGLInstalledFragProc::ShaderVarHandle(texCoordIndex);

    SkString name;
    for (int t = 0; t < numTransforms; ++t) {
        GrSLType type = processorStage.isPerspectiveCoordTransform(t) ? kVec3f_GrSLType :
                                                                        kVec2f_GrSLType;

        name.printf("%s(gl_TexCoord[%i])", GrGLSLTypeString(type), texCoordIndex++);
        SkNEW_APPEND_TO_TARRAY(outCoords, GrGLProcessor::TransformedCoords, (name, type));
    }
}

GrGLProgram* GrGLLegacyNvprProgramBuilder::createProgram(GrGLuint programID) {
    return SkNEW_ARGS(GrGLLegacyNvprProgram, (fGpu, fDesc, fUniformHandles, programID, fUniforms,
            fGeometryProcessor, fXferProcessor, fFragmentProcessors.get(),
                                              fTexCoordSetCnt));
}
