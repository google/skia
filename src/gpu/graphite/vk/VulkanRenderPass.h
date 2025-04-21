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

    // Struct to store Vulkan information surrounding a RenderPassDesc
    struct Metadata {
        Metadata(const RenderPassDesc& renderPassDesc, bool compatibleOnly);

        bool operator==(const Metadata&) const;
        bool operator!=(const Metadata& other) const { return !(*this == other); }

        bool fLoadMSAAFromResolve;

        // TODO: Extend RenderPassDesc to have subpasses that index into the attachments of the
        // RenderPassDesc. For a given subpass, we can simplify the description to assume there's
        // at most 1 color, resolve, and depth-stencil attachment. For now there's only one main
        // subpass that can be configured (MSAA load is special) and it's assumed that if there's
        // an input attachment, it references the color attachment. These index into `fAttachments`,
        // or are -1 if there is no attachment. Additionally, once subpasses may only reference
        // a subset of the net attachments, it may be necessary to distinguish between no attachment
        // at all, and VK_ATTACHMENT_UNUSED.
        int8_t fColorAttachIndex;
        int8_t fColorResolveIndex;
        int8_t fDepthStencilIndex;
        // To minimize pipeline compiles, there is always a self-dependency input attachment for the
        // color attachment, but for a given render pass it may never actually be used.
        bool fUsesInputAttachment;

        // Accumulate attachments into a container to mimic future structure in RenderPassDesc
        // Currently there can be up to three: color, resolve, depth+stencil
        skia_private::STArray<3, AttachmentDesc> fAttachments;

        int keySize() const;

        // TODO: Extend RenderPassDesc to describe generalized subpasses and subpass dependencies,
        // at which point these can be inferred from there. It's undecided if the load-from-resolve
        // subpass will be an extra +1, or if RenderPassDesc will explicitly include it.
        int subpassCount() const { return fLoadMSAAFromResolve ? 2 : 1; }
        int subpassDependencyCount() const { return fLoadMSAAFromResolve ? 1 : 0; }

        void addToKey(ResourceKey::Builder&, int& builderIdx);
    };

    static sk_sp<VulkanRenderPass> Make(const VulkanSharedContext*, const Metadata& rpMetadata);

private:
    void freeGpuData() override;

    VulkanRenderPass(const VulkanSharedContext*, VkRenderPass, VkExtent2D granularity);

    const VulkanSharedContext* fSharedContext;
    VkRenderPass fRenderPass;
    VkExtent2D fGranularity;
};
} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanRenderPass_DEFINED
