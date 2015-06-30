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
#include "GrGLXferProcessor.h"
#include "GrPathProcessor.h"
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
                         GrGLInstalledFragProcs* fragmentProcessors,
                         SkTArray<UniformHandle>* passSamplerUniforms)
    : fColor(GrColor_ILLEGAL)
    , fCoverage(0)
    , fDstTextureUnit(-1)
    , fBuiltinUniformHandles(builtinUniforms)
    , fProgramID(programID)
    , fGeometryProcessor(geometryProcessor)
    , fXferProcessor(xferProcessor)
    , fFragmentProcessors(SkRef(fragmentProcessors))
    , fDesc(desc)
    , fGpu(gpu)
    , fProgramDataManager(gpu, uniforms) {
    fSamplerUniforms.swap(passSamplerUniforms);
    // Assign texture units to sampler uniforms one time up front.
    GL_CALL(UseProgram(fProgramID));
    for (int i = 0; i < fSamplerUniforms.count(); i++) {
        fProgramDataManager.setSampler(fSamplerUniforms[i], i);
    }
}

GrGLProgram::~GrGLProgram() {
    if (fProgramID) {
        GL_CALL(DeleteProgram(fProgramID));
    }
}

void GrGLProgram::abandon() {
    fProgramID = 0;
}

///////////////////////////////////////////////////////////////////////////////

template <class Proc>
static void append_texture_bindings(const Proc* ip,
                                    const GrProcessor& processor,
                                    SkTArray<const GrTextureAccess*>* textureBindings) {
    if (int numTextures = processor.numTextures()) {
        SkASSERT(textureBindings->count() == ip->fSamplersIdx);
        const GrTextureAccess** bindings = textureBindings->push_back_n(numTextures);
        int i = 0;
        do {
            bindings[i] = &processor.textureAccess(i);
        } while (++i < numTextures);
    }
}

void GrGLProgram::setData(const GrPrimitiveProcessor& primProc,
                          const GrPipeline& pipeline,
                          const GrBatchTracker& batchTracker,
                          SkTArray<const GrTextureAccess*>* textureBindings) {
    this->setRenderTargetState(primProc, pipeline);

    // we set the textures, and uniforms for installed processors in a generic way, but subclasses
    // of GLProgram determine how to set coord transforms
    fGeometryProcessor->fGLProc->setData(fProgramDataManager, primProc, batchTracker);
    append_texture_bindings(fGeometryProcessor.get(), primProc, textureBindings);

    this->setFragmentData(primProc, pipeline, textureBindings);

    const GrXferProcessor& xp = *pipeline.getXferProcessor();
    fXferProcessor->fGLProc->setData(fProgramDataManager, xp);
    append_texture_bindings(fXferProcessor.get(), xp, textureBindings);

    // Some of GrGLProgram subclasses need to update state here
    this->didSetData();
}

void GrGLProgram::setFragmentData(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline,
                                  SkTArray<const GrTextureAccess*>* textureBindings) {
    int numProcessors = fFragmentProcessors->fProcs.count();
    for (int e = 0; e < numProcessors; ++e) {
        const GrPendingFragmentStage& stage = pipeline.getFragmentStage(e);
        const GrProcessor& processor = *stage.processor();
        fFragmentProcessors->fProcs[e]->fGLProc->setData(fProgramDataManager, processor);
        this->setTransformData(primProc,
                               stage,
                               e,
                               fFragmentProcessors->fProcs[e]);
        append_texture_bindings(fFragmentProcessors->fProcs[e], processor, textureBindings);
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

