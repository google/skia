/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkResourceProvider.h"

#include "GrTextureParams.h"
#include "GrVkCommandBuffer.h"
#include "GrVkPipeline.h"
#include "GrVkRenderPass.h"
#include "GrVkSampler.h"
#include "GrVkUtil.h"

#ifdef SK_TRACE_VK_RESOURCES
SkTDynamicHash<GrVkResource, uint32_t> GrVkResource::fTrace;
SkRandom GrVkResource::fRandom;
#endif

GrVkResourceProvider::GrVkResourceProvider(GrVkGpu* gpu) : fGpu(gpu)
                                                         , fPipelineCache(VK_NULL_HANDLE) {
    fPipelineStateCache = new PipelineStateCache(gpu);
}

GrVkResourceProvider::~GrVkResourceProvider() {
    SkASSERT(0 == fSimpleRenderPasses.count());
    SkASSERT(VK_NULL_HANDLE == fPipelineCache);
    delete fPipelineStateCache;
}

void GrVkResourceProvider::init() {
    VkPipelineCacheCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.initialDataSize = 0;
    createInfo.pInitialData = nullptr;
    VkResult result = GR_VK_CALL(fGpu->vkInterface(),
                                 CreatePipelineCache(fGpu->device(), &createInfo, nullptr,
                                                     &fPipelineCache));
    SkASSERT(VK_SUCCESS == result);
    if (VK_SUCCESS != result) {
        fPipelineCache = VK_NULL_HANDLE;
    }
}

GrVkPipeline* GrVkResourceProvider::createPipeline(const GrPipeline& pipeline,
                                                   const GrPrimitiveProcessor& primProc,
                                                   VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                                   int shaderStageCount,
                                                   GrPrimitiveType primitiveType,
                                                   const GrVkRenderPass& renderPass,
                                                   VkPipelineLayout layout) {

    return GrVkPipeline::Create(fGpu, pipeline, primProc, shaderStageInfo, shaderStageCount,
                                primitiveType, renderPass, layout, fPipelineCache);
}


// To create framebuffers, we first need to create a simple RenderPass that is
// only used for framebuffer creation. When we actually render we will create
// RenderPasses as needed that are compatible with the framebuffer.
const GrVkRenderPass*
GrVkResourceProvider::findOrCreateCompatibleRenderPass(const GrVkRenderTarget& target) {
    for (int i = 0; i < fSimpleRenderPasses.count(); ++i) {
        GrVkRenderPass* renderPass = fSimpleRenderPasses[i];
        if (renderPass->isCompatible(target)) {
            renderPass->ref();
            return renderPass;
        }
    }

    GrVkRenderPass* renderPass = new GrVkRenderPass();
    renderPass->initSimple(fGpu, target);
    fSimpleRenderPasses.push_back(renderPass);
    renderPass->ref();
    return renderPass;
}

GrVkDescriptorPool* GrVkResourceProvider::findOrCreateCompatibleDescriptorPool(
                                                            VkDescriptorType type, uint32_t count) {
    return new GrVkDescriptorPool(fGpu, type, count);
}

GrVkSampler* GrVkResourceProvider::findOrCreateCompatibleSampler(const GrTextureParams& params) {
    GrVkSampler* sampler = fSamplers.find(GrVkSampler::GenerateKey(params));
    if (!sampler) {
        sampler = GrVkSampler::Create(fGpu, params);
        fSamplers.add(sampler);
    }
    SkASSERT(sampler);
    sampler->ref();
    return sampler;
}

sk_sp<GrVkPipelineState> GrVkResourceProvider::findOrCreateCompatiblePipelineState(
                                                                 const GrPipeline& pipeline,
                                                                 const GrPrimitiveProcessor& proc,
                                                                 GrPrimitiveType primitiveType,
                                                                 const GrVkRenderPass& renderPass) {
    return fPipelineStateCache->refPipelineState(pipeline, proc, primitiveType, renderPass);
}

GrVkCommandBuffer* GrVkResourceProvider::createCommandBuffer() {
    GrVkCommandBuffer* cmdBuffer = GrVkCommandBuffer::Create(fGpu, fGpu->cmdPool());
    fActiveCommandBuffers.push_back(cmdBuffer);
    cmdBuffer->ref();
    return cmdBuffer;
}

void GrVkResourceProvider::checkCommandBuffers() {
    for (int i = fActiveCommandBuffers.count()-1; i >= 0; --i) {
        if (fActiveCommandBuffers[i]->finished(fGpu)) {
            fActiveCommandBuffers[i]->unref(fGpu);
            fActiveCommandBuffers.removeShuffle(i);
        }
    }
}

void GrVkResourceProvider::destroyResources() {
    // release our current command buffers
    for (int i = 0; i < fActiveCommandBuffers.count(); ++i) {
        SkASSERT(fActiveCommandBuffers[i]->finished(fGpu));
        SkASSERT(fActiveCommandBuffers[i]->unique());
        fActiveCommandBuffers[i]->unref(fGpu);
    }
    fActiveCommandBuffers.reset();

    // loop over all render passes to make sure we destroy all the internal VkRenderPasses
    for (int i = 0; i < fSimpleRenderPasses.count(); ++i) {
        fSimpleRenderPasses[i]->unref(fGpu);
    }
    fSimpleRenderPasses.reset();

    // Iterate through all store GrVkSamplers and unref them before resetting the hash.
    SkTDynamicHash<GrVkSampler, uint8_t>::Iter iter(&fSamplers);
    for (; !iter.done(); ++iter) {
        (*iter).unref(fGpu);
    }
    fSamplers.reset();

    fPipelineStateCache->release();

#ifdef SK_TRACE_VK_RESOURCES
    SkASSERT(0 == GrVkResource::fTrace.count());
#endif

    GR_VK_CALL(fGpu->vkInterface(), DestroyPipelineCache(fGpu->device(), fPipelineCache, nullptr));
    fPipelineCache = VK_NULL_HANDLE;
}

void GrVkResourceProvider::abandonResources() {
    // release our current command buffers
    for (int i = 0; i < fActiveCommandBuffers.count(); ++i) {
        SkASSERT(fActiveCommandBuffers[i]->finished(fGpu));
        fActiveCommandBuffers[i]->unrefAndAbandon();
    }
    fActiveCommandBuffers.reset();

    for (int i = 0; i < fSimpleRenderPasses.count(); ++i) {
        fSimpleRenderPasses[i]->unrefAndAbandon();
    }
    fSimpleRenderPasses.reset();

    // Iterate through all store GrVkSamplers and unrefAndAbandon them before resetting the hash.
    SkTDynamicHash<GrVkSampler, uint8_t>::Iter iter(&fSamplers);
    for (; !iter.done(); ++iter) {
        (*iter).unrefAndAbandon();
    }
    fSamplers.reset();

    fPipelineStateCache->abandon();

#ifdef SK_TRACE_VK_RESOURCES
    SkASSERT(0 == GrVkResource::fTrace.count());
#endif
    fPipelineCache = VK_NULL_HANDLE;
}
