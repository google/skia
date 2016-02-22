/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkResourceProvider.h"

#include "GrVkCommandBuffer.h"
#include "GrVkPipeline.h"
#include "GrVkRenderPass.h"
#include "GrVkUtil.h"

#ifdef SK_TRACE_VK_RESOURCES
SkTDynamicHash<GrVkResource, uint32_t> GrVkResource::fTrace;
SkRandom GrVkResource::fRandom;
#endif

GrVkResourceProvider::GrVkResourceProvider(GrVkGpu* gpu) : fGpu(gpu) {
}

GrVkResourceProvider::~GrVkResourceProvider() {
    SkASSERT(0 == fSimpleRenderPasses.count());
}

GrVkPipeline* GrVkResourceProvider::createPipeline(const GrPipeline& pipeline,
                                                   const GrPrimitiveProcessor& primProc,
                                                   VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                                   int shaderStageCount,
                                                   GrPrimitiveType primitiveType,
                                                   const GrVkRenderPass& renderPass,
                                                   VkPipelineLayout layout) {

    return GrVkPipeline::Create(fGpu, pipeline, primProc, shaderStageInfo, shaderStageCount,
                                primitiveType, renderPass, layout);
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
                                       const GrVkDescriptorPool::DescriptorTypeCounts& typeCounts) {
    return new GrVkDescriptorPool(fGpu, typeCounts);
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

#ifdef SK_TRACE_VK_RESOURCES
    SkASSERT(0 == GrVkResource::fTrace.count());
#endif

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

#ifdef SK_TRACE_VK_RESOURCES
    SkASSERT(0 == GrVkResource::fTrace.count());
#endif
}