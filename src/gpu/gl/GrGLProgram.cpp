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

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLProgram::GrGLProgram(GrGLGpu* gpu,
                         const GrProgramDesc& desc,
                         const BuiltinUniformHandles& builtinUniforms,
                         GrGLuint programID,
                         const UniformInfoArray& uniforms,
                         const UniformInfoArray& textureSamplers,
                         const UniformInfoArray& texelBuffers,
                         const VaryingInfoArray& pathProcVaryings,
                         std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
                         std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
                         const GrGLSLFragProcs& fragmentProcessors)
    : fBuiltinUniformHandles(builtinUniforms)
    , fProgramID(programID)
    , fGeometryProcessor(std::move(geometryProcessor))
    , fXferProcessor(std::move(xferProcessor))
    , fFragmentProcessors(fragmentProcessors)
    , fDesc(desc)
    , fGpu(gpu)
    , fProgramDataManager(gpu, programID, uniforms, pathProcVaryings)
    , fNumTextureSamplers(textureSamplers.count())
    , fNumTexelBuffers(texelBuffers.count()) {
    // Assign texture units to sampler uniforms one time up front.
    GL_CALL(UseProgram(fProgramID));
    fProgramDataManager.setSamplerUniforms(textureSamplers, 0);
    fProgramDataManager.setSamplerUniforms(texelBuffers, fNumTextureSamplers);
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
    this->setRenderTargetState(primProc, pipeline.proxy());

    // we set the textures, and uniforms for installed processors in a generic way, but subclasses
    // of GLProgram determine how to set coord transforms

    // We must bind to texture units in the same order in which we set the uniforms in
    // GrGLProgramDataManager. That is first all texture samplers and then texel buffers.
    // Within each group we will bind them in primProc, fragProcs, XP order.
    int nextTexSamplerIdx = 0;
    int nextTexelBufferIdx = fNumTextureSamplers;
    fGeometryProcessor->setData(fProgramDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));
    this->bindTextures(primProc, pipeline.getAllowSRGBInputs(), &nextTexSamplerIdx,
                       &nextTexelBufferIdx);

    this->setFragmentData(primProc, pipeline, &nextTexSamplerIdx, &nextTexelBufferIdx);

    const GrXferProcessor& xp = pipeline.getXferProcessor();
    SkIPoint offset;
    GrTexture* dstTexture = pipeline.peekDstTexture(&offset);

    fXferProcessor->setData(fProgramDataManager, xp, dstTexture, offset);
    if (dstTexture) {
        fGpu->bindTexture(nextTexSamplerIdx++, GrSamplerState::ClampNearest(), true,
                          static_cast<GrGLTexture*>(dstTexture),
                          pipeline.dstTextureProxy()->origin());
    }
    SkASSERT(nextTexSamplerIdx == fNumTextureSamplers);
    SkASSERT(nextTexelBufferIdx == fNumTextureSamplers + fNumTexelBuffers);
}

void GrGLProgram::generateMipmaps(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline) {
    this->generateMipmaps(primProc, pipeline.getAllowSRGBInputs());

    GrFragmentProcessor::Iter iter(pipeline);
    while (const GrFragmentProcessor* fp  = iter.next()) {
        this->generateMipmaps(*fp, pipeline.getAllowSRGBInputs());
    }
}

void GrGLProgram::setFragmentData(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline,
                                  int* nextTexSamplerIdx,
                                  int* nextTexelBufferIdx) {
    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.begin(),
                                           fFragmentProcessors.count());
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fProgramDataManager, *fp);
        this->bindTextures(*fp, pipeline.getAllowSRGBInputs(), nextTexSamplerIdx,
                           nextTexelBufferIdx);
        fp = iter.next();
        glslFP = glslIter.next();
    }
    SkASSERT(!fp && !glslFP);
}


void GrGLProgram::setRenderTargetState(const GrPrimitiveProcessor& primProc,
                                       const GrRenderTargetProxy* proxy) {
    GrRenderTarget* rt = proxy->priv().peekRenderTarget();
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize size;
    size.set(rt->width(), rt->height());
    if (!primProc.isPathRendering()) {
        if (fRenderTargetState.fRenderTargetOrigin != proxy->origin() ||
            fRenderTargetState.fRenderTargetSize != size) {
            fRenderTargetState.fRenderTargetSize = size;
            fRenderTargetState.fRenderTargetOrigin = proxy->origin();

            float rtAdjustmentVec[4];
            fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
            fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
        }
    } else {
        SkASSERT(fGpu->glCaps().shaderCaps()->pathRenderingSupport());
        const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
        fGpu->glPathRendering()->setProjectionMatrix(pathProc.viewMatrix(),
                                                     size, proxy->origin());
    }
}

void GrGLProgram::bindTextures(const GrResourceIOProcessor& processor,
                               bool allowSRGBInputs,
                               int* nextTexSamplerIdx,
                               int* nextTexelBufferIdx) {
    for (int i = 0; i < processor.numTextureSamplers(); ++i) {
        const GrResourceIOProcessor::TextureSampler& sampler = processor.textureSampler(i);
        fGpu->bindTexture((*nextTexSamplerIdx)++, sampler.samplerState(), allowSRGBInputs,
                          static_cast<GrGLTexture*>(sampler.peekTexture()),
                          sampler.proxy()->origin());
    }
    for (int i = 0; i < processor.numBuffers(); ++i) {
        const GrResourceIOProcessor::BufferAccess& access = processor.bufferAccess(i);
        fGpu->bindTexelBuffer((*nextTexelBufferIdx)++, access.texelConfig(),
                              static_cast<GrGLBuffer*>(access.buffer()));
    }
}

void GrGLProgram::generateMipmaps(const GrResourceIOProcessor& processor, bool allowSRGBInputs) {
    for (int i = 0; i < processor.numTextureSamplers(); ++i) {
        const GrResourceIOProcessor::TextureSampler& sampler = processor.textureSampler(i);
        fGpu->generateMipmaps(sampler.samplerState(), allowSRGBInputs,
                              static_cast<GrGLTexture*>(sampler.peekTexture()),
                              sampler.proxy()->origin());
    }
}
