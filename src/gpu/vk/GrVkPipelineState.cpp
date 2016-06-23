/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkPipelineState.h"

#include "GrPipeline.h"
#include "GrTexturePriv.h"
#include "GrVkCommandBuffer.h"
#include "GrVkDescriptorPool.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrVkMemory.h"
#include "GrVkPipeline.h"
#include "GrVkRenderTarget.h"
#include "GrVkSampler.h"
#include "GrVkTexture.h"
#include "GrVkUniformBuffer.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLXferProcessor.h"
#include "SkMipMap.h"

GrVkPipelineState::GrVkPipelineState(GrVkGpu* gpu,
                                     const GrVkPipelineState::Desc& desc,
                                     GrVkPipeline* pipeline,
                                     VkPipelineLayout layout,
                                     VkDescriptorSetLayout dsSamplerLayout,
                                     const BuiltinUniformHandles& builtinUniformHandles,
                                     const UniformInfoArray& uniforms,
                                     uint32_t vertexUniformSize,
                                     uint32_t fragmentUniformSize,
                                     uint32_t numSamplers,
                                     GrGLSLPrimitiveProcessor* geometryProcessor,
                                     GrGLSLXferProcessor* xferProcessor,
                                     const GrGLSLFragProcs& fragmentProcessors)
    : fPipeline(pipeline)
    , fPipelineLayout(layout)
    , fStartDS(SK_MaxS32)
    , fDSCount(0)
    , fBuiltinUniformHandles(builtinUniformHandles)
    , fGeometryProcessor(geometryProcessor)
    , fXferProcessor(xferProcessor)
    , fFragmentProcessors(fragmentProcessors)
    , fDesc(desc)
    , fDataManager(uniforms, vertexUniformSize, fragmentUniformSize)
    , fSamplerPoolManager(dsSamplerLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                          numSamplers, gpu)
    , fCurrentUniformDescPool(nullptr) {
    fSamplers.setReserve(numSamplers);
    fTextureViews.setReserve(numSamplers);
    fTextures.setReserve(numSamplers);

    fDescriptorSets[0] = VK_NULL_HANDLE;
    fDescriptorSets[1] = VK_NULL_HANDLE;

    // Currently we are always binding a descriptor set for uniform buffers.
    if (vertexUniformSize || fragmentUniformSize) {
        fDSCount++;
        fStartDS = GrVkUniformHandler::kUniformBufferDescSet;
    }
    if (numSamplers) {
        fDSCount++;
        fStartDS = SkTMin(fStartDS, (int)GrVkUniformHandler::kSamplerDescSet);
    }

    fVertexUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, vertexUniformSize, true));
    fFragmentUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, fragmentUniformSize, true));

    fNumSamplers = numSamplers;
}

GrVkPipelineState::~GrVkPipelineState() {
    // Must of freed all GPU resources before this is destroyed
    SkASSERT(!fPipeline);
    SkASSERT(!fPipelineLayout);
    SkASSERT(!fSamplers.count());
    SkASSERT(!fTextureViews.count());
    SkASSERT(!fTextures.count());
    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        delete fFragmentProcessors[i];
    }
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
        GR_VK_CALL(gpu->vkInterface(), DestroyPipelineLayout(gpu->device(),
                                                             fPipelineLayout,
                                                             nullptr));
        fPipelineLayout = VK_NULL_HANDLE;
    }

    if (fVertexUniformBuffer) {
        fVertexUniformBuffer->release(gpu);
    }

    if (fFragmentUniformBuffer) {
        fFragmentUniformBuffer->release(gpu);
    }

    fSamplerPoolManager.freeGPUResources(gpu);
    if (fCurrentUniformDescPool) {
        fCurrentUniformDescPool->unref(gpu);
        fCurrentUniformDescPool = nullptr;
    }

    this->freeTempResources(gpu);
}

