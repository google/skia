/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

namespace { // anonymous namespace
static VkDescriptorType ds_type_enum_to_vk_ds(DescriptorType type) {
    switch (type) {
        case DescriptorType::kUniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::kTextureSampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::kTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::kCombinedTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::kStorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::kInputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    }
    SkUNREACHABLE;
}
} // anonymous namespace

sk_sp<VulkanDescriptorPool> VulkanDescriptorPool::Make(
        const VulkanSharedContext* context,
        SkSpan<DescTypeAndCount> requestedDescCounts) {

    if (requestedDescCounts.empty()) {
        return nullptr;
    }

    // For each requested descriptor type and count, create a VkDescriptorPoolSize struct which
    // specifies the descriptor type and quantity for pool creation.
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorPoolSize> poolSizes;
    for (size_t i = 0; i < requestedDescCounts.size(); i++) {
        SkASSERT(requestedDescCounts[i].count > 0);
        if (requestedDescCounts[i].count > kMaxNumDescriptors) {
            SkDebugf("The number of descriptors requested, %d, exceeds the maximum allowed (%d).\n",
                     requestedDescCounts[i].count,
                     kMaxNumDescriptors);
            return nullptr;
        }
        VkDescriptorPoolSize* poolSize = &poolSizes.at(i);
        memset(poolSize, 0, sizeof(VkDescriptorPoolSize));
        // Map each DescriptorSetType to the appropriate backend VkDescriptorType
        poolSize->type = ds_type_enum_to_vk_ds(requestedDescCounts[i].type);
        // Create a pool large enough to accommodate the maximum possible number of descriptor sets
        poolSize->descriptorCount = requestedDescCounts[i].count * kMaxNumSets;
    }

    VkDescriptorPoolCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.maxSets = kMaxNumSets;
    createInfo.poolSizeCount = requestedDescCounts.size();
    createInfo.pPoolSizes = &poolSizes.front();

    VkDescriptorPool pool;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result, CreateDescriptorPool(context->device(),
                                                                          &createInfo,
                                                                          nullptr,
                                                                          &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanDescriptorPool>(new VulkanDescriptorPool(context, pool));
}

VulkanDescriptorPool::VulkanDescriptorPool(const VulkanSharedContext* context,
                                           VkDescriptorPool pool)
        : fSharedContext(context)
        , fDescPool(pool) {}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    // Destroying the VkDescriptorPool will automatically free and delete any VkDescriptorSets
    // allocated from the pool.
    VULKAN_CALL(fSharedContext->interface(),
                DestroyDescriptorPool(fSharedContext->device(), fDescPool, nullptr));
}

} // namespace skgpu::graphite
