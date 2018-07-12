/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkPipelineState.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrPipeline.h"
#include "GrRenderTarget.h"
#include "GrTexturePriv.h"
#include "GrVkBufferView.h"
#include "GrVkCommandBuffer.h"
#include "GrVkDescriptorPool.h"
#include "GrVkDescriptorSet.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrVkMemory.h"
#include "GrVkPipeline.h"
#include "GrVkPipelineLayout.h"
#include "GrVkSampler.h"
#include "GrVkTexture.h"
#include "GrVkUniformBuffer.h"
#include "SkMipMap.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLXferProcessor.h"

GrVkPipelineState::GrVkPipelineState(
        GrVkGpu* gpu,
        GrVkPipeline* pipeline,
        VkPipelineLayout layout,
        const GrVkDescriptorSetManager::Handle& samplerDSHandle,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        uint32_t geometryUniformSize,
        uint32_t fragmentUniformSize,
        uint32_t numSamplers,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
        int fragmentProcessorCnt)
        : fPipeline(pipeline)
        , fPipelineLayout(new GrVkPipelineLayout(layout))
        , fUniformDescriptorSet(nullptr)
        , fSamplerDescriptorSet(nullptr)
        , fSamplerDSHandle(samplerDSHandle)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fGeometryProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFragmentProcessors(std::move(fragmentProcessors))
        , fFragmentProcessorCnt(fragmentProcessorCnt)
        , fDataManager(uniforms, geometryUniformSize, fragmentUniformSize) {
    fSamplers.setReserve(numSamplers);
    fTextureViews.setReserve(numSamplers);
    fTextures.setReserve(numSamplers);

    fDescriptorSets[0] = VK_NULL_HANDLE;
    fDescriptorSets[1] = VK_NULL_HANDLE;
    fDescriptorSets[2] = VK_NULL_HANDLE;

    fGeometryUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, geometryUniformSize));
    fFragmentUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, fragmentUniformSize));

    fNumSamplers = numSamplers;
}

GrVkPipelineState::~GrVkPipelineState() {
    // Must have freed all GPU resources before this is destroyed
    SkASSERT(!fPipeline);
    SkASSERT(!fPipelineLayout);
    SkASSERT(!fSamplers.count());
    SkASSERT(!fTextureViews.count());
    SkASSERT(!fTextures.count());
}

