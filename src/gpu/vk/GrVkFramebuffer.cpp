/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkFramebuffer.h"

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkRenderPass.h"

GrVkFramebuffer* GrVkFramebuffer::Create(GrVkGpu* gpu,
                                         int width, int height,
                                         const GrVkRenderPass* renderPass,
                                         const GrVkImageView* colorAttachment,
                                         const GrVkImageView* stencilAttachment) {
    // At the very least we need a renderPass and a colorAttachment
    SkASSERT(renderPass);
    SkASSERT(colorAttachment);

    VkImageView attachments[3];
    attachments[0] = colorAttachment->imageView();
    int numAttachments = 1;
    if (stencilAttachment) {
        attachments[numAttachments++] = stencilAttachment->imageView();
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
    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateFramebuffer(gpu->device(),
                                                                    &createInfo,
                                                                    nullptr,
                                                                    &framebuffer));
    if (err) {
        return nullptr;
    }

    return new GrVkFramebuffer(framebuffer);
}

void GrVkFramebuffer::freeGPUData(GrVkGpu* gpu) const {
    SkASSERT(fFramebuffer);
    GR_VK_CALL(gpu->vkInterface(), DestroyFramebuffer(gpu->device(), fFramebuffer, nullptr));
}
