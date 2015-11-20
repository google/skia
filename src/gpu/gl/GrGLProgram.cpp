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
#include "GrGLGpu.h"
#include "GrGLPathRendering.h"
#include "GrPathProcessor.h"
#include "GrPipeline.h"
#include "GrXferProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLXferProcessor.h"
#include "SkXfermode.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLProgram::GrGLProgram(GrGLGpu* gpu,
                         const GrProgramDesc& desc,
                         const BuiltinUniformHandles& builtinUniforms,
                         GrGLuint programID,
                         const UniformInfoArray& uniforms,
                         const VaryingInfoArray& pathProcVaryings,
                         GrGLInstalledGeoProc* geometryProcessor,
                         GrGLInstalledXferProc* xferProcessor,
                         GrGLInstalledFragProcs* fragmentProcessors,
                         SkTArray<UniformHandle>* passSamplerUniforms)
    : fBuiltinUniformHandles(builtinUniforms)
    , fProgramID(programID)
    , fGeometryProcessor(geometryProcessor)
    , fXferProcessor(xferProcessor)
    , fFragmentProcessors(SkRef(fragmentProcessors))
    , fDesc(desc)
    , fGpu(gpu)
    , fProgramDataManager(gpu, programID, uniforms, pathProcVaryings) {
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
                          SkTArray<const GrTextureAccess*>* textureBindings) {
    this->setRenderTargetState(primProc, pipeline);

    // we set the textures, and uniforms for installed processors in a generic way, but subclasses
    // of GLProgram determine how to set coord transforms
    fGeometryProcessor->fGLProc->setData(fProgramDataManager, primProc);
    append_texture_bindings(fGeometryProcessor.get(), primProc, textureBindings);

    this->setFragmentData(primProc, pipeline, textureBindings);

    const GrXferProcessor& xp = *pipeline.getXferProcessor();
    fXferProcessor->fGLProc->setData(fProgramDataManager, xp);
    append_texture_bindings(fXferProcessor.get(), xp, textureBindings);
}

void GrGLProgram::setFragmentData(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline,
                                  SkTArray<const GrTextureAccess*>* textureBindings) {
    int numProcessors = fFragmentProcessors->fProcs.count();
    for (int i = 0; i < numProcessors; ++i) {
        const GrFragmentProcessor& processor = pipeline.getFragmentProcessor(i);
        fFragmentProcessors->fProcs[i]->fGLProc->setData(fProgramDataManager, processor);
        this->setTransformData(primProc,
                               processor,
                               i,
                               fFragmentProcessors->fProcs[i]);
        append_texture_bindings(fFragmentProcessors->fProcs[i], processor, textureBindings);
    }
}
void GrGLProgram::setTransformData(const GrPrimitiveProcessor& primProc,
                                   const GrFragmentProcessor& processor,
                                   int index,
                                   GrGLInstalledFragProc* ip) {
    GrGLSLPrimitiveProcessor* gp = fGeometryProcessor.get()->fGLProc.get();
    gp->setTransformData(primProc, fProgramDataManager, index,
                         processor.coordTransforms());
}

void GrGLProgram::setRenderTargetState(const GrPrimitiveProcessor& primProc,
                                       const GrPipeline& pipeline) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != pipeline.getRenderTarget()->height()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni,
                                   SkIntToScalar(pipeline.getRenderTarget()->height()));
    }

    // set RT adjustment
    const GrRenderTarget* rt = pipeline.getRenderTarget();
    SkISize size;
    size.set(rt->width(), rt->height());
    if (!primProc.isPathRendering()) {
        if (fRenderTargetState.fRenderTargetOrigin != rt->origin() ||
            fRenderTargetState.fRenderTargetSize != size) {
            fRenderTargetState.fRenderTargetSize = size;
            fRenderTargetState.fRenderTargetOrigin = rt->origin();

            float rtAdjustmentVec[4];
            fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
            fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
        }
    } else {
        SkASSERT(fGpu->glCaps().shaderCaps()->pathRenderingSupport());
        const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
        fGpu->glPathRendering()->setProjectionMatrix(pathProc.viewMatrix(),
                                                     size, rt->origin());
    }
}
