/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanUtils.h"

namespace skgpu::graphite {

sk_sp<VulkanCommandBuffer> VulkanCommandBuffer::Make(const VulkanSharedContext* sharedContext,
                                                     VulkanResourceProvider* resourceProvider) {
    // Create VkCommandPool
    VkCommandPoolCreateFlags cmdPoolCreateFlags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (sharedContext->isProtected() == Protected::kYes) {
        cmdPoolCreateFlags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    }

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
        nullptr,                                     // pNext
        cmdPoolCreateFlags,                          // CmdPoolCreateFlags
        sharedContext->queueIndex(),                 // queueFamilyIndex
    };
    auto interface = sharedContext->interface();
    VkResult result;
    VkCommandPool pool;
    VULKAN_CALL_RESULT(interface, result, CreateCommandPool(sharedContext->device(),
                                                            &cmdPoolInfo,
                                                            nullptr,
                                                            &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        pool,                                             // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
        1                                                 // bufferCount
    };

    VkCommandBuffer primaryCmdBuffer;
    VULKAN_CALL_RESULT(interface, result, AllocateCommandBuffers(sharedContext->device(),
                                                                 &cmdInfo,
                                                                 &primaryCmdBuffer));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(interface, DestroyCommandPool(sharedContext->device(), pool, nullptr));
        return nullptr;
    }

    return sk_sp<VulkanCommandBuffer>(new VulkanCommandBuffer(pool,
                                                              primaryCmdBuffer,
                                                              sharedContext,
                                                              resourceProvider));
}

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool pool,
                                         VkCommandBuffer primaryCommandBuffer,
                                         const VulkanSharedContext* sharedContext,
                                         VulkanResourceProvider* resourceProvider)
        : fPool(pool)
        , fPrimaryCommandBuffer(primaryCommandBuffer)
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider) {

    // TODO: Remove these lines. They are only here to hide compiler warnings/errors about unused
    // member variables.
    (void) fPool;
    (void) fPrimaryCommandBuffer;
    (void) fSharedContext;
    (void) fResourceProvider;
}

VulkanCommandBuffer::~VulkanCommandBuffer() {}

bool VulkanCommandBuffer::onAddRenderPass(
        const RenderPassDesc&,
        const Texture* colorTexture,
        const Texture* resolveTexture,
        const Texture* depthStencilTexture,
        const std::vector<std::unique_ptr<DrawPass>>& drawPasses) {
    return false;
}

bool VulkanCommandBuffer::onAddComputePass(const ComputePassDesc&,
                                           const ComputePipeline*,
                                           const std::vector<ResourceBinding>& bindings) {
    return false;
}

bool VulkanCommandBuffer::onCopyTextureToBuffer(const Texture*,
                                                SkIRect srcRect,
                                                const Buffer*,
                                                size_t bufferOffset,
                                                size_t bufferRowBytes) {
    return false;
}

bool VulkanCommandBuffer::onCopyBufferToTexture(const Buffer*,
                                                const Texture*,
                                                const BufferTextureCopyData* copyData,
                                                int count) {
    return false;
}

bool VulkanCommandBuffer::onCopyTextureToTexture(const Texture* src,
                                                 SkIRect srcRect,
                                                 const Texture* dst,
                                                 SkIPoint dstPoint) {
    return false;
}

bool VulkanCommandBuffer::onSynchronizeBufferToCpu(const Buffer*, bool* outDidResultInWork) {
    return false;
}

#ifdef SK_ENABLE_PIET_GPU
void VulkanCommandBuffer::onRenderPietScene(const skgpu::piet::Scene& scene,
                                            const Texture* target) {}
#endif

} // namespace skgpu::graphite

