/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkRenderPass.h"

#include "src/gpu/GrProcessor.h"
#include "src/gpu/vk/GrVkFramebuffer.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "src/gpu/vk/GrVkUtil.h"

typedef GrVkRenderPass::AttachmentsDescriptor::AttachmentDesc AttachmentDesc;

void setup_vk_attachment_description(VkAttachmentDescription* attachment,
                                     const AttachmentDesc& desc,
                                     VkImageLayout startLayout,
                                     VkImageLayout endLayout) {
    attachment->flags = 0;
    attachment->format = desc.fFormat;
    SkAssertResult(GrSampleCountToVkSampleCount(desc.fSamples, &attachment->samples));
    switch (startLayout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        case VK_IMAGE_LAYOUT_GENERAL:
            attachment->loadOp = desc.fLoadStoreOps.fLoadOp;
            attachment->storeOp = desc.fLoadStoreOps.fStoreOp;
            attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            attachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment->stencilLoadOp = desc.fLoadStoreOps.fLoadOp;
            attachment->stencilStoreOp = desc.fLoadStoreOps.fStoreOp;
            break;
        default:
            SK_ABORT("Unexpected attachment layout");
    }

    attachment->initialLayout = startLayout;
    attachment->finalLayout = endLayout == VK_IMAGE_LAYOUT_UNDEFINED ? startLayout : endLayout;
}

GrVkRenderPass* GrVkRenderPass::CreateSimple(GrVkGpu* gpu,
                                             AttachmentsDescriptor* attachmentsDescriptor,
                                             AttachmentFlags attachmentFlags,
                                             SelfDependencyFlags selfDepFlags,
                                             LoadFromResolve loadFromResolve) {
    static const GrVkRenderPass::LoadStoreOps kBasicLoadStoreOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                                                 VK_ATTACHMENT_STORE_OP_STORE);
    switch (loadFromResolve) {
        case LoadFromResolve::kNo:
            return Create(gpu, attachmentFlags, attachmentsDescriptor, kBasicLoadStoreOps,
                          kBasicLoadStoreOps, kBasicLoadStoreOps, selfDepFlags, loadFromResolve);
        case LoadFromResolve::kLoad: {
            static const GrVkRenderPass::LoadStoreOps kDiscardLoadStoreOps(
                    VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE);
            return Create(gpu, attachmentFlags, attachmentsDescriptor, kDiscardLoadStoreOps,
                          kBasicLoadStoreOps, kBasicLoadStoreOps, selfDepFlags, loadFromResolve);
        }
    }
    SkUNREACHABLE;
}

GrVkRenderPass* GrVkRenderPass::Create(GrVkGpu* gpu,
                                       const GrVkRenderPass& compatibleRenderPass,
                                       const LoadStoreOps& colorOp,
                                       const LoadStoreOps& resolveOp,
                                       const LoadStoreOps& stencilOp) {
    AttachmentFlags attachmentFlags = compatibleRenderPass.fAttachmentFlags;
    AttachmentsDescriptor attachmentsDescriptor = compatibleRenderPass.fAttachmentsDescriptor;
    SelfDependencyFlags selfDepFlags = compatibleRenderPass.fSelfDepFlags;
    LoadFromResolve loadFromResolve = compatibleRenderPass.fLoadFromResolve;
    return Create(gpu, attachmentFlags, &attachmentsDescriptor, colorOp, resolveOp, stencilOp,
                  selfDepFlags, loadFromResolve);
}

