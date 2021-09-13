/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkPipelineState.h"

#include "src/core/SkMipmap.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/vk/GrVkBuffer.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkDescriptorPool.h"
#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "src/gpu/vk/GrVkPipeline.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "src/gpu/vk/GrVkSampler.h"
#include "src/gpu/vk/GrVkTexture.h"

GrVkPipelineState::GrVkPipelineState(
        GrVkGpu* gpu,
        sk_sp<const GrVkPipeline> pipeline,
        const GrVkDescriptorSetManager::Handle& samplerDSHandle,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        uint32_t uniformSize,
        bool usePushConstants,
        const UniformInfoArray& samplers,
        std::unique_ptr<GrGeometryProcessor::ProgramImpl> gpImpl,
        std::unique_ptr<GrXferProcessor::ProgramImpl> xpImpl,
        std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls)
        : fPipeline(std::move(pipeline))
        , fSamplerDSHandle(samplerDSHandle)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fGPImpl(std::move(gpImpl))
        , fXPImpl(std::move(xpImpl))
        , fFPImpls(std::move(fpImpls))
        , fDataManager(uniforms, uniformSize, usePushConstants) {
    fNumSamplers = samplers.count();
    for (const auto& sampler : samplers.items()) {
        // We store the immutable samplers here and take a ref on the sampler. Once we switch to
        // using sk_sps here we should just move the immutable samplers to save the extra ref/unref.
        if (sampler.fImmutableSampler) {
            sampler.fImmutableSampler->ref();
        }
        fImmutableSamplers.push_back(sampler.fImmutableSampler);
    }
}

GrVkPipelineState::~GrVkPipelineState() {
    // Must have freed all GPU resources before this is destroyed
    SkASSERT(!fPipeline);
}

void GrVkPipelineState::freeGPUResources(GrVkGpu* gpu) {
    fPipeline.reset();
    fDataManager.releaseData();
    for (int i = 0; i < fImmutableSamplers.count(); ++i) {
        if (fImmutableSamplers[i]) {
            fImmutableSamplers[i]->unref();
            fImmutableSamplers[i] = nullptr;
        }
    }
}

bool GrVkPipelineState::setAndBindUniforms(GrVkGpu* gpu,
                                           SkISize colorAttachmentDimensions,
                                           const GrProgramInfo& programInfo,
                                           GrVkCommandBuffer* commandBuffer) {
    this->setRenderTargetState(colorAttachmentDimensions, programInfo.origin());

    fGPImpl->setData(fDataManager, *gpu->caps()->shaderCaps(), programInfo.geomProc());

    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        const auto& fp = programInfo.pipeline().getFragmentProcessor(i);
        fp.visitWithImpls([&](const GrFragmentProcessor& fp,
                              GrFragmentProcessor::ProgramImpl& impl) {
            impl.setData(fDataManager, fp);
        }, *fFPImpls[i]);
    }

    programInfo.pipeline().setDstTextureUniforms(fDataManager, &fBuiltinUniformHandles);
    fXPImpl->setData(fDataManager, programInfo.pipeline().getXferProcessor());

    // Upload uniform data and bind descriptor set.
    auto [uniformBuffer, success] = fDataManager.uploadUniforms(gpu, fPipeline->layout(),
                                                                commandBuffer);
    if (!success) {
        return false;
    }
    if (uniformBuffer) {
        const GrVkBuffer* vkBuffer = static_cast<GrVkBuffer*>(uniformBuffer.get());
        static const int kUniformDSIdx = GrVkUniformHandler::kUniformBufferDescSet;
        commandBuffer->bindDescriptorSets(gpu, fPipeline->layout(), kUniformDSIdx, /*setCount=*/1,
                                          vkBuffer->uniformDescriptorSet(),
                                          /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);
        commandBuffer->addGrBuffer(std::move(uniformBuffer));
    }
    return true;
}

