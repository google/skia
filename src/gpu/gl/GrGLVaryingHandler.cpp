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
             glPB->fArgs.fPrimitiveProcessor->isPathRendering() &&
             !glPB->fArgs.fPrimitiveProcessor->willUseGeoShader() &&
             glPB->fArgs.fPrimitiveProcessor->numAttribs() == 0);
#endif
    this->addVarying(name, v, fsPrecision);
    VaryingInfo& varyingInfo = fPathProcVaryingInfos.push_back();
    varyingInfo.fVariable = fFragInputs.back();
    varyingInfo.fLocation = fPathProcVaryingInfos.count() - 1;
    return VaryingHandle(varyingInfo.fLocation);
}
