/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkRenderPass.h"

#include "GrProcessor.h"
#include "GrVkFramebuffer.h"
#include "GrVkGpu.h"
#include "GrVkRenderTarget.h"
#include "GrVkUtil.h"

void setup_simple_vk_attachment_description(VkAttachmentDescription* attachment,
                                            VkFormat format,
                                            uint32_t samples,
                                            VkImageLayout layout) {
    attachment->flags = 0;
    attachment->format = format;
    SkAssertResult(GrSampleCountToVkSampleCount(samples, &attachment->samples));
    attachment->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment->initialLayout = layout;
    attachment->finalLayout = layout;
}

void GrVkRenderPass::initSimple(const GrVkGpu* gpu, const GrVkRenderTarget& target) {
    // Get attachment information from render target. This includes which attachments the render
    // target has (color, resolve, stencil) and the attachments format and sample count.
    target.getAttachmentsDescriptor(&fAttachmentsDescriptor, &fAttachmentFlags);

    uint32_t numAttachments = fAttachmentsDescriptor.fAttachmentCount;
    // Attachment descriptions to be set on the render pass
    SkTArray<VkAttachmentDescription> attachments(numAttachments);
    attachments.reset(numAttachments);
    memset(attachments.begin(), 0, numAttachments*sizeof(VkAttachmentDescription));

    // Refs to attachments on the render pass (as described by teh VkAttachmentDescription above),
    // that are used by the subpass.
    VkAttachmentReference colorRef;
    VkAttachmentReference resolveRef;
    VkAttachmentReference stencilRef;
    uint32_t currentAttachment = 0;

    // Go through each of the attachment types (color, resolve, stencil) and set the necessary
    // on the various Vk structs.
    VkSubpassDescription subpassDesc;
    memset(&subpassDesc, 0, sizeof(VkSubpassDescription));
    subpassDesc.flags = 0;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    if (fAttachmentFlags & kColor_AttachmentFlag) {
        // set up color attachment
        setup_simple_vk_attachment_description(&attachments[currentAttachment],
                                               fAttachmentsDescriptor.fColor.fFormat,
                                               fAttachmentsDescriptor.fColor.fSamples,
                                               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        // setup subpass use of attachment
        colorRef.attachment = currentAttachment++;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpassDesc.colorAttachmentCount = 1;
    } else {
        // I don't think there should ever be a time where we don't have a color attachment
        SkASSERT(false);
        colorRef.attachment = VK_ATTACHMENT_UNUSED;
        colorRef.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        subpassDesc.colorAttachmentCount = 0;
    }
    subpassDesc.pColorAttachments = &colorRef;

    if (fAttachmentFlags & kResolve_AttachmentFlag) {
        // set up resolve attachment
        setup_simple_vk_attachment_description(&attachments[currentAttachment],
                                               fAttachmentsDescriptor.fResolve.fFormat,
                                               fAttachmentsDescriptor.fResolve.fSamples,
                                               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        // setup subpass use of attachment
        resolveRef.attachment = currentAttachment++;
        // I'm really not sure what the layout should be for the resolve textures.
        resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpassDesc.pResolveAttachments = &resolveRef;
    } else {
        subpassDesc.pResolveAttachments = nullptr;
    }

    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        // set up stencil attachment
        setup_simple_vk_attachment_description(&attachments[currentAttachment],
                                               fAttachmentsDescriptor.fStencil.fFormat,
                                               fAttachmentsDescriptor.fStencil.fSamples,
                                               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        // setup subpass use of attachment
        stencilRef.attachment = currentAttachment++;
        stencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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

    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateRenderPass(gpu->device(),
                                                             &createInfo,
                                                             nullptr,
                                                             &fRenderPass));
}

void GrVkRenderPass::freeGPUData(const GrVkGpu* gpu) const {
    GR_VK_CALL(gpu->vkInterface(), DestroyRenderPass(gpu->device(), fRenderPass, nullptr));
}

// Works under the assumption that color attachment will always be the first attachment in our
// attachment array if it exists.
bool GrVkRenderPass::colorAttachmentIndex(uint32_t* index) const {
    *index = 0;
    if (fAttachmentFlags & kColor_AttachmentFlag) {
        return true;
    }
    return false;
}

// Works under the assumption that resolve attachment will always be after the color attachment.
bool GrVkRenderPass::resolveAttachmentIndex(uint32_t* index) const {
    *index = 0;
    if (fAttachmentFlags & kColor_AttachmentFlag) {
        ++(*index);
    }
    if (fAttachmentFlags & kResolve_AttachmentFlag) {
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
    if (fAttachmentFlags & kResolve_AttachmentFlag) {
        ++(*index);
    }
    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        return true;
    }
    return false;
}

void GrVkRenderPass::getBeginInfo(const GrVkRenderTarget& target,
                                  VkRenderPassBeginInfo* beginInfo,
                                  VkSubpassContents* contents) const {
    SkASSERT(this->isCompatible(target));

    VkRect2D renderArea;
    renderArea.offset = { 0, 0 };
    renderArea.extent = { (uint32_t)target.width(), (uint32_t)target.height() };

    memset(beginInfo, 0, sizeof(VkRenderPassBeginInfo));
    beginInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo->pNext = nullptr;
    beginInfo->renderPass = fRenderPass;
    beginInfo->framebuffer = target.framebuffer()->framebuffer();
    beginInfo->renderArea = renderArea;
    beginInfo->clearValueCount = 0;
    beginInfo->pClearValues = nullptr;

    // Currently just assuming no secondary cmd buffers. This value will need to be update if we
    // have them.
    *contents = VK_SUBPASS_CONTENTS_INLINE;
}

bool GrVkRenderPass::isCompatible(const GrVkRenderTarget& target) const {
    AttachmentsDescriptor desc;
    AttachmentFlags flags;
    target.getAttachmentsDescriptor(&desc, &flags);

    if (flags != fAttachmentFlags) {
        return false;
    }

    if (fAttachmentFlags & kColor_AttachmentFlag) {
        if (fAttachmentsDescriptor.fColor != desc.fColor) {
            return false;
        }
    }
    if (fAttachmentFlags & kResolve_AttachmentFlag) {
        if (fAttachmentsDescriptor.fResolve != desc.fResolve) {
            return false;
        }
    }
    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        if (fAttachmentsDescriptor.fStencil != desc.fStencil) {
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
    if (fAttachmentFlags & kResolve_AttachmentFlag) {
        b->add32(fAttachmentsDescriptor.fResolve.fFormat);
        b->add32(fAttachmentsDescriptor.fResolve.fSamples);
    }
    if (fAttachmentFlags & kStencil_AttachmentFlag) {
        b->add32(fAttachmentsDescriptor.fStencil.fFormat);
        b->add32(fAttachmentsDescriptor.fStencil.fSamples);
    }
}