void GrVkPipelineState::abandonGPUResources() {
    fPipeline->unrefAndAbandon();
    fPipeline = nullptr;

    fPipelineLayout = VK_NULL_HANDLE;

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

    fSamplerPoolManager.abandonGPUResources();
    if (fCurrentUniformDescPool) {
        fCurrentUniformDescPool->unrefAndAbandon();
        fCurrentUniformDescPool = nullptr;
    }
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

void GrVkPipelineState::setData(GrVkGpu* gpu,
                                const GrPrimitiveProcessor& primProc,
                                const GrPipeline& pipeline) {
    // This is here to protect against someone calling setData multiple times in a row without
    // freeing the tempData between calls.
    this->freeTempResources(gpu);

    this->setRenderTargetState(pipeline);

    SkSTArray<8, const GrTextureAccess*> textureBindings;

    fGeometryProcessor->setData(fDataManager, primProc);
    append_texture_bindings(primProc, &textureBindings);

    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        const GrFragmentProcessor& processor = pipeline.getFragmentProcessor(i);
        fFragmentProcessors[i]->setData(fDataManager, processor);
        fGeometryProcessor->setTransformData(primProc, fDataManager, i,
                                             processor.coordTransforms());
        append_texture_bindings(processor, &textureBindings);
    }

    fXferProcessor->setData(fDataManager, pipeline.getXferProcessor());
    append_texture_bindings(pipeline.getXferProcessor(), &textureBindings);

    // Get new descriptor sets
    if (fNumSamplers) {
        fSamplerPoolManager.getNewDescriptorSet(gpu,
                                             &fDescriptorSets[GrVkUniformHandler::kSamplerDescSet]);
        this->writeSamplers(gpu, textureBindings, pipeline.getAllowSRGBInputs());
    }

    if (fVertexUniformBuffer.get() || fFragmentUniformBuffer.get()) {
        if (fDataManager.uploadUniformBuffers(gpu, fVertexUniformBuffer, fFragmentUniformBuffer) ||
            VK_NULL_HANDLE == fDescriptorSets[GrVkUniformHandler::kUniformBufferDescSet]) {
            const GrVkDescriptorPool* pool;
            int uniformDSIdx = GrVkUniformHandler::kUniformBufferDescSet;
            gpu->resourceProvider().getUniformDescriptorSet(&fDescriptorSets[uniformDSIdx],
                                                            &pool);
            if (pool != fCurrentUniformDescPool) {
                if (fCurrentUniformDescPool) {
                    fCurrentUniformDescPool->unref(gpu);
                }
                fCurrentUniformDescPool = pool;
                fCurrentUniformDescPool->ref();
            }
            this->writeUniformBuffers(gpu);
        }
    }
}

