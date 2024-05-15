/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanRenderPass.h"

#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

#include <limits>

namespace skgpu::graphite {

namespace { // anonymous namespace

int determine_uint32_count(int rpAttachmentCount, int subpassCount, int subpassDependencyCount ) {
    // The key will be formed such that bigger-picture items (such as the total attachment count)
    // will be near the front of the key to more quickly eliminate incompatible keys. Each
    // renderpass key will start with the total number of attachments associated with it
    // followed by how many subpasses and subpass dependencies the renderpass has.Packed together,
    // these will use one uint32.
    int num32DataCnt = 1;
    SkASSERT(static_cast<uint32_t>(rpAttachmentCount) <= (1u << 8));
    SkASSERT(static_cast<uint32_t>(subpassCount) <= (1u << 8));
    SkASSERT(static_cast<uint32_t>(subpassDependencyCount) <= (1u << 8));

    // The key will then contain key information for each attachment. This includes format, sample
    // count, and load/store operation information.
    num32DataCnt += 3 * rpAttachmentCount;
    // Then, subpass information will be added in the form of attachment reference indices. Reserve
    // one int32 for each possible attachment reference type, of which there are 4.
    // There are 4 possible attachment reference types. Pack all 4 attachment reference indices into
    // one uint32.
    num32DataCnt += subpassCount;
    // Each subpass dependency will be allotted 6 int32s to store all its pertinent information.
    num32DataCnt += 6 * subpassDependencyCount;

    return num32DataCnt;
}

void add_attachment_description_info_to_key(ResourceKey::Builder& builder,
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

void add_subpass_info_to_key(ResourceKey::Builder& builder,
                             int& builderIdx,
                             bool hasColorAttachment,
                             bool hasColorResolveAttachment,
                             bool hasDepthStencilAttachment,
                             bool loadMSAAFromResolve,
                             int subpassCount,
                             int subpassDependencyCount) {
    // TODO: Fetch actual attachment reference and index information for each
    // subpass from RenderPassDesc. For now, determine subpass data based upon whether we are
    // loading from MSAA or not.
    const int mainSubpassIdx = loadMSAAFromResolve ? 1 : 0;
    // Assign a smaller value to represent VK_ATTACHMENT_UNUSED.
    static constexpr int kAttachmentUnused = std::numeric_limits<uint8_t>::max();

    // The following key structure assumes that we only have up to one reference of each type per
    // subpass and that attachments are indexed in order of color, resolve, depth/stencil, then
    // input attachments. These indices are statically defined in the VulkanRenderPass header file.
    for (int j = 0; j < subpassCount; j++) {
        if (j == mainSubpassIdx) {
            uint32_t attachmentIdxKeyInfo;
            attachmentIdxKeyInfo = hasColorAttachment ? VulkanRenderPass::kColorAttachmentIdx
                                                      : kAttachmentUnused;
            attachmentIdxKeyInfo |=
                    (hasColorResolveAttachment ? VulkanRenderPass::kColorResolveAttachmentIdx
                                               : kAttachmentUnused) << 8;
            attachmentIdxKeyInfo |=
                    (hasDepthStencilAttachment ? VulkanRenderPass::kDepthStencilAttachmentIdx
                                               : kAttachmentUnused) << 16;
            // TODO: Add input attachment info to key once supported for use in main subpass
            attachmentIdxKeyInfo |= kAttachmentUnused << 24;

            builder[builderIdx++] = attachmentIdxKeyInfo;
        } else { // Loading MSAA from resolve subpass
            SkASSERT(hasColorAttachment);
            builder[builderIdx++] =
                    VulkanRenderPass::kColorAttachmentIdx | // color attachment
                    (kAttachmentUnused << 8)              | // No color resolve attachment
                    (kAttachmentUnused << 16)             | // No depth/stencil attachment
                    // The input attachment for the load subpass is the color resolve texture.
                    (VulkanRenderPass::kColorResolveAttachmentIdx << 24);
        }
    }

    // TODO: Query RenderPassDesc for subpass dependency information & populate the key accordingly.
    // For now, we know that the only subpass dependency will be that expected for loading MSAA from
    // resolve.
    for (int i = 0; i < subpassDependencyCount; i++) {
        builder[builderIdx++] = 0 | (mainSubpassIdx << 8); // srcSubpass, dstSubpass
        builder[builderIdx++] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // srcStageMask
        builder[builderIdx++] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // dstStageMask
        builder[builderIdx++] = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          // srcAccessMask
        builder[builderIdx++] = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |          // dstAccessMask
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        builder[builderIdx++] = VK_DEPENDENCY_BY_REGION_BIT;                   // dependencyFlags
    }
}

void populate_key(VulkanRenderPass::VulkanRenderPassMetaData& rpMetaData,
                  ResourceKey::Builder& builder,
                  int& builderIdx,
                  bool compatibleOnly) {
    builder[builderIdx++] = rpMetaData.fAttachments.size()  |
                            (rpMetaData.fSubpassCount << 8) |
                            (rpMetaData.fSubpassDependencyCount << 16);

    // Iterate through each renderpass attachment to add its information
    for (int i = 0; i < rpMetaData.fAttachments.size(); i++) {
        add_attachment_description_info_to_key(
                builder,
                rpMetaData.fAttachments[i]->fTextureInfo,
                builderIdx,
                // Assign LoadOp::kLoad and StoreOp::kStore as default load/store operations for
                // compatible render passes where load/store ops don't need to match.
                compatibleOnly ? LoadOp::kLoad   : rpMetaData.fAttachments[i]->fLoadOp,
                compatibleOnly ? StoreOp::kStore : rpMetaData.fAttachments[i]->fStoreOp);
    }

    add_subpass_info_to_key(builder,
                            builderIdx,
                            rpMetaData.fHasColorAttachment,
                            rpMetaData.fHasColorResolveAttachment,
                            rpMetaData.fHasDepthStencilAttachment,
                            rpMetaData.fLoadMSAAFromResolve,
                            rpMetaData.fSubpassCount,
                            rpMetaData.fSubpassDependencyCount);
}

} // anonymous namespace

VulkanRenderPass::VulkanRenderPassMetaData::VulkanRenderPassMetaData(
        const RenderPassDesc& renderPassDesc) {
    fLoadMSAAFromResolve = renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid() &&
                           renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
    fHasColorAttachment        = renderPassDesc.fColorAttachment.fTextureInfo.isValid();
    fHasColorResolveAttachment =
            renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid();
    fHasDepthStencilAttachment =
            renderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid();

    // TODO: Query for more attachments once the RenderPassDesc struct contains that information.
    // For now, we only ever expect to see 0 or 1 of each attachment type (color, resolve, and
    // depth/stencil), so the count of each of those can simply be determined with a bool.
    fNumColorAttachments        = fHasColorAttachment ? 1 : 0;
    fNumResolveAttachments      = fHasColorResolveAttachment ? 1 : 0;
    fNumDepthStencilAttachments = fHasDepthStencilAttachment ? 1 : 0;

    // Accumulate attachments into a container to mimic future structure in RenderPassDesc
    fAttachments = skia_private::TArray<const AttachmentDesc*>(fNumColorAttachments   +
                                                               fNumResolveAttachments +
                                                               fNumDepthStencilAttachments);
    if (fHasColorAttachment) {
        fAttachments.push_back(&renderPassDesc.fColorAttachment);
    }
    if (fHasColorResolveAttachment) {
        fAttachments.push_back(&renderPassDesc.fColorResolveAttachment);
    }
    if (fHasDepthStencilAttachment) {
        fAttachments.push_back(&renderPassDesc.fDepthStencilAttachment);
    }

    // TODO: Reference RenderPassDesc to determine number and makeup of subpasses and their
    // dependencies. For now, we only ever expect 1 (in most cases) or 2 (when loading MSAA).
    fSubpassCount = fLoadMSAAFromResolve ? 2 : 1;
    fSubpassDependencyCount = fLoadMSAAFromResolve ? 1 : 0;
    fUint32DataCnt = determine_uint32_count(
            fAttachments.size(), fSubpassCount,  fSubpassDependencyCount);
}

GraphiteResourceKey VulkanRenderPass::MakeRenderPassKey(
        const RenderPassDesc& renderPassDesc, bool compatibleOnly) {

    VulkanRenderPassMetaData rpMetaData = VulkanRenderPassMetaData(renderPassDesc);

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, rpMetaData.fUint32DataCnt, Shareable::kYes);

