/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/graphite/vk/VulkanRenderPass.h"

#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
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
    if (textureInfo.isValid()) {
        const auto& vkTexInfo = TextureInfoPriv::Get<VulkanTextureInfo>(textureInfo);
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
                             const VulkanRenderPass::VulkanRenderPassMetaData& rpData,
                             bool compatibleOnly) {
    SkASSERT(rpData.fHasColorAttachment); // We expect to always have a valid color attachment

    // TODO: Fetch actual attachment reference and index information for each
    // subpass from RenderPassDesc. For now, determine subpass data based upon whether we are
    // loading from MSAA or not.
    const int mainSubpassIdx = rpData.fLoadMSAAFromResolve ? 1 : 0;
    // Assign a smaller value to represent VK_ATTACHMENT_UNUSED.
    static constexpr int kAttachmentUnused = std::numeric_limits<uint8_t>::max();

    // The following key structure assumes that we only have up to one reference of each type per
    // subpass and that attachments are indexed in order of color, resolve, depth/stencil, then
    // input attachments. These indices are statically defined in the VulkanRenderPass header file.
    for (int j = 0; j < rpData.fSubpassCount; j++) {
        if (j == mainSubpassIdx) {
            uint32_t attachmentIdxKeyInfo;
            attachmentIdxKeyInfo = rpData.fHasColorAttachment
                    ? VulkanRenderPass::kColorAttachmentIdx : kAttachmentUnused;
            attachmentIdxKeyInfo |= (rpData.fHasColorResolveAttachment
                    ? VulkanRenderPass::kColorResolveAttachmentIdx : kAttachmentUnused) << 8;
            attachmentIdxKeyInfo |= (rpData.fHasDepthStencilAttachment
                    ? VulkanRenderPass::kDepthStencilAttachmentIdx : kAttachmentUnused) << 16;

            // For keying purposes, make all compatible renderpasses and those that require dst
            // reads use the color attachment as the input attachment on the main subpass. This
            // enables sharing compatible renderpasses for pipeline creation for both pipelines that
            // do and do not read from the dst texture. This helps avoid costly pipeline compilation
            // at draw time.
            // When creating a full, non-compatible only renderpass for usage by the command buffer,
            // though, we want to differentiate its key such that we can use
            // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL for the color attachment. Simply assign
            // kAttachmentUnused (even though the main subpass's VkSubpassDescription still
            // indicates using the color attachment as an input attachment - even when unused - to
            // preserve renderpass compatibility across pipelines).
            attachmentIdxKeyInfo |= (!compatibleOnly && !rpData.fHasInputAttachment
                    ? kAttachmentUnused
                    : VulkanRenderPass::kColorAttachmentIdx) << 24;

            builder[builderIdx++] = attachmentIdxKeyInfo;
        } else { // Loading MSAA from resolve subpass
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
    for (int i = 0; i < rpData.fSubpassDependencyCount; i++) {
        builder[builderIdx++] = 0 | (mainSubpassIdx << 8); // srcSubpass, dstSubpass
        builder[builderIdx++] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // srcStageMask
        builder[builderIdx++] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // dstStageMask
        builder[builderIdx++] = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          // srcAccessMask
        builder[builderIdx++] = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |          // dstAccessMask
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        builder[builderIdx++] = VK_DEPENDENCY_BY_REGION_BIT;                   // dependencyFlags
    }
}

void populate_key(const VulkanRenderPass::VulkanRenderPassMetaData& rpMetaData,
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

    add_subpass_info_to_key(builder, builderIdx, rpMetaData, compatibleOnly);
}

} // anonymous namespace

VulkanRenderPass::VulkanRenderPassMetaData::VulkanRenderPassMetaData(const RenderPassDesc& rpDesc) {
    fLoadMSAAFromResolve       = rpDesc.fColorResolveAttachment.fTextureInfo.isValid() &&
                                 rpDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
    fHasColorAttachment        = rpDesc.fColorAttachment.fTextureInfo.isValid();
    fHasColorResolveAttachment = rpDesc.fColorResolveAttachment.fTextureInfo.isValid();
    fHasDepthStencilAttachment = rpDesc.fDepthStencilAttachment.fTextureInfo.isValid();
    fHasInputAttachment        = rpDesc.fDstReadStrategy == DstReadStrategy::kReadFromInput;

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
        fAttachments.push_back(&rpDesc.fColorAttachment);
    }
    if (fHasColorResolveAttachment) {
        fAttachments.push_back(&rpDesc.fColorResolveAttachment);
    }
    if (fHasDepthStencilAttachment) {
        fAttachments.push_back(&rpDesc.fDepthStencilAttachment);
    }

    // TODO: Reference RenderPassDesc to determine number and makeup of subpasses and their
    // dependencies. For now, we only ever expect 1 (in most cases) or 2 (when loading MSAA).
    fSubpassCount = fLoadMSAAFromResolve ? 2 : 1;
    // Note: Omit the subpass self-dependency from this count, which is for unique key generation
    // purposes, since it's the same between all renderpasses.
    fSubpassDependencyCount = fLoadMSAAFromResolve ? 1 : 0;
    fUint32DataCnt =
            determine_uint32_count(fAttachments.size(), fSubpassCount,  fSubpassDependencyCount);
}

