/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanDescriptorSet> VulkanDescriptorSet::Make(const VulkanSharedContext* ctxt,
                                                     const sk_sp<VulkanDescriptorPool>& pool) {
    VkDescriptorSet descSet;
    VkDescriptorSetAllocateInfo dsAllocateInfo = {};
    dsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocateInfo.descriptorPool = pool->descPool();
    dsAllocateInfo.descriptorSetCount = 1;
    dsAllocateInfo.pSetLayouts = pool->descSetLayout();

    VkResult result;
    VULKAN_CALL_RESULT(
            ctxt, result, AllocateDescriptorSets(ctxt->device(), &dsAllocateInfo, &descSet));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanDescriptorSet>(new VulkanDescriptorSet(ctxt, descSet, pool));
}

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanSharedContext* ctxt,
                                         VkDescriptorSet set,
                                         sk_sp<VulkanDescriptorPool> pool)
        : Resource(ctxt,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0)
        , fDescSet(set)
        , fPool(pool) {
    fPool->ref();
}

void VulkanDescriptorSet::freeGpuData() {
    fPool->unref();
}

} // namespace skgpu::graphite
