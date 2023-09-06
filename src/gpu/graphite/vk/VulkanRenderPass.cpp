/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanRenderPass.h"

#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {
void add_attachment_description_info_to_key(GraphiteResourceKey::Builder& builder,
                                            const TextureInfo& textureInfo,
                                            int& builderIdx,
                                            LoadOp loadOp,
                                            StoreOp storeOp) {
    VulkanTextureInfo vkTexInfo;
    if (textureInfo.isValid() && textureInfo.getVulkanTextureInfo(&vkTexInfo)) {
        builder[builderIdx++] = vkTexInfo.fFormat;
        builder[builderIdx++] = vkTexInfo.fSampleCount;
        SkASSERT(sizeof(loadOp)  < (1u << 8));
        SkASSERT(sizeof(storeOp) < (1u << 8));
        builder[builderIdx++] = static_cast<uint8_t>(loadOp) << 8 | static_cast<uint8_t>(storeOp);
    }
}

GraphiteResourceKey VulkanRenderPass::MakeRenderPassKey(
        const RenderPassDesc& renderPassDesc, bool compatibleOnly) {
    static constexpr uint32_t kKeyValueUnused = UINT32_MAX;
    bool  hasColorAttachment        = renderPassDesc.fColorAttachment.fTextureInfo.isValid();
    bool  hasColorResolveAttachment = renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid();
    bool  hasDepthStencilAttachment = renderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid();
    bool  hasInputAttachment        = false; // TODO: Determine this through RenderPassDesc
    // TODO: Query for more attachments once the RenderPassDesc struct contains that information.
    // For now, we only ever expect to see 0 or 1 of each attachment type (color, resolve, and
    // depth/stencil), so the count of each of those can simply be determined with a bool.
    const int numColorAttachments        = hasColorAttachment        ? 1 : 0;
    const int numResolveAttachments      = hasColorResolveAttachment ? 1 : 0;
    const int numDepthStencilAttachments = hasDepthStencilAttachment ? 1 : 0;
    const int numInputAttachments        = hasInputAttachment        ? 1 : 0;
    const int totalRenderPassAttachments = numColorAttachments + numResolveAttachments +
                                           numDepthStencilAttachments + numInputAttachments;

    // Accumulate attachments into a container to mimic future structure in RenderPassDesc
    skia_private::TArray<const AttachmentDesc*> rpAttachments(totalRenderPassAttachments);
    if (hasColorAttachment) {
        rpAttachments.push_back(&renderPassDesc.fColorAttachment);
    }
    if (hasColorResolveAttachment) {
        rpAttachments.push_back(&renderPassDesc.fColorResolveAttachment);
    }
    if (hasDepthStencilAttachment) {
        rpAttachments.push_back(&renderPassDesc.fDepthStencilAttachment);
    }
    // TODO: Add input attachment from RenderPassDesc if it has one

    // Calculate how many int32s we need to capture all relevant information.
    int num32DataCnt = 0;
    // Each renderpass key will start with the total number of attachments associated with it.
    // This will use one uint32.
    num32DataCnt++;
    // The key will then contain key information for each attachment. This includes format, sample
    // count, and load/store operation information.
    num32DataCnt += 3 * totalRenderPassAttachments;
    // Then, subpass information will be added in the form of attachment reference indices. Reserve
    // one int32 for each possible attachment reference type, of which there are 4.
    // TODO: Reference RenderPassDesc to determine number and makeup of subpasses. For now, we only
    // ever expect 1 (in most cases) or 2 (when loading MSAA from resolve).
    int subpassCount = 1;
    num32DataCnt += 4 * subpassCount;
    // Finally, subpass dependency information will be recorded. Each dependency will be allotted 7
    // int32s to store all its pertinent information. Relevant once we support multiple subpasses.
    int subpassDependencyCount = 0;
    num32DataCnt += 7 * subpassDependencyCount;

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, num32DataCnt, Shareable::kYes);

    // Now we can actually populate the key.
    int builderIdx = 0;
    builder[builderIdx++] = totalRenderPassAttachments;
    // Iterate through each renderpass attachment to add its information
    for (int i = 0; i < totalRenderPassAttachments; i++) {
        add_attachment_description_info_to_key(
                builder,
                rpAttachments[i]->fTextureInfo,
                builderIdx,
                // Assign LoadOp::kLoad and StoreOp::kStore as default load/store operations for
                // compatible render passes where load/store ops don't need to match.
                compatibleOnly ? LoadOp::kLoad   : rpAttachments[i]->fLoadOp,
                compatibleOnly ? StoreOp::kStore : rpAttachments[i]->fStoreOp);
    }
    // Add subpass information to the key
    for (int j = 0; j < subpassCount; j++) {
        // The following assumes that we only have up to one reference of each type per subpass and
        // that attachments are indexed in order of color, resolve, depth/stencil, then input
        // attachments. TODO: Fetch actual attachment reference and index information for each
        // subpass from RenderPassDesc. For now, we only support 1 subpass, so we can infer the
        // subpass simply references the attachments of the renderpass itself.
        int attachmentIdx = 0;
        builder[builderIdx++] = hasColorAttachment        ? attachmentIdx++ : VK_ATTACHMENT_UNUSED;
        builder[builderIdx++] = hasColorResolveAttachment ? attachmentIdx++ : VK_ATTACHMENT_UNUSED;
        builder[builderIdx++] = hasDepthStencilAttachment ? attachmentIdx++ : VK_ATTACHMENT_UNUSED;
        builder[builderIdx++] = hasInputAttachment        ? attachmentIdx++ : VK_ATTACHMENT_UNUSED;
    }
    // TODO: Query RenderPassDesc for subpass dependency information & populate the key accordingly
    for (int i = 0; i < subpassDependencyCount; i++) {
        builder[builderIdx++] = kKeyValueUnused; // srcSubpass index
        builder[builderIdx++] = kKeyValueUnused; // dstSubpass index
        builder[builderIdx++] = kKeyValueUnused; // srcStageMask
        builder[builderIdx++] = kKeyValueUnused; // dstStageMask
        builder[builderIdx++] = kKeyValueUnused; // srcAccessMask
        builder[builderIdx++] = kKeyValueUnused; // dstAccessMask
        builder[builderIdx++] = kKeyValueUnused; // dependencyFlags
    }

    builder.finish();
    return key;
}

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeRenderPass(const VulkanSharedContext* context,
                                                         const RenderPassDesc& renderPassDesc,
                                                         bool compatibleOnly) {
    // TODO: Create VkRenderPass.
    VkRenderPass renderPass = VK_NULL_HANDLE;
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass));
}

VulkanRenderPass::VulkanRenderPass(const VulkanSharedContext* context, VkRenderPass renderPass)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kYes, /*gpuMemorySize=*/0)
        , fSharedContext(context)
        , fRenderPass (renderPass) {
}

void VulkanRenderPass::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

} // namespace skgpu::graphite
