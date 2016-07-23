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
#include "GrGLBuffer.h"
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
                         const SkTArray<GrGLSampler>& samplers,
                         const VaryingInfoArray& pathProcVaryings,
                         GrGLSLPrimitiveProcessor* geometryProcessor,
                         GrGLSLXferProcessor* xferProcessor,
                         const GrGLSLFragProcs& fragmentProcessors)
    : fBuiltinUniformHandles(builtinUniforms)
    , fProgramID(programID)
    , fGeometryProcessor(geometryProcessor)
    , fXferProcessor(xferProcessor)
    , fFragmentProcessors(fragmentProcessors)
    , fDesc(desc)
    , fGpu(gpu)
    , fProgramDataManager(gpu, programID, uniforms, pathProcVaryings) {
    // Assign texture units to sampler uniforms one time up front.
    GL_CALL(UseProgram(fProgramID));
    fProgramDataManager.setSamplers(samplers);
}

GrGLProgram::~GrGLProgram() {
    if (fProgramID) {
        GL_CALL(DeleteProgram(fProgramID));
    }
    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        delete fFragmentProcessors[i];
    }
}

void GrGLProgram::abandon() {
    fProgramID = 0;
}

///////////////////////////////////////////////////////////////////////////////

void GrGLProgram::setData(const GrPrimitiveProcessor& primProc, const GrPipeline& pipeline) {
    this->setRenderTargetState(primProc, pipeline);

    // we set the textures, and uniforms for installed processors in a generic way, but subclasses
    // of GLProgram determine how to set coord transforms
    int nextSamplerIdx = 0;
    fGeometryProcessor->setData(fProgramDataManager, primProc);
    this->bindTextures(primProc, pipeline.getAllowSRGBInputs(), &nextSamplerIdx);

    this->setFragmentData(primProc, pipeline, &nextSamplerIdx);

    if (primProc.getPixelLocalStorageState() !=
        GrPixelLocalStorageState::kDraw_GrPixelLocalStorageState) {
        const GrXferProcessor& xp = pipeline.getXferProcessor();
        fXferProcessor->setData(fProgramDataManager, xp);
        this->bindTextures(xp, pipeline.getAllowSRGBInputs(), &nextSamplerIdx);
    }
}

void GrGLProgram::generateMipmaps(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline) {
    this->generateMipmaps(primProc, pipeline.getAllowSRGBInputs());

    int numProcessors = fFragmentProcessors.count();
    for (int i = 0; i < numProcessors; ++i) {
        const GrFragmentProcessor& processor = pipeline.getFragmentProcessor(i);
        this->generateMipmaps(processor, pipeline.getAllowSRGBInputs());
    }

    if (primProc.getPixelLocalStorageState() !=
        GrPixelLocalStorageState::kDraw_GrPixelLocalStorageState) {
        const GrXferProcessor& xp = pipeline.getXferProcessor();
        this->generateMipmaps(xp, pipeline.getAllowSRGBInputs());
    }
}

void GrGLProgram::setFragmentData(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline,
                                  int* nextSamplerIdx) {
    int numProcessors = fFragmentProcessors.count();
    for (int i = 0; i < numProcessors; ++i) {
        const GrFragmentProcessor& processor = pipeline.getFragmentProcessor(i);
        fFragmentProcessors[i]->setData(fProgramDataManager, processor);
        this->setTransformData(primProc, processor, i);
        this->bindTextures(processor, pipeline.getAllowSRGBInputs(), nextSamplerIdx);
    }
}
void GrGLProgram::setTransformData(const GrPrimitiveProcessor& primProc,
                                   const GrFragmentProcessor& processor,
                                   int index) {
    fGeometryProcessor->setTransformData(primProc, fProgramDataManager, index,
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

void GrGLProgram::bindTextures(const GrProcessor& processor,
                               bool allowSRGBInputs,
                               int* nextSamplerIdx) {
    for (int i = 0; i < processor.numTextures(); ++i) {
        const GrTextureAccess& access = processor.textureAccess(i);
        fGpu->bindTexture((*nextSamplerIdx)++, access.getParams(),
                          allowSRGBInputs, static_cast<GrGLTexture*>(access.getTexture()));
    }
    for (int i = 0; i < processor.numBuffers(); ++i) {
        const GrBufferAccess& access = processor.bufferAccess(i);
        fGpu->bindTexelBuffer((*nextSamplerIdx)++, access.texelConfig(),
                              static_cast<GrGLBuffer*>(access.buffer()));
    }
}

void GrGLProgram::generateMipmaps(const GrProcessor& processor,
                                  bool allowSRGBInputs) {
    for (int i = 0; i < processor.numTextures(); ++i) {
        const GrTextureAccess& access = processor.textureAccess(i);
        fGpu->generateMipmaps(access.getParams(), allowSRGBInputs,
                              static_cast<GrGLTexture*>(access.getTexture()));
    }
}
