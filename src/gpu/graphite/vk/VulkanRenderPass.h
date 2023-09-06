/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanRenderPass_DEFINED
#define skgpu_graphite_VulkanRenderPass_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

namespace skgpu::graphite {

class VulkanSharedContext;

/**
 * Wrapper around VkRenderPass.
*/
class VulkanRenderPass : public Resource {
public:
    // Methods to create compatible (needed when creating a framebuffer and graphics pipeline) or
    // full (needed when beginning a render pass from the command buffer) render passes and keys.
    static GraphiteResourceKey MakeRenderPassKey(const RenderPassDesc&, bool compatibleOnly);
    static sk_sp<VulkanRenderPass> MakeRenderPass(
            const VulkanSharedContext*, const RenderPassDesc&, bool compatibleOnly);

    VkRenderPass* renderPass() {
        SkASSERT(fRenderPass != VK_NULL_HANDLE);
        return &fRenderPass;
    }

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*, VkRenderPass);

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
