/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanFramebuffer.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

sk_sp<VulkanFramebuffer> VulkanFramebuffer::Make(const VulkanSharedContext* context,
                                                 const VkFramebufferCreateInfo& framebufferInfo) {
    VkFramebuffer framebuffer;
    VkResult result;
    VULKAN_CALL_RESULT(
            context,
            result,
            CreateFramebuffer(context->device(), &framebufferInfo, nullptr, &framebuffer));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    return sk_sp<VulkanFramebuffer>(new VulkanFramebuffer(context, framebuffer));
}

VulkanFramebuffer::VulkanFramebuffer(const VulkanSharedContext* context, VkFramebuffer framebuffer)
        : Resource(context,
                   Ownership::kOwned,
                   skgpu::Budgeted::kYes,
                   /*gpuMemorySize=*/0)
        , fSharedContext(context)
        , fFramebuffer(framebuffer) {}

void VulkanFramebuffer::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyFramebuffer(fSharedContext->device(), fFramebuffer, nullptr));
}

} // namespace skgpu::graphite
