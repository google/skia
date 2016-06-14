/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkGpuCommandBuffer.h"

#include "GrVkCommandBuffer.h"
#include "GrVkGpu.h"
#include "GrVkRenderPass.h"
#include "GrVkRenderTarget.h"
#include "GrVkResourceProvider.h"

void get_vk_load_store_ops(GrGpuCommandBuffer::LoadAndStoreOp op,
                           VkAttachmentLoadOp* loadOp, VkAttachmentStoreOp* storeOp) {
    switch (op) {
        case GrGpuCommandBuffer::kLoadAndStore_LoadAndStoreOp:
            *loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            *storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case GrGpuCommandBuffer::kLoadAndDiscard_LoadAndStoreOp:
            *loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            *storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        case GrGpuCommandBuffer::kClearAndStore_LoadAndStoreOp:
            *loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            *storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case GrGpuCommandBuffer::kClearAndDiscard_LoadAndStoreOp:
            *loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            *storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        case GrGpuCommandBuffer::kDiscardAndStore_LoadAndStoreOp:
            *loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            *storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case GrGpuCommandBuffer::kDiscardAndDiscard_LoadAndStoreOp:
            *loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            *storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        default:
            SK_ABORT("Invalid LoadAndStoreOp");
            *loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            *storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            break;
    }
}

GrVkGpuCommandBuffer::GrVkGpuCommandBuffer(GrVkGpu* gpu,
                                           const GrVkRenderTarget& target,
                                           LoadAndStoreOp colorOp, GrColor colorClear,
                                           LoadAndStoreOp stencilOp, GrColor stencilClear)
    : fGpu(gpu) {
    VkAttachmentLoadOp vkLoadOp;
    VkAttachmentStoreOp vkStoreOp;

    get_vk_load_store_ops(colorOp, &vkLoadOp, &vkStoreOp);
    GrVkRenderPass::LoadStoreOps vkColorOps(vkLoadOp, vkStoreOp);

    get_vk_load_store_ops(stencilOp, &vkLoadOp, &vkStoreOp);
    GrVkRenderPass::LoadStoreOps vkStencilOps(vkLoadOp, vkStoreOp);
    
    GrVkRenderPass::LoadStoreOps vkResolveOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                              VK_ATTACHMENT_STORE_OP_STORE);

    const GrVkResourceProvider::CompatibleRPHandle& rpHandle = target.compatibleRenderPassHandle();
    if (rpHandle.isValid()) {
        fRenderPass = fGpu->resourceProvider().findRenderPass(rpHandle,
                                                              vkColorOps,
                                                              vkResolveOps,
                                                              vkStencilOps);
    } else {
        fRenderPass = fGpu->resourceProvider().findRenderPass(target,
                                                              vkColorOps,
                                                              vkResolveOps,
                                                              vkStencilOps);
    }

    fCommandBuffer = GrVkSecondaryCommandBuffer::Create(gpu, gpu->cmdPool(), fRenderPass);
    fCommandBuffer->begin(gpu, target.framebuffer());
}

GrVkGpuCommandBuffer::~GrVkGpuCommandBuffer() {
    fCommandBuffer->unref(fGpu);
    fRenderPass->unref(fGpu);
}

void GrVkGpuCommandBuffer::end() {
    fCommandBuffer->end(fGpu);
}

void GrVkGpuCommandBuffer::submit() {
    fGpu->submitSecondaryCommandBuffer(fCommandBuffer);
}

