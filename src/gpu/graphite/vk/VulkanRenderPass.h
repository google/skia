/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanRenderPass_DEFINED
#define skgpu_graphite_VulkanRenderPass_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/RenderPassDesc.h"

namespace skgpu::graphite {

class VulkanCommandBuffer;
class VulkanSharedContext;

/**
 * Wrapper around VkRenderPass.
*/
class VulkanRenderPass : public Resource {
public:
    VkRenderPass renderPass() const {
        SkASSERT(fRenderPass != VK_NULL_HANDLE);
        return fRenderPass;
    }

    VkExtent2D granularity() { return fGranularity; }

    const char* getResourceType() const override { return "Vulkan RenderPass"; }

    // The relevant information in RenderPassDesc to identify a VkRenderPass fits in one uint32. If
    // compatibleForPipelineKey, the load/store ops do not contribute to the key (with the exception
    // of load-from-resolve info if necessary).
    static uint32_t GetRenderPassKey(const RenderPassDesc& renderPassDesc,
                                     bool compatibleForPipelineKey);
    // A way back from the render pass key to RenderPassDesc. The write swizzle and dst read
    // strategy is not part of the key as calculated by GetRenderPassKey and so must be passed in to
    // reconstruct RenderPassDesc fully.
    static void ExtractRenderPassDesc(uint32_t key,
                                      Swizzle writeSwizzle,
                                      DstReadStrategy dstReadStrategy,
                                      RenderPassDesc* renderPassDesc);

    static sk_sp<VulkanRenderPass> Make(const VulkanSharedContext*, const RenderPassDesc&);

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*, VkRenderPass, VkExtent2D granularity);

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
    VkExtent2D fGranularity;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
