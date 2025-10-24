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

static constexpr uint32_t kColorFormatOffset = 0;
static constexpr uint32_t kDepthStencilFormatOffset = kColorFormatOffset + 8;
static constexpr uint32_t kSampleCountOffset = kDepthStencilFormatOffset + 8;
static constexpr uint32_t kColorLoadOpOffset = kSampleCountOffset + 5;
static constexpr uint32_t kColorStoreOpOffset = kColorLoadOpOffset + 2;
static constexpr uint32_t kResolveLoadOpOffset = kColorStoreOpOffset + 1;
static constexpr uint32_t kResolveStoreOpOffset = kResolveLoadOpOffset + 2;
static constexpr uint32_t kMSRTTOffset = kResolveStoreOpOffset + 1;

static constexpr uint32_t kFormatMask = 0xFF;
static constexpr uint32_t kSampleCountMask = 0x1F;
static constexpr uint32_t kLoadOpMask = 0x3;
static constexpr uint32_t kStoreOpMask = 0x1;
static constexpr uint32_t kMSRTTMask = 0x1;

namespace skgpu::graphite {

uint32_t VulkanRenderPass::GetRenderPassKey(const RenderPassDesc& originalRenderPassDesc,
                                            bool compatibleForPipelineKey) {
    const RenderPassDesc renderPassDesc =
            compatibleForPipelineKey ? MakePipelineCompatibleRenderPass(originalRenderPassDesc)
                                     : originalRenderPassDesc;
    const AttachmentDesc color = renderPassDesc.fColorAttachment;
    const AttachmentDesc resolve = renderPassDesc.fColorResolveAttachment;
    const AttachmentDesc depthStencil = renderPassDesc.fDepthStencilAttachment;

    const bool hasResolveAttachment = resolve.fFormat != TextureFormat::kUnsupported;
    const bool isMultisampledRenderToSingleSampled =
            RenderPassDescWillImplicitlyLoadMSAA(renderPassDesc);
    // Silence warning about unused variable in release builds.
    (void)hasResolveAttachment;

    // Current assumptions in the render pass desc, allowing us to create a smaller key:
    SkASSERT(color.fFormat != TextureFormat::kUnsupported);
    SkASSERT(!hasResolveAttachment || resolve.fFormat == color.fFormat);
    SkASSERT(depthStencil.fFormat == TextureFormat::kUnsupported ||
             depthStencil.fSampleCount == color.fSampleCount);
    SkASSERT(depthStencil.fFormat == TextureFormat::kUnsupported ||
             depthStencil.fLoadOp == LoadOp::kClear);
    SkASSERT(depthStencil.fFormat == TextureFormat::kUnsupported ||
             depthStencil.fStoreOp == StoreOp::kDiscard);
    SkASSERT(!hasResolveAttachment || resolve.fSampleCount == 1);
    SkASSERT(color.fSampleCount == renderPassDesc.fSampleCount ||
             isMultisampledRenderToSingleSampled);
    SkASSERT(!hasResolveAttachment || color.fLoadOp == LoadOp::kDiscard ||
             color.fLoadOp == LoadOp::kClear);
    SkASSERT(!hasResolveAttachment || resolve.fLoadOp == LoadOp::kDiscard ||
             resolve.fLoadOp == LoadOp::kLoad);
    SkASSERT(!hasResolveAttachment || color.fStoreOp == StoreOp::kDiscard);

    // The following information uniquely defines the render pass:
    //
    // Color format (CF): TextureFormat, fits in 6 bits
    // Depth/stencil format (DSF): TextureFormat, fits in 6 bits
    // Sample count (M): Up to 16, fits in 5 bit
    // Color load op (L): LoadOp, fits in 2 bits
    // Color store op (S): StoreOp, fits in 1 bit
    // Whether multisampled data s loaded from resolve attachment: fits in 1 bit
    // Whether rendering multisampled->single-sampled (MSRTSS): fits in 1 bit
    //
    // Note that technically, renderable color formats can fit in 5 bits and depth/stencil formats
    // in 3 bits if more packing is needed. Sample count can also be `log()`'ed to reduce the bit
    // count.
    //
    // Depth/stencil load op is always kClear, and store op is always kDiscard.
    // Color load op is either found in fColorAttachment if no resolve attachment, or can be derived
    // from a combination of the load ops specified in the color and resolve attachments:
    //   * If color attachment is kClear, load op is kClear
    //   * Otherwise, if resolve attachment is kLoad, load op is kLoad
    //   * Otherwise, it's kDiscard
    // Color store op is either found in fColorAttachment if no resolve attachment, or it's found in
    // fColorResolveAttachment.
    //
    // There are currently lots of free bits, so with regards to load/store ops, we don't try too
    // hard to pack things.  Including the color and resolve's load and store ops obviates the need
    // to store a "load MSAA from resolve" bit.  The format of the key is thus:
    //
    //       LSB                                                         MSB
    //       +----+-----+---+---+---+------------+------------+--------+---+
    //       | CF | DSF | M | L | S | L(resolve) | S(resolve) | MSRTSS | 0 |
    //       +----+-----+---+---+---+------------+------------+--------+---+
    //  bits   8     8    5   2   1       2            1           1     4
    //
    SkASSERT(renderPassDesc.fSampleCount < (1 << 5));
    static_assert(static_cast<uint32_t>(TextureFormat::kLast) < (1 << 8));
    static_assert(static_cast<uint32_t>(LoadOp::kLast) < (1 << 2));
    static_assert(static_cast<uint32_t>(StoreOp::kLast) < (1 << 1));

    const uint32_t key =
            (static_cast<uint32_t>(color.fFormat) << kColorFormatOffset) |
            (static_cast<uint32_t>(depthStencil.fFormat) << kDepthStencilFormatOffset) |
            (static_cast<uint32_t>(renderPassDesc.fSampleCount) << kSampleCountOffset) |
            (static_cast<uint32_t>(color.fLoadOp) << kColorLoadOpOffset) |
            (static_cast<uint32_t>(color.fStoreOp) << kColorStoreOpOffset) |
            (static_cast<uint32_t>(resolve.fLoadOp) << kResolveLoadOpOffset) |
            (static_cast<uint32_t>(resolve.fStoreOp) << kResolveStoreOpOffset) |
            (static_cast<uint32_t>(isMultisampledRenderToSingleSampled) << kMSRTTOffset);

    return key;
}

void VulkanRenderPass::ExtractRenderPassDesc(uint32_t key,
                                             Swizzle writeSwizzle,
                                             DstReadStrategy dstReadStrategy,
                                             RenderPassDesc* renderPassDesc) {
    // See comment in GetRenderPassKey() describing the format of the key.
    const TextureFormat colorFormat =
            SkTo<TextureFormat>((key >> kColorFormatOffset) & kFormatMask);
    const TextureFormat depthStencilFormat =
            SkTo<TextureFormat>((key >> kDepthStencilFormatOffset) & kFormatMask);
    const uint8_t sampleCount = SkTo<uint8_t>((key >> kSampleCountOffset) & kSampleCountMask);
    const LoadOp colorLoadOp = SkTo<LoadOp>((key >> kColorLoadOpOffset) & kLoadOpMask);
    const StoreOp colorStoreOp = SkTo<StoreOp>((key >> kColorStoreOpOffset) & kStoreOpMask);
    const LoadOp resolveLoadOp = SkTo<LoadOp>((key >> kResolveLoadOpOffset) & kLoadOpMask);
    const StoreOp resolveStoreOp = SkTo<StoreOp>((key >> kResolveStoreOpOffset) & kStoreOpMask);
    const bool isMultisampledRenderToSingleSampled = SkTo<bool>((key >> kMSRTTOffset) & kMSRTTMask);

    // Relationship between render pass sample count, attachment's sample counts and resolve
    // attachments:
    //
    //                  | RP Samples == 1 | RP Samples > 1 && MSRTSS | RP Samples > 1 && !MSRTSS |
    //                  +-----------------+--------------------------+---------------------------+
    //  Has Resolve?    |       No        |            No            |           Yes             |
    //                  +-----------------+--------------------------+---------------------------+
    //  Resolve Format  |   Unsupported   |       Unsupported        |       Color Format        |
    //                  +-----------------+--------------------------+---------------------------+
    //  Color Samples   | RP Samples (1)  |            1             |        RP Samples         |
    //                  +-----------------+--------------------------+---------------------------+
    //  D/S Samples     | RP Samples (1)  |            1             |        RP Samples         |
    //                  +-----------------+--------------------------+---------------------------+
    //
    // The color and resolve attachment's load/store op are already stored in the key. For
    // depth/stencil, load op is always Clear and store op is always Discard.
    const uint8_t attachmentSamples = isMultisampledRenderToSingleSampled ? 1 : sampleCount;

    *renderPassDesc = {};
    renderPassDesc->fColorAttachment = {colorFormat, colorLoadOp, colorStoreOp, attachmentSamples};
    renderPassDesc->fDepthStencilAttachment = {
            depthStencilFormat, LoadOp::kClear, StoreOp::kDiscard, attachmentSamples};
    if (attachmentSamples > 1 && !isMultisampledRenderToSingleSampled) {
        renderPassDesc->fColorResolveAttachment = {colorFormat,
                                                   resolveLoadOp,
                                                   resolveStoreOp,
                                                   /*fSampleCount=*/1};
    }
    renderPassDesc->fSampleCount = sampleCount;
    renderPassDesc->fWriteSwizzle = writeSwizzle;
    renderPassDesc->fDstReadStrategy = dstReadStrategy;
}

namespace {

template <typename AttachmentDescription>
void setup_vk_attachment_description(AttachmentDescription* outAttachment,
                                     const AttachmentDesc& desc,
                                     const AttachmentDescription& defaultAttachmentDescription,
                                     bool isColor) {
    static_assert((int) LoadOp::kLoad     == (int) VK_ATTACHMENT_LOAD_OP_LOAD);
    static_assert((int) LoadOp::kClear    == (int) VK_ATTACHMENT_LOAD_OP_CLEAR);
    static_assert((int) LoadOp::kDiscard  == (int) VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    static_assert((int) StoreOp::kStore   == (int) VK_ATTACHMENT_STORE_OP_STORE);
    static_assert((int) StoreOp::kDiscard == (int) VK_ATTACHMENT_STORE_OP_DONT_CARE);

    VkAttachmentLoadOp vkLoadOp = static_cast<VkAttachmentLoadOp>(desc.fLoadOp);
    VkAttachmentStoreOp vkStoreOp = static_cast<VkAttachmentStoreOp>(desc.fStoreOp);

    *outAttachment = defaultAttachmentDescription;
    outAttachment->format = TextureFormatToVkFormat(desc.fFormat);
    VkSampleCountFlagBits sampleCount;
    SkAssertResult(SampleCountToVkSampleCount(desc.fSampleCount, &sampleCount));
    outAttachment->samples = sampleCount;

    outAttachment->loadOp = vkLoadOp;
    outAttachment->storeOp = vkStoreOp;
    if (isColor) {
        outAttachment->initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        outAttachment->finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        outAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        outAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    } else {
        outAttachment->initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        outAttachment->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        outAttachment->stencilLoadOp = vkLoadOp;
        outAttachment->stencilStoreOp = vkStoreOp;
    }
}

template <typename AttachmentDescription, typename AttachmentReference>
void populate_attachment_refs(const RenderPassDesc& renderPassDesc,
                              bool needLoadMSAAFromResolveSubpass,
                              skia_private::TArray<AttachmentDescription>& descs,
                              AttachmentReference& colorRef,
                              AttachmentReference& resolveRef,
                              AttachmentReference& resolveLoadInputRef,
                              AttachmentReference& depthStencilRef,
                              const AttachmentDescription& defaultAttachmentDescription) {
    // Note: See assumptions regarding RenderPassDesc structure in AddToKey().
    const AttachmentDesc color = renderPassDesc.fColorAttachment;
    const AttachmentDesc resolve = renderPassDesc.fColorResolveAttachment;
    const AttachmentDesc depthStencil = renderPassDesc.fDepthStencilAttachment;
    SkASSERT(color.fFormat != TextureFormat::kUnsupported);
    // If MSAA data needs to be loaded from the resolve attachment, there must be a resolve
    // attachment!
    SkASSERT(!needLoadMSAAFromResolveSubpass || resolve.fFormat != TextureFormat::kUnsupported);

    // If reading from the dst as an input attachment, we must use VK_IMAGE_LAYOUT_GENERAL
    // for the color attachment description. Use a general image layout for all renderpasses to
    // support this.
    //
    // As long as the initial/final layouts are VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, the
    // choice of the VK_IMAGE_LAYOUT_GENERAL for the subpass layout is efficient; the few GPUs
    // that treat VK_IMAGE_LAYOUT_GENERAL differently recognize this pattern and keep the
    // internal layout optimal.
    colorRef.layout = VK_IMAGE_LAYOUT_GENERAL;
    colorRef.attachment = descs.size();
    AttachmentDescription& vkColorAttachDesc = descs.push_back();
    setup_vk_attachment_description(&vkColorAttachDesc,
                                    color,
                                    defaultAttachmentDescription,
                                    /*isColor=*/true);

    if (resolve.fFormat != TextureFormat::kUnsupported) {
        resolveRef.attachment = descs.size();
        resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (needLoadMSAAFromResolveSubpass) {
            resolveLoadInputRef.attachment = resolveRef.attachment;
            // The resolve attachment is used as input attachment in the first subpass.  In that
            // subpass, it is only read from so the layout could have been read-only.  To avoid
            // special-casing this input attachment though, the VK_IMAGE_LAYOUT_GENERAL layout
            // is used like with the color attachment.  This is no less efficient than the
            // read-only layout when specified on the attachment ref.
            resolveLoadInputRef.layout = VK_IMAGE_LAYOUT_GENERAL;
        } else {
            resolveLoadInputRef.attachment = VK_ATTACHMENT_UNUSED;
        }

        AttachmentDescription& vkResolveAttachDesc = descs.push_back();
        setup_vk_attachment_description(&vkResolveAttachDesc,
                                        resolve,
                                        defaultAttachmentDescription,
                                        /*isColor=*/true);
    } else {
        resolveRef.attachment = resolveLoadInputRef.attachment = VK_ATTACHMENT_UNUSED;
    }

    if (depthStencil.fFormat != TextureFormat::kUnsupported) {
        depthStencilRef.attachment = descs.size();
        depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        AttachmentDescription& vkDepthStencilAttachDesc = descs.push_back();
        setup_vk_attachment_description(&vkDepthStencilAttachDesc,
                                        depthStencil,
                                        defaultAttachmentDescription,
                                        /*isColor=*/false);
    } else {
        depthStencilRef.attachment = VK_ATTACHMENT_UNUSED;
    }
}

template <typename SubpassDependency, typename SubpassDependencyArray>
void populate_subpass_dependencies(const VulkanSharedContext* context,
                                   SubpassDependencyArray& deps,
                                   bool needLoadMSAAFromResolveSubpass,
                                   const SubpassDependency& defaultSubpassDependency) {
    const int mainSubpassIdx = needLoadMSAAFromResolveSubpass ? 1 : 0;

    // Adding a single subpass self-dependency for color attachments is basically free, so apply
    // one to every RenderPass which has an input attachment on the main subpass. This is useful
    // because it means that as we perform draw calls, if we encounter a draw that uses a blend
    // operation requiring a dst read, we can avoid having to switch RenderPasses.

    const bool hasRasterizationOrderColorAttachmentAccess =
            context->vulkanCaps().supportsRasterizationOrderColorAttachmentAccess();
    const bool hasNonCoherentAdvancedBlend = context->vulkanCaps().blendEquationSupport() ==
                                             Caps::BlendEquationSupport::kAdvancedNoncoherent;

    if (!hasRasterizationOrderColorAttachmentAccess || hasNonCoherentAdvancedBlend) {
        SubpassDependency& selfDependency = deps.push_back();
        selfDependency = defaultSubpassDependency;
        selfDependency.srcSubpass = mainSubpassIdx;
        selfDependency.dstSubpass = mainSubpassIdx;
        selfDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        selfDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        selfDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        selfDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        selfDependency.dstAccessMask = 0;

        if (!hasRasterizationOrderColorAttachmentAccess) {
            selfDependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            selfDependency.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        }
        if (hasNonCoherentAdvancedBlend) {
            selfDependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
        }
    }

    // If loading MSAA from resolve, enforce that subpass goes first with a subpass dependency.
    if (needLoadMSAAFromResolveSubpass) {
        SubpassDependency& dependency = deps.push_back();
        dependency = defaultSubpassDependency;
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

template <typename SubpassDescription, typename AttachmentReference>
void populate_main_subpass_desc(const VulkanCaps& caps,
                                SubpassDescription& mainSubpassDesc,
                                const AttachmentReference& colorRef,
                                const AttachmentReference& resolveRef,
                                const AttachmentReference& depthStencilRef) {
    mainSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // In the main subpass, the input attachment is the color attachment.  We could have excluded
    // this attachment if unused, but always assigning it to be the color attachment even when
    // unused allows for compatible-only renderpasses to be shared for pipelines that do read from
    // the dst and those that do not.
    mainSubpassDesc.inputAttachmentCount = 1;
    mainSubpassDesc.pInputAttachments = &colorRef;
    mainSubpassDesc.colorAttachmentCount = 1;
    mainSubpassDesc.pColorAttachments = &colorRef;
    mainSubpassDesc.pResolveAttachments = &resolveRef;
    mainSubpassDesc.pDepthStencilAttachment = &depthStencilRef;
}

void populate_subpass_descs(const VulkanCaps& caps,
                            skia_private::TArray<VkSubpassDescription>& descs,
                            const VkAttachmentReference& colorRef,
                            const VkAttachmentReference& resolveRef,
                            const VkAttachmentReference& resolveLoadInputRef,
                            const VkAttachmentReference& depthStencilRef) {
    // If loading MSAA from resolve, add the additional subpass to do so.
    if (resolveLoadInputRef.attachment != VK_ATTACHMENT_UNUSED) {
        VkSubpassDescription& loadSubpassDesc = descs.push_back();
        loadSubpassDesc = {};
        loadSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        loadSubpassDesc.inputAttachmentCount = 1;
        loadSubpassDesc.pInputAttachments = &resolveLoadInputRef;
        loadSubpassDesc.colorAttachmentCount = 1;
        loadSubpassDesc.pColorAttachments = &colorRef;
    }

    VkSubpassDescription& mainSubpassDesc = descs.push_back();
    mainSubpassDesc = {};
    populate_main_subpass_desc(caps, mainSubpassDesc, colorRef, resolveRef, depthStencilRef);

    if (caps.supportsRasterizationOrderColorAttachmentAccess()) {
        for (VkSubpassDescription& desc : descs) {
            desc.flags = VK_SUBPASS_DESCRIPTION_RASTERIZATION_ORDER_ATTACHMENT_COLOR_ACCESS_BIT_EXT;
        }
    }
}

} // anonymous namespace

sk_sp<VulkanRenderPass> VulkanRenderPass::Make(const VulkanSharedContext* context,
                                               const RenderPassDesc& renderPassDesc) {
    const bool needLoadMSAAFromResolveSubpass =
            RenderPassDescWillLoadMSAAFromResolve(renderPassDesc);
    const bool isMultisampledRenderToSingleSampled =
            RenderPassDescWillImplicitlyLoadMSAA(renderPassDesc);

    VkResult result;
    VkRenderPass renderPass = VK_NULL_HANDLE;

    // VK_EXT_multisampled_render_to_single_sampled usage requires vkCreateRenderPass2.
    if (isMultisampledRenderToSingleSampled) {
        // Set up attachment descriptions + references. Declare them before having a helper populate
        // their values so we can reference them later during RP creation.
        SkASSERT(renderPassDesc.fColorResolveAttachment.fFormat == TextureFormat::kUnsupported);
        SkASSERT(!needLoadMSAAFromResolveSubpass);

        VkAttachmentDescription2 defaultAttachmentDescription = {};
        defaultAttachmentDescription.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;

        VkAttachmentReference2 defaultAttachmentReference = {};
        defaultAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
        defaultAttachmentReference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        skia_private::TArray<VkAttachmentDescription2> attachmentDescs;
        VkAttachmentReference2 colorRef = defaultAttachmentReference;
        VkAttachmentReference2 resolveRef = defaultAttachmentReference;
        VkAttachmentReference2 resolveLoadInputRef = defaultAttachmentReference;
        VkAttachmentReference2 depthStencilRef = defaultAttachmentReference;
        depthStencilRef.aspectMask =
                GetVkImageAspectFlags(renderPassDesc.fDepthStencilAttachment.fFormat);

        populate_attachment_refs(renderPassDesc,
                                 /*needLoadMSAAFromResolveSubpass=*/false,
                                 attachmentDescs,
                                 colorRef,
                                 resolveRef,
                                 resolveLoadInputRef,
                                 depthStencilRef,
                                 defaultAttachmentDescription);
        SkASSERT(resolveLoadInputRef.attachment == VK_ATTACHMENT_UNUSED);

        // Assemble subpass information before creating the renderpass. Each renderpass has at least
        // one subpass dependency (self-dependency for reading the dst texture).
        VkSubpassDescription2 mainSubpassDesc = {};
        mainSubpassDesc.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
        populate_main_subpass_desc(
                context->vulkanCaps(), mainSubpassDesc, colorRef, resolveRef, depthStencilRef);

        // Enable multisampled render to single-sampled with the render pass's sample count.
        VkMultisampledRenderToSingleSampledInfoEXT msrtss = {};
        msrtss.sType = VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT;
        msrtss.multisampledRenderToSingleSampledEnable = VK_TRUE;
        SampleCountToVkSampleCount(renderPassDesc.fSampleCount, &msrtss.rasterizationSamples);
        AddToPNextChain(&mainSubpassDesc, &msrtss);

        // Depth/stencil resolve needs additional configuration (even though Graphite always
        // discards depth/stencil).
        SkASSERT(renderPassDesc.fDepthStencilAttachment.fStoreOp == StoreOp::kDiscard);
        VkSubpassDescriptionDepthStencilResolve dsResolve = {};
        dsResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
        dsResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
        dsResolve.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
        AddToPNextChain(&mainSubpassDesc, &dsResolve);

        VkSubpassDependency2 defaultSubpassDependency = {};
        defaultSubpassDependency.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;

        skia_private::STArray<1, VkSubpassDependency2> dependencies;
        populate_subpass_dependencies(context,
                                      dependencies,
                                      /*needLoadMSAAFromResolveSubpass=*/false,
                                      defaultSubpassDependency);

        // Create VkRenderPass
        VkRenderPassCreateInfo2 renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &mainSubpassDesc;
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();
        renderPassInfo.attachmentCount = attachmentDescs.size();
        renderPassInfo.pAttachments = attachmentDescs.data();

        VULKAN_CALL_RESULT(
                context,
                result,
                CreateRenderPass2(context->device(), &renderPassInfo, nullptr, &renderPass));
    } else {
        // Same thing as above, but using Vulkan 1.0 render pass API.  Practically every usable
        // driver supports VK_KHR_create_renderpass2, but that is not yet a minimum requirement of
        // Graphite.
        skia_private::TArray<VkAttachmentDescription> attachmentDescs;
        VkAttachmentReference colorRef = {};
        VkAttachmentReference resolveRef = {};
        VkAttachmentReference resolveLoadInputRef = {};
        VkAttachmentReference depthStencilRef = {};
        populate_attachment_refs(renderPassDesc,
                                 needLoadMSAAFromResolveSubpass,
                                 attachmentDescs,
                                 colorRef,
                                 resolveRef,
                                 resolveLoadInputRef,
                                 depthStencilRef,
                                 /*defaultAttachmentDescription=*/{});

        // If loading MSAA from resolve, an extra subpass and an additional dependency is needed.
        skia_private::STArray<2, VkSubpassDescription> subpassDescs;
        populate_subpass_descs(context->vulkanCaps(),
                               subpassDescs,
                               colorRef,
                               resolveRef,
                               resolveLoadInputRef,
                               depthStencilRef);
        skia_private::STArray<2, VkSubpassDependency> dependencies;
        populate_subpass_dependencies(context,
                                      dependencies,
                                      needLoadMSAAFromResolveSubpass,
                                      /*defaultSubpassDependency=*/VkSubpassDependency{});

        // Create VkRenderPass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.subpassCount = subpassDescs.size();
        renderPassInfo.pSubpasses = subpassDescs.begin();
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();
        renderPassInfo.attachmentCount = attachmentDescs.size();
        renderPassInfo.pAttachments = attachmentDescs.data();

        VULKAN_CALL_RESULT(
                context,
                result,
                CreateRenderPass(context->device(), &renderPassInfo, nullptr, &renderPass));
    }

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
                   /*gpuMemorySize=*/0,
                   /*reusableRequiresPurgeable=*/false)
        , fSharedContext(context)
        , fRenderPass(renderPass)
        , fGranularity(granularity) {}

void VulkanRenderPass::freeGpuData() {
    VULKAN_CALL(fSharedContext->interface(),
                DestroyRenderPass(fSharedContext->device(), fRenderPass, nullptr));
}

}  // namespace skgpu::graphite