bool GrVkPipelineState::setAndBindTextures(GrVkGpu* gpu,
                                           const GrGeometryProcessor& geomProc,
                                           const GrPipeline& pipeline,
                                           const GrSurfaceProxy* const geomProcTextures[],
                                           GrVkCommandBuffer* commandBuffer) {
    SkASSERT(geomProcTextures || !geomProc.numTextureSamplers());
    if (!fNumSamplers) {
        return true;
    }
    struct SamplerBindings {
        GrSamplerState fState;
        GrVkTexture* fTexture;
    };
    SkAutoSTArray<8, SamplerBindings> samplerBindings(fNumSamplers);
    int currTextureBinding = 0;

    for (int i = 0; i < geomProc.numTextureSamplers(); ++i) {
        SkASSERT(geomProcTextures[i]->asTextureProxy());
        const auto& sampler = geomProc.textureSampler(i);
        auto texture = static_cast<GrVkTexture*>(geomProcTextures[i]->peekTexture());
        samplerBindings[currTextureBinding++] = {sampler.samplerState(), texture};
    }


    if (GrTexture* dstTexture = pipeline.peekDstTexture()) {
        samplerBindings[currTextureBinding++] = {GrSamplerState::Filter::kNearest,
                                                 static_cast<GrVkTexture*>(dstTexture)};
    }

    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        GrSamplerState samplerState = te.samplerState();
        auto* texture = static_cast<GrVkTexture*>(te.texture());
        samplerBindings[currTextureBinding++] = {samplerState, texture};
    });

    // Get new descriptor set
    SkASSERT(fNumSamplers == currTextureBinding);
    static const int kSamplerDSIdx = GrVkUniformHandler::kSamplerDescSet;

    if (fNumSamplers == 1) {
        auto texture = samplerBindings[0].fTexture;
        auto texAttachment = texture->textureAttachment();
        const auto& samplerState = samplerBindings[0].fState;
        const GrVkDescriptorSet* descriptorSet = texture->cachedSingleDescSet(samplerState);
        if (descriptorSet) {
            commandBuffer->addGrSurface(sk_ref_sp<const GrSurface>(texture));
            commandBuffer->addResource(texAttachment->textureView());
            commandBuffer->addResource(texAttachment->resource());
            commandBuffer->addRecycledResource(descriptorSet);
            commandBuffer->bindDescriptorSets(gpu, fPipeline->layout(), kSamplerDSIdx,
                                              /*setCount=*/1, descriptorSet->descriptorSet(),
                                              /*dynamicOffsetCount=*/0,
                                              /*dynamicOffsets=*/nullptr);
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
        auto texAttachment = texture->textureAttachment();

        const GrVkImageView* textureView = texAttachment->textureView();
        const GrVkSampler* sampler = nullptr;
        if (fImmutableSamplers[i]) {
            sampler = fImmutableSamplers[i];
        } else {
            sampler = gpu->resourceProvider().findOrCreateCompatibleSampler(
                    state, texAttachment->ycbcrConversionInfo());
            if (!sampler) {
                descriptorSet->recycle();
                return false;
            }
        }
        SkASSERT(sampler);

        VkDescriptorImageInfo imageInfo;
        memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
        imageInfo.sampler = fImmutableSamplers[i] ? VK_NULL_HANDLE : sampler->sampler();
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
        commandBuffer->addResource(textureView);
        commandBuffer->addResource(texAttachment->resource());
    }
    if (fNumSamplers == 1) {
        GrSamplerState state = samplerBindings[0].fState;
        GrVkTexture* texture = samplerBindings[0].fTexture;
        texture->addDescriptorSetToCache(descriptorSet, state);
    }

    commandBuffer->bindDescriptorSets(gpu, fPipeline->layout(), kSamplerDSIdx, /*setCount=*/1,
                                      descriptorSet->descriptorSet(),
                                      /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);
    commandBuffer->addRecycledResource(descriptorSet);
    descriptorSet->recycle();
    return true;
}

bool GrVkPipelineState::setAndBindInputAttachment(GrVkGpu* gpu,
                                                  gr_rp<const GrVkDescriptorSet> inputDescSet,
                                                  GrVkCommandBuffer* commandBuffer) {
    SkASSERT(inputDescSet);
    commandBuffer->bindDescriptorSets(gpu, fPipeline->layout(), GrVkUniformHandler::kInputDescSet,
                                      /*setCount=*/1, inputDescSet->descriptorSet(),
                                      /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);
    // We don't add the input resource to the command buffer to track since the input will be
    // the same as the color attachment which is already tracked on the command buffer.
    commandBuffer->addRecycledResource(std::move(inputDescSet));
    return true;
}

void GrVkPipelineState::setRenderTargetState(SkISize colorAttachmentDimensions,
                                             GrSurfaceOrigin origin) {
    // Set RT adjustment and RT flip
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != colorAttachmentDimensions) {
        fRenderTargetState.fRenderTargetSize = colorAttachmentDimensions;
        fRenderTargetState.fRenderTargetOrigin = origin;

        // The client will mark a swap buffer as kTopLeft when making a SkSurface because
        // Vulkan's framebuffer space has (0, 0) at the top left. This agrees with Skia's device
        // coords and with Vulkan's NDC that has (-1, -1) in the top left. So a flip is needed when
        // surface origin is kBottomLeft rather than kTopLeft.
        bool flip = (origin == kBottomLeft_GrSurfaceOrigin);
        std::array<float, 4> v = SkSL::Compiler::GetRTAdjustVector(colorAttachmentDimensions, flip);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, v.data());
        if (fBuiltinUniformHandles.fRTFlipUni.isValid()) {
            std::array<float, 2> d =
                    SkSL::Compiler::GetRTFlipVector(colorAttachmentDimensions.height(), flip);
            fDataManager.set2fv(fBuiltinUniformHandles.fRTFlipUni, 1, d.data());
        }
    }
}

void GrVkPipelineState::bindPipeline(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer) {
    commandBuffer->bindPipeline(gpu, fPipeline);
}