    int startingIdx = 0;
    populate_key(rpMetaData, builder, startingIdx, compatibleOnly);

    builder.finish();
    return key;
}

void VulkanRenderPass::AddRenderPassInfoToKey(VulkanRenderPassMetaData& rpMetaData,
                                              ResourceKey::Builder& builder,
                                              int& builderIdx,
                                              bool compatibleOnly) {
    populate_key(rpMetaData, builder, builderIdx, /*compatibleOnly=*/true);
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
            // The loadOp and storeOp refer to the depth part of the attachment and the stencil*Ops
            // refer to the stencil bits in the attachment.
            outAttachment->loadOp = vkLoadOp[static_cast<int>(loadOp)];
            outAttachment->storeOp = vkStoreOp[static_cast<int>(storeOp)];
            outAttachment->stencilLoadOp = vkLoadOp[static_cast<int>(loadOp)];
            outAttachment->stencilStoreOp = vkStoreOp[static_cast<int>(storeOp)];
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
    VkAttachmentReference resolveLoadInputRef;
    VkAttachmentReference depthStencilRef;

    bool loadMSAAFromResolve = false;
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
            loadMSAAFromResolve = renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
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
                    loadMSAAFromResolve ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                        : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
    renderPassInfo.subpassCount = loadMSAAFromResolve ? 2 : 1;

    skia_private::TArray<VkSubpassDescription> subpassDescs(renderPassInfo.subpassCount);
    memset(subpassDescs.begin(), 0, renderPassInfo.subpassCount * sizeof(VkSubpassDescription));

    // If we are loading MSAA from resolve, that subpass must always be first.
    VkSubpassDependency dependency;
    if (loadMSAAFromResolve) {
        resolveLoadInputRef.attachment = resolveRef.attachment;
        resolveLoadInputRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkSubpassDescription& loadSubpassDesc = subpassDescs.push_back();
        memset(&loadSubpassDesc, 0, sizeof(VkSubpassDescription));
        loadSubpassDesc.flags = 0;
        loadSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        loadSubpassDesc.inputAttachmentCount = 1;
        loadSubpassDesc.pInputAttachments = &resolveLoadInputRef;
        loadSubpassDesc.colorAttachmentCount = 1;
        loadSubpassDesc.pColorAttachments = &colorRef;
        loadSubpassDesc.pResolveAttachments = nullptr;
        loadSubpassDesc.pDepthStencilAttachment = nullptr;
        loadSubpassDesc.preserveAttachmentCount = 0;
        loadSubpassDesc.pPreserveAttachments = nullptr;

        // Set up the subpass dependency
        const int mainSubpassIdx = loadMSAAFromResolve ? 1 : 0;
        dependency.srcSubpass = 0;
        dependency.dstSubpass = mainSubpassIdx;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    VkSubpassDescription& mainSubpassDesc = subpassDescs.push_back();
    memset(&mainSubpassDesc, 0, sizeof(VkSubpassDescription));
    mainSubpassDesc.flags = 0;
    mainSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    mainSubpassDesc.inputAttachmentCount = 0; // TODO: Add input attachment support in main subpass
    mainSubpassDesc.pInputAttachments = nullptr;
    mainSubpassDesc.colorAttachmentCount = 1;
    mainSubpassDesc.pColorAttachments = &colorRef;
    mainSubpassDesc.pResolveAttachments = &resolveRef;
    mainSubpassDesc.pDepthStencilAttachment = &depthStencilRef;
    mainSubpassDesc.preserveAttachmentCount = 0;
    mainSubpassDesc.pPreserveAttachments = nullptr;

    renderPassInfo.pSubpasses = subpassDescs.begin();
    renderPassInfo.dependencyCount = loadMSAAFromResolve ? 1 : 0;
    renderPassInfo.pDependencies = loadMSAAFromResolve ? &dependency : VK_NULL_HANDLE;
    renderPassInfo.attachmentCount = attachmentDescs.size();
    renderPassInfo.pAttachments = attachmentDescs.begin();

    VkResult result;
    VULKAN_CALL_RESULT(context,
                       result,
                       CreateRenderPass(context->device(), &renderPassInfo, nullptr, &renderPass));
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
        : Resource(context,
                   Ownership::kOwned,
                   skgpu::Budgeted::kYes,
                   /*gpuMemorySize=*/0)
        , fSharedContext(context)
        , fRenderPass(renderPass)
        , fGranularity(granularity) {}

void VulkanRenderPass::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

} // namespace skgpu::graphite
