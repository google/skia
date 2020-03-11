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
                                     VkImageLayout layout) {
    attachment->flags = 0;
    attachment->format = desc.fFormat;
    SkAssertResult(GrSampleCountToVkSampleCount(desc.fSamples, &attachment->samples));
    switch (layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
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

    attachment->initialLayout = layout;
    attachment->finalLayout = layout;
}

GrVkRenderPass* GrVkRenderPass::CreateSimple(GrVkGpu* gpu, const GrVkRenderTarget& target) {
    static const GrVkRenderPass::LoadStoreOps kBasicLoadStoreOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                                                 VK_ATTACHMENT_STORE_OP_STORE);

    AttachmentFlags attachmentFlags;
    AttachmentsDescriptor attachmentsDescriptor;
    // Get attachment information from render target. This includes which attachments the render
    // target has (color, stencil) and the attachments format and sample count.
    target.getAttachmentsDescriptor(&attachmentsDescriptor, &attachmentFlags);
    return Create(gpu, attachmentFlags, attachmentsDescriptor, kBasicLoadStoreOps,
                  kBasicLoadStoreOps);
}

GrVkRenderPass* GrVkRenderPass::Create(GrVkGpu* gpu,
                            const GrVkRenderPass& compatibleRenderPass,
                            const LoadStoreOps& colorOp,
                            const LoadStoreOps& stencilOp) {
    AttachmentFlags attachmentFlags = compatibleRenderPass.fAttachmentFlags;
    AttachmentsDescriptor attachmentsDescriptor = compatibleRenderPass.fAttachmentsDescriptor;
    return Create(gpu, attachmentFlags, attachmentsDescriptor, colorOp, stencilOp);
}

GrVkRenderPass* GrVkRenderPass::Create(GrVkGpu* gpu,
                                       AttachmentFlags attachmentFlags,
                                       AttachmentsDescriptor& attachmentsDescriptor,
                                       const LoadStoreOps& colorOp,
                                       const LoadStoreOps& stencilOp) {
    uint32_t numAttachments = attachmentsDescriptor.fAttachmentCount;
    // Attachment descriptions to be set on the render pass
    SkTArray<VkAttachmentDescription> attachments(numAttachments);
    attachments.reset(numAttachments);
    memset(attachments.begin(), 0, numAttachments * sizeof(VkAttachmentDescription));

    // Refs to attachments on the render pass (as described by teh VkAttachmentDescription above),
    // that are used by the subpass.
    VkAttachmentReference colorRef;
    VkAttachmentReference stencilRef;
    uint32_t currentAttachment = 0;

    // Go through each of the attachment types (color, stencil) and set the necessary
    // on the various Vk structs.
    VkSubpassDescription subpassDesc;
    memset(&subpassDesc, 0, sizeof(VkSubpassDescription));
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.pResolveAttachments = nullptr;

    uint32_t clearValueCount = 0;

    if (attachmentFlags & kColor_AttachmentFlag) {
        // set up color attachment
        attachmentsDescriptor.fColor.fLoadStoreOps = colorOp;
        setup_vk_attachment_description(&attachments[currentAttachment],
                                        attachmentsDescriptor.fColor,
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        // setup subpass use of attachment
        colorRef.attachment = currentAttachment++;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpassDesc.colorAttachmentCount = 1;

        if (VK_ATTACHMENT_LOAD_OP_CLEAR == colorOp.fLoadOp) {
            clearValueCount = colorRef.attachment + 1;
        }
    } else {
        // I don't think there should ever be a time where we don't have a color attachment
        SkASSERT(false);
        colorRef.attachment = VK_ATTACHMENT_UNUSED;
        colorRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        subpassDesc.colorAttachmentCount = 0;
    }
    subpassDesc.pColorAttachments = &colorRef;

    if (attachmentFlags & kStencil_AttachmentFlag) {
        // set up stencil attachment
        attachmentsDescriptor.fStencil.fLoadStoreOps = stencilOp;
        setup_vk_attachment_description(&attachments[currentAttachment],
                                        attachmentsDescriptor.fStencil,
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
    subpassDesc.pDepthStencilAttachment = &stencilRef;

    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;

    SkASSERT(numAttachments == currentAttachment);

    // Create the VkRenderPass compatible with the attachment descriptions above
    VkRenderPassCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkRenderPassCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = numAttachments;
    createInfo.pAttachments = attachments.begin();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpassDesc;
    createInfo.dependencyCount = 0;
    createInfo.pDependencies = nullptr;

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

    return new GrVkRenderPass(gpu, renderPass, attachmentFlags, attachmentsDescriptor, granularity,
                              clearValueCount);
}

GrVkRenderPass::GrVkRenderPass(const GrVkGpu* gpu, VkRenderPass renderPass, AttachmentFlags flags,
                               const AttachmentsDescriptor& descriptor,
                               const VkExtent2D& granularity, uint32_t clearValueCount)
        : INHERITED(gpu)
        , fRenderPass(renderPass)
        , fAttachmentFlags(flags)
        , fAttachmentsDescriptor(descriptor)
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
// attachment.
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
                                  const AttachmentFlags& flags) const {
    SkASSERT(!(fAttachmentFlags & kExternal_AttachmentFlag));
    if (flags != fAttachmentFlags) {
        return false;
    }

    if (fAttachmentFlags & kColor_AttachmentFlag) {
        if (!fAttachmentsDescriptor.fColor.isCompatible(desc.fColor)) {
            return false;
        }
    }
    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        if (!fAttachmentsDescriptor.fStencil.isCompatible(desc.fStencil)) {
            return false;
        }
    }

    return true;
}

bool GrVkRenderPass::isCompatible(const GrVkRenderTarget& target) const {
    SkASSERT(!(fAttachmentFlags & kExternal_AttachmentFlag));
    AttachmentsDescriptor desc;
    AttachmentFlags flags;
    target.getAttachmentsDescriptor(&desc, &flags);

    return this->isCompatible(desc, flags);
}

bool GrVkRenderPass::isCompatible(const GrVkRenderPass& renderPass) const {
    SkASSERT(!(fAttachmentFlags & kExternal_AttachmentFlag));
    return this->isCompatible(renderPass.fAttachmentsDescriptor, renderPass.fAttachmentFlags);
}

bool GrVkRenderPass::isCompatibleExternalRP(VkRenderPass renderPass) const {
    SkASSERT(fAttachmentFlags & kExternal_AttachmentFlag);
    return fRenderPass == renderPass;
}

bool GrVkRenderPass::equalLoadStoreOps(const LoadStoreOps& colorOps,
                                       const LoadStoreOps& stencilOps) const {
    SkASSERT(!(fAttachmentFlags & kExternal_AttachmentFlag));
    if (fAttachmentFlags & kColor_AttachmentFlag) {
        if (fAttachmentsDescriptor.fColor.fLoadStoreOps != colorOps) {
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
    b->add32(fAttachmentFlags);
    if (fAttachmentFlags & kColor_AttachmentFlag) {
        b->add32(fAttachmentsDescriptor.fColor.fFormat);
        b->add32(fAttachmentsDescriptor.fColor.fSamples);
    }
    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        b->add32(fAttachmentsDescriptor.fStencil.fFormat);
        b->add32(fAttachmentsDescriptor.fStencil.fSamples);
    }
    if (fAttachmentFlags & kExternal_AttachmentFlag) {
        SkASSERT(!(fAttachmentFlags & ~kExternal_AttachmentFlag));
        uint64_t handle = (uint64_t)fRenderPass;
        b->add32((uint32_t)(handle & 0xFFFFFFFF));
        b->add32((uint32_t)(handle>>32));
    }
}
