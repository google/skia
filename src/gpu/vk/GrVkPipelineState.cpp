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
        , fSamplerDSHandle(samplerDSHandle)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fGeometryProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFragmentProcessors(std::move(fragmentProcessors))
        , fFragmentProcessorCnt(fragmentProcessorCnt)
        , fDataManager(uniforms, uniformSize) {
    fUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, uniformSize));

    fNumSamplers = samplers.count();
    for (const auto& sampler : samplers.items()) {
        // We store the immutable samplers here and take ownership of the ref from the
        // GrVkUnformHandler.
        fImmutableSamplers.push_back(sampler.fImmutableSampler);
    }
}

GrVkPipelineState::~GrVkPipelineState() {
    // Must have freed all GPU resources before this is destroyed
    SkASSERT(!fPipeline);
}

void GrVkPipelineState::freeGPUResources() {
    if (fPipeline) {
        fPipeline->unref();
        fPipeline = nullptr;
    }

    if (fUniformBuffer) {
        fUniformBuffer->release();
        fUniformBuffer.reset();
    }
}

bool GrVkPipelineState::setAndBindUniforms(GrVkGpu* gpu,
                                           const GrRenderTarget* renderTarget,
                                           const GrProgramInfo& programInfo,
                                           GrVkCommandBuffer* commandBuffer) {
    this->setRenderTargetState(renderTarget, programInfo.origin());

    GrFragmentProcessor::PipelineCoordTransformRange transformRange(programInfo.pipeline());
    fGeometryProcessor->setData(fDataManager, programInfo.primProc(), transformRange);
    GrFragmentProcessor::CIter fpIter(programInfo.pipeline());
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    for (; fpIter && glslIter; ++fpIter, ++glslIter) {
        glslIter->setData(fDataManager, *fpIter);
    }
    SkASSERT(!fpIter && !glslIter);

    {
        SkIPoint offset;
        GrTexture* dstTexture = programInfo.pipeline().peekDstTexture(&offset);

        fXferProcessor->setData(fDataManager, programInfo.pipeline().getXferProcessor(),
                                dstTexture, offset);
    }

    // Get new descriptor set
    if (fUniformBuffer) {
        fDataManager.uploadUniformBuffers(gpu, fUniformBuffer.get());
        static const int kUniformDSIdx = GrVkUniformHandler::kUniformBufferDescSet;
        commandBuffer->bindDescriptorSets(gpu, this, fPipeline->layout(), kUniformDSIdx, 1,
                                          fUniformBuffer->descriptorSet(), 0, nullptr);
        commandBuffer->addRecycledResource(fUniformBuffer->resource());
    }
    return true;
}

bool GrVkPipelineState::setAndBindTextures(GrVkGpu* gpu,
                                           const GrPrimitiveProcessor& primProc,
                                           const GrPipeline& pipeline,
                                           const GrSurfaceProxy* const primProcTextures[],
                                           GrVkCommandBuffer* commandBuffer) {
    SkASSERT(primProcTextures || !primProc.numTextureSamplers());

    struct SamplerBindings {
        GrSamplerState fState;
        GrVkTexture* fTexture;
    };
    SkAutoSTMalloc<8, SamplerBindings> samplerBindings(fNumSamplers);
    int currTextureBinding = 0;

    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        SkASSERT(primProcTextures[i]->asTextureProxy());
        const auto& sampler = primProc.textureSampler(i);
        auto texture = static_cast<GrVkTexture*>(primProcTextures[i]->peekTexture());
        samplerBindings[currTextureBinding++] = {sampler.samplerState(), texture};
    }

    GrFragmentProcessor::CIter fpIter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    for (; fpIter && glslIter; ++fpIter, ++glslIter) {
        for (int i = 0; i < fpIter->numTextureSamplers(); ++i) {
            const auto& sampler = fpIter->textureSampler(i);
            samplerBindings[currTextureBinding++] =
                    {sampler.samplerState(), static_cast<GrVkTexture*>(sampler.peekTexture())};
        }
    }
    SkASSERT(!fpIter && !glslIter);

    if (GrTexture* dstTexture = pipeline.peekDstTexture()) {
        samplerBindings[currTextureBinding++] = {
                GrSamplerState::Filter::kNearest, static_cast<GrVkTexture*>(dstTexture)};
    }

    // Get new descriptor set
    SkASSERT(fNumSamplers == currTextureBinding);
    if (fNumSamplers) {
        static const int kSamplerDSIdx = GrVkUniformHandler::kSamplerDescSet;

        if (fNumSamplers == 1) {
            auto texture = samplerBindings[0].fTexture;
            const auto& samplerState = samplerBindings[0].fState;
            const GrVkDescriptorSet* descriptorSet = texture->cachedSingleDescSet(samplerState);
            if (descriptorSet) {
                commandBuffer->addResource(texture->textureView());
                commandBuffer->addResource(texture->resource());
                commandBuffer->addRecycledResource(descriptorSet);
                commandBuffer->bindDescriptorSets(gpu, this, fPipeline->layout(), kSamplerDSIdx, 1,
                                                  descriptorSet->descriptorSet(), 0, nullptr);
                return true;
            }
        }

        const GrVkDescriptorSet* descriptorSet =
                gpu->resourceProvider().getSamplerDescriptorSet(fSamplerDSHandle);
        if (!descriptorSet) {
            return false;
        }

        for (int i = 0; i < fNumSamplers; ++i) {
            GrSamplerState state = samplerBindings[i].fState;
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
            writeInfo.dstSet = *descriptorSet->descriptorSet();
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
                sampler->unref();
            }
            commandBuffer->addResource(samplerBindings[i].fTexture->textureView());
            commandBuffer->addResource(samplerBindings[i].fTexture->resource());
        }
        if (fNumSamplers == 1) {
            GrSamplerState state = samplerBindings[0].fState;
            GrVkTexture* texture = samplerBindings[0].fTexture;
            texture->addDescriptorSetToCache(descriptorSet, state);
        }

        commandBuffer->bindDescriptorSets(gpu, this, fPipeline->layout(), kSamplerDSIdx, 1,
                                          descriptorSet->descriptorSet(), 0, nullptr);
        commandBuffer->addRecycledResource(descriptorSet);
        descriptorSet->recycle();
    }
    return true;
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

void GrVkPipelineState::setRenderTargetState(const GrRenderTarget* rt, GrSurfaceOrigin origin) {

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize dimensions = rt->dimensions();
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != dimensions) {
        fRenderTargetState.fRenderTargetSize = dimensions;
        fRenderTargetState.fRenderTargetOrigin = origin;

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

void GrVkPipelineState::bindPipeline(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer) {
    commandBuffer->bindPipeline(gpu, fPipeline);
}
