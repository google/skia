/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanDescriptorSet> VulkanDescriptorSet::Make(const VulkanSharedContext* ctxt,
                                                     const sk_sp<VulkanDescriptorPool>& pool,
                                                     const VkDescriptorSetLayout layout) {
    SkASSERT(layout != VK_NULL_HANDLE && pool);
    VkDescriptorSet descSet;
    VkDescriptorSetAllocateInfo dsAllocateInfo;
    memset(&dsAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    dsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocateInfo.pNext = nullptr;
    dsAllocateInfo.descriptorPool = pool->descPool();
    dsAllocateInfo.descriptorSetCount = 1;
    dsAllocateInfo.pSetLayouts = &layout;

    VkResult result;
    VULKAN_CALL_RESULT(ctxt->interface(),
                       result,
                       AllocateDescriptorSets(ctxt->device(),
                                              &dsAllocateInfo,
                                              &descSet));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanDescriptorSet>(new VulkanDescriptorSet(ctxt, descSet, pool));
}

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanSharedContext* ctxt,
                                         VkDescriptorSet set,
                                         sk_sp<VulkanDescriptorPool> pool)
        : Resource(ctxt, Ownership::kOwned, skgpu::Budgeted::kNo, /*gpuMemorySize=*/0)
        , fDescSet (set)
        , fPool (pool) {
    fPool->ref();
}

void VulkanDescriptorSet::freeGpuData() {
    fPool->unref();
}

} // namespace skgpu::graphite
