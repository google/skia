/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanDescriptorPool> VulkanDescriptorPool::Make(const VulkanSharedContext* context,
                                                       SkSpan<DescriptorData> requestedDescCounts,
                                                       VkDescriptorSetLayout layout) {

    if (requestedDescCounts.empty()) {
        return nullptr;
    }

    // For each requested descriptor type and count, create a VkDescriptorPoolSize struct which
    // specifies the descriptor type and quantity for pool creation. Multiple pool size structures
    // may contain the same descriptor type - the pool will be created with enough storage for the
    // total number of descriptors of each type. Source:
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#VkDescriptorPoolSize
    // Note: The kMaxNumDescriptors limit could be evaded since we do not currently track and check
    // the cumulative quantities of each type of descriptor, but this is an internal call and it is
    // highly unexpected for us to exceed this limit in practice.
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorPoolSize> poolSizes;
    for (size_t i = 0; i < requestedDescCounts.size(); i++) {
        SkASSERT(requestedDescCounts[i].fCount > 0);
        if (requestedDescCounts[i].fCount > kMaxNumDescriptors) {
            SkDebugf("The number of descriptors requested, %u, exceeds the maximum allowed (%d).\n",
                     requestedDescCounts[i].fCount,
                     kMaxNumDescriptors);
            return nullptr;
        }
        VkDescriptorPoolSize& poolSize = poolSizes.push_back();
        poolSize = {};
        // Map each DescriptorSetType to the appropriate backend VkDescriptorType
        poolSize.type = DsTypeEnumToVkDs(requestedDescCounts[i].fType);
        // Create a pool large enough to accommodate the maximum possible number of descriptor sets
        poolSize.descriptorCount = requestedDescCounts[i].fCount * kMaxNumSets;
    }

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.maxSets = kMaxNumSets;
    createInfo.poolSizeCount = requestedDescCounts.size();
    createInfo.pPoolSizes = &poolSizes.front();

    VkDescriptorPool pool;
    VkResult result;
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateDescriptorPool(context->device(),
                                            &createInfo,
                                            /*const VkAllocationCallbacks*=*/nullptr,
                                            &pool));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(context->interface(),
                    DestroyDescriptorSetLayout(context->device(), layout, nullptr));
        return nullptr;
    }
    return sk_sp<VulkanDescriptorPool>(new VulkanDescriptorPool(context, pool, layout));
}

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanSharedContext* context,
                                           VkDescriptorPool pool,
                                           VkDescriptorSetLayout layout)
        : fSharedContext(context)
        , fDescPool(pool)
        , fDescSetLayout(layout) {}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    VULKAN_CALL(fSharedContext->interface(),
                DestroyDescriptorPool(fSharedContext->device(), fDescPool, nullptr));
    if (fDescSetLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyDescriptorSetLayout(fSharedContext->device(), fDescSetLayout, nullptr));
        fDescSetLayout = VK_NULL_HANDLE;
    }
}

} // namespace skgpu::graphite
