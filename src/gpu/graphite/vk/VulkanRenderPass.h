/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef skgpu_graphite_VulkanRenderPass_DEFINED
#define skgpu_graphite_VulkanRenderPass_DEFINED

#include "src/gpu/graphite/Resource.h"

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

namespace skgpu::graphite {

class VulkanCommandBuffer;
class VulkanSharedContext;

const static VkAttachmentStoreOp vkStoreOp[] {
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE
};
const static VkAttachmentLoadOp vkLoadOp[] {
    VK_ATTACHMENT_LOAD_OP_LOAD,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_LOAD_OP_DONT_CARE
};

/**
 * Wrapper around VkRenderPass.
*/
class VulkanRenderPass : public Resource {
public:
    // Statically assign attachment indices until such information can be fetched from
    // graphite-level structures (likely RenderPassDesc)
    static constexpr int kColorAttachmentIdx = 0;
    static constexpr int kColorResolveAttachmentIdx = 1;
    static constexpr int kDepthStencilAttachmentIdx = 2;

    static constexpr int kMaxExpectedAttachmentCount = kDepthStencilAttachmentIdx + 1;

    // Methods to create compatible (needed when creating a framebuffer and graphics pipeline) or
    // full (needed when beginning a render pass from the command buffer) render passes and keys.
    static GraphiteResourceKey MakeRenderPassKey(const RenderPassDesc&, bool compatibleOnly);

    static sk_sp<VulkanRenderPass> MakeRenderPass(const VulkanSharedContext*,
                                                  const RenderPassDesc&,
                                                  bool compatibleOnly);

    VkRenderPass renderPass() const {
        SkASSERT(fRenderPass != VK_NULL_HANDLE);
        return fRenderPass;
    }

    VkExtent2D granularity() { return fGranularity; }

    const char* getResourceType() const override { return "Vulkan RenderPass"; }

    // Struct to store Vulkan information surrounding a RenderPassDesc
    struct VulkanRenderPassMetaData {
        VulkanRenderPassMetaData(const RenderPassDesc& renderPassDesc);

        bool fLoadMSAAFromResolve;
        bool fHasColorAttachment;
        bool fHasColorResolveAttachment;
        bool fHasDepthStencilAttachment;

        int fNumColorAttachments;
        int fNumResolveAttachments;
        int fNumDepthStencilAttachments;
        int fSubpassCount;
        int fSubpassDependencyCount;
        int fUint32DataCnt;

        // Accumulate attachments into a container to mimic future structure in RenderPassDesc
        skia_private::TArray<const AttachmentDesc*> fAttachments;
    };

    static void AddRenderPassInfoToKey(VulkanRenderPassMetaData& rpMetaData,
                                       ResourceKey::Builder& builder,
                                       int& builderIdx,
                                       bool compatibleOnly);

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*, VkRenderPass, VkExtent2D granularity);

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
    VkExtent2D fGranularity;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