GraphiteResourceKey VulkanRenderPass::MakeRenderPassKey(
        const RenderPassDesc& renderPassDesc, bool compatibleOnly) {

    const VulkanRenderPassMetaData rpMetaData = VulkanRenderPassMetaData(renderPassDesc);

    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();
    GraphiteResourceKey key;
    GraphiteResourceKey::Builder builder(&key, kType, rpMetaData.fUint32DataCnt);

    int startingIdx = 0;
    populate_key(rpMetaData, builder, startingIdx, compatibleOnly);

    builder.finish();
    return key;
}

void VulkanRenderPass::AddRenderPassInfoToKey(const VulkanRenderPassMetaData& rpMetaData,
                                              ResourceKey::Builder& builder,
                                              int& builderIdx,
                                              bool compatibleOnly) {
    populate_key(rpMetaData, builder, builderIdx, compatibleOnly);
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

void populate_attachment_refs(const RenderPassDesc& rpDesc,
                              bool compatibleOnly,
                              skia_private::TArray<VkAttachmentDescription>& descs,
                              VkAttachmentReference& colorRef,
                              VkAttachmentReference& resolveRef,
                              VkAttachmentReference& resolveLoadInputRef,
                              VkAttachmentReference& depthStencilRef,
                              VkAttachmentReference& inputAttachRef) {
    const auto& colorAttachmentTextureInfo        = rpDesc.fColorAttachment.fTextureInfo;
    const auto& colorResolveAttachmentTextureInfo = rpDesc.fColorResolveAttachment.fTextureInfo;
    const auto& depthStencilAttachmentTextureInfo = rpDesc.fDepthStencilAttachment.fTextureInfo;
    const bool hasColorAttachment        = colorAttachmentTextureInfo.isValid();
    const bool hasColorResolveAttachment = colorResolveAttachmentTextureInfo.isValid();
    const bool hasDepthStencilAttachment = depthStencilAttachmentTextureInfo.isValid();

    if (hasColorAttachment) {
        const auto& vkColorAttachInfo =
                TextureInfoPriv::Get<VulkanTextureInfo>(colorAttachmentTextureInfo);
        auto& colorAttachDesc = rpDesc.fColorAttachment;
        colorRef.attachment = descs.size();

        // If reading from the dst as an input attachment, we must use VK_IMAGE_LAYOUT_GENERAL
        // for the color attachment description. Use a general image layout for all compatible
        // renderpasses as well.
        //
        // This is necessary in order to avoid validation layer errors. Despite attachment layouts
        // not being a an aspect of RP compatibility, main subpass description attachment
        // indices are. So to maintain compatibility between RPs that do and do not read
        // from the dst, we always assign the main subpass's input attachment to be the color
        // attachment. However, using VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL for an input
        // attachment reference triggers validation layer errors (even if the the RP is
        // compatible-only or, in the case where no dst read is needed, never end up actually
        // reading from the input attachment).
        //
        // Therefore, only use VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL for the color attachment
        // description for full RPs that do not read from the dst as an input attachment.
        colorRef.layout = VK_IMAGE_LAYOUT_GENERAL;
        bool useOptimalLayoutForAttachmentDesc =
                !compatibleOnly && rpDesc.fDstReadStrategy != DstReadStrategy::kReadFromInput;
        VkImageLayout layout =
                useOptimalLayoutForAttachmentDesc ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                                  : VK_IMAGE_LAYOUT_GENERAL;

        VkAttachmentDescription& vkColorAttachDesc = descs.push_back();
        memset(&vkColorAttachDesc, 0, sizeof(VkAttachmentDescription));
        setup_vk_attachment_description(
                &vkColorAttachDesc,
                vkColorAttachInfo,
                colorAttachDesc,
                compatibleOnly ? LoadOp::kDiscard  : colorAttachDesc.fLoadOp,
                compatibleOnly ? StoreOp::kDiscard : colorAttachDesc.fStoreOp,
                layout,
                layout);

        if (hasColorResolveAttachment) {
            bool loadMSAAFromResolve = rpDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
            SkASSERT(rpDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore);
            VulkanTextureInfo resolveAttachTexInfo;
            SkAssertResult(TextureInfos::GetVulkanTextureInfo(colorResolveAttachmentTextureInfo,
                                                              &resolveAttachTexInfo));
            auto& resolveAttachDesc = rpDesc.fColorResolveAttachment;

            // If we are loading MSAA from resolve, we do not expect to later treat the resolve
            // texture as a dst that we can read as an input attachment. Therefore, we do not have
            // to worry about using a general layout for dst reads.
            VkImageLayout initialResolveLayout =
                    loadMSAAFromResolve ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                        : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            VkImageLayout finalResolveLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            resolveRef.attachment = descs.size();
            resolveRef.layout = finalResolveLayout; // Attachment ref expects final layout

            VkAttachmentDescription& vkResolveAttachDesc = descs.push_back();
            memset(&vkResolveAttachDesc, 0, sizeof(VkAttachmentDescription));
            setup_vk_attachment_description(
                    &vkResolveAttachDesc,
                    resolveAttachTexInfo,
                    resolveAttachDesc,
                    compatibleOnly ? LoadOp::kDiscard  : resolveAttachDesc.fLoadOp,
                    compatibleOnly ? StoreOp::kDiscard : resolveAttachDesc.fStoreOp,
                    initialResolveLayout,
                    finalResolveLayout);
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
        const auto& vkDepthStencilInfo =
                TextureInfoPriv::Get<VulkanTextureInfo>(depthStencilAttachmentTextureInfo);
        auto& depthStencilAttachDesc = rpDesc.fDepthStencilAttachment;

        depthStencilRef.attachment = descs.size();
        depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription& vkDepthStencilAttachDesc = descs.push_back();
        memset(&vkDepthStencilAttachDesc, 0, sizeof(VkAttachmentDescription));
        setup_vk_attachment_description(
                &vkDepthStencilAttachDesc,
                vkDepthStencilInfo,
                depthStencilAttachDesc,
                compatibleOnly ? LoadOp::kDiscard  : depthStencilAttachDesc.fLoadOp,
                compatibleOnly ? StoreOp::kDiscard : depthStencilAttachDesc.fStoreOp,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    } else {
        depthStencilRef.attachment = VK_ATTACHMENT_UNUSED;
        depthStencilRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    // No VkAttachmentDescription is needed for the inputAttachRef because it merely points to one
    // of the existing attachments. Assign the input attachment ref's attachment to be the color
    // attachment even for renderpasses that do not use an input attachment on the main subpass.
    // Normally, we would want to use VK_ATTACHMENT_UNUSED in such cases, but always assigning it to
    // be the color attachment even when unused allows for compatible-only renderpasses to be shared
    // for pipelines that do read from the dst and those that do not.
    inputAttachRef.attachment = VulkanRenderPass::kColorAttachmentIdx;

    // Even though attachment layouts are not an aspect of renderpass compatibility, indicating the
    // usage of a layout incompatible with input attachment usage preemptively triggers validation
    // layer errors since we do not use VK_ATTACHMENT_UNUSED, so use VK_IMAGE_LAYOUT_GENERAL here.
    // A full, non-compatible only renderpass that performs no dst reads can still use
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL for the color attachment without validation layer
    // complaints so long as the input attachment is not actually used by the command buffer.
    // On non-tiler GPUs, most drivers have optimizations surrounding this case (an attachment
    // reference that is not actually read from using a layout that differs from the
    // VkAttachmentDescription's initial and final layout), meaning performance should not be
    // notably impacted.
    inputAttachRef.layout = VK_IMAGE_LAYOUT_GENERAL;
}

void populate_subpass_dependencies(skia_private::STArray<2, VkSubpassDependency>& deps,
                                   bool loadMSAAFromResolve) {
    const int mainSubpassIdx = loadMSAAFromResolve ? 1 : 0;

    // Adding a single subpass self-dependency for color attachments is basically free, so apply
    // one to every RenderPass which has an input attachment on the main subpass. This is useful
    // because it means that as we perform draw calls, if we encounter a draw that uses a blend
    // operation requiring a dst read, we can avoid having to switch RenderPasses.
    VkSubpassDependency& selfDependency = deps.push_back();
    selfDependency.srcSubpass = mainSubpassIdx;
    selfDependency.dstSubpass = mainSubpassIdx;
    selfDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    selfDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    selfDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    selfDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    selfDependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

    // If loading MSAA from resolve, enforce that subpass goes first with a subpass dependency.
    if (loadMSAAFromResolve) {
        VkSubpassDependency& dependency = deps.push_back();
        dependency.srcSubpass = 0;
        dependency.dstSubpass = mainSubpassIdx;
        dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
}

void populate_subpass_descs(skia_private::TArray<VkSubpassDescription>& descs,
                            bool loadMSAAFromResolve,
                            bool dstMayBeReadAsInput,
                            VkAttachmentReference& colorRef,
                            VkAttachmentReference& resolveRef,
                            VkAttachmentReference& resolveLoadInputRef,
                            VkAttachmentReference& depthStencilRef,
                            VkAttachmentReference& inputAttachRef) {
    // If loading MSAA from resolve, add the additional subpass to do so.
    if (loadMSAAFromResolve) {
        resolveLoadInputRef.attachment = resolveRef.attachment;
        resolveLoadInputRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkSubpassDescription& loadSubpassDesc = descs.push_back();
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
    }

    VkSubpassDescription& mainSubpassDesc = descs.push_back();
    memset(&mainSubpassDesc, 0, sizeof(VkSubpassDescription));
    mainSubpassDesc.flags = 0;
    mainSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // Always include one input attachment on the main subpass which can optionally be used or not
    mainSubpassDesc.inputAttachmentCount = 1;
    mainSubpassDesc.pInputAttachments = &inputAttachRef;
    mainSubpassDesc.colorAttachmentCount = 1;
    mainSubpassDesc.pColorAttachments = &colorRef;
    mainSubpassDesc.pResolveAttachments = &resolveRef;
    mainSubpassDesc.pDepthStencilAttachment = &depthStencilRef;
    mainSubpassDesc.preserveAttachmentCount = 0;
    mainSubpassDesc.pPreserveAttachments = nullptr;
}

} // anonymous namespace

sk_sp<VulkanRenderPass> VulkanRenderPass::MakeRenderPass(const VulkanSharedContext* context,
                                                         const RenderPassDesc& renderPassDesc,
                                                         bool compatibleOnly) {
    VkRenderPass renderPass;
    renderPass = VK_NULL_HANDLE;

    // Determine whether this renderpass loads MSAA from resolve and whether it must support reading
    // the dst texture as an input attachment. These have impacts on multiple aspects of RP setup.
    const bool loadMSAAFromResolve =
            renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid() &&
            renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
    const bool dstMayBeReadAsInput =
            renderPassDesc.fDstReadStrategy == DstReadStrategy::kReadFromInput;

    // Set up attachment descriptions + references. Declare them before having a helper populate
    // their values so we can reference them later during RP creation.
    skia_private::TArray<VkAttachmentDescription> attachmentDescs;
    VkAttachmentReference colorRef;
    VkAttachmentReference resolveRef;
    VkAttachmentReference resolveLoadInputRef;
    VkAttachmentReference depthStencilRef;
    VkAttachmentReference inputAttachRef;
    populate_attachment_refs(renderPassDesc,
                             compatibleOnly,
                             attachmentDescs,
                             colorRef,
                             resolveRef,
                             resolveLoadInputRef,
                             depthStencilRef,
                             inputAttachRef);

    // Assemble subpass information before creating the renderpass. Each renderpass has at least one
    // subpass dependency (self-dependency for reading the dst texture). If loading MSAA from
    // resolve, that adds another subpass and an additional dependency.
    skia_private::STArray<2, VkSubpassDescription> subpassDescs;
    populate_subpass_descs(subpassDescs,
                           loadMSAAFromResolve,
                           dstMayBeReadAsInput,
                           colorRef,
                           resolveRef,
                           resolveLoadInputRef,
                           depthStencilRef,
                           inputAttachRef);
    skia_private::STArray<2, VkSubpassDependency> dependencies;
    populate_subpass_dependencies(dependencies, loadMSAAFromResolve);

    // Create VkRenderPass
    VkRenderPassCreateInfo renderPassInfo;
    memset(&renderPassInfo, 0, sizeof(VkRenderPassCreateInfo));
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.flags = 0;
    renderPassInfo.subpassCount = loadMSAAFromResolve ? 2 : 1;
    renderPassInfo.pSubpasses = subpassDescs.begin();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();
    renderPassInfo.attachmentCount = attachmentDescs.size();
    renderPassInfo.pAttachments = attachmentDescs.data();

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
                   /*gpuMemorySize=*/0)
        , fSharedContext(context)
        , fRenderPass(renderPass)
        , fGranularity(granularity) {}

void VulkanRenderPass::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

} // namespace skgpu::graphite
