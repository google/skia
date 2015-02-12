/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgram.h"

#include "GrAllocator.h"
#include "GrProcessor.h"
#include "GrCoordTransform.h"
#include "GrGLGeometryProcessor.h"
#include "GrGLGpu.h"
#include "GrGLPathProcessor.h"
#include "GrGLPathRendering.h"
#include "GrGLShaderVar.h"
#include "GrGLSL.h"
#include "GrGLXferProcessor.h"
#include "GrPipeline.h"
#include "GrXferProcessor.h"
#include "SkXfermode.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLProgram::GrGLProgram(GrGLGpu* gpu,
                         const GrProgramDesc& desc,
                         const BuiltinUniformHandles& builtinUniforms,
                         GrGLuint programID,
                         const UniformInfoArray& uniforms,
                         GrGLInstalledGeoProc* geometryProcessor,
                         GrGLInstalledXferProc* xferProcessor,
                         GrGLInstalledFragProcs* fragmentProcessors)
    : fColor(GrColor_ILLEGAL)
    , fCoverage(0)
    , fDstCopyTexUnit(-1)
    , fBuiltinUniformHandles(builtinUniforms)
    , fProgramID(programID)
    , fGeometryProcessor(geometryProcessor)
    , fXferProcessor(xferProcessor)
    , fFragmentProcessors(SkRef(fragmentProcessors))
    , fDesc(desc)
    , fGpu(gpu)
    , fProgramDataManager(gpu, uniforms) {
    this->initSamplerUniforms();
}

GrGLProgram::~GrGLProgram() {
    if (fProgramID) {
        GL_CALL(DeleteProgram(fProgramID));
    }
}

void GrGLProgram::abandon() {
    fProgramID = 0;
}

void GrGLProgram::initSamplerUniforms() {
    GL_CALL(UseProgram(fProgramID));
    GrGLint texUnitIdx = 0;
    this->initSamplers(fGeometryProcessor.get(), &texUnitIdx);
    if (fXferProcessor.get()) {
        this->initSamplers(fXferProcessor.get(), &texUnitIdx);
    }
    int numProcs = fFragmentProcessors->fProcs.count();
    for (int i = 0; i < numProcs; i++) {
        this->initSamplers(fFragmentProcessors->fProcs[i], &texUnitIdx);
    }
}

template <class Proc>
void GrGLProgram::initSamplers(Proc* ip, int* texUnitIdx) {
    SkTArray<typename Proc::Sampler, true>& samplers = ip->fSamplers;
    int numSamplers = samplers.count();
    for (int s = 0; s < numSamplers; ++s) {
        SkASSERT(samplers[s].fUniform.isValid());
        fProgramDataManager.setSampler(samplers[s].fUniform, *texUnitIdx);
        samplers[s].fTextureUnit = (*texUnitIdx)++;
    }
}

template <class Proc>
void GrGLProgram::bindTextures(const Proc* ip, const GrProcessor& processor) {
    const SkTArray<typename Proc::Sampler, true>& samplers = ip->fSamplers;
    int numSamplers = samplers.count();
    SkASSERT(numSamplers == processor.numTextures());
    for (int s = 0; s < numSamplers; ++s) {
        SkASSERT(samplers[s].fTextureUnit >= 0);
        const GrTextureAccess& textureAccess = processor.textureAccess(s);
        fGpu->bindTexture(samplers[s].fTextureUnit,
                          textureAccess.getParams(),
                          static_cast<GrGLTexture*>(textureAccess.getTexture()));
    }
}


///////////////////////////////////////////////////////////////////////////////

void GrGLProgram::setData(const GrPrimitiveProcessor& primProc, const GrPipeline& pipeline,
                          const GrBatchTracker& batchTracker) {
    this->setRenderTargetState(primProc, pipeline);

    // we set the textures, and uniforms for installed processors in a generic way, but subclasses
    // of GLProgram determine how to set coord transforms
    fGeometryProcessor->fGLProc->setData(fProgramDataManager, primProc, batchTracker);
    this->bindTextures(fGeometryProcessor.get(), primProc);

    const GrXferProcessor& xp = *pipeline.getXferProcessor();
    fXferProcessor->fGLProc->setData(fProgramDataManager, xp);
    this->bindTextures(fXferProcessor.get(), xp);

    this->setFragmentData(primProc, pipeline);

    // Some of GrGLProgram subclasses need to update state here
    this->didSetData();
}

