/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GrGLVaryingHandler.h"

#include "gl/GrGLGpu.h"
#include "gl/builders/GrGLProgramBuilder.h"


GrGLSLVaryingHandler::VaryingHandle GrGLVaryingHandler::addPathProcessingVarying(
                                                                       const char* name,
                                                                       GrGLSLVertToFrag* v,
                                                                       GrSLPrecision fsPrecision) {
#ifdef SK_DEBUG
    GrGLProgramBuilder* glPB = (GrGLProgramBuilder*) fProgramBuilder;
    // This call is not used for non-NVPR backends.
    SkASSERT(glPB->gpu()->glCaps().shaderCaps()->pathRenderingSupport() &&
             glPB->fPrimProc.isPathRendering() &&
             !glPB->fPrimProc.willUseGeoShader() &&
             glPB->fPrimProc.numAttribs() == 0);
#endif
    this->addVarying(name, v, fsPrecision);
    auto varyingInfo = fPathProcVaryingInfos.push_back();
    varyingInfo.fLocation = fPathProcVaryingInfos.count() - 1;
    return VaryingHandle(varyingInfo.fLocation);
}

void GrGLVaryingHandler::onFinalize() {
    SkASSERT(fPathProcVaryingInfos.empty() || fPathProcVaryingInfos.count() == fFragInputs.count());
    for (int i = 0; i < fPathProcVaryingInfos.count(); ++i) {
        fPathProcVaryingInfos[i].fVariable = fFragInputs[i];
    }
}
