/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkDescriptorSetManager.h"

#include "GrVkDescriptorPool.h"
#include "GrVkDescriptorSet.h"
#include "GrVkGpu.h"

GrVkDescriptorSetManager::GrVkDescriptorSetManager(GrVkGpu* gpu,
                                                   VkDescriptorSetLayout layout,
                                                   VkDescriptorType type,
                                                   uint32_t samplerCount)
    : fPoolManager(layout, type, samplerCount, gpu)
    , fNumSamplerBindings(samplerCount) {
}

const GrVkDescriptorSet* GrVkDescriptorSetManager::getDescriptorSet(GrVkGpu* gpu,
                                                                    const Handle& handle) {
    const GrVkDescriptorSet* ds = nullptr;
    int count = fFreeSets.count();
    if (count > 0) {
        ds = fFreeSets[count - 1];
        fFreeSets.removeShuffle(count - 1);
    } else {
        VkDescriptorSet vkDS;
        fPoolManager.getNewDescriptorSet(gpu, &vkDS);

        ds = new GrVkDescriptorSet(vkDS, fPoolManager.fPool, handle);
    }
    SkASSERT(ds);
    return ds;
}

void GrVkDescriptorSetManager::recycleDescriptorSet(const GrVkDescriptorSet* descSet) {
    SkASSERT(descSet);
    fFreeSets.push_back(descSet);
}

void GrVkDescriptorSetManager::release(const GrVkGpu* gpu) {
    fPoolManager.freeGPUResources(gpu);

    for (int i = 0; i < fFreeSets.count(); ++i) {
        fFreeSets[i]->unref(gpu);
    }
    fFreeSets.reset();
}

void GrVkDescriptorSetManager::abandon() {
    fPoolManager.abandonGPUResources();

    for (int i = 0; i < fFreeSets.count(); ++i) {
        fFreeSets[i]->unrefAndAbandon();
    }
    fFreeSets.reset();
}

////////////////////////////////////////////////////////////////////////////////

void GrVkDescriptorSetManager::DescriptorPoolManager::getNewPool(GrVkGpu* gpu) {
    if (fPool) {
        fPool->unref(gpu);
        uint32_t newPoolSize = fMaxDescriptors + ((fMaxDescriptors + 1) >> 1);
        if (newPoolSize < kMaxDescriptors) {
            fMaxDescriptors = newPoolSize;
        } else {
            fMaxDescriptors = kMaxDescriptors;
        }

    }
    fPool = gpu->resourceProvider().findOrCreateCompatibleDescriptorPool(fDescType,
                                                                         fMaxDescriptors);
    SkASSERT(fPool);
}

void GrVkDescriptorSetManager::DescriptorPoolManager::getNewDescriptorSet(GrVkGpu* gpu,
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

void GrVkDescriptorSetManager::DescriptorPoolManager::freeGPUResources(const GrVkGpu* gpu) {
    // The layout should be owned by the class which owns the DescriptorSetManager so it will
    // take care of destroying it.
    fDescLayout = VK_NULL_HANDLE;

    if (fPool) {
        fPool->unref(gpu);
        fPool = nullptr;
    }
}

void GrVkDescriptorSetManager::DescriptorPoolManager::abandonGPUResources() {
    fDescLayout = VK_NULL_HANDLE;
    if (fPool) {
        fPool->unrefAndAbandon();
        fPool = nullptr;
    }
}
