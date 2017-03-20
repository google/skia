/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkResourceProvider.h"

#include "GrSamplerParams.h"
#include "GrVkCommandBuffer.h"
#include "GrVkCopyPipeline.h"
#include "GrVkPipeline.h"
#include "GrVkRenderTarget.h"
#include "GrVkSampler.h"
#include "GrVkUniformBuffer.h"
#include "GrVkUtil.h"

#ifdef SK_TRACE_VK_RESOURCES
GrVkResource::Trace GrVkResource::fTrace;
uint32_t GrVkResource::fKeyCounter = 0;
#endif

GrVkResourceProvider::GrVkResourceProvider(GrVkGpu* gpu)
    : fGpu(gpu)
    , fPipelineCache(VK_NULL_HANDLE) {
    fPipelineStateCache = new PipelineStateCache(gpu);
}

GrVkResourceProvider::~GrVkResourceProvider() {
    SkASSERT(0 == fRenderPassArray.count());
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

    // Init uniform descriptor objects
    fDescriptorSetManagers.emplace_back(fGpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    SkASSERT(1 == fDescriptorSetManagers.count());
    fUniformDSHandle = GrVkDescriptorSetManager::Handle(0);
}

GrVkPipeline* GrVkResourceProvider::createPipeline(const GrPipeline& pipeline,
                                                   const GrStencilSettings& stencil,
                                                   const GrPrimitiveProcessor& primProc,
                                                   VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                                   int shaderStageCount,
                                                   GrPrimitiveType primitiveType,
                                                   const GrVkRenderPass& renderPass,
                                                   VkPipelineLayout layout) {

    return GrVkPipeline::Create(fGpu, pipeline, stencil, primProc, shaderStageInfo,
                                shaderStageCount, primitiveType, renderPass, layout,
                                fPipelineCache);
}

GrVkCopyPipeline* GrVkResourceProvider::findOrCreateCopyPipeline(
        const GrVkRenderTarget* dst,
        VkPipelineShaderStageCreateInfo* shaderStageInfo,
        VkPipelineLayout pipelineLayout) {
    // Find or Create a compatible pipeline
    GrVkCopyPipeline* pipeline = nullptr;
    for (int i = 0; i < fCopyPipelines.count() && !pipeline; ++i) {
        if (fCopyPipelines[i]->isCompatible(*dst->simpleRenderPass())) {
            pipeline = fCopyPipelines[i];
        }
    }
    if (!pipeline) {
        pipeline = GrVkCopyPipeline::Create(fGpu, shaderStageInfo,
                                            pipelineLayout,
                                            dst->numColorSamples(),
                                            *dst->simpleRenderPass(),
                                            fPipelineCache);
        fCopyPipelines.push_back(pipeline);
    }
    SkASSERT(pipeline);
    pipeline->ref();
    return pipeline;
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
                                                     const GrVkRenderPass::LoadStoreOps& stencilOps,
                                                     CompatibleRPHandle* compatibleHandle) {
    GrVkResourceProvider::CompatibleRPHandle tempRPHandle;
    GrVkResourceProvider::CompatibleRPHandle* pRPHandle = compatibleHandle ? compatibleHandle
                                                                           : &tempRPHandle;
    *pRPHandle = target.compatibleRenderPassHandle();

    // This will get us the handle to (and possible create) the compatible set for the specific
    // GrVkRenderPass we are looking for.
    this->findCompatibleRenderPass(target, compatibleHandle);
    return this->findRenderPass(*pRPHandle, colorOps, stencilOps);
}

const GrVkRenderPass*
GrVkResourceProvider::findRenderPass(const CompatibleRPHandle& compatibleHandle,
                                     const GrVkRenderPass::LoadStoreOps& colorOps,
                                     const GrVkRenderPass::LoadStoreOps& stencilOps) {
    SkASSERT(compatibleHandle.isValid() && compatibleHandle.toIndex() < fRenderPassArray.count());
    CompatibleRenderPassSet& compatibleSet = fRenderPassArray[compatibleHandle.toIndex()];
    const GrVkRenderPass* renderPass = compatibleSet.getRenderPass(fGpu,
                                                                   colorOps,
                                                                   stencilOps);
    renderPass->ref();
    return renderPass;
}

GrVkDescriptorPool* GrVkResourceProvider::findOrCreateCompatibleDescriptorPool(
                                                            VkDescriptorType type, uint32_t count) {
    return new GrVkDescriptorPool(fGpu, type, count);
}

GrVkSampler* GrVkResourceProvider::findOrCreateCompatibleSampler(const GrSamplerParams& params,
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

void GrVkResourceProvider::getSamplerDescriptorSetHandle(const GrVkUniformHandler& uniformHandler,
                                                         GrVkDescriptorSetManager::Handle* handle) {
    SkASSERT(handle);
    for (int i = 0; i < fDescriptorSetManagers.count(); ++i) {
        if (fDescriptorSetManagers[i].isCompatible(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                   &uniformHandler)) {
           *handle = GrVkDescriptorSetManager::Handle(i);
           return;
        }
    }

    fDescriptorSetManagers.emplace_back(fGpu, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        &uniformHandler);
    *handle = GrVkDescriptorSetManager::Handle(fDescriptorSetManagers.count() - 1);
}

void GrVkResourceProvider::getSamplerDescriptorSetHandle(const SkTArray<uint32_t>& visibilities,
                                                         GrVkDescriptorSetManager::Handle* handle) {
    SkASSERT(handle);
    for (int i = 0; i < fDescriptorSetManagers.count(); ++i) {
        if (fDescriptorSetManagers[i].isCompatible(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                   visibilities)) {
            *handle = GrVkDescriptorSetManager::Handle(i);
            return;
        }
    }

    fDescriptorSetManagers.emplace_back(fGpu, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        visibilities);
    *handle = GrVkDescriptorSetManager::Handle(fDescriptorSetManagers.count() - 1);
}

VkDescriptorSetLayout GrVkResourceProvider::getUniformDSLayout() const {
    SkASSERT(fUniformDSHandle.isValid());
    return fDescriptorSetManagers[fUniformDSHandle.toIndex()].layout();
}

VkDescriptorSetLayout GrVkResourceProvider::getSamplerDSLayout(
        const GrVkDescriptorSetManager::Handle& handle) const {
    SkASSERT(handle.isValid());
    return fDescriptorSetManagers[handle.toIndex()].layout();
}

const GrVkDescriptorSet* GrVkResourceProvider::getUniformDescriptorSet() {
    SkASSERT(fUniformDSHandle.isValid());
    return fDescriptorSetManagers[fUniformDSHandle.toIndex()].getDescriptorSet(fGpu,
                                                                               fUniformDSHandle);
}

const GrVkDescriptorSet* GrVkResourceProvider::getSamplerDescriptorSet(
        const GrVkDescriptorSetManager::Handle& handle) {
    SkASSERT(handle.isValid());
    return fDescriptorSetManagers[handle.toIndex()].getDescriptorSet(fGpu, handle);
}

void GrVkResourceProvider::recycleDescriptorSet(const GrVkDescriptorSet* descSet,
                                                const GrVkDescriptorSetManager::Handle& handle) {
    SkASSERT(descSet);
    SkASSERT(handle.isValid());
    int managerIdx = handle.toIndex();
    SkASSERT(managerIdx < fDescriptorSetManagers.count());
    fDescriptorSetManagers[managerIdx].recycleDescriptorSet(descSet);
}

GrVkPrimaryCommandBuffer* GrVkResourceProvider::findOrCreatePrimaryCommandBuffer() {
    GrVkPrimaryCommandBuffer* cmdBuffer = nullptr;
    int count = fAvailableCommandBuffers.count();
    if (count > 0) {
        cmdBuffer = fAvailableCommandBuffers[count - 1];
        SkASSERT(cmdBuffer->finished(fGpu));
        fAvailableCommandBuffers.removeShuffle(count - 1);
    } else {
        cmdBuffer = GrVkPrimaryCommandBuffer::Create(fGpu, fGpu->cmdPool());
    }
    fActiveCommandBuffers.push_back(cmdBuffer);
    cmdBuffer->ref();
    return cmdBuffer;
}

void GrVkResourceProvider::checkCommandBuffers() {
    for (int i = fActiveCommandBuffers.count()-1; i >= 0; --i) {
        if (fActiveCommandBuffers[i]->finished(fGpu)) {
            GrVkPrimaryCommandBuffer* cmdBuffer = fActiveCommandBuffers[i];
            cmdBuffer->reset(fGpu);
            fAvailableCommandBuffers.push_back(cmdBuffer);
            fActiveCommandBuffers.removeShuffle(i);
        }
    }
}

GrVkSecondaryCommandBuffer* GrVkResourceProvider::findOrCreateSecondaryCommandBuffer() {
    GrVkSecondaryCommandBuffer* cmdBuffer = nullptr;
    int count = fAvailableSecondaryCommandBuffers.count();
    if (count > 0) {
        cmdBuffer = fAvailableSecondaryCommandBuffers[count-1];
        fAvailableSecondaryCommandBuffers.removeShuffle(count - 1);
    } else {
        cmdBuffer = GrVkSecondaryCommandBuffer::Create(fGpu, fGpu->cmdPool());
    }
    return cmdBuffer;
}

void GrVkResourceProvider::recycleSecondaryCommandBuffer(GrVkSecondaryCommandBuffer* cb) {
    cb->reset(fGpu);
    fAvailableSecondaryCommandBuffers.push_back(cb);
}

const GrVkResource* GrVkResourceProvider::findOrCreateStandardUniformBufferResource() {
    const GrVkResource* resource = nullptr;
    int count = fAvailableUniformBufferResources.count();
    if (count > 0) {
        resource = fAvailableUniformBufferResources[count - 1];
        fAvailableUniformBufferResources.removeShuffle(count - 1);
    } else {
        resource = GrVkUniformBuffer::CreateResource(fGpu, GrVkUniformBuffer::kStandardSize);
    }
    return resource;
}

void GrVkResourceProvider::recycleStandardUniformBufferResource(const GrVkResource* resource) {
    fAvailableUniformBufferResources.push_back(resource);
}

void GrVkResourceProvider::destroyResources(bool deviceLost) {
    // release our active command buffers
    for (int i = 0; i < fActiveCommandBuffers.count(); ++i) {
        SkASSERT(deviceLost || fActiveCommandBuffers[i]->finished(fGpu));
        SkASSERT(fActiveCommandBuffers[i]->unique());
        fActiveCommandBuffers[i]->reset(fGpu);
        fActiveCommandBuffers[i]->unref(fGpu);
    }
    fActiveCommandBuffers.reset();
    // release our available command buffers
    for (int i = 0; i < fAvailableCommandBuffers.count(); ++i) {
        SkASSERT(deviceLost || fAvailableCommandBuffers[i]->finished(fGpu));
        SkASSERT(fAvailableCommandBuffers[i]->unique());
        fAvailableCommandBuffers[i]->unref(fGpu);
    }
    fAvailableCommandBuffers.reset();

    // release our available secondary command buffers
    for (int i = 0; i < fAvailableSecondaryCommandBuffers.count(); ++i) {
        SkASSERT(fAvailableSecondaryCommandBuffers[i]->unique());
        fAvailableSecondaryCommandBuffers[i]->unref(fGpu);
    }
    fAvailableSecondaryCommandBuffers.reset();

    // Release all copy pipelines
    for (int i = 0; i < fCopyPipelines.count(); ++i) {
        fCopyPipelines[i]->unref(fGpu);
    }

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

    GR_VK_CALL(fGpu->vkInterface(), DestroyPipelineCache(fGpu->device(), fPipelineCache, nullptr));
    fPipelineCache = VK_NULL_HANDLE;

    // We must release/destroy all command buffers and pipeline states before releasing the
    // GrVkDescriptorSetManagers
    for (int i = 0; i < fDescriptorSetManagers.count(); ++i) {
        fDescriptorSetManagers[i].release(fGpu);
    }
    fDescriptorSetManagers.reset();

    // release our uniform buffers
    for (int i = 0; i < fAvailableUniformBufferResources.count(); ++i) {
        SkASSERT(fAvailableUniformBufferResources[i]->unique());
        fAvailableUniformBufferResources[i]->unref(fGpu);
    }
    fAvailableUniformBufferResources.reset();
}

void GrVkResourceProvider::abandonResources() {
    // release our active command buffers
    for (int i = 0; i < fActiveCommandBuffers.count(); ++i) {
        SkASSERT(fActiveCommandBuffers[i]->finished(fGpu));
        SkASSERT(fActiveCommandBuffers[i]->unique());
        fActiveCommandBuffers[i]->unrefAndAbandon();
    }
    fActiveCommandBuffers.reset();
    // release our available command buffers
    for (int i = 0; i < fAvailableCommandBuffers.count(); ++i) {
        SkASSERT(fAvailableCommandBuffers[i]->finished(fGpu));
        SkASSERT(fAvailableCommandBuffers[i]->unique());
        fAvailableCommandBuffers[i]->unrefAndAbandon();
    }
    fAvailableCommandBuffers.reset();

    // release our available secondary command buffers
    for (int i = 0; i < fAvailableSecondaryCommandBuffers.count(); ++i) {
        SkASSERT(fAvailableSecondaryCommandBuffers[i]->unique());
        fAvailableSecondaryCommandBuffers[i]->unrefAndAbandon();
    }
    fAvailableSecondaryCommandBuffers.reset();

    // Abandon all copy pipelines
    for (int i = 0; i < fCopyPipelines.count(); ++i) {
        fCopyPipelines[i]->unrefAndAbandon();
    }

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

    fPipelineCache = VK_NULL_HANDLE;

    // We must abandon all command buffers and pipeline states before abandoning the
    // GrVkDescriptorSetManagers
    for (int i = 0; i < fDescriptorSetManagers.count(); ++i) {
        fDescriptorSetManagers[i].abandon();
    }
    fDescriptorSetManagers.reset();

    // release our uniform buffers
    for (int i = 0; i < fAvailableUniformBufferResources.count(); ++i) {
        SkASSERT(fAvailableUniformBufferResources[i]->unique());
        fAvailableUniformBufferResources[i]->unrefAndAbandon();
    }
    fAvailableUniformBufferResources.reset();
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
                                                   const GrVkRenderPass::LoadStoreOps& stencilOps) {
    for (int i = 0; i < fRenderPasses.count(); ++i) {
        int idx = (i + fLastReturnedIndex) % fRenderPasses.count();
        if (fRenderPasses[idx]->equalLoadStoreOps(colorOps, stencilOps)) {
            fLastReturnedIndex = idx;
            return fRenderPasses[idx];
        }
    }
    GrVkRenderPass* renderPass = fRenderPasses.emplace_back(new GrVkRenderPass());
    renderPass->init(gpu, *this->getCompatibleRenderPass(), colorOps, stencilOps);
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
