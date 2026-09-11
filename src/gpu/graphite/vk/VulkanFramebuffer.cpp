/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanFramebuffer.h"

#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

namespace skgpu::graphite {

sk_sp<VulkanFramebuffer> VulkanFramebuffer::Make(const VulkanSharedContext* context,
                                                 const VkFramebufferCreateInfo& framebufferInfo,
                                                 const RenderPassDesc& renderPassDesc,
                                                 sk_sp<VulkanTexture> msaaTexture,
                                                 sk_sp<VulkanTexture> depthStencilTexture) {
    // All attachments used in a VkFramebuffer must be at least the size of the VkFramebuffer.
    SkASSERT(!msaaTexture ||
             (msaaTexture->dimensions().width() >= SkTo<int>(framebufferInfo.width) &&
              msaaTexture->dimensions().height() >= SkTo<int>(framebufferInfo.height)));
    SkASSERT(!depthStencilTexture ||
             (depthStencilTexture->dimensions().width() >= SkTo<int>(framebufferInfo.width) &&
              depthStencilTexture->dimensions().height() >= SkTo<int>(framebufferInfo.height)));

    VkFramebuffer framebuffer;
    VkResult result;
    VULKAN_CALL_RESULT(
            context,
            result,
            CreateFramebuffer(context->device(), &framebufferInfo, nullptr, &framebuffer));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    bool loadMSAAFromResolve = RenderPassDescWillLoadMSAAFromResolve(renderPassDesc);

    return sk_sp<VulkanFramebuffer>(new VulkanFramebuffer(context,
                                                          framebuffer,
                                                          {SkTo<int>(framebufferInfo.width),
                                                           SkTo<int>(framebufferInfo.height)},
                                                          std::move(msaaTexture),
                                                          std::move(depthStencilTexture),
                                                          loadMSAAFromResolve));
}

VulkanFramebuffer::VulkanFramebuffer(const VulkanSharedContext* context,
                                     VkFramebuffer framebuffer,
                                     SkISize dimensions,
                                     sk_sp<VulkanTexture> msaaTexture,
                                     sk_sp<VulkanTexture> depthStencilTexture,
                                     bool loadMSAAFromResolve)
        : Resource(context,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0)
        , fFramebuffer(framebuffer)
        , fDimensions(dimensions)
        , fMsaaTexture(std::move(msaaTexture))
        , fDepthStencilTexture(std::move(depthStencilTexture))
        , fLoadMSAAFromResolve(loadMSAAFromResolve) {}

bool VulkanFramebuffer::compatible(const RenderPassDesc& renderPassDesc,
                                   const VulkanTexture* msaaTexture,
                                   const VulkanTexture* depthStencilTexture) {
    auto compatibleTextures = [](const VulkanTexture* tex1, const VulkanTexture* tex2) {
        if (tex1 && tex2) {
            return tex1->uniqueID() == tex2->uniqueID();
        } else if (!tex1 && !tex2){
            return true;
        }
        return false;
    };

    bool loadMSAAFromResolve = RenderPassDescWillLoadMSAAFromResolve(renderPassDesc);

    return fLoadMSAAFromResolve == loadMSAAFromResolve &&
           compatibleTextures(msaaTexture, fMsaaTexture.get()) &&
           compatibleTextures(depthStencilTexture, fDepthStencilTexture.get());
}

void VulkanFramebuffer::freeGpuData() {
    auto context = static_cast<const VulkanSharedContext*>(this->sharedContext());
    VULKAN_CALL(context->interface(), DestroyFramebuffer(context->device(), fFramebuffer, nullptr));
}

} // namespace skgpu::graphite
