/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"

#include "include/private/SkTArray.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanDescriptorPool> VulkanDescriptorPool::Make(const VulkanSharedContext* context,
                                                       SkSpan<DescriptorData> requestedDescCounts,
                                                       VkDescriptorSetLayout layout,
                                                       uint32_t numSets) {
    if (requestedDescCounts.empty() || numSets == 0) {
        // Note: On failure, we do not destroy `layout` here or below because the caller
        // (VulkanResourceProvider) retains ownership and destroys `layout` if pool creation fails.
        // VulkanDescriptorPool only takes ownership of `layout` on success.
        return nullptr;
    }

    // Multiple bindings within a descriptor set may request the same descriptor type (e.g. multiple
    // storage buffers in compute steps). Consolidate identical descriptor types so each
    // VkDescriptorType appears at most once in poolSizes. This guarantees that poolSizes contains
    // at most kDescriptorTypeCount entries. We also validate cumulative per-set descriptor counts
    // against kMaxNumDescriptors to prevent limits from being evaded across bindings.
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorPoolSize> poolSizes;

    for (const DescriptorData& desc : requestedDescCounts) {
        SkASSERT(desc.fCount > 0 && desc.fCount <= kMaxNumDescriptors);
        VkDescriptorType descType = DsTypeEnumToVkDs(desc.fType);
        VkDescriptorPoolSize* entry = nullptr;
        for (auto& existing : poolSizes) {
            if (existing.type == descType) {
                entry = &existing;
                break;
            }
        }
        if (!entry) {
            entry = &poolSizes.push_back();
            entry->type = descType;
            entry->descriptorCount = 0;
        }
        entry->descriptorCount += desc.fCount;
    }

    for (auto& poolSize : poolSizes) {
        if (poolSize.descriptorCount > kMaxNumDescriptors) {
            SkDebugf("The cumulative number of descriptors requested (%u) for type %d "
                     "exceeds the maximum allowed per set (%d).\n",
                     poolSize.descriptorCount,
                     poolSize.type,
                     kMaxNumDescriptors);
            return nullptr;
        }
        poolSize.descriptorCount *= numSets;
    }

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.maxSets = numSets;
    createInfo.poolSizeCount = SkTo<uint32_t>(poolSizes.size());
    createInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool pool;
    VkResult result;
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateDescriptorPool(context->device(),
                                            &createInfo,
                                            /*const VkAllocationCallbacks*=*/nullptr,
                                            &pool));
    if (result != VK_SUCCESS) {
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
