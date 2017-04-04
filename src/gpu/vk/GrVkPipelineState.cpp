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
#include "GrVkDescriptorSet.h"
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
                                     const GrVkDescriptorSetManager::Handle& samplerDSHandle,
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
    , fUniformDescriptorSet(nullptr)
    , fSamplerDescriptorSet(nullptr)
    , fSamplerDSHandle(samplerDSHandle)
    , fStartDS(SK_MaxS32)
    , fDSCount(0)
    , fBuiltinUniformHandles(builtinUniformHandles)
    , fGeometryProcessor(geometryProcessor)
    , fXferProcessor(xferProcessor)
    , fFragmentProcessors(fragmentProcessors)
    , fDesc(desc)
    , fDataManager(uniforms, vertexUniformSize, fragmentUniformSize) {
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

    fVertexUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, vertexUniformSize));
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
    // We don't support image storages in VK.
    SkASSERT(!processor.numImageStorages());
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

    this->setRenderTargetState(pipeline.getRenderTarget());

    SkSTArray<8, const GrResourceIOProcessor::TextureSampler*> textureBindings;

    fGeometryProcessor->setData(fDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));
    append_texture_bindings(primProc, &textureBindings);

    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.begin(),
                                           fFragmentProcessors.count());
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fDataManager, *fp);
        append_texture_bindings(*fp, &textureBindings);
        fp = iter.next();
        glslFP = glslIter.next();
    }
    SkASSERT(!fp && !glslFP);

    SkIPoint offset;
    GrTexture* dstTexture = pipeline.dstTexture(&offset);
    fXferProcessor->setData(fDataManager, pipeline.getXferProcessor(), dstTexture, offset);
    GrResourceIOProcessor::TextureSampler dstTextureSampler;
    if (dstTexture) {
        dstTextureSampler.reset(dstTexture);
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
        this->writeSamplers(gpu, textureBindings, pipeline.getAllowSRGBInputs());
    }

    if (fVertexUniformBuffer.get() || fFragmentUniformBuffer.get()) {
        if (fDataManager.uploadUniformBuffers(gpu,
                                              fVertexUniformBuffer.get(),
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
        vertBufferInfo.offset = fVertexUniformBuffer->offset();
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
        fragBufferInfo.offset = fFragmentUniformBuffer->offset();
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

void GrVkPipelineState::writeSamplers(
        GrVkGpu* gpu,
        const SkTArray<const GrResourceIOProcessor::TextureSampler*>& textureBindings,
        bool allowSRGBInputs) {
    SkASSERT(fNumSamplers == textureBindings.count());

    for (int i = 0; i < textureBindings.count(); ++i) {
        const GrSamplerParams& params = textureBindings[i]->params();

        GrVkTexture* texture = static_cast<GrVkTexture*>(textureBindings[i]->texture());

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

void GrVkPipelineState::setRenderTargetState(const GrRenderTarget* rt) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
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
    if (fUniformDescriptorSet) {
        commandBuffer.addRecycledResource(fUniformDescriptorSet);
    }
    if (fSamplerDescriptorSet) {
        commandBuffer.addRecycledResource(fSamplerDescriptorSet);
    }

    if (fVertexUniformBuffer.get()) {
        commandBuffer.addRecycledResource(fVertexUniformBuffer->resource());
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

////////////////////////////////////////////////////////////////////////////////

void GrVkPipelineState::DescriptorPoolManager::getNewPool(GrVkGpu* gpu) {
    if (fPool) {
        fPool->unref(gpu);
        uint32_t newPoolSize = fMaxDescriptors + ((fMaxDescriptors + 1) >> 1);
        if (newPoolSize < kMaxDescLimit) {
            fMaxDescriptors = newPoolSize;
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

bool GrVkPipelineState::Desc::Build(Desc* desc,
                                    const GrPrimitiveProcessor& primProc,
                                    const GrPipeline& pipeline,
                                    const GrStencilSettings& stencil,
                                    GrPrimitiveType primitiveType,
                                    const GrShaderCaps& caps) {
    if (!INHERITED::Build(desc, primProc, primitiveType == kPoints_GrPrimitiveType, pipeline,
                          caps)) {
        return false;
    }

    GrProcessorKeyBuilder b(&desc->key());
    GrVkRenderTarget* vkRT = (GrVkRenderTarget*)pipeline.getRenderTarget();
    vkRT->simpleRenderPass()->genKey(&b);

    stencil.genKey(&b);

    SkASSERT(sizeof(GrDrawFace) <= sizeof(uint32_t));
    b.add32((int32_t)pipeline.getDrawFace());

    b.add32(get_blend_info_key(pipeline));

    b.add32(primitiveType);

    return true;
}