void GrVkPipelineState::freeTempResources(const GrVkGpu* gpu) {
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

void GrVkPipelineState::freeGPUResources(const GrVkGpu* gpu) {
    if (fPipeline) {
        fPipeline->unref(gpu);
        fPipeline = nullptr;
    }

    if (fPipelineLayout) {
        fPipelineLayout->unref(gpu);
        fPipelineLayout = nullptr;
    }

    if (fGeometryUniformBuffer) {
        fGeometryUniformBuffer->release(gpu);
    }

    if (fFragmentUniformBuffer) {
        fFragmentUniformBuffer->release(gpu);
    }

    if (fUniformDescriptorSet) {
        fUniformDescriptorSet->recycle(const_cast<GrVkGpu*>(gpu));
        fUniformDescriptorSet = nullptr;
    }

    if (fSamplerDescriptorSet) {
        fSamplerDescriptorSet->recycle(const_cast<GrVkGpu*>(gpu));
        fSamplerDescriptorSet = nullptr;
    }

    this->freeTempResources(gpu);
}

void GrVkPipelineState::abandonGPUResources() {
    if (fPipeline) {
        fPipeline->unrefAndAbandon();
        fPipeline = nullptr;
    }

    if (fPipelineLayout) {
        fPipelineLayout->unrefAndAbandon();
        fPipelineLayout = nullptr;
    }

    fGeometryUniformBuffer->abandon();
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

    if (fUniformDescriptorSet) {
        fUniformDescriptorSet->unrefAndAbandon();
        fUniformDescriptorSet = nullptr;
    }

    if (fSamplerDescriptorSet) {
        fSamplerDescriptorSet->unrefAndAbandon();
        fSamplerDescriptorSet = nullptr;
    }
}

static void append_texture_bindings(
        const GrResourceIOProcessor& processor,
        SkTArray<const GrResourceIOProcessor::TextureSampler*>* textureBindings) {
    if (int numTextureSamplers = processor.numTextureSamplers()) {
        const GrResourceIOProcessor::TextureSampler** bindings =
                textureBindings->push_back_n(numTextureSamplers);
        int i = 0;
        do {
            bindings[i] = &processor.textureSampler(i);
        } while (++i < numTextureSamplers);
    }
}

void GrVkPipelineState::setData(GrVkGpu* gpu,
                                const GrPrimitiveProcessor& primProc,
                                const GrPipeline& pipeline) {
    // This is here to protect against someone calling setData multiple times in a row without
    // freeing the tempData between calls.
    this->freeTempResources(gpu);

    this->setRenderTargetState(pipeline.proxy());

    SkSTArray<8, const GrResourceIOProcessor::TextureSampler*> textureBindings;

    fGeometryProcessor->setData(fDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));
    append_texture_bindings(primProc, &textureBindings);

    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fDataManager, *fp);
        append_texture_bindings(*fp, &textureBindings);
        fp = iter.next();
        glslFP = glslIter.next();
    }
    SkASSERT(!fp && !glslFP);

    {
        SkIPoint offset;
        GrTexture* dstTexture = pipeline.peekDstTexture(&offset);

        fXferProcessor->setData(fDataManager, pipeline.getXferProcessor(), dstTexture, offset);
    }

    GrResourceProvider* resourceProvider = gpu->getContext()->contextPriv().resourceProvider();

    GrResourceIOProcessor::TextureSampler dstTextureSampler;
    if (GrTextureProxy* dstTextureProxy = pipeline.dstTextureProxy()) {
        dstTextureSampler.reset(sk_ref_sp(dstTextureProxy));
        SkAssertResult(dstTextureSampler.instantiate(resourceProvider));
        textureBindings.push_back(&dstTextureSampler);
    }

    // Get new descriptor sets
    if (fNumSamplers) {
        if (fSamplerDescriptorSet) {
            fSamplerDescriptorSet->recycle(gpu);
        }
        fSamplerDescriptorSet = gpu->resourceProvider().getSamplerDescriptorSet(fSamplerDSHandle);
        int samplerDSIdx = GrVkUniformHandler::kSamplerDescSet;
        fDescriptorSets[samplerDSIdx] = fSamplerDescriptorSet->descriptorSet();
        this->writeSamplers(gpu, textureBindings);
    }

    if (fGeometryUniformBuffer || fFragmentUniformBuffer) {
        if (fDataManager.uploadUniformBuffers(gpu,
                                              fGeometryUniformBuffer.get(),
                                              fFragmentUniformBuffer.get())
            || !fUniformDescriptorSet)
        {
            if (fUniformDescriptorSet) {
                fUniformDescriptorSet->recycle(gpu);
            }
            fUniformDescriptorSet = gpu->resourceProvider().getUniformDescriptorSet();
            int uniformDSIdx = GrVkUniformHandler::kUniformBufferDescSet;
            fDescriptorSets[uniformDSIdx] = fUniformDescriptorSet->descriptorSet();
            this->writeUniformBuffers(gpu);
        }
    }
}