void GrVkPipelineState::writeUniformBuffers(const GrVkGpu* gpu) {
    VkWriteDescriptorSet descriptorWrites[2];
    memset(descriptorWrites, 0, 2 * sizeof(VkWriteDescriptorSet));

    uint32_t firstUniformWrite = 0;
    uint32_t uniformBindingUpdateCount = 0;

    VkDescriptorBufferInfo vertBufferInfo;
    // Vertex Uniform Buffer
    if (fVertexUniformBuffer.get()) {
        ++uniformBindingUpdateCount;
        memset(&vertBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
        vertBufferInfo.buffer = fVertexUniformBuffer->buffer();
        vertBufferInfo.offset = 0;
        vertBufferInfo.range = fVertexUniformBuffer->size();

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].pNext = nullptr;
        descriptorWrites[0].dstSet = fDescriptorSets[GrVkUniformHandler::kUniformBufferDescSet];
        descriptorWrites[0].dstBinding = GrVkUniformHandler::kVertexBinding;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].pImageInfo = nullptr;
        descriptorWrites[0].pBufferInfo = &vertBufferInfo;
        descriptorWrites[0].pTexelBufferView = nullptr;
    }

    VkDescriptorBufferInfo fragBufferInfo;
    // Fragment Uniform Buffer
    if (fFragmentUniformBuffer.get()) {
        if (0 == uniformBindingUpdateCount) {
            firstUniformWrite = 1;
        }
        ++uniformBindingUpdateCount;
        memset(&fragBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
        fragBufferInfo.buffer = fFragmentUniformBuffer->buffer();
        fragBufferInfo.offset = 0;
        fragBufferInfo.range = fFragmentUniformBuffer->size();

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].pNext = nullptr;
        descriptorWrites[1].dstSet = fDescriptorSets[GrVkUniformHandler::kUniformBufferDescSet];
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

void GrVkPipelineState::writeSamplers(GrVkGpu* gpu,
                                      const SkTArray<const GrTextureAccess*>& textureBindings,
                                      bool allowSRGBInputs) {
    SkASSERT(fNumSamplers == textureBindings.count());

    for (int i = 0; i < textureBindings.count(); ++i) {
        const GrTextureParams& params = textureBindings[i]->getParams();

        GrVkTexture* texture = static_cast<GrVkTexture*>(textureBindings[i]->getTexture());

        fSamplers.push(gpu->resourceProvider().findOrCreateCompatibleSampler(params,
                                                          texture->texturePriv().maxMipMapLevel()));

        const GrVkResource* textureResource = texture->resource();
        textureResource->ref();
        fTextures.push(textureResource);

        const GrVkImageView* textureView = texture->textureView(allowSRGBInputs);
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

void GrVkPipelineState::setRenderTargetState(const GrPipeline& pipeline) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != pipeline.getRenderTarget()->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni,
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
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

void GrVkPipelineState::bind(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer) {
    commandBuffer->bindPipeline(gpu, fPipeline);

    if (fDSCount) {
        commandBuffer->bindDescriptorSets(gpu, this, fPipelineLayout, fStartDS, fDSCount,
                                          &fDescriptorSets[fStartDS], 0, nullptr);
    }
}

void GrVkPipelineState::addUniformResources(GrVkCommandBuffer& commandBuffer) {
    if (fSamplerPoolManager.fPool) {
        commandBuffer.addResource(fSamplerPoolManager.fPool);
    }
    if (fCurrentUniformDescPool) {
        commandBuffer.addResource(fCurrentUniformDescPool);
    }

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
}

////////////////////////////////////////////////////////////////////////////////

void GrVkPipelineState::DescriptorPoolManager::getNewPool(GrVkGpu* gpu) {
    if (fPool) {
        fPool->unref(gpu);
        if (fMaxDescriptors < kMaxDescLimit >> 1) {
            fMaxDescriptors = fMaxDescriptors << 1;
        } else {
            fMaxDescriptors = kMaxDescLimit;
        }

    }
    if (fMaxDescriptors) {
        fPool = gpu->resourceProvider().findOrCreateCompatibleDescriptorPool(fDescType,
                                                                             fMaxDescriptors);
    }
    SkASSERT(fPool || !fMaxDescriptors);
}

void GrVkPipelineState::DescriptorPoolManager::getNewDescriptorSet(GrVkGpu* gpu,
                                                                   VkDescriptorSet* ds) {
    if (!fMaxDescriptors) {
        return;
    }
    fCurrentDescriptorCount += fDescCountPerSet;
    if (fCurrentDescriptorCount > fMaxDescriptors) {
        this->getNewPool(gpu);
        fCurrentDescriptorCount = fDescCountPerSet;
    }

    VkDescriptorSetAllocateInfo dsAllocateInfo;
    memset(&dsAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    dsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocateInfo.pNext = nullptr;
    dsAllocateInfo.descriptorPool = fPool->descPool();
    dsAllocateInfo.descriptorSetCount = 1;
    dsAllocateInfo.pSetLayouts = &fDescLayout;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), AllocateDescriptorSets(gpu->device(),
                                                                   &dsAllocateInfo,
                                                                   ds));
}

void GrVkPipelineState::DescriptorPoolManager::freeGPUResources(const GrVkGpu* gpu) {
    if (fDescLayout) {
        GR_VK_CALL(gpu->vkInterface(), DestroyDescriptorSetLayout(gpu->device(), fDescLayout,
                                                                  nullptr));
        fDescLayout = VK_NULL_HANDLE;
    }

    if (fPool) {
        fPool->unref(gpu);
        fPool = nullptr;
    }
}

void GrVkPipelineState::DescriptorPoolManager::abandonGPUResources() {
    fDescLayout = VK_NULL_HANDLE;
    if (fPool) {
        fPool->unrefAndAbandon();
        fPool = nullptr;
    }
}

uint32_t get_blend_info_key(const GrPipeline& pipeline) {
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);

    static const uint32_t kBlendWriteShift = 1;
    static const uint32_t kBlendCoeffShift = 5;
    GR_STATIC_ASSERT(kLast_GrBlendCoeff < (1 << kBlendCoeffShift));
    GR_STATIC_ASSERT(kFirstAdvancedGrBlendEquation - 1 < 4);

    uint32_t key = blendInfo.fWriteColor;
    key |= (blendInfo.fSrcBlend << kBlendWriteShift);
    key |= (blendInfo.fDstBlend << (kBlendWriteShift + kBlendCoeffShift));
    key |= (blendInfo.fEquation << (kBlendWriteShift + 2 * kBlendCoeffShift));

    return key;
}

void GrVkPipelineState::BuildStateKey(const GrPipeline& pipeline, GrPrimitiveType primitiveType,
                                      SkTArray<uint8_t, true>* key) {
    // Save room for the key length and key header
    key->reset();
    key->push_back_n(kData_StateKeyOffset);

    GrProcessorKeyBuilder b(key);

    GrVkRenderTarget* vkRT = (GrVkRenderTarget*)pipeline.getRenderTarget();
    vkRT->simpleRenderPass()->genKey(&b);

    pipeline.getStencil().genKey(&b);

    SkASSERT(sizeof(GrPipelineBuilder::DrawFace) <= sizeof(uint32_t));
    b.add32(pipeline.getDrawFace());

    b.add32(get_blend_info_key(pipeline));

    b.add32(primitiveType);

    // Set key length
    int keyLength = key->count();
    SkASSERT(0 == (keyLength % 4));
    *reinterpret_cast<uint32_t*>(key->begin()) = SkToU32(keyLength);
}
