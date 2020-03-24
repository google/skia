/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLVaryingHandler.h"

#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/builders/GrGLProgramBuilder.h"


GrGLSLVaryingHandler::VaryingHandle GrGLVaryingHandler::addPathProcessingVarying(
                                                                       const char* name,
                                                                       GrGLSLVarying* v) {
#ifdef SK_DEBUG
    GrGLProgramBuilder* glPB = (GrGLProgramBuilder*) fProgramBuilder;
    // This call is not used for non-NVPR backends.
    SkASSERT(glPB->gpu()->glCaps().shaderCaps()->pathRenderingSupport() &&
             fProgramBuilder->fProgramInfo.isNVPR());
#endif
    this->addVarying(name, v);
    auto varyingInfo = fPathProcVaryingInfos.push_back();
    varyingInfo.fLocation = fPathProcVaryingInfos.count() - 1;
    return VaryingHandle(varyingInfo.fLocation);
}

void GrGLVaryingHandler::onFinalize() {
    SkASSERT(fPathProcVaryingInfos.empty() || fPathProcVaryingInfos.count() == fFragInputs.count());
    VarArray::Iter::Item fragInputIter = fFragInputs.items().begin();
    for (auto& varyingInfo : fPathProcVaryingInfos.items()) {
        varyingInfo.fVariable = *fragInputIter;
        ++fragInputIter;
    }
}