void set_uniform_descriptor_writes(VkWriteDescriptorSet* descriptorWrite,
                                   VkDescriptorBufferInfo* bufferInfo,
                                   const GrVkUniformBuffer* buffer,
                                   VkDescriptorSet descriptorSet,
                                   uint32_t binding) {

    memset(bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    bufferInfo->buffer = buffer->buffer();
    bufferInfo->offset = buffer->offset();
    bufferInfo->range = buffer->size();

    memset(descriptorWrite, 0, sizeof(VkWriteDescriptorSet));
    descriptorWrite->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite->pNext = nullptr;
    descriptorWrite->dstSet = descriptorSet;
    descriptorWrite->dstBinding = binding;
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

    // Geometry Uniform Buffer
    if (fGeometryUniformBuffer.get()) {
        set_uniform_descriptor_writes(&descriptorWrites[writeCount],
                                      &bufferInfos[writeCount],
                                      fGeometryUniformBuffer.get(),
                                      fDescriptorSets[GrVkUniformHandler::kUniformBufferDescSet],
                                      GrVkUniformHandler::kGeometryBinding);
        ++writeCount;
    }

    // Fragment Uniform Buffer
    if (fFragmentUniformBuffer.get()) {
        set_uniform_descriptor_writes(&descriptorWrites[writeCount],
                                      &bufferInfos[writeCount],
                                      fFragmentUniformBuffer.get(),
                                      fDescriptorSets[GrVkUniformHandler::kUniformBufferDescSet],
                                      GrVkUniformHandler::kFragBinding);
        ++writeCount;
    }

    if (writeCount) {
        GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                            writeCount,
                                                            descriptorWrites,
                                                            0, nullptr));
    }
}

void GrVkPipelineState::writeSamplers(
        GrVkGpu* gpu,
        const SkTArray<const GrResourceIOProcessor::TextureSampler*>& textureBindings) {
    SkASSERT(fNumSamplers == textureBindings.count());

    for (int i = 0; i < textureBindings.count(); ++i) {
        GrSamplerState state = textureBindings[i]->samplerState();

        GrVkTexture* texture = static_cast<GrVkTexture*>(textureBindings[i]->peekTexture());

        fSamplers.push(gpu->resourceProvider().findOrCreateCompatibleSampler(
                state, texture->texturePriv().maxMipMapLevel()));

        const GrVkResource* textureResource = texture->resource();
        textureResource->ref();
        fTextures.push(textureResource);

        const GrVkImageView* textureView = texture->textureView();
        textureView->ref();
        fTextureViews.push(textureView);

        VkDescriptorImageInfo imageInfo;
        memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
        imageInfo.sampler = fSamplers[i]->sampler();
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

        GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                            1,
                                                            &writeInfo,
                                                            0,
                                                            nullptr));
    }
}

void GrVkPipelineState::setRenderTargetState(const GrRenderTargetProxy* proxy) {
    GrRenderTarget* rt = proxy->priv().peekRenderTarget();

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
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

void GrVkPipelineState::bind(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer) {
    commandBuffer->bindPipeline(gpu, fPipeline);

    if (fGeometryUniformBuffer || fFragmentUniformBuffer) {
        int dsIndex = GrVkUniformHandler::kUniformBufferDescSet;
        commandBuffer->bindDescriptorSets(gpu, this, fPipelineLayout,
                                          dsIndex, 1,
                                          &fDescriptorSets[dsIndex], 0, nullptr);
    }
    if (fNumSamplers) {
        int dsIndex = GrVkUniformHandler::kSamplerDescSet;
        commandBuffer->bindDescriptorSets(gpu, this, fPipelineLayout,
                                          dsIndex, 1,
                                          &fDescriptorSets[dsIndex], 0, nullptr);
    }
}

void GrVkPipelineState::addUniformResources(GrVkCommandBuffer& commandBuffer) {
    if (fUniformDescriptorSet) {
        commandBuffer.addRecycledResource(fUniformDescriptorSet);
    }
    if (fSamplerDescriptorSet) {
        commandBuffer.addRecycledResource(fSamplerDescriptorSet);
    }

    if (fGeometryUniformBuffer.get()) {
        commandBuffer.addRecycledResource(fGeometryUniformBuffer->resource());
    }
    if (fFragmentUniformBuffer.get()) {
        commandBuffer.addRecycledResource(fFragmentUniformBuffer->resource());
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
}
