/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkFramebuffer.h"

#include "src/gpu/vk/GrVkAttachment.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkRenderPass.h"

GrVkFramebuffer* GrVkFramebuffer::Create(
        GrVkGpu* gpu,
        int width, int height,
        const GrVkRenderPass* renderPass,
        const GrVkAttachment* colorAttachment,
        const GrVkAttachment* resolveAttachment,
        const GrVkAttachment* stencilAttachment,
        GrVkResourceProvider::CompatibleRPHandle compatibleRenderPassHandle) {
    // At the very least we need a renderPass and a colorAttachment
    SkASSERT(renderPass);
    SkASSERT(colorAttachment);

    VkImageView attachments[3];
    attachments[0] = colorAttachment->framebufferView()->imageView();
    int numAttachments = 1;
    if (resolveAttachment) {
        attachments[numAttachments++] = resolveAttachment->framebufferView()->imageView();
    }
    if (stencilAttachment) {
        attachments[numAttachments++] = stencilAttachment->framebufferView()->imageView();
    }

    VkFramebufferCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkFramebufferCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.renderPass = renderPass->vkRenderPass();
    createInfo.attachmentCount = numAttachments;
    createInfo.pAttachments = attachments;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;

    VkFramebuffer framebuffer;
    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, CreateFramebuffer(gpu->device(), &createInfo, nullptr,
                                                  &framebuffer));
    if (err) {
        return nullptr;
    }

    return new GrVkFramebuffer(gpu, framebuffer, sk_ref_sp(colorAttachment),
                               sk_ref_sp(resolveAttachment), sk_ref_sp(stencilAttachment),
                               compatibleRenderPassHandle);
}

GrVkFramebuffer::~GrVkFramebuffer() {}

void GrVkFramebuffer::freeGPUData() const {
    SkASSERT(fFramebuffer);
    GR_VK_CALL(fGpu->vkInterface(), DestroyFramebuffer(fGpu->device(), fFramebuffer, nullptr));
}
