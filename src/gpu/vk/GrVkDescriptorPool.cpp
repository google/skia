/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkDescriptorPool.h"

#include "GrVkGpu.h"
#include "SkTemplates.h"


GrVkDescriptorPool::GrVkDescriptorPool(const GrVkGpu* gpu, const DescriptorTypeCounts& typeCounts)
    : INHERITED()
    , fTypeCounts(typeCounts) {
    int numPools = fTypeCounts.numPoolSizes();
    SkAutoTDeleteArray<VkDescriptorPoolSize> poolSizes(new VkDescriptorPoolSize[numPools]);
    int currentPool = 0;
    for (int i = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; i < VK_DESCRIPTOR_TYPE_END_RANGE; ++i) {
        if (fTypeCounts.fDescriptorTypeCount[i]) {
            VkDescriptorPoolSize& poolSize = poolSizes.get()[currentPool++];
            poolSize.type = (VkDescriptorType)i;
            poolSize.descriptorCount = fTypeCounts.fDescriptorTypeCount[i];
        }
    }
    SkASSERT(currentPool == numPools);

    VkDescriptorPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.maxSets = 2;  // Currently we allow one set for samplers and one set for uniforms
    createInfo.poolSizeCount = numPools;
    createInfo.pPoolSizes = poolSizes.get();

    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateDescriptorPool(gpu->device(),
                                                                 &createInfo,
                                                                 nullptr,
                                                                 &fDescPool));
}

bool GrVkDescriptorPool::isCompatible(const DescriptorTypeCounts& typeCounts) const {
    return fTypeCounts.isSuperSet(typeCounts);
}

void GrVkDescriptorPool::reset(const GrVkGpu* gpu) {
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), ResetDescriptorPool(gpu->device(), fDescPool, 0));
}

void GrVkDescriptorPool::freeGPUData(const GrVkGpu* gpu) const {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    GR_VK_CALL(gpu->vkInterface(), DestroyDescriptorPool(gpu->device(), fDescPool, nullptr));
}

///////////////////////////////////////////////////////////////////////////////

int GrVkDescriptorPool::DescriptorTypeCounts::numPoolSizes() const {
    int count = 0;
    for (int i = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; i < VK_DESCRIPTOR_TYPE_END_RANGE; ++i) {
        count += fDescriptorTypeCount[i] ? 1 : 0;
    }
    return count;
}

bool GrVkDescriptorPool::DescriptorTypeCounts::isSuperSet(const DescriptorTypeCounts& that) const {
    for (int i = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; i < VK_DESCRIPTOR_TYPE_END_RANGE; ++i) {
        if (that.fDescriptorTypeCount[i] > fDescriptorTypeCount[i]) {
            return false;
        }
    }
    return true;
}

void GrVkDescriptorPool::DescriptorTypeCounts::setTypeCount(VkDescriptorType type, uint8_t count) {
    fDescriptorTypeCount[type] = count;
}
