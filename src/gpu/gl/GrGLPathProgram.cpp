/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLPathProgram.h"
#include "GrGLPathProcessor.h"
#include "GrGLGpu.h"
#include "GrPathProcessor.h"

GrGLPathProgram::GrGLPathProgram(GrGLGpu* gpu,
                                 const GrProgramDesc& desc,
                                 const BuiltinUniformHandles& builtinUniforms,
                                 GrGLuint programID,
                                 const UniformInfoArray& uniforms,
                                 const SeparableVaryingInfoArray& separableVaryings,
                                 GrGLInstalledGeoProc* primProc,
                                 GrGLInstalledXferProc* xferProcessor,
                                 GrGLInstalledFragProcs* fragmentProcessors,
                                 SkTArray<UniformHandle>* passSamplerUniforms)
    : INHERITED(gpu, desc, builtinUniforms, programID, uniforms, primProc,
                xferProcessor, fragmentProcessors, passSamplerUniforms)
    , fPathProgramDataManager(gpu, fProgramID, separableVaryings) {
}
void GrGLPathProgram::didSetData() {
    GrGLPathProcessor* pathProc =
            static_cast<GrGLPathProcessor*>(fGeometryProcessor.get()->fGLProc.get());
    pathProc->didSetData(fGpu->glPathRendering());
}

void GrGLPathProgram::setTransformData(const GrPrimitiveProcessor& primProc,
                                       const GrPendingFragmentStage& proc,
                                       int index,
                                       GrGLInstalledFragProc* ip) {
    GrGLPathProcessor* pathProc =
            static_cast<GrGLPathProcessor*>(fGeometryProcessor.get()->fGLProc.get());
    pathProc->setTransformData(primProc, fPathProgramDataManager, index,
                               proc.processor()->coordTransforms());
}

void GrGLPathProgram::onSetRenderTargetState(const GrPrimitiveProcessor& primProc,
                                             const GrPipeline& pipeline) {
    SkASSERT(!primProc.willUseGeoShader() && primProc.numAttribs() == 0);
    const GrRenderTarget* rt = pipeline.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());
    const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
    fGpu->glPathRendering()->setProjectionMatrix(pathProc.viewMatrix(),
                                                 size, rt->origin());
}