GrVkRenderPass* GrVkRenderPass::Create(GrVkGpu* gpu,
                                       AttachmentFlags attachmentFlags,
                                       AttachmentsDescriptor* attachmentsDescriptor,
                                       const LoadStoreOps& colorOp,
                                       const LoadStoreOps& resolveOp,
                                       const LoadStoreOps& stencilOp,
                                       SelfDependencyFlags selfDepFlags,
                                       LoadFromResolve loadFromResolve) {
    SkASSERT(!SkToBool(selfDepFlags & SelfDependencyFlags::kForNonCoherentAdvBlend) ||
             gpu->caps()->advancedBlendEquationSupport());
    SkASSERT(!SkToBool(selfDepFlags & SelfDependencyFlags::kForInputAttachment) ||
             gpu->caps()->textureBarrierSupport());

    // If we have a resolve attachment, we will always do a resolve into it. Thus it doesn't make
    // sense not to store the resolve attachment at the end of the render pass.
    //
    // Currently today (when not using discardable msaa images) we load and store the the msaa image
    // and then use the copy resolve command to handle the resolving. If instead we moved to doing
    // the resolving at the end of the last render pass, we would probably want a separate flag
    // for having a resolve attachment versus actually doing the resolving. This would allow us to
    // use the same VkPiplines for render passes where we resolve and those we don't since each will
    // always have the resolve attachment. The actual resolving or not does not affect render pass
    // compatibility if there is only one sub pass, just the presence of the attachment or not.
    SkASSERT(!SkToBool(attachmentFlags & kResolve_AttachmentFlag) ||
             resolveOp.fStoreOp == VK_ATTACHMENT_STORE_OP_STORE);

    SkASSERT(loadFromResolve == LoadFromResolve::kNo ||
             (SkToBool(attachmentFlags & kColor_AttachmentFlag) &&
              SkToBool(attachmentFlags & kResolve_AttachmentFlag)));

#ifdef SK_DEBUG
    if (loadFromResolve == LoadFromResolve::kLoad) {
        // If we are loading the resolve image into the msaa color attachment then we should not be
        // loading or storing the msaa attachment. Additionally we need to make sure we are loading
        // the resolve so it can be copied into the msaa color attachment.
        SkASSERT(colorOp.fLoadOp == VK_ATTACHMENT_LOAD_OP_DONT_CARE);
        SkASSERT(colorOp.fStoreOp == VK_ATTACHMENT_STORE_OP_DONT_CARE);
        SkASSERT(resolveOp.fLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD);
    }
#endif

    uint32_t numAttachments = attachmentsDescriptor->fAttachmentCount;
    // Attachment descriptions to be set on the render pass
    SkTArray<VkAttachmentDescription> attachments(numAttachments);
    attachments.reset(numAttachments);
    memset(attachments.begin(), 0, numAttachments * sizeof(VkAttachmentDescription));

    // Refs to attachments on the render pass (as described by the VkAttachmentDescription above),
    // that are used by the subpass.
    VkAttachmentReference colorRef;
    VkAttachmentReference resolveRef;
    VkAttachmentReference resolveLoadInputRef;
    VkAttachmentReference stencilRef;
    uint32_t currentAttachment = 0;

    // Go through each of the attachment types (color, stencil) and set the necessary
    // on the various Vk structs.
    VkSubpassDescription subpassDescs[2];
    memset(subpassDescs, 0, 2*sizeof(VkSubpassDescription));
    const int mainSubpass = loadFromResolve == LoadFromResolve::kLoad ? 1 : 0;
    VkSubpassDescription& subpassDescMain = subpassDescs[mainSubpass];
    subpassDescMain.flags = 0;
    subpassDescMain.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescMain.inputAttachmentCount = 0;
    subpassDescMain.pInputAttachments = nullptr;
    subpassDescMain.pResolveAttachments = nullptr;

    uint32_t clearValueCount = 0;

    VkSubpassDependency dependencies[2];
    int currentDependency = 0;

    if (attachmentFlags & kColor_AttachmentFlag) {
        // set up color attachment
        bool needsGeneralLayout = SkToBool(selfDepFlags & SelfDependencyFlags::kForInputAttachment);
        VkImageLayout layout = needsGeneralLayout ? VK_IMAGE_LAYOUT_GENERAL
                                                  : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachmentsDescriptor->fColor.fLoadStoreOps = colorOp;

        setup_vk_attachment_description(&attachments[currentAttachment],
                                        attachmentsDescriptor->fColor,
                                        layout, layout);
        // setup subpass use of attachment
        colorRef.attachment = currentAttachment++;
        colorRef.layout = layout;
        subpassDescMain.colorAttachmentCount = 1;

        if (selfDepFlags != SelfDependencyFlags::kNone) {
            VkSubpassDependency& dependency = dependencies[currentDependency++];
            dependency.srcSubpass = mainSubpass;
            dependency.dstSubpass = mainSubpass;
            dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency.dstStageMask = 0;
            dependency.dstAccessMask = 0;

            if (selfDepFlags & SelfDependencyFlags::kForNonCoherentAdvBlend) {
                // If we have coherent support we shouldn't be needing a self dependency
                SkASSERT(!gpu->caps()->advancedCoherentBlendEquationSupport());
                dependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
            }
            if (selfDepFlags & SelfDependencyFlags::kForInputAttachment) {
                SkASSERT(gpu->vkCaps().maxInputAttachmentDescriptors());

                subpassDescMain.inputAttachmentCount = 1;
                subpassDescMain.pInputAttachments = &colorRef;

                dependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependency.dstAccessMask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            }
        }

        if (VK_ATTACHMENT_LOAD_OP_CLEAR == colorOp.fLoadOp) {
            clearValueCount = colorRef.attachment + 1;
        }
    } else {
        // I don't think there should ever be a time where we don't have a color attachment
        SkASSERT(false);
        SkASSERT(selfDepFlags == SelfDependencyFlags::kNone);
        colorRef.attachment = VK_ATTACHMENT_UNUSED;
        colorRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        subpassDescMain.colorAttachmentCount = 0;
    }

    subpassDescMain.pColorAttachments = &colorRef;

    if (attachmentFlags & kResolve_AttachmentFlag) {
        attachmentsDescriptor->fResolve.fLoadStoreOps = resolveOp;

        VkImageLayout layout = loadFromResolve == LoadFromResolve::kLoad
                                       ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                       : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        setup_vk_attachment_description(&attachments[currentAttachment],
                                        attachmentsDescriptor->fResolve,
                                        layout,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // setup main subpass use of attachment
        resolveRef.attachment = currentAttachment++;
        resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        subpassDescMain.pResolveAttachments = &resolveRef;

        // Setup the load subpass and set subpass dependendcies
        if (loadFromResolve == LoadFromResolve::kLoad) {
            resolveLoadInputRef.attachment = resolveRef.attachment;
            resolveLoadInputRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            // The load subpass will always be the first
            VkSubpassDescription& subpassDescLoad = subpassDescs[0];
            subpassDescLoad.flags = 0;
            subpassDescLoad.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescLoad.inputAttachmentCount = 1;
            subpassDescLoad.pInputAttachments = &resolveLoadInputRef;
            subpassDescLoad.colorAttachmentCount = 1;
            subpassDescLoad.pColorAttachments = &colorRef;
            subpassDescLoad.pResolveAttachments = nullptr;
            subpassDescLoad.pDepthStencilAttachment = nullptr;
            subpassDescLoad.preserveAttachmentCount = 0;
            subpassDescLoad.pPreserveAttachments = nullptr;

            VkSubpassDependency& dependency = dependencies[currentDependency++];
            dependency.srcSubpass = 0;
            dependency.dstSubpass = mainSubpass;
            dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstAccessMask =
                    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        }
    }


    if (attachmentFlags & kStencil_AttachmentFlag) {
        // set up stencil attachment
        attachmentsDescriptor->fStencil.fLoadStoreOps = stencilOp;
        setup_vk_attachment_description(&attachments[currentAttachment],
                                        attachmentsDescriptor->fStencil,
                                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        // setup subpass use of attachment
        stencilRef.attachment = currentAttachment++;
        stencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        if (VK_ATTACHMENT_LOAD_OP_CLEAR == stencilOp.fLoadOp) {
            clearValueCount = std::max(clearValueCount, stencilRef.attachment + 1);
        }
    } else {
        stencilRef.attachment = VK_ATTACHMENT_UNUSED;
        stencilRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    subpassDescMain.pDepthStencilAttachment = &stencilRef;

    subpassDescMain.preserveAttachmentCount = 0;
    subpassDescMain.pPreserveAttachments = nullptr;

    SkASSERT(numAttachments == currentAttachment);

    uint32_t subpassCount = loadFromResolve == LoadFromResolve::kLoad ? 2 : 1;

    // Create the VkRenderPass compatible with the attachment descriptions above
    VkRenderPassCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkRenderPassCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = numAttachments;
    createInfo.pAttachments = attachments.begin();
    createInfo.subpassCount = subpassCount;
    createInfo.pSubpasses = subpassDescs;
    createInfo.dependencyCount = currentDependency;
    createInfo.pDependencies = dependencies;

    VkResult result;
    VkRenderPass renderPass;
    GR_VK_CALL_RESULT(gpu, result, CreateRenderPass(gpu->device(),
                                                    &createInfo,
                                                    nullptr,
                                                    &renderPass));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    VkExtent2D granularity;
    // Get granularity for this render pass
    GR_VK_CALL(gpu->vkInterface(), GetRenderAreaGranularity(gpu->device(),
                                                            renderPass,
                                                            &granularity));

    return new GrVkRenderPass(gpu, renderPass, attachmentFlags, *attachmentsDescriptor,
                              selfDepFlags, loadFromResolve, granularity, clearValueCount);
}

GrVkRenderPass::GrVkRenderPass(const GrVkGpu* gpu, VkRenderPass renderPass, AttachmentFlags flags,
                               const AttachmentsDescriptor& descriptor,
                               SelfDependencyFlags selfDepFlags,
                               LoadFromResolve loadFromResolve,
                               const VkExtent2D& granularity, uint32_t clearValueCount)
        : INHERITED(gpu)
        , fRenderPass(renderPass)
        , fAttachmentFlags(flags)
        , fAttachmentsDescriptor(descriptor)
        , fSelfDepFlags(selfDepFlags)
        , fLoadFromResolve(loadFromResolve)
        , fGranularity(granularity)
        , fClearValueCount(clearValueCount) {
}

void GrVkRenderPass::freeGPUData() const {
    if (!(fAttachmentFlags & kExternal_AttachmentFlag)) {
        GR_VK_CALL(fGpu->vkInterface(), DestroyRenderPass(fGpu->device(), fRenderPass, nullptr));
    }
}

bool GrVkRenderPass::colorAttachmentIndex(uint32_t* index) const {
    *index = fColorAttachmentIndex;
    if ((fAttachmentFlags & kColor_AttachmentFlag) ||
        (fAttachmentFlags & kExternal_AttachmentFlag)) {
        return true;
    }
    return false;
}

// Works under the assumption that stencil attachment will always be after the color and resolve
// attachments.
bool GrVkRenderPass::stencilAttachmentIndex(uint32_t* index) const {
    *index = 0;
    if (fAttachmentFlags & kColor_AttachmentFlag) {
        ++(*index);
    }
    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        return true;
    }
    return false;
}

bool GrVkRenderPass::isCompatible(const AttachmentsDescriptor& desc,
                                  const AttachmentFlags& flags,
                                  SelfDependencyFlags selfDepFlags,
                                  LoadFromResolve loadFromResolve) const {
    SkASSERT(!(fAttachmentFlags & kExternal_AttachmentFlag));
    if (flags != fAttachmentFlags) {
        return false;
    }

    if (fAttachmentFlags & kColor_AttachmentFlag) {
        if (!fAttachmentsDescriptor.fColor.isCompatible(desc.fColor)) {
            return false;
        }
    }
    if (fAttachmentFlags & kResolve_AttachmentFlag) {
        if (!fAttachmentsDescriptor.fResolve.isCompatible(desc.fResolve)) {
            return false;
        }
    }
    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        if (!fAttachmentsDescriptor.fStencil.isCompatible(desc.fStencil)) {
            return false;
        }
    }

    if (fSelfDepFlags != selfDepFlags) {
        return false;
    }

    if (fLoadFromResolve != loadFromResolve) {
        return false;
    }

    return true;
}

bool GrVkRenderPass::isCompatible(const GrVkRenderTarget& target,
                                  SelfDependencyFlags selfDepFlags,
                                  LoadFromResolve loadFromResolve) const {
    SkASSERT(!(fAttachmentFlags & kExternal_AttachmentFlag));

    AttachmentsDescriptor desc;
    AttachmentFlags flags;
    target.getAttachmentsDescriptor(&desc, &flags, this->hasResolveAttachment(),
                                    this->hasStencilAttachment());

    return this->isCompatible(desc, flags, selfDepFlags, loadFromResolve);
}

bool GrVkRenderPass::isCompatible(const GrVkRenderPass& renderPass) const {
    SkASSERT(!(fAttachmentFlags & kExternal_AttachmentFlag));
    return this->isCompatible(renderPass.fAttachmentsDescriptor, renderPass.fAttachmentFlags,
                              renderPass.fSelfDepFlags, renderPass.fLoadFromResolve);
}

bool GrVkRenderPass::isCompatibleExternalRP(VkRenderPass renderPass) const {
    SkASSERT(fAttachmentFlags & kExternal_AttachmentFlag);
    return fRenderPass == renderPass;
}

bool GrVkRenderPass::equalLoadStoreOps(const LoadStoreOps& colorOps,
                                       const LoadStoreOps& resolveOps,
                                       const LoadStoreOps& stencilOps) const {
    SkASSERT(!(fAttachmentFlags & kExternal_AttachmentFlag));
    if (fAttachmentFlags & kColor_AttachmentFlag) {
        if (fAttachmentsDescriptor.fColor.fLoadStoreOps != colorOps) {
            return false;
        }
    }
    if (fAttachmentFlags & kResolve_AttachmentFlag) {
        if (fAttachmentsDescriptor.fResolve.fLoadStoreOps != resolveOps) {
            return false;
        }
    }
    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        if (fAttachmentsDescriptor.fStencil.fLoadStoreOps != stencilOps) {
            return false;
        }
    }
    return true;
}

