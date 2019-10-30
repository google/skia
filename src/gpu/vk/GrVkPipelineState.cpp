/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/gpu/GrContext.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"
#include "src/gpu/vk/GrVkBufferView.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkDescriptorPool.h"
#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "src/gpu/vk/GrVkPipeline.h"
#include "src/gpu/vk/GrVkPipelineState.h"
#include "src/gpu/vk/GrVkSampler.h"
#include "src/gpu/vk/GrVkTexture.h"
#include "src/gpu/vk/GrVkUniformBuffer.h"

GrVkPipelineState::GrVkPipelineState(
        GrVkGpu* gpu,
        GrVkPipeline* pipeline,
        const GrVkDescriptorSetManager::Handle& samplerDSHandle,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        uint32_t uniformSize,
        const UniformInfoArray& samplers,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
        int fragmentProcessorCnt)
        : fPipeline(pipeline)
        , fUniformDescriptorSet(nullptr)
        , fSamplerDescriptorSet(nullptr)
        , fSamplerDSHandle(samplerDSHandle)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fGeometryProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFragmentProcessors(std::move(fragmentProcessors))
        , fFragmentProcessorCnt(fragmentProcessorCnt)
        , fDataManager(uniforms, uniformSize) {
    fDescriptorSets[0] = VK_NULL_HANDLE;
    fDescriptorSets[1] = VK_NULL_HANDLE;
    fDescriptorSets[2] = VK_NULL_HANDLE;

    fUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, uniformSize));

    fNumSamplers = samplers.count();

    for (int i = 0; i < fNumSamplers; ++i) {
        // We store the immutable samplers here and take ownership of the ref from the
        // GrVkUnformHandler.
        fImmutableSamplers.push_back(samplers[i].fImmutableSampler);
    }
}

GrVkPipelineState::~GrVkPipelineState() {
    // Must have freed all GPU resources before this is destroyed
    SkASSERT(!fPipeline);
}

void GrVkPipelineState::freeGPUResources(GrVkGpu* gpu) {
    if (fPipeline) {
        fPipeline->unref(gpu);
        fPipeline = nullptr;
    }

    if (fUniformBuffer) {
        fUniformBuffer->release(gpu);
        fUniformBuffer.reset();
    }

    if (fUniformDescriptorSet) {
        fUniformDescriptorSet->recycle(const_cast<GrVkGpu*>(gpu));
        fUniformDescriptorSet = nullptr;
    }

    if (fSamplerDescriptorSet) {
        fSamplerDescriptorSet->recycle(const_cast<GrVkGpu*>(gpu));
        fSamplerDescriptorSet = nullptr;
    }
}

void GrVkPipelineState::abandonGPUResources() {
    if (fPipeline) {
        fPipeline->unrefAndAbandon();
        fPipeline = nullptr;
    }

    if (fUniformBuffer) {
        fUniformBuffer->abandon();
        fUniformBuffer.reset();
    }

    if (fUniformDescriptorSet) {
        fUniformDescriptorSet->unrefAndAbandon();
        fUniformDescriptorSet = nullptr;
    }

    if (fSamplerDescriptorSet) {
        fSamplerDescriptorSet->unrefAndAbandon();
        fSamplerDescriptorSet = nullptr;
    }
}

void GrVkPipelineState::setAndBindUniforms(GrVkGpu* gpu,
                                           const GrRenderTarget* renderTarget,
                                           const GrProgramInfo& programInfo,
                                           GrVkCommandBuffer* commandBuffer) {
    this->setRenderTargetState(renderTarget, programInfo.origin());

    fGeometryProcessor->setData(fDataManager, programInfo.primProc(),
                                GrFragmentProcessor::CoordTransformIter(programInfo.pipeline()));
    GrFragmentProcessor::Iter iter(programInfo.pipeline());
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fDataManager, *fp);
        fp = iter.next();
        glslFP = glslIter.next();
    }
    SkASSERT(!fp && !glslFP);

    {
        SkIPoint offset;
        GrTexture* dstTexture = programInfo.pipeline().peekDstTexture(&offset);

        fXferProcessor->setData(fDataManager, programInfo.pipeline().getXferProcessor(),
                                dstTexture, offset);
    }

    // Get new descriptor set
    if (fUniformBuffer) {
        int uniformDSIdx = GrVkUniformHandler::kUniformBufferDescSet;
        if (fDataManager.uploadUniformBuffers(gpu, fUniformBuffer.get()) ||
            !fUniformDescriptorSet) {
            if (fUniformDescriptorSet) {
                fUniformDescriptorSet->recycle(gpu);
            }
            fUniformDescriptorSet = gpu->resourceProvider().getUniformDescriptorSet();
            fDescriptorSets[uniformDSIdx] = fUniformDescriptorSet->descriptorSet();
            this->writeUniformBuffers(gpu);
        }
        commandBuffer->bindDescriptorSets(gpu, this, fPipeline->layout(), uniformDSIdx, 1,
                                          &fDescriptorSets[uniformDSIdx], 0, nullptr);
        if (fUniformDescriptorSet) {
            commandBuffer->addRecycledResource(fUniformDescriptorSet);
        }
        if (fUniformBuffer) {
            commandBuffer->addRecycledResource(fUniformBuffer->resource());
        }
    }
}

