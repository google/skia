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
#include "GrVkRenderTarget.h"
#include "GrVkSampler.h"
#include "GrVkUtil.h"

#ifdef SK_TRACE_VK_RESOURCES
SkTDynamicHash<GrVkResource, uint32_t> GrVkResource::fTrace;
SkRandom GrVkResource::fRandom;
#endif

GrVkResourceProvider::GrVkResourceProvider(GrVkGpu* gpu)
    : fGpu(gpu)
    , fPipelineCache(VK_NULL_HANDLE)
    , fUniformDescPool(nullptr)
    , fCurrentUniformDescCount(0) {
    fPipelineStateCache = new PipelineStateCache(gpu);
}

GrVkResourceProvider::~GrVkResourceProvider() {
    SkASSERT(0 == fRenderPassArray.count());
    SkASSERT(VK_NULL_HANDLE == fPipelineCache);
    delete fPipelineStateCache;
}

void GrVkResourceProvider::initUniformDescObjects() {
    // Create Uniform Buffer Descriptor
    // The vertex uniform buffer will have binding 0 and the fragment binding 1.
    VkDescriptorSetLayoutBinding dsUniBindings[2];
    memset(&dsUniBindings, 0, 2 * sizeof(VkDescriptorSetLayoutBinding));
    dsUniBindings[0].binding = GrVkUniformHandler::kVertexBinding;
    dsUniBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsUniBindings[0].descriptorCount = 1;
    dsUniBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dsUniBindings[0].pImmutableSamplers = nullptr;
    dsUniBindings[1].binding = GrVkUniformHandler::kFragBinding;
    dsUniBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsUniBindings[1].descriptorCount = 1;
    dsUniBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsUniBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo dsUniformLayoutCreateInfo;
    memset(&dsUniformLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dsUniformLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsUniformLayoutCreateInfo.pNext = nullptr;
    dsUniformLayoutCreateInfo.flags = 0;
    dsUniformLayoutCreateInfo.bindingCount = 2;
    dsUniformLayoutCreateInfo.pBindings = dsUniBindings;

    GR_VK_CALL_ERRCHECK(fGpu->vkInterface(), CreateDescriptorSetLayout(fGpu->device(),
                                                                       &dsUniformLayoutCreateInfo,
                                                                       nullptr,
                                                                       &fUniformDescLayout));
    fCurrMaxUniDescriptors = kStartNumUniformDescriptors;
    fUniformDescPool = this->findOrCreateCompatibleDescriptorPool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                                  fCurrMaxUniDescriptors);
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

    this->initUniformDescObjects();
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
GrVkResourceProvider::findCompatibleRenderPass(const GrVkRenderTarget& target,
                                               CompatibleRPHandle* compatibleHandle) {
    for (int i = 0; i < fRenderPassArray.count(); ++i) {
        if (fRenderPassArray[i].isCompatible(target)) {
            const GrVkRenderPass* renderPass = fRenderPassArray[i].getCompatibleRenderPass();
            renderPass->ref();
            if (compatibleHandle) {
                *compatibleHandle = CompatibleRPHandle(i);
            }
            return renderPass;
        }
    }

    const GrVkRenderPass* renderPass =
        fRenderPassArray.emplace_back(fGpu, target).getCompatibleRenderPass();
    renderPass->ref();

    if (compatibleHandle) {
        *compatibleHandle = CompatibleRPHandle(fRenderPassArray.count() - 1);
    }
    return renderPass;
}

const GrVkRenderPass*
GrVkResourceProvider::findCompatibleRenderPass(const CompatibleRPHandle& compatibleHandle) {
    SkASSERT(compatibleHandle.isValid() && compatibleHandle.toIndex() < fRenderPassArray.count());
    int index = compatibleHandle.toIndex();
    const GrVkRenderPass* renderPass = fRenderPassArray[index].getCompatibleRenderPass();
    renderPass->ref();
    return renderPass;
}

const GrVkRenderPass* GrVkResourceProvider::findRenderPass(
                                                     const GrVkRenderTarget& target,
                                                     const GrVkRenderPass::LoadStoreOps& colorOps,
                                                     const GrVkRenderPass::LoadStoreOps& resolveOps,
                                                     const GrVkRenderPass::LoadStoreOps& stencilOps,
                                                     CompatibleRPHandle* compatibleHandle) {
    GrVkResourceProvider::CompatibleRPHandle tempRPHandle;
    GrVkResourceProvider::CompatibleRPHandle* pRPHandle = compatibleHandle ? compatibleHandle
                                                                           : &tempRPHandle;
    *pRPHandle = target.compatibleRenderPassHandle();

    // This will get us the handle to (and possible create) the compatible set for the specific
    // GrVkRenderPass we are looking for.
    this->findCompatibleRenderPass(target, compatibleHandle);
    return this->findRenderPass(*pRPHandle, colorOps, resolveOps, stencilOps);
}

const GrVkRenderPass*
GrVkResourceProvider::findRenderPass(const CompatibleRPHandle& compatibleHandle,
                                     const GrVkRenderPass::LoadStoreOps& colorOps,
                                     const GrVkRenderPass::LoadStoreOps& resolveOps,
                                     const GrVkRenderPass::LoadStoreOps& stencilOps) {
    SkASSERT(compatibleHandle.isValid() && compatibleHandle.toIndex() < fRenderPassArray.count());
    CompatibleRenderPassSet& compatibleSet = fRenderPassArray[compatibleHandle.toIndex()];
    const GrVkRenderPass* renderPass = compatibleSet.getRenderPass(fGpu,
                                                                   colorOps,
                                                                   resolveOps,
                                                                   stencilOps);
    renderPass->ref();
    return renderPass;
}

GrVkDescriptorPool* GrVkResourceProvider::findOrCreateCompatibleDescriptorPool(
                                                            VkDescriptorType type, uint32_t count) {
    return new GrVkDescriptorPool(fGpu, type, count);
}

GrVkSampler* GrVkResourceProvider::findOrCreateCompatibleSampler(const GrTextureParams& params,
                                                                 uint32_t mipLevels) {
    GrVkSampler* sampler = fSamplers.find(GrVkSampler::GenerateKey(params, mipLevels));
    if (!sampler) {
        sampler = GrVkSampler::Create(fGpu, params, mipLevels);
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

void GrVkResourceProvider::getUniformDescriptorSet(VkDescriptorSet* ds,
                                                   const GrVkDescriptorPool** outPool) {
    fCurrentUniformDescCount += kNumUniformDescPerSet;
    if (fCurrentUniformDescCount > fCurrMaxUniDescriptors) {
        fUniformDescPool->unref(fGpu);
        if (fCurrMaxUniDescriptors < kMaxUniformDescriptors >> 1) {
            fCurrMaxUniDescriptors = fCurrMaxUniDescriptors << 1;
        } else {
            fCurrMaxUniDescriptors = kMaxUniformDescriptors;
        }
        fUniformDescPool =
            this->findOrCreateCompatibleDescriptorPool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                       fCurrMaxUniDescriptors);
        fCurrentUniformDescCount = kNumUniformDescPerSet;
    }
    SkASSERT(fUniformDescPool);

    VkDescriptorSetAllocateInfo dsAllocateInfo;
    memset(&dsAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    dsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocateInfo.pNext = nullptr;
    dsAllocateInfo.descriptorPool = fUniformDescPool->descPool();
    dsAllocateInfo.descriptorSetCount = 1;
    dsAllocateInfo.pSetLayouts = &fUniformDescLayout;
    GR_VK_CALL_ERRCHECK(fGpu->vkInterface(), AllocateDescriptorSets(fGpu->device(),
                                                                    &dsAllocateInfo,
                                                                    ds));
    *outPool = fUniformDescPool;
}

GrVkPrimaryCommandBuffer* GrVkResourceProvider::createPrimaryCommandBuffer() {
    GrVkPrimaryCommandBuffer* cmdBuffer = GrVkPrimaryCommandBuffer::Create(fGpu, fGpu->cmdPool());
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

    // loop over all render pass sets to make sure we destroy all the internal VkRenderPasses
    for (int i = 0; i < fRenderPassArray.count(); ++i) {
        fRenderPassArray[i].releaseResources(fGpu);
    }
    fRenderPassArray.reset();

    // Iterate through all store GrVkSamplers and unref them before resetting the hash.
    SkTDynamicHash<GrVkSampler, uint16_t>::Iter iter(&fSamplers);
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

    if (fUniformDescLayout) {
        GR_VK_CALL(fGpu->vkInterface(), DestroyDescriptorSetLayout(fGpu->device(),
                                                                   fUniformDescLayout,
                                                                   nullptr));
        fUniformDescLayout = VK_NULL_HANDLE;
    }
    fUniformDescPool->unref(fGpu);
}

void GrVkResourceProvider::abandonResources() {
    // release our current command buffers
    for (int i = 0; i < fActiveCommandBuffers.count(); ++i) {
        SkASSERT(fActiveCommandBuffers[i]->finished(fGpu));
        fActiveCommandBuffers[i]->unrefAndAbandon();
    }
    fActiveCommandBuffers.reset();

    // loop over all render pass sets to make sure we destroy all the internal VkRenderPasses
    for (int i = 0; i < fRenderPassArray.count(); ++i) {
        fRenderPassArray[i].abandonResources();
    }
    fRenderPassArray.reset();

    // Iterate through all store GrVkSamplers and unrefAndAbandon them before resetting the hash.
    SkTDynamicHash<GrVkSampler, uint16_t>::Iter iter(&fSamplers);
    for (; !iter.done(); ++iter) {
        (*iter).unrefAndAbandon();
    }
    fSamplers.reset();

    fPipelineStateCache->abandon();

#ifdef SK_TRACE_VK_RESOURCES
    SkASSERT(0 == GrVkResource::fTrace.count());
#endif
    fPipelineCache = VK_NULL_HANDLE;

    fUniformDescLayout = VK_NULL_HANDLE;
    fUniformDescPool->unrefAndAbandon();
}

////////////////////////////////////////////////////////////////////////////////

GrVkResourceProvider::CompatibleRenderPassSet::CompatibleRenderPassSet(
                                                                     const GrVkGpu* gpu,
                                                                     const GrVkRenderTarget& target)
    : fLastReturnedIndex(0) {
    fRenderPasses.emplace_back(new GrVkRenderPass());
    fRenderPasses[0]->initSimple(gpu, target);
}

bool GrVkResourceProvider::CompatibleRenderPassSet::isCompatible(
                                                             const GrVkRenderTarget& target) const {
    // The first GrVkRenderpass should always exists since we create the basic load store
    // render pass on create
    SkASSERT(fRenderPasses[0]);
    return fRenderPasses[0]->isCompatible(target);
}

GrVkRenderPass* GrVkResourceProvider::CompatibleRenderPassSet::getRenderPass(
                                                   const GrVkGpu* gpu,
                                                   const GrVkRenderPass::LoadStoreOps& colorOps,
                                                   const GrVkRenderPass::LoadStoreOps& resolveOps,
                                                   const GrVkRenderPass::LoadStoreOps& stencilOps) {
    for (int i = 0; i < fRenderPasses.count(); ++i) {
        int idx = (i + fLastReturnedIndex) % fRenderPasses.count();
        if (fRenderPasses[idx]->equalLoadStoreOps(colorOps, resolveOps, stencilOps)) {
            fLastReturnedIndex = idx;
            return fRenderPasses[idx];
        }
    }
    GrVkRenderPass* renderPass = fRenderPasses.emplace_back(new GrVkRenderPass());
    renderPass->init(gpu, *this->getCompatibleRenderPass(), colorOps, resolveOps, stencilOps);
    fLastReturnedIndex = fRenderPasses.count() - 1;
    return renderPass;
}

void GrVkResourceProvider::CompatibleRenderPassSet::releaseResources(const GrVkGpu* gpu) {
    for (int i = 0; i < fRenderPasses.count(); ++i) {
        if (fRenderPasses[i]) {
            fRenderPasses[i]->unref(gpu);
            fRenderPasses[i] = nullptr;
        }
    }
}

void GrVkResourceProvider::CompatibleRenderPassSet::abandonResources() {
    for (int i = 0; i < fRenderPasses.count(); ++i) {
        if (fRenderPasses[i]) {
            fRenderPasses[i]->unrefAndAbandon();
            fRenderPasses[i] = nullptr;
        }
    }
}
