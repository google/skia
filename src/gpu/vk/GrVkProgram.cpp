/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkProgram.h"

#include "GrPipeline.h"
#include "GrVkCommandBuffer.h"
#include "GrVkDescriptorPool.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrVkMemory.h"
#include "GrVkPipeline.h"
#include "GrVkSampler.h"
#include "GrVkTexture.h"
#include "GrVkUniformBuffer.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLXferProcessor.h"

GrVkProgram::GrVkProgram(GrVkGpu* gpu,
                         GrVkPipeline* pipeline,
                         VkPipelineLayout layout,
                         VkDescriptorSetLayout dsLayout[2],
                         GrVkDescriptorPool* descriptorPool,
                         VkDescriptorSet descriptorSets[2],
                         const BuiltinUniformHandles& builtinUniformHandles,
                         const UniformInfoArray& uniforms,
                         uint32_t vertexUniformSize,
                         uint32_t fragmentUniformSize,
                         uint32_t numSamplers,
                         GrGLSLPrimitiveProcessor* geometryProcessor,
                         GrGLSLXferProcessor* xferProcessor,
                         const GrGLSLFragProcs& fragmentProcessors)
    : fDescriptorPool(descriptorPool)
    , fPipeline(pipeline)
    , fPipelineLayout(layout)
    , fBuiltinUniformHandles(builtinUniformHandles)
    , fGeometryProcessor(geometryProcessor)
    , fXferProcessor(xferProcessor)
    , fFragmentProcessors(fragmentProcessors)
    , fProgramDataManager(uniforms, vertexUniformSize, fragmentUniformSize) {
    fSamplers.setReserve(numSamplers);
    fTextureViews.setReserve(numSamplers);
    fTextures.setReserve(numSamplers);

    memcpy(fDSLayout, dsLayout, 2 * sizeof(VkDescriptorSetLayout));
    memcpy(fDescriptorSets, descriptorSets, 2 * sizeof(VkDescriptorSetLayout));

    fVertexUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, vertexUniformSize, true));
    fFragmentUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, fragmentUniformSize, true));

#ifdef SK_DEBUG
    fNumSamplers = numSamplers;
#endif
}

GrVkProgram::~GrVkProgram() {
    // Must of freed all GPU resources before this is destroyed
    SkASSERT(!fPipeline);
    SkASSERT(!fDescriptorPool);
    SkASSERT(!fPipelineLayout);
    SkASSERT(!fDSLayout[0]);
    SkASSERT(!fDSLayout[1]);
    SkASSERT(!fSamplers.count());
    SkASSERT(!fTextureViews.count());
    SkASSERT(!fTextures.count());
}

void GrVkProgram::freeTempResources(const GrVkGpu* gpu) {
    for (int i = 0; i < fSamplers.count(); ++i) {
        fSamplers[i]->unref(gpu);
    }
    fSamplers.rewind();

    for (int i = 0; i < fTextureViews.count(); ++i) {
            fTextureViews[i]->unref(gpu);
    }
    fTextureViews.rewind();

    for (int i = 0; i < fTextures.count(); ++i) {
            fTextures[i]->unref(gpu);
    }
    fTextures.rewind();
}

void GrVkProgram::freeGPUResources(const GrVkGpu* gpu) {
    if (fPipeline) {
        fPipeline->unref(gpu);
        fPipeline = nullptr;
    }
    if (fDescriptorPool) {
        fDescriptorPool->unref(gpu);
        fDescriptorPool = nullptr;
    }
    if (fPipelineLayout) {
        GR_VK_CALL(gpu->vkInterface(), DestroyPipelineLayout(gpu->device(),
                                                             fPipelineLayout,
                                                             nullptr));
        fPipelineLayout = nullptr;
    }

    if (fDSLayout[0]) {
        GR_VK_CALL(gpu->vkInterface(), DestroyDescriptorSetLayout(gpu->device(), fDSLayout[0],
                                                                  nullptr));
        fDSLayout[0] = nullptr;
    }
    if (fDSLayout[1]) {
        GR_VK_CALL(gpu->vkInterface(), DestroyDescriptorSetLayout(gpu->device(), fDSLayout[1],
                                                                  nullptr));
        fDSLayout[1] = nullptr;
    }

    if (fVertexUniformBuffer) {
        fVertexUniformBuffer->release(gpu);
    }

    if (fFragmentUniformBuffer) {
        fFragmentUniformBuffer->release(gpu);
    }
    this->freeTempResources(gpu);
}

void GrVkProgram::abandonGPUResources() {
    fPipeline->unrefAndAbandon();
    fPipeline = nullptr;
    fDescriptorPool->unrefAndAbandon();
    fDescriptorPool = nullptr;
    fPipelineLayout = nullptr;
    fDSLayout[0] = nullptr;
    fDSLayout[1] = nullptr;

    fVertexUniformBuffer->abandon();
    fFragmentUniformBuffer->abandon();

    for (int i = 0; i < fSamplers.count(); ++i) {
        fSamplers[i]->unrefAndAbandon();
    }
    fSamplers.rewind();

    for (int i = 0; i < fTextureViews.count(); ++i) {
        fTextureViews[i]->unrefAndAbandon();
    }
    fTextureViews.rewind();

    for (int i = 0; i < fTextures.count(); ++i) {
        fTextures[i]->unrefAndAbandon();
    }
    fTextures.rewind();
}

static void append_texture_bindings(const GrProcessor& processor,
                                    SkTArray<const GrTextureAccess*>* textureBindings) {
    if (int numTextures = processor.numTextures()) {
        const GrTextureAccess** bindings = textureBindings->push_back_n(numTextures);
        int i = 0;
        do {
            bindings[i] = &processor.textureAccess(i);
        } while (++i < numTextures);
    }
}

void GrVkProgram::setData(const GrVkGpu* gpu,
                          const GrPrimitiveProcessor& primProc,
                          const GrPipeline& pipeline) {
    // This is here to protect against someone calling setData multiple times in a row without
    // freeing the tempData between calls.
    this->freeTempResources(gpu);

    this->setRenderTargetState(pipeline);

    SkSTArray<8, const GrTextureAccess*> textureBindings;

    fGeometryProcessor->setData(fProgramDataManager, primProc);
    append_texture_bindings(primProc, &textureBindings);

    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        const GrFragmentProcessor& processor = pipeline.getFragmentProcessor(i);
        fFragmentProcessors[i]->setData(fProgramDataManager, processor);
        fGeometryProcessor->setTransformData(primProc, fProgramDataManager, i,
                                             processor.coordTransforms());
        append_texture_bindings(processor, &textureBindings);
    }

    fXferProcessor->setData(fProgramDataManager, pipeline.getXferProcessor());
    append_texture_bindings(pipeline.getXferProcessor(), &textureBindings);

    this->writeUniformBuffers(gpu);

    this->writeSamplers(gpu, textureBindings);
}

