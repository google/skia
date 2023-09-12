/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanRenderPass.h"

#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

namespace skgpu::graphite {

namespace { // anonymous namespace

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
    // We only count attachments that are valid textures when calculating the total number of
    // render pass attachments, so if a texture is invalid, simply skip it rather than using
    // VK_ATTACHMENT_UNUSED and incrementing the builderIdx. Attachments can be differentiated from
    // one another by their sample count and format (i.e. depth/stencil attachments will have a
    // depth/stencil format).
}

} // anonymous namespace

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

namespace { // anonymous namespace
void setup_vk_attachment_description(VkAttachmentDescription* outAttachment,
                                     const VulkanTextureInfo& textureInfo,
                                     const AttachmentDesc& desc,
                                     const LoadOp loadOp,
                                     const StoreOp storeOp,
                                     const VkImageLayout initialLayout,
                                     const VkImageLayout finalLayout) {
    static_assert((int)LoadOp::kLoad == 0);
    static_assert((int)LoadOp::kClear == 1);
    static_assert((int)LoadOp::kDiscard == 2);
    static_assert(std::size(vkLoadOp) == kLoadOpCount);
    static_assert((int)StoreOp::kStore == 0);
    static_assert((int)StoreOp::kDiscard == 1);
    static_assert(std::size(vkStoreOp) == kStoreOpCount);

    outAttachment->flags = 0;
    outAttachment->format = textureInfo.fFormat;
    VkSampleCountFlagBits sampleCount;
    SkAssertResult(
            skgpu::SampleCountToVkSampleCount(textureInfo.fSampleCount, &sampleCount));
    outAttachment->samples = sampleCount;
    switch (initialLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_GENERAL:
            outAttachment->loadOp = vkLoadOp[static_cast<int>(loadOp)];
            outAttachment->storeOp = vkStoreOp[static_cast<int>(storeOp)];
            outAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            outAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            outAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            outAttachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            outAttachment->loadOp = vkLoadOp[static_cast<int>(loadOp)];
            outAttachment->storeOp = vkStoreOp[static_cast<int>(storeOp)];
            break;
        default:
            SK_ABORT("Unexpected attachment layout");
    }
    outAttachment->initialLayout = initialLayout;
    outAttachment->finalLayout = finalLayout == VK_IMAGE_LAYOUT_UNDEFINED ? initialLayout
                                                                          : finalLayout;
}
} // anonymous namespace

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeRenderPass(const VulkanSharedContext* context,
                                                         const RenderPassDesc& renderPassDesc,
                                                         bool compatibleOnly) {
    VkRenderPass renderPass;
    renderPass = VK_NULL_HANDLE;
    auto& colorAttachmentTextureInfo        = renderPassDesc.fColorAttachment.fTextureInfo;
    auto& colorResolveAttachmentTextureInfo = renderPassDesc.fColorResolveAttachment.fTextureInfo;
    auto& depthStencilAttachmentTextureInfo = renderPassDesc.fDepthStencilAttachment.fTextureInfo;
    bool hasColorAttachment        = colorAttachmentTextureInfo.isValid();
    bool hasColorResolveAttachment = colorResolveAttachmentTextureInfo.isValid();
    bool hasDepthStencilAttachment = depthStencilAttachmentTextureInfo.isValid();

    skia_private::TArray<VkAttachmentDescription> attachmentDescs;
    // Create and track attachment references for the subpass.
    VkAttachmentReference colorRef;
    VkAttachmentReference resolveRef;
    VkAttachmentReference depthStencilRef;

    if (hasColorAttachment) {
        VulkanTextureInfo colorAttachTexInfo;
        colorAttachmentTextureInfo.getVulkanTextureInfo(&colorAttachTexInfo);
        auto& colorAttachDesc = renderPassDesc.fColorAttachment;

        colorRef.attachment = attachmentDescs.size();
        VkAttachmentDescription& vkColorAttachDesc = attachmentDescs.push_back();
        memset(&vkColorAttachDesc, 0, sizeof(VkAttachmentDescription));
        setup_vk_attachment_description(
                &vkColorAttachDesc,
                colorAttachTexInfo,
                colorAttachDesc,
                compatibleOnly ? LoadOp::kDiscard  : colorAttachDesc.fLoadOp,
                compatibleOnly ? StoreOp::kDiscard : colorAttachDesc.fStoreOp,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (hasColorResolveAttachment) {
            SkASSERT(renderPassDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore);
            VulkanTextureInfo resolveAttachTexInfo;
            colorResolveAttachmentTextureInfo.getVulkanTextureInfo(&resolveAttachTexInfo);
            auto& resolveAttachDesc = renderPassDesc.fColorResolveAttachment;

            resolveRef.attachment = attachmentDescs.size();
            VkAttachmentDescription& vkResolveAttachDesc = attachmentDescs.push_back();
            memset(&vkResolveAttachDesc, 0, sizeof(VkAttachmentDescription));
            setup_vk_attachment_description(
                    &vkResolveAttachDesc,
                    resolveAttachTexInfo,
                    resolveAttachDesc,
                    compatibleOnly ? LoadOp::kDiscard  : resolveAttachDesc.fLoadOp,
                    compatibleOnly ? StoreOp::kDiscard : resolveAttachDesc.fStoreOp,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else {
            resolveRef.attachment = VK_ATTACHMENT_UNUSED;
            resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    } else {
        SkASSERT(false);
        colorRef.attachment = VK_ATTACHMENT_UNUSED;
        colorRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveRef.attachment = VK_ATTACHMENT_UNUSED;
        resolveRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    if (hasDepthStencilAttachment) {
        VulkanTextureInfo depthStencilTexInfo;
        depthStencilAttachmentTextureInfo.getVulkanTextureInfo(&depthStencilTexInfo);
        auto& depthStencilAttachDesc = renderPassDesc.fDepthStencilAttachment;

        depthStencilRef.attachment = attachmentDescs.size();
        VkAttachmentDescription& vkDepthStencilAttachDesc = attachmentDescs.push_back();
        setup_vk_attachment_description(
                &vkDepthStencilAttachDesc,
                depthStencilTexInfo,
                depthStencilAttachDesc,
                compatibleOnly ? LoadOp::kDiscard   : depthStencilAttachDesc.fLoadOp,
                compatibleOnly ? StoreOp::kDiscard  : depthStencilAttachDesc.fStoreOp,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } else {
        depthStencilRef.attachment = VK_ATTACHMENT_UNUSED;
        depthStencilRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    // Create VkRenderPass
    VkRenderPassCreateInfo renderPassInfo;
    memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;
    // TODO: Support multiple subpasses. 2 are needed for loading MSAA from resolve.
    renderPassInfo.subpassCount = 1;

    VkSubpassDescription subpassDesc;
    memset(&subpassDesc, 0, sizeof(VkSubpassDescription));
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // TODO: Add support for input attachments
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorRef;
    subpassDesc.pResolveAttachments = &resolveRef;
    subpassDesc.pDepthStencilAttachment = &depthStencilRef;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;

    renderPassInfo.pSubpasses = &subpassDesc;
    renderPassInfo.dependencyCount = 0;
    renderPassInfo.pDependencies = VK_NULL_HANDLE;
    renderPassInfo.attachmentCount = attachmentDescs.size();
    renderPassInfo.pAttachments = attachmentDescs.begin();

    VkResult result;
    VULKAN_CALL_RESULT(context->interface(), result, CreateRenderPass(context->device(),
                                                                      &renderPassInfo,
                                                                      nullptr,
                                                                      &renderPass));
    if (result != VK_SUCCESS) {
        return nullptr;
    }
    VkExtent2D granularity;
    VULKAN_CALL(context->interface(), GetRenderAreaGranularity(context->device(),
                                                               renderPass,
                                                               &granularity));
    return sk_sp<VulkanRenderPass>(new VulkanRenderPass(context, renderPass, granularity));
}

VulkanRenderPass::VulkanRenderPass(const VulkanSharedContext* context,
                                   VkRenderPass renderPass,
                                   VkExtent2D granularity)
        : Resource(context, Ownership::kOwned, skgpu::Budgeted::kYes, /*gpuMemorySize=*/0)
        , fSharedContext(context)
        , fRenderPass (renderPass)
        , fGranularity (granularity) {
}

void VulkanRenderPass::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

} // namespace skgpu::graphite