void GrVkPipelineState::setAndBindTextures(GrVkGpu* gpu,
                                           const GrPrimitiveProcessor& primProc,
                                           const GrPipeline& pipeline,
                                           const GrTextureProxy* const primProcTextures[],
                                           GrVkCommandBuffer* commandBuffer) {
    SkASSERT(primProcTextures || !primProc.numTextureSamplers());

    struct SamplerBindings {
        GrSamplerState fState;
        GrVkTexture* fTexture;
    };
    SkAutoSTMalloc<8, SamplerBindings> samplerBindings(fNumSamplers);
    int currTextureBinding = 0;

    fGeometryProcessor->setData(fDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        const auto& sampler = primProc.textureSampler(i);
        auto texture = static_cast<GrVkTexture*>(primProcTextures[i]->peekTexture());
        samplerBindings[currTextureBinding++] = {sampler.samplerState(), texture};
    }

    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            const auto& sampler = fp->textureSampler(i);
            samplerBindings[currTextureBinding++] =
                    {sampler.samplerState(), static_cast<GrVkTexture*>(sampler.peekTexture())};
        }
        fp = iter.next();
        glslFP = glslIter.next();
    }
    SkASSERT(!fp && !glslFP);

    if (GrTextureProxy* dstTextureProxy = pipeline.dstTextureProxy()) {
        samplerBindings[currTextureBinding++] = {
                GrSamplerState::ClampNearest(),
                static_cast<GrVkTexture*>(dstTextureProxy->peekTexture())};
    }

    // Get new descriptor set
    SkASSERT(fNumSamplers == currTextureBinding);
    if (fNumSamplers) {
        if (fSamplerDescriptorSet) {
            fSamplerDescriptorSet->recycle(gpu);
        }
        fSamplerDescriptorSet = gpu->resourceProvider().getSamplerDescriptorSet(fSamplerDSHandle);
        int samplerDSIdx = GrVkUniformHandler::kSamplerDescSet;
        fDescriptorSets[samplerDSIdx] = fSamplerDescriptorSet->descriptorSet();
        for (int i = 0; i < fNumSamplers; ++i) {
            const GrSamplerState& state = samplerBindings[i].fState;
            GrVkTexture* texture = samplerBindings[i].fTexture;

            const GrVkImageView* textureView = texture->textureView();
            const GrVkSampler* sampler = nullptr;
            if (fImmutableSamplers[i]) {
                sampler = fImmutableSamplers[i];
            } else {
                sampler = gpu->resourceProvider().findOrCreateCompatibleSampler(
                    state, texture->ycbcrConversionInfo());
            }
            SkASSERT(sampler);

            VkDescriptorImageInfo imageInfo;
            memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
            imageInfo.sampler = sampler->sampler();
            imageInfo.imageView = textureView->imageView();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet writeInfo;
            memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = fDescriptorSets[GrVkUniformHandler::kSamplerDescSet];
            writeInfo.dstBinding = i;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeInfo.pImageInfo = &imageInfo;
            writeInfo.pBufferInfo = nullptr;
            writeInfo.pTexelBufferView = nullptr;

            GR_VK_CALL(gpu->vkInterface(),
                       UpdateDescriptorSets(gpu->device(), 1, &writeInfo, 0, nullptr));
            commandBuffer->addResource(sampler);
            if (!fImmutableSamplers[i]) {
                sampler->unref(gpu);
            }
            commandBuffer->addResource(samplerBindings[i].fTexture->textureView());
            commandBuffer->addResource(samplerBindings[i].fTexture->resource());
        }

        commandBuffer->bindDescriptorSets(gpu, this, fPipeline->layout(), samplerDSIdx, 1,
                                          &fDescriptorSets[samplerDSIdx], 0, nullptr);
        commandBuffer->addRecycledResource(fSamplerDescriptorSet);
    }
}

void set_uniform_descriptor_writes(VkWriteDescriptorSet* descriptorWrite,
                                   VkDescriptorBufferInfo* bufferInfo,
                                   const GrVkUniformBuffer* buffer,
                                   VkDescriptorSet descriptorSet) {

    memset(bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    bufferInfo->buffer = buffer->buffer();
    bufferInfo->offset = buffer->offset();
    bufferInfo->range = buffer->size();

    memset(descriptorWrite, 0, sizeof(VkWriteDescriptorSet));
    descriptorWrite->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite->pNext = nullptr;
    descriptorWrite->dstSet = descriptorSet;
    descriptorWrite->dstBinding = GrVkUniformHandler::kUniformBinding;
    descriptorWrite->dstArrayElement = 0;
    descriptorWrite->descriptorCount = 1;
    descriptorWrite->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite->pImageInfo = nullptr;
    descriptorWrite->pBufferInfo = bufferInfo;
    descriptorWrite->pTexelBufferView = nullptr;
}

void GrVkPipelineState::writeUniformBuffers(const GrVkGpu* gpu) {
    VkWriteDescriptorSet descriptorWrites[3];
    VkDescriptorBufferInfo bufferInfos[3];

    uint32_t writeCount = 0;

    if (fUniformBuffer.get()) {
        set_uniform_descriptor_writes(&descriptorWrites[writeCount],
                                      &bufferInfos[writeCount],
                                      fUniformBuffer.get(),
                                      fDescriptorSets[GrVkUniformHandler::kUniformBufferDescSet]);
        ++writeCount;
    }

    if (writeCount) {
        GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                            writeCount,
                                                            descriptorWrites,
                                                            0, nullptr));
    }
}

void GrVkPipelineState::setRenderTargetState(const GrRenderTarget* rt, GrSurfaceOrigin origin) {

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize size;
    size.set(rt->width(), rt->height());
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != size) {
        fRenderTargetState.fRenderTargetSize = size;
        fRenderTargetState.fRenderTargetOrigin = origin;

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

void GrVkPipelineState::bindPipeline(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer) {
    commandBuffer->bindPipeline(gpu, fPipeline);
}