void GrVkProgram::writeUniformBuffers(const GrVkGpu* gpu) {
    fProgramDataManager.uploadUniformBuffers(gpu, fVertexUniformBuffer, fFragmentUniformBuffer);

    VkWriteDescriptorSet descriptorWrites[2];
    memset(descriptorWrites, 0, 2 * sizeof(VkWriteDescriptorSet));

    uint32_t firstUniformWrite = 0;
    uint32_t uniformBindingUpdateCount = 0;

    // Vertex Uniform Buffer
    if (fVertexUniformBuffer.get()) {
        ++uniformBindingUpdateCount;
        VkDescriptorBufferInfo vertBufferInfo;
        memset(&vertBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
        vertBufferInfo.buffer = fVertexUniformBuffer->buffer();
        vertBufferInfo.offset = 0;
        vertBufferInfo.range = fVertexUniformBuffer->size();

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].pNext = nullptr;
        descriptorWrites[0].dstSet = fDescriptorSets[1];
        descriptorWrites[0].dstBinding = GrVkUniformHandler::kVertexBinding;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].pImageInfo = nullptr;
        descriptorWrites[0].pBufferInfo = &vertBufferInfo;
        descriptorWrites[0].pTexelBufferView = nullptr;
    }

    // Fragment Uniform Buffer
    if (fFragmentUniformBuffer.get()) {
        if (0 == uniformBindingUpdateCount) {
            firstUniformWrite = 1;
        }
        ++uniformBindingUpdateCount;
        VkDescriptorBufferInfo fragBufferInfo;
        memset(&fragBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
        fragBufferInfo.buffer = fFragmentUniformBuffer->buffer();
        fragBufferInfo.offset = 0;
        fragBufferInfo.range = fFragmentUniformBuffer->size();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].pNext = nullptr;
        descriptorWrites[1].dstSet = fDescriptorSets[1];
        descriptorWrites[1].dstBinding = GrVkUniformHandler::kFragBinding;;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].pImageInfo = nullptr;
        descriptorWrites[1].pBufferInfo = &fragBufferInfo;
        descriptorWrites[1].pTexelBufferView = nullptr;
    }

    if (uniformBindingUpdateCount) {
        GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                            uniformBindingUpdateCount,
                                                            &descriptorWrites[firstUniformWrite],
                                                            0, nullptr));
    }
}

void GrVkProgram::writeSamplers(const GrVkGpu* gpu,
                                const SkTArray<const GrTextureAccess*>& textureBindings) {
    SkASSERT(fNumSamplers == textureBindings.count());

    for (int i = 0; i < textureBindings.count(); ++i) {
        fSamplers.push(GrVkSampler::Create(gpu, *textureBindings[i]));

        GrVkTexture* texture = static_cast<GrVkTexture*>(textureBindings[i]->getTexture());

        const GrVkImage::Resource* textureResource = texture->resource();
        textureResource->ref();
        fTextures.push(textureResource);

        const GrVkImageView* textureView = texture->textureView();
        textureView->ref();
        fTextureViews.push(textureView);

        // Change texture layout so it can be read in shader
        VkImageLayout layout = texture->currentLayout();
        VkPipelineStageFlags srcStageMask = GrVkMemory::LayoutToPipelineStageFlags(layout);
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(layout);
        VkAccessFlags dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        texture->setImageLayout(gpu,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                srcAccessMask,
                                dstAccessMask,
                                srcStageMask,
                                dstStageMask,
                                false);

        VkDescriptorImageInfo imageInfo;
        memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
        imageInfo.sampler = fSamplers[i]->sampler();
        imageInfo.imageView = texture->textureView()->imageView();
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

        GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                            1,
                                                            &writeInfo,
                                                            0,
                                                            nullptr));
    }
}

void GrVkProgram::setRenderTargetState(const GrPipeline& pipeline) {
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
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != rt->origin() ||
        fRenderTargetState.fRenderTargetSize != size) {
        fRenderTargetState.fRenderTargetSize = size;
        fRenderTargetState.fRenderTargetOrigin = rt->origin();

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

void GrVkProgram::bind(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer) {
    commandBuffer->bindPipeline(gpu, fPipeline);
    commandBuffer->bindDescriptorSets(gpu, this, fPipelineLayout, 0, 2, fDescriptorSets, 0,
                                      nullptr);
}

void GrVkProgram::addUniformResources(GrVkCommandBuffer& commandBuffer) {
#if 1
    commandBuffer.addResource(fDescriptorPool);
    if (fVertexUniformBuffer.get()) {
        commandBuffer.addResource(fVertexUniformBuffer->resource());
    }
    if (fFragmentUniformBuffer.get()) {
        commandBuffer.addResource(fFragmentUniformBuffer->resource());
    }
    for (int i = 0; i < fSamplers.count(); ++i) {
        commandBuffer.addResource(fSamplers[i]);
    }

    for (int i = 0; i < fTextureViews.count(); ++i) {
        commandBuffer.addResource(fTextureViews[i]);
    }

    for (int i = 0; i < fTextures.count(); ++i) {
        commandBuffer.addResource(fTextures[i]);
    }
#endif
}
