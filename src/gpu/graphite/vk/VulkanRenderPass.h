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
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

namespace skgpu::graphite {

struct AttachmentDesc;
struct RenderPassDesc;
class VulkanCommandBuffer;
class VulkanSharedContext;

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
    struct Metadata {
        Metadata(const RenderPassDesc&);

        bool fLoadMSAAFromResolve;

        // TODO: Extend RenderPassDesc to have subpasses that index into the attachments of the
        // RenderPassDesc. For a given subpass, we can simplify the description to assume there's
        // at most 1 color, resolve, and depth-stencil attachment. For now these bools represent
        // the main subpass's attachment "refs".
        bool fHasColorAttachment;
        bool fHasColorResolveAttachment;
        bool fHasDepthStencilAttachment;
        bool fHasInputAttachment;

        // Accumulate attachments into a container to mimic future structure in RenderPassDesc
        // Currently there can be up to three: color, resolve, depth+stencil
        skia_private::STArray<3, const AttachmentDesc*> fAttachments;

        int keySize() const;

        // TODO: Extend RenderPassDesc to describe generalized subpasses and subpass dependencies,
        // at which point these can be inferred from there. It's undecided if the load-from-resolve
        // subpass will be an extra +1, or if RenderPassDesc will explicitly include it.
        int subpassCount() const { return fLoadMSAAFromResolve ? 2 : 1; }
        int subpassDependencyCount() const { return fLoadMSAAFromResolve ? 1 : 0; }

        void addToKey(ResourceKey::Builder&, int& builderIdx, bool compatibleOnly);
    };

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*, VkRenderPass, VkExtent2D granularity);

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
    VkExtent2D fGranularity;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