void GrGLProgram::setFragmentData(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline) {
    int numProcessors = fFragmentProcessors->fProcs.count();
    for (int e = 0; e < numProcessors; ++e) {
        const GrPendingFragmentStage& stage = pipeline.getFragmentStage(e);
        const GrProcessor& processor = *stage.processor();
        fFragmentProcessors->fProcs[e]->fGLProc->setData(fProgramDataManager, processor);
        this->setTransformData(primProc,
                               stage,
                               e,
                               fFragmentProcessors->fProcs[e]);
        this->bindTextures(fFragmentProcessors->fProcs[e], processor);
    }
}
void GrGLProgram::setTransformData(const GrPrimitiveProcessor& primProc,
                                   const GrPendingFragmentStage& processor,
                                   int index,
                                   GrGLInstalledFragProc* ip) {
    GrGLGeometryProcessor* gp =
            static_cast<GrGLGeometryProcessor*>(fGeometryProcessor.get()->fGLProc.get());
    gp->setTransformData(primProc, fProgramDataManager, index,
                         processor.processor()->coordTransforms());
}

void GrGLProgram::setRenderTargetState(const GrPrimitiveProcessor& primProc,
                                       const GrPipeline& pipeline) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != pipeline.getRenderTarget()->height()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni,
                                   SkIntToScalar(pipeline.getRenderTarget()->height()));
    }

    // call subclasses to set the actual view matrix
    this->onSetRenderTargetState(primProc, pipeline);
}

void GrGLProgram::onSetRenderTargetState(const GrPrimitiveProcessor&,
                                         const GrPipeline& pipeline) {
    const GrRenderTarget* rt = pipeline.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());
    if (fRenderTargetState.fRenderTargetOrigin != rt->origin() ||
        fRenderTargetState.fRenderTargetSize != size) {
        fRenderTargetState.fRenderTargetSize = size;
        fRenderTargetState.fRenderTargetOrigin = rt->origin();

        GrGLfloat rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

GrGLNvprProgram::GrGLNvprProgram(GrGLGpu* gpu,
                                 const GrProgramDesc& desc,
                                 const BuiltinUniformHandles& builtinUniforms,
                                 GrGLuint programID,
                                 const UniformInfoArray& uniforms,
                                 GrGLInstalledGeoProc* primProc,
                                 GrGLInstalledXferProc* xferProcessor,
                                 GrGLInstalledFragProcs* fragmentProcessors)
    : INHERITED(gpu, desc, builtinUniforms, programID, uniforms, primProc,
                xferProcessor, fragmentProcessors) {
}
void GrGLNvprProgram::didSetData() {
    GrGLPathProcessor* pathProc =
            static_cast<GrGLPathProcessor*>(fGeometryProcessor.get()->fGLProc.get());
    pathProc->didSetData(fGpu->glPathRendering());
}

void GrGLNvprProgram::setTransformData(const GrPrimitiveProcessor& primProc,
                                       const GrPendingFragmentStage& proc,
                                       int index,
                                       GrGLInstalledFragProc* ip) {
    GrGLPathProcessor* pathProc =
            static_cast<GrGLPathProcessor*>(fGeometryProcessor.get()->fGLProc.get());
    pathProc->setTransformData(primProc, index, proc.processor()->coordTransforms(),
                               fGpu->glPathRendering(), fProgramID);
}

void GrGLNvprProgram::onSetRenderTargetState(const GrPrimitiveProcessor& primProc,
                                             const GrPipeline& pipeline) {
    SkASSERT(!primProc.willUseGeoShader() && primProc.numAttribs() == 0);
    const GrRenderTarget* rt = pipeline.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());
    fGpu->glPathRendering()->setProjectionMatrix(primProc.viewMatrix(),
                                                 size, rt->origin());
}