void GrVkRenderPass::genKey(GrProcessorKeyBuilder* b) const {
    GenKey(b, fAttachmentFlags, fAttachmentsDescriptor, fSelfDepFlags,
           fLoadFromResolve, (uint64_t)fRenderPass);
}

void GrVkRenderPass::GenKey(GrProcessorKeyBuilder* b,
                            AttachmentFlags attachmentFlags,
                            const AttachmentsDescriptor& attachmentsDescriptor,
                            SelfDependencyFlags selfDepFlags,
                            LoadFromResolve loadFromResolve,
                            uint64_t externalRenderPass) {
    b->add32(attachmentFlags);
    if (attachmentFlags & kColor_AttachmentFlag) {
        b->add32(attachmentsDescriptor.fColor.fFormat);
        b->add32(attachmentsDescriptor.fColor.fSamples);
    }
    if (attachmentFlags & kResolve_AttachmentFlag) {
        b->add32(attachmentsDescriptor.fResolve.fFormat);
        b->add32(attachmentsDescriptor.fResolve.fSamples);
    }
    if (attachmentFlags & kStencil_AttachmentFlag) {
        b->add32(attachmentsDescriptor.fStencil.fFormat);
        b->add32(attachmentsDescriptor.fStencil.fSamples);
    }

    uint32_t extraFlags = (uint32_t)selfDepFlags;
    SkASSERT(extraFlags < (1 << 30));
    SkASSERT((uint32_t)loadFromResolve <= 2);
    extraFlags |= ((uint32_t)loadFromResolve << 30);

    b->add32(extraFlags);

    if (attachmentFlags & kExternal_AttachmentFlag) {
        SkASSERT(!(attachmentFlags & ~kExternal_AttachmentFlag));
        b->add32((uint32_t)(externalRenderPass & 0xFFFFFFFF));
        b->add32((uint32_t)(externalRenderPass>>32));
    }
}
