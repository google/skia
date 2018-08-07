/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlPipelineState.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrPipeline.h"
#include "GrRenderTarget.h"
#include "GrTexturePriv.h"
#include "GrMtlBuffer.h"
#include "GrMtlGpu.h"
#include "GrMtlSampler.h"
#include "GrMtlTexture.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLXferProcessor.h"

GrMtlPipelineState::GrMtlPipelineState(
        GrMtlGpu* gpu,
        id<MTLRenderPipelineState> pipelineState,
        MTLPixelFormat pixelFormat,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        GrMtlBuffer* geometryUniformBuffer,
        GrMtlBuffer* fragmentUniformBuffer,
        uint32_t numSamplers,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
        int fragmentProcessorCnt)
        : fGpu(gpu)
        , fPipelineState(pipelineState)
        , fPixelFormat(pixelFormat)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fGeometryUniformBuffer(geometryUniformBuffer)
        , fFragmentUniformBuffer(fragmentUniformBuffer)
        , fNumSamplers(numSamplers)
        , fGeometryProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFragmentProcessors(std::move(fragmentProcessors))
        , fFragmentProcessorCnt(fragmentProcessorCnt)
        , fDataManager(uniforms, geometryUniformBuffer->sizeInBytes(),
                       fragmentUniformBuffer->sizeInBytes()) {
    (void) fPixelFormat; // Suppress unused-var warning.
}

void GrMtlPipelineState::setData(const GrPrimitiveProcessor& primProc,
                                 const GrPipeline& pipeline,
                                 const GrTextureProxy* const primProcTextures[]) {
    this->setRenderTargetState(pipeline.proxy());

    SkTArray<SamplerBindings> samplerBindings;

    fGeometryProcessor->setData(fDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));

    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        const auto& sampler = primProc.textureSampler(i);
        auto texture = static_cast<GrMtlTexture*>(primProcTextures[i]->peekTexture());
        samplerBindings.emplace_back(sampler.samplerState(), texture);
    }

    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
       glslFP->setData(fDataManager, *fp);
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            const auto& sampler = fp->textureSampler(i);
            samplerBindings.emplace_back(sampler.samplerState(), sampler.peekTexture());
        }
        fp = iter.next();
        glslFP = glslIter.next();
    }
    SkASSERT(!fp && !glslFP);

    {
        SkIPoint offset;
        GrTexture* dstTexture = pipeline.peekDstTexture(&offset);

        fXferProcessor->setData(fDataManager, pipeline.getXferProcessor(), dstTexture, offset);
    }


    if (GrTextureProxy* dstTextureProxy = pipeline.dstTextureProxy()) {
        samplerBindings.emplace_back(GrSamplerState::ClampNearest(),
                                     dstTextureProxy->peekTexture());
    }

    SkASSERT(fNumSamplers == samplerBindings.count());
    if (fNumSamplers) {
        this->writeSamplers(samplerBindings);
    }

    if (fGeometryUniformBuffer || fFragmentUniformBuffer) {
        fDataManager.uploadUniformBuffers(fGpu, fGeometryUniformBuffer.get(),
                                          fFragmentUniformBuffer.get());
    }
}

void GrMtlPipelineState::bind(id<MTLRenderCommandEncoder> renderCmdEncoder) {
    if (fGeometryUniformBuffer) {
        [renderCmdEncoder setVertexBuffer: fGeometryUniformBuffer->mtlBuffer()
                                   offset: 0
                                  atIndex: GrMtlUniformHandler::kGeometryBinding];
    }
    if (fFragmentUniformBuffer) {
        [renderCmdEncoder setFragmentBuffer: fFragmentUniformBuffer->mtlBuffer()
                                     offset: 0
                                    atIndex: GrMtlUniformHandler::kFragBinding];
    }
    SkASSERT(fTextures.count() == fSamplers.count());
    for (int index = 0; index < fTextures.count(); ++index) {
        [renderCmdEncoder setFragmentTexture: fTextures[index]->mtlTexture()
                                     atIndex: index];
        [renderCmdEncoder setFragmentSamplerState: fSamplers[index]->mtlSamplerState()
                                          atIndex: index];
    }
}

void GrMtlPipelineState::setRenderTargetState(const GrRenderTargetProxy* proxy) {
    GrRenderTarget* rt = proxy->peekRenderTarget();

    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize size;
    size.set(rt->width(), rt->height());
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != proxy->origin() ||
        fRenderTargetState.fRenderTargetSize != size) {
        fRenderTargetState.fRenderTargetSize = size;
        fRenderTargetState.fRenderTargetOrigin = proxy->origin();

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

void GrMtlPipelineState::writeSamplers(const SkTArray<SamplerBindings>& bindings) {
    for (int i = 0; i < fNumSamplers; ++i) {
        // TODO: use resource provider to get sampler state.
        fSamplers.push(GrMtlSampler::Create(fGpu, bindings[i].fState,
                                            bindings[i].fTexture->texturePriv().maxMipMapLevel()));
        fTextures.push(static_cast<GrMtlTexture*>(bindings[i].fTexture));
    }
}
