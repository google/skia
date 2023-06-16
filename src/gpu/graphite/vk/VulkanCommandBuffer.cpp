/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/DescriptorTypes.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"

#define SK_DISABLE_VULKAN_RENDERING

using namespace skia_private;

namespace skgpu::graphite {

class VulkanDescriptorSet;

std::unique_ptr<VulkanCommandBuffer> VulkanCommandBuffer::Make(
        const VulkanSharedContext* sharedContext,
        VulkanResourceProvider* resourceProvider) {
    // Create VkCommandPool
    VkCommandPoolCreateFlags cmdPoolCreateFlags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (sharedContext->isProtected() == Protected::kYes) {
        cmdPoolCreateFlags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    }

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
        nullptr,                                     // pNext
        cmdPoolCreateFlags,                          // CmdPoolCreateFlags
        sharedContext->queueIndex(),                 // queueFamilyIndex
    };
    auto interface = sharedContext->interface();
    VkResult result;
    VkCommandPool pool;
    VULKAN_CALL_RESULT(interface, result, CreateCommandPool(sharedContext->device(),
                                                            &cmdPoolInfo,
                                                            nullptr,
                                                            &pool));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    const VkCommandBufferAllocateInfo cmdInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
        nullptr,                                          // pNext
        pool,                                             // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
        1                                                 // bufferCount
    };

    VkCommandBuffer primaryCmdBuffer;
    VULKAN_CALL_RESULT(interface, result, AllocateCommandBuffers(sharedContext->device(),
                                                                 &cmdInfo,
                                                                 &primaryCmdBuffer));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(interface, DestroyCommandPool(sharedContext->device(), pool, nullptr));
        return nullptr;
    }

    return std::unique_ptr<VulkanCommandBuffer>(new VulkanCommandBuffer(pool,
                                                                        primaryCmdBuffer,
                                                                        sharedContext,
                                                                        resourceProvider));
}

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool pool,
                                         VkCommandBuffer primaryCommandBuffer,
                                         const VulkanSharedContext* sharedContext,
                                         VulkanResourceProvider* resourceProvider)
        : fPool(pool)
        , fPrimaryCommandBuffer(primaryCommandBuffer)
        , fSharedContext(sharedContext)
        , fResourceProvider(resourceProvider) {
    // When making a new command buffer, we automatically begin the command buffer
    this->begin();
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    if (fActive) {
        // Need to end command buffer before deleting it
        VULKAN_CALL(fSharedContext->interface(), EndCommandBuffer(fPrimaryCommandBuffer));
        fActive = false;
    }

    if (VK_NULL_HANDLE != fSubmitFence) {
        VULKAN_CALL(fSharedContext->interface(), DestroyFence(fSharedContext->device(),
                                                              fSubmitFence,
                                                              nullptr));
    }
    // This should delete any command buffers as well.
    VULKAN_CALL(fSharedContext->interface(), DestroyCommandPool(fSharedContext->device(),
                                                                fPool,
                                                                nullptr));
}

void VulkanCommandBuffer::onResetCommandBuffer() {
    SkASSERT(!fActive);
    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), ResetCommandPool(fSharedContext->device(),
                                                                       fPool,
                                                                       0));
    fActiveGraphicsPipeline = nullptr;
    fBindUniformBuffers = true;
    fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
    fUniformBuffersToBind.clear();
}

bool VulkanCommandBuffer::setNewCommandBufferResources() {
    this->begin();
    return true;
}

void VulkanCommandBuffer::begin() {
    SkASSERT(!fActive);
    VkCommandBufferBeginInfo cmdBufferBeginInfo;
    memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.pNext = nullptr;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmdBufferBeginInfo.pInheritanceInfo = nullptr;

    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), BeginCommandBuffer(fPrimaryCommandBuffer,
                                                                         &cmdBufferBeginInfo));
    fActive = true;
}

void VulkanCommandBuffer::end() {
    SkASSERT(fActive);
    SkASSERT(!fActiveRenderPass);

    this->submitPipelineBarriers();

    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), EndCommandBuffer(fPrimaryCommandBuffer));

    fActive = false;
}

static bool submit_to_queue(const VulkanInterface* interface,
                            VkQueue queue,
                            VkFence fence,
                            uint32_t waitCount,
                            const VkSemaphore* waitSemaphores,
                            const VkPipelineStageFlags* waitStages,
                            uint32_t commandBufferCount,
                            const VkCommandBuffer* commandBuffers,
                            uint32_t signalCount,
                            const VkSemaphore* signalSemaphores,
                            Protected protectedContext) {
    VkProtectedSubmitInfo protectedSubmitInfo;
    if (protectedContext == Protected::kYes) {
        memset(&protectedSubmitInfo, 0, sizeof(VkProtectedSubmitInfo));
        protectedSubmitInfo.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.pNext = nullptr;
        protectedSubmitInfo.protectedSubmit = VK_TRUE;
    }

    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = protectedContext == Protected::kYes ? &protectedSubmitInfo : nullptr;
    submitInfo.waitSemaphoreCount = waitCount;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = commandBuffers;
    submitInfo.signalSemaphoreCount = signalCount;
    submitInfo.pSignalSemaphores = signalSemaphores;
    VkResult result;
    VULKAN_CALL_RESULT(interface, result, QueueSubmit(queue, 1, &submitInfo, fence));
    if (result != VK_SUCCESS) {
        return false;
    }
    return true;
}

bool VulkanCommandBuffer::submit(VkQueue queue) {
    this->end();

    auto interface = fSharedContext->interface();
    auto device = fSharedContext->device();
    VkResult err;

    if (fSubmitFence == VK_NULL_HANDLE) {
        VkFenceCreateInfo fenceInfo;
        memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VULKAN_CALL_RESULT(interface, err, CreateFence(device,
                                                       &fenceInfo,
                                                       nullptr,
                                                       &fSubmitFence));
        if (err) {
            fSubmitFence = VK_NULL_HANDLE;
            return false;
        }
    } else {
        // This cannot return DEVICE_LOST so we assert we succeeded.
        VULKAN_CALL_RESULT(interface, err, ResetFences(device, 1, &fSubmitFence));
        SkASSERT(err == VK_SUCCESS);
    }

    SkASSERT(fSubmitFence != VK_NULL_HANDLE);

    bool submitted = submit_to_queue(interface,
                                     queue,
                                     fSubmitFence,
                                     /*waitCount=*/0,
                                     /*waitSemaphores=*/nullptr,
                                     /*waitStages=*/nullptr,
                                     /*commandBufferCount*/1,
                                     &fPrimaryCommandBuffer,
                                     /*signalCount=*/0,
                                     /*signalSemaphores=*/nullptr,
                                     fSharedContext->isProtected());
    if (!submitted) {
        // Destroy the fence or else we will try to wait forever for it to finish.
        VULKAN_CALL(interface, DestroyFence(device, fSubmitFence, nullptr));
        fSubmitFence = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

bool VulkanCommandBuffer::isFinished() {
    SkASSERT(!fActive);
    if (VK_NULL_HANDLE == fSubmitFence) {
        return true;
    }

    VkResult err;
    VULKAN_CALL_RESULT_NOCHECK(fSharedContext->interface(), err,
                               GetFenceStatus(fSharedContext->device(), fSubmitFence));
    switch (err) {
        case VK_SUCCESS:
        case VK_ERROR_DEVICE_LOST:
            return true;

        case VK_NOT_READY:
            return false;

        default:
            SKGPU_LOG_F("Error calling vkGetFenceStatus. Error: %d", err);
            SK_ABORT("Got an invalid fence status");
            return false;
    }
}

void VulkanCommandBuffer::waitUntilFinished() {
    if (fSubmitFence == VK_NULL_HANDLE) {
        return;
    }
    VULKAN_CALL_ERRCHECK(fSharedContext->interface(), WaitForFences(fSharedContext->device(),
                                                                    1,
                                                                    &fSubmitFence,
                                                                    /*waitAll=*/true,
                                                                    /*timeout=*/UINT64_MAX));
}

bool VulkanCommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                          const Texture* colorTexture,
                                          const Texture* resolveTexture,
                                          const Texture* depthStencilTexture,
                                          SkRect viewport,
                                          const DrawPassList& drawPasses) {
    if (!this->beginRenderPass(renderPassDesc, colorTexture, resolveTexture, depthStencilTexture)) {
        return false;
    }

    VkViewport vkViewport = {
        viewport.fLeft,
        viewport.fTop,
        viewport.width(),
        viewport.height(),
        0.0f, // minDepth
        1.0f, // maxDepth
    };
    VULKAN_CALL(fSharedContext->interface(),
                CmdSetViewport(fPrimaryCommandBuffer,
                               /*firstViewport=*/0,
                               /*viewportCount=*/1,
                               &vkViewport));

    for (const auto& drawPass : drawPasses) {
        this->addDrawPass(drawPass.get());
    }

    this->endRenderPass();
    return true;
}

bool VulkanCommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc,
                                          const Texture* colorTexture,
                                          const Texture* resolveTexture,
                                          const Texture* depthStencilTexture) {
    const static VkAttachmentLoadOp vkLoadOp[] {
        VK_ATTACHMENT_LOAD_OP_LOAD,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE
    };
    static_assert((int)LoadOp::kLoad == 0);
    static_assert((int)LoadOp::kClear == 1);
    static_assert((int)LoadOp::kDiscard == 2);
    static_assert(std::size(vkLoadOp) == kLoadOpCount);

    const static VkAttachmentStoreOp vkStoreOp[] {
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE
    };
    static_assert((int)StoreOp::kStore == 0);
    static_assert((int)StoreOp::kDiscard == 1);
    static_assert(std::size(vkStoreOp) == kStoreOpCount);

    // Get render pass descriptor
    VkRenderingInfoKHR renderingInfo;
    memset(&renderingInfo, 0, sizeof(VkRenderingInfoKHR));
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.renderArea = {{ 0, 0 },
                                { (unsigned int) colorTexture->dimensions().width(),
                                  (unsigned int) colorTexture->dimensions().height() }};
    renderingInfo.layerCount = 1;

    // Set up color attachment
    VkRenderingAttachmentInfoKHR colorAttachment;
    auto& colorInfo = renderPassDesc.fColorAttachment;
    if (colorTexture) {
        memset(&colorAttachment, 0, sizeof(VkRenderingAttachmentInfoKHR));
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        VulkanTexture* vulkanTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(colorTexture));
        colorAttachment.imageView =
                vulkanTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = vkLoadOp[static_cast<int>(colorInfo.fLoadOp)];
        colorAttachment.storeOp = vkStoreOp[static_cast<int>(colorInfo.fStoreOp)];
        memcpy(&colorAttachment.clearValue.color.float32,
               &renderPassDesc.fClearColor,
               4*sizeof(float));
        vulkanTexture->setImageLayout(this, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, false);
        // Set up resolve attachment
        if (resolveTexture) {
            SkASSERT(renderPassDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore);
            // TODO: check Texture matches RenderPassDesc
            vulkanTexture =
                const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(resolveTexture));
            colorAttachment.resolveImageView =
                    vulkanTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();
            colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            SkASSERT(colorAttachment.storeOp == VK_ATTACHMENT_STORE_OP_DONT_CARE);
            vulkanTexture->setImageLayout(this, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, false);
        }

        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        this->trackResource(sk_ref_sp(colorTexture));
    }

    // Set up depth/stencil attachments
    VkRenderingAttachmentInfoKHR depthAttachment;
    VkRenderingAttachmentInfoKHR stencilAttachment;
    auto& depthStencilInfo = renderPassDesc.fDepthStencilAttachment;
    if (depthStencilTexture) {
        VulkanTexture* vulkanTexture =
                const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(depthStencilTexture));
        VkImageView imageView =
                vulkanTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();
        VulkanTextureInfo vkTexInfo;
        depthStencilTexture->textureInfo().getVulkanTextureInfo(&vkTexInfo);

        if (VkFormatIsDepth(vkTexInfo.fFormat)) {
            memset(&depthAttachment, 0, sizeof(VkRenderingAttachmentInfoKHR));
            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            depthAttachment.imageView = imageView;
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp = vkLoadOp[static_cast<int>(depthStencilInfo.fLoadOp)];
            depthAttachment.storeOp = vkStoreOp[static_cast<int>(depthStencilInfo.fStoreOp)];
            depthAttachment.clearValue.depthStencil.depth = renderPassDesc.fClearDepth;

            renderingInfo.pDepthAttachment = &depthAttachment;
        }

        if (VkFormatIsStencil(vkTexInfo.fFormat)) {
            memset(&stencilAttachment, 0, sizeof(VkRenderingAttachmentInfoKHR));
            stencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            stencilAttachment.imageView = imageView;
            stencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            stencilAttachment.loadOp = vkLoadOp[static_cast<int>(depthStencilInfo.fLoadOp)];
            stencilAttachment.storeOp = vkStoreOp[static_cast<int>(depthStencilInfo.fStoreOp)];
            stencilAttachment.clearValue.depthStencil.stencil = renderPassDesc.fClearStencil;

            renderingInfo.pStencilAttachment = &stencilAttachment;
        }

        vulkanTexture->setImageLayout(this, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, false);
        this->trackResource(sk_ref_sp(depthStencilTexture));
    }

    // TODO: If needed, load MSAA from resolve
    // Only possible with RenderPass interface, not beginRendering()

    VULKAN_CALL(fSharedContext->interface(),
                CmdBeginRendering(fPrimaryCommandBuffer, &renderingInfo));
    fActiveRenderPass = true;

    return true;
}

void VulkanCommandBuffer::endRenderPass() {
    SkASSERT(fActive);
    VULKAN_CALL(fSharedContext->interface(), CmdEndRendering(fPrimaryCommandBuffer));
    fActiveRenderPass = false;
}

void VulkanCommandBuffer::addDrawPass(const DrawPass* drawPass) {
    drawPass->addResourceRefs(this);
    for (auto [type, cmdPtr] : drawPass->commands()) {
        switch (type) {
            case DrawPassCommands::Type::kBindGraphicsPipeline: {
                auto bgp = static_cast<DrawPassCommands::BindGraphicsPipeline*>(cmdPtr);
                this->bindGraphicsPipeline(drawPass->getPipeline(bgp->fPipelineIndex));
                break;
            }
            case DrawPassCommands::Type::kSetBlendConstants: {
                auto sbc = static_cast<DrawPassCommands::SetBlendConstants*>(cmdPtr);
                this->setBlendConstants(sbc->fBlendConstants);
                break;
            }
            case DrawPassCommands::Type::kBindUniformBuffer: {
#ifndef SK_DISABLE_VULKAN_RENDERING
                auto bub = static_cast<DrawPassCommands::BindUniformBuffer*>(cmdPtr);
                this->recordBufferBindingInfo(bub->fInfo, bub->fSlot);
#endif
                break;
            }
            case DrawPassCommands::Type::kBindDrawBuffers: {
                auto bdb = static_cast<DrawPassCommands::BindDrawBuffers*>(cmdPtr);
                this->bindDrawBuffers(
                        bdb->fVertices, bdb->fInstances, bdb->fIndices, bdb->fIndirect);
                break;
            }
            case DrawPassCommands::Type::kBindTexturesAndSamplers: {
#ifndef SK_DISABLE_VULKAN_RENDERING
                auto bts = static_cast<DrawPassCommands::BindTexturesAndSamplers*>(cmdPtr);
                this->recordTextureAndSamplerDescSet(*drawPass, *bts);
#endif
                break;
            }
            case DrawPassCommands::Type::kSetScissor: {
                auto ss = static_cast<DrawPassCommands::SetScissor*>(cmdPtr);
                const SkIRect& rect = ss->fScissor;
                this->setScissor(rect.fLeft, rect.fTop, rect.width(), rect.height());
                break;
            }
            case DrawPassCommands::Type::kDraw: {
#ifndef SK_DISABLE_VULKAN_RENDERING
                auto draw = static_cast<DrawPassCommands::Draw*>(cmdPtr);
                this->draw(draw->fType, draw->fBaseVertex, draw->fVertexCount);
#endif
                break;
            }
            case DrawPassCommands::Type::kDrawIndexed: {
#ifndef SK_DISABLE_VULKAN_RENDERING
                auto draw = static_cast<DrawPassCommands::DrawIndexed*>(cmdPtr);
                this->drawIndexed(
                        draw->fType, draw->fBaseIndex, draw->fIndexCount, draw->fBaseVertex);
#endif
                break;
            }
            case DrawPassCommands::Type::kDrawInstanced: {
#ifndef SK_DISABLE_VULKAN_RENDERING
                auto draw = static_cast<DrawPassCommands::DrawInstanced*>(cmdPtr);
                this->drawInstanced(draw->fType,
                                    draw->fBaseVertex,
                                    draw->fVertexCount,
                                    draw->fBaseInstance,
                                    draw->fInstanceCount);
#endif
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedInstanced: {
#ifndef SK_DISABLE_VULKAN_RENDERING
                auto draw = static_cast<DrawPassCommands::DrawIndexedInstanced*>(cmdPtr);
                this->drawIndexedInstanced(draw->fType,
                                           draw->fBaseIndex,
                                           draw->fIndexCount,
                                           draw->fBaseVertex,
                                           draw->fBaseInstance,
                                           draw->fInstanceCount);
#endif
                break;
            }
            case DrawPassCommands::Type::kDrawIndirect: {
#ifndef SK_DISABLE_VULKAN_RENDERING
                auto draw = static_cast<DrawPassCommands::DrawIndirect*>(cmdPtr);
                this->drawIndirect(draw->fType);
#endif
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedIndirect: {
#ifndef SK_DISABLE_VULKAN_RENDERING
                auto draw = static_cast<DrawPassCommands::DrawIndexedIndirect*>(cmdPtr);
                this->drawIndexedIndirect(draw->fType);
#endif
                break;
            }
        }
    }
}

void VulkanCommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
    // TODO: Implement.
    // So long as 2 pipelines have the same pipeline layout, descriptor sets do not need to be
    // re-bound. If the layouts differ, we should set fBindUniformBuffers to true.
    fActiveGraphicsPipeline = static_cast<const VulkanGraphicsPipeline*>(graphicsPipeline);
}

void VulkanCommandBuffer::setBlendConstants(float* blendConstants) {
    // TODO: Implement
}

void VulkanCommandBuffer::recordBufferBindingInfo(const BindBufferInfo& info, UniformSlot slot) {
    unsigned int bufferIndex = 0;
    switch (slot) {
        case UniformSlot::kRenderStep:
            bufferIndex = VulkanGraphicsPipeline::kRenderStepUniformBufferIndex;
            break;
        case UniformSlot::kPaint:
            bufferIndex = VulkanGraphicsPipeline::kPaintUniformBufferIndex;
            break;
        default:
            SkASSERT(false);
    }

    fUniformBuffersToBind[bufferIndex] = info;
    fBindUniformBuffers = true;
}

void VulkanCommandBuffer::syncDescriptorSets() {
    if (fBindUniformBuffers) {
        this->bindUniformBuffers();
        // Changes to descriptor sets in lower slot numbers disrupt later set bindings. Currently,
        // the descriptor set which houses uniform buffers is at a lower slot than the texture /
        // sampler set, so rebinding uniform buffers necessitates re-binding any texture/samplers.
        fBindTextureSamplers = true;
    }
    if (fBindTextureSamplers) {
        this->bindTextureSamplers();
    }
}

void VulkanCommandBuffer::bindUniformBuffers() {
    fBindUniformBuffers = false;

    STArray<3, DescTypeAndCount> descriptors;
    // We always bind at least one uniform buffer descriptor for intrinsic uniforms.
    uint32_t numBuffers = 1;
    descriptors[0].type = DescriptorType::kUniformBuffer;
    descriptors[0].count = 1;

    if (fActiveGraphicsPipeline->hasStepUniforms() &&
            fUniformBuffersToBind[VulkanGraphicsPipeline::kRenderStepUniformBufferIndex]) {
        descriptors[numBuffers].type = DescriptorType::kUniformBuffer;
        descriptors[numBuffers].count = 1;
        ++numBuffers;
    }
    if (fActiveGraphicsPipeline->hasFragment() &&
            fUniformBuffersToBind[VulkanGraphicsPipeline::kPaintUniformBufferIndex]) {
        descriptors[numBuffers].type = DescriptorType::kUniformBuffer;
        descriptors[numBuffers].count = 1;
        ++numBuffers;
    }

    VulkanDescriptorSet* set = fResourceProvider->findOrCreateDescriptorSet(
            SkSpan<DescTypeAndCount>{&descriptors.front(), numBuffers});

    if (!set) {
        SKGPU_LOG_E("Unable to find or create descriptor set");
    } else {
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;
        for (uint32_t i = 0; i < numBuffers; i++) {
            VkDescriptorBufferInfo bufferInfo;
            memset(&bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
            auto vulkanBuffer = static_cast<const VulkanBuffer*>(fUniformBuffersToBind[i].fBuffer);
            bufferInfo.buffer = vulkanBuffer->vkBuffer();
            bufferInfo.offset = fUniformBuffersToBind[i].fOffset;
            bufferInfo.range = vulkanBuffer->size();

            VkWriteDescriptorSet writeInfo;
            memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = *set->descriptorSet();
            writeInfo.dstBinding = i;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeInfo.pImageInfo = nullptr;
            writeInfo.pBufferInfo = &bufferInfo;
            writeInfo.pTexelBufferView = nullptr;

            writeDescriptorSets[i] = writeInfo;
        }

        VULKAN_CALL(fSharedContext->interface(),
                    UpdateDescriptorSets(fSharedContext->device(),
                                         numBuffers,
                                         &writeDescriptorSets[0],
                                         /*descriptorCopyCount=*/0,
                                         /*pDescriptorCopies=*/nullptr));
    }
}

void VulkanCommandBuffer::bindDrawBuffers(const BindBufferInfo& vertices,
                                          const BindBufferInfo& instances,
                                          const BindBufferInfo& indices,
                                          const BindBufferInfo& indirect) {
    this->bindVertexBuffers(vertices.fBuffer,
                            vertices.fOffset,
                            instances.fBuffer,
                            instances.fOffset);
    this->bindIndexBuffer(indices.fBuffer, indices.fOffset);
    this->bindIndirectBuffer(indirect.fBuffer, indirect.fOffset);
}

void VulkanCommandBuffer::bindVertexBuffers(const Buffer* vertexBuffer,
                                            size_t vertexOffset,
                                            const Buffer* instanceBuffer,
                                            size_t instanceOffset) {
    this->bindInputBuffer(vertexBuffer, vertexOffset,
                          VulkanGraphicsPipeline::kVertexBufferIndex);
    this->bindInputBuffer(instanceBuffer, instanceOffset,
                          VulkanGraphicsPipeline::kInstanceBufferIndex);
}

void VulkanCommandBuffer::bindInputBuffer(const Buffer* buffer, VkDeviceSize offset,
                                          uint32_t binding) {
    if (buffer) {
        VkBuffer vkBuffer = static_cast<const VulkanBuffer*>(buffer)->vkBuffer();
        SkASSERT(vkBuffer != VK_NULL_HANDLE);
        if (vkBuffer != fBoundInputBuffers[binding] ||
            offset != fBoundInputBufferOffsets[binding]) {
            VULKAN_CALL(fSharedContext->interface(),
                        CmdBindVertexBuffers(fPrimaryCommandBuffer,
                                             binding,
                                             /*bindingCount=*/1,
                                             &vkBuffer,
                                             &offset));
            fBoundInputBuffers[binding] = vkBuffer;
            fBoundInputBufferOffsets[binding] = offset;
            this->trackResource(sk_ref_sp(buffer));
        }
    }
}

void VulkanCommandBuffer::bindIndexBuffer(const Buffer* indexBuffer, size_t offset) {
    if (indexBuffer) {
        VkBuffer vkBuffer = static_cast<const VulkanBuffer*>(indexBuffer)->vkBuffer();
        SkASSERT(vkBuffer != VK_NULL_HANDLE);
        if (vkBuffer != fBoundIndexBuffer || offset != fBoundIndexBufferOffset) {
            VULKAN_CALL(fSharedContext->interface(), CmdBindIndexBuffer(fPrimaryCommandBuffer,
                                                                        vkBuffer,
                                                                        offset,
                                                                        VK_INDEX_TYPE_UINT16));
            fBoundIndexBuffer = vkBuffer;
            fBoundIndexBufferOffset = offset;
            this->trackResource(sk_ref_sp(indexBuffer));
        }
    } else {
        fBoundIndexBuffer = VK_NULL_HANDLE;
        fBoundIndexBufferOffset = 0;
    }
}

void VulkanCommandBuffer::bindIndirectBuffer(const Buffer* indirectBuffer, size_t offset) {
    // Indirect buffers are not bound via the command buffer, but specified in the draw cmd.
    if (indirectBuffer) {
        fBoundIndirectBuffer = static_cast<const VulkanBuffer*>(indirectBuffer)->vkBuffer();
        fBoundIndirectBufferOffset = offset;
        this->trackResource(sk_ref_sp(indirectBuffer));
    } else {
        fBoundIndirectBuffer = VK_NULL_HANDLE;
        fBoundIndirectBufferOffset = 0;
    }
}

void VulkanCommandBuffer::recordTextureAndSamplerDescSet(
        const DrawPass& drawPass, const DrawPassCommands::BindTexturesAndSamplers& command) {
    // Query resource provider to obtain a descriptor set for the texture/samplers
    std::vector<DescTypeAndCount> descriptors;
    for (int i = 0; i < command.fNumTexSamplers; i++) {
        descriptors.push_back({DescriptorType::kCombinedTextureSampler, 1});
    }
    VulkanDescriptorSet* set = fResourceProvider->findOrCreateDescriptorSet(
            SkSpan<DescTypeAndCount>{&descriptors.front(), descriptors.size()});

    if (!set) {
        SKGPU_LOG_E("Unable to find or create descriptor set");
    } else {
        // Populate the descriptor set with texture/sampler descriptors
        std::vector<VkWriteDescriptorSet> writeDescriptorSets;
        for (int i = 0; i < command.fNumTexSamplers; ++i) {
            auto texture = static_cast<const VulkanTexture*>(
                    drawPass.getTexture(command.fTextureIndices[i]));
            auto sampler = static_cast<const VulkanSampler*>(
                    drawPass.getSampler(command.fSamplerIndices[i]));

            VkDescriptorImageInfo textureInfo;
            memset(&textureInfo, 0, sizeof(VkDescriptorImageInfo));
            textureInfo.sampler = sampler->vkSampler();
            textureInfo.imageView = VK_NULL_HANDLE; // TODO: Obtain texture view from VulkanImage.
            textureInfo.imageLayout = texture->currentLayout();

            VkWriteDescriptorSet writeInfo;
            memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = *set->descriptorSet();
            writeInfo.dstBinding = i;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeInfo.pImageInfo = &textureInfo;
            writeInfo.pBufferInfo = nullptr;
            writeInfo.pTexelBufferView = nullptr;

            writeDescriptorSets[i] = writeInfo;
        }

        VULKAN_CALL(fSharedContext->interface(),
                UpdateDescriptorSets(fSharedContext->device(),
                                     command.fNumTexSamplers,
                                     &writeDescriptorSets[0],
                                     /*descriptorCopyCount=*/0,
                                     /*pDescriptorCopies=*/nullptr));

        // Store the updated descriptor set to be actually bound later on. This avoids binding and
        // potentially having to re-bind in cases where earlier descriptor sets change while going
        // through drawpass commands.
        fTextureSamplerDescSetToBind = *(set->descriptorSet());
        fBindTextureSamplers = true;
    }
}

void VulkanCommandBuffer::bindTextureSamplers() {
    fBindTextureSamplers = false;
    if (fTextureSamplerDescSetToBind != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    CmdBindDescriptorSets(fPrimaryCommandBuffer,
                                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                                          fActiveGraphicsPipeline->layout(),
                                          VulkanGraphicsPipeline::kTextureBindDescSetIndex,
                                          /*setCount=*/1,
                                          &fTextureSamplerDescSetToBind,
                                          /*dynamicOffsetCount=*/0,
                                          /*dynamicOffsets=*/nullptr));
    }
}

void VulkanCommandBuffer::setScissor(unsigned int left, unsigned int top, unsigned int width,
                                     unsigned int height) {
    VkRect2D scissor = {
        {(int32_t)left, (int32_t)top},
        {width, height}
    };
    VULKAN_CALL(fSharedContext->interface(),
                CmdSetScissor(fPrimaryCommandBuffer,
                              /*firstScissor=*/0,
                              /*scissorCount=*/1,
                              &scissor));
}

void VulkanCommandBuffer::draw(PrimitiveType,
                               unsigned int baseVertex,
                               unsigned int vertexCount) {
    SkASSERT(fActiveRenderPass);
    this->syncDescriptorSets();
    // TODO: set primitive type via dynamic state if available
    VULKAN_CALL(fSharedContext->interface(),
                CmdDraw(fPrimaryCommandBuffer,
                        vertexCount,
                        /*instanceCount=*/0,
                        baseVertex,
                        /*firstInstance=*/0));
}

void VulkanCommandBuffer::drawIndexed(PrimitiveType,
                                      unsigned int baseIndex,
                                      unsigned int indexCount,
                                      unsigned int baseVertex) {
    SkASSERT(fActiveRenderPass);
    this->syncDescriptorSets();
    // TODO: set primitive type via dynamic state if available
    VULKAN_CALL(fSharedContext->interface(),
                CmdDrawIndexed(fPrimaryCommandBuffer,
                               indexCount,
                               /*instanceCount=*/0,
                               baseIndex,
                               baseVertex,
                               /*firstInstance=*/0));
}

void VulkanCommandBuffer::drawInstanced(PrimitiveType,
                                        unsigned int baseVertex,
                                        unsigned int vertexCount,
                                        unsigned int baseInstance,
                                        unsigned int instanceCount) {
    SkASSERT(fActiveRenderPass);
    this->syncDescriptorSets();
    // TODO: set primitive type via dynamic state if available
    VULKAN_CALL(fSharedContext->interface(),
                CmdDraw(fPrimaryCommandBuffer,
                        vertexCount,
                        instanceCount,
                        baseVertex,
                        baseInstance));
}

void VulkanCommandBuffer::drawIndexedInstanced(PrimitiveType,
                                               unsigned int baseIndex,
                                               unsigned int indexCount,
                                               unsigned int baseVertex,
                                               unsigned int baseInstance,
                                               unsigned int instanceCount) {
    SkASSERT(fActiveRenderPass);
    this->syncDescriptorSets();
    // TODO: set primitive type via dynamic state if available
    VULKAN_CALL(fSharedContext->interface(),
                CmdDrawIndexed(fPrimaryCommandBuffer,
                               indexCount,
                               instanceCount,
                               baseIndex,
                               baseVertex,
                               baseInstance));
}

void VulkanCommandBuffer::drawIndirect(PrimitiveType) {
    SkASSERT(fActiveRenderPass);
    this->syncDescriptorSets();
    // TODO: set primitive type via dynamic state if available
    // Currently we can only support doing one indirect draw operation at a time,
    // so stride is irrelevant.
    VULKAN_CALL(fSharedContext->interface(),
                CmdDrawIndirect(fPrimaryCommandBuffer,
                                fBoundIndirectBuffer,
                                fBoundIndirectBufferOffset,
                                /*drawCount=*/1,
                                /*stride=*/0));
}

void VulkanCommandBuffer::drawIndexedIndirect(PrimitiveType) {
    SkASSERT(fActiveRenderPass);
    this->syncDescriptorSets();
    // TODO: set primitive type via dynamic state if available
    // Currently we can only support doing one indirect draw operation at a time,
    // so stride is irrelevant.
    VULKAN_CALL(fSharedContext->interface(),
                CmdDrawIndexedIndirect(fPrimaryCommandBuffer,
                                       fBoundIndirectBuffer,
                                       fBoundIndirectBufferOffset,
                                       /*drawCount=*/1,
                                       /*stride=*/0));
}

bool VulkanCommandBuffer::onAddComputePass(const DispatchGroupList&) { return false; }

bool VulkanCommandBuffer::onCopyBufferToBuffer(const Buffer* srcBuffer,
                                               size_t srcOffset,
                                               const Buffer* dstBuffer,
                                               size_t dstOffset,
                                               size_t size) {
    auto vkSrcBuffer = static_cast<const VulkanBuffer*>(srcBuffer);
    auto vkDstBuffer = static_cast<const VulkanBuffer*>(dstBuffer);

    SkASSERT(vkSrcBuffer->bufferUsageFlags() & VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    SkASSERT(vkDstBuffer->bufferUsageFlags() & VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    VkBufferCopy region;
    memset(&region, 0, sizeof(VkBufferCopy));
    region.srcOffset = srcOffset;
    region.dstOffset = dstOffset;
    region.size = size;

    this->submitPipelineBarriers();

    VULKAN_CALL(fSharedContext->interface(),
                CmdCopyBuffer(fPrimaryCommandBuffer,
                              vkSrcBuffer->vkBuffer(),
                              vkDstBuffer->vkBuffer(),
                              /*regionCount=*/1,
                              &region));

    return true;
}

bool VulkanCommandBuffer::onCopyTextureToBuffer(const Texture* texture,
                                                SkIRect srcRect,
                                                const Buffer* buffer,
                                                size_t bufferOffset,
                                                size_t bufferRowBytes) {
    const VulkanTexture* srcTexture = static_cast<const VulkanTexture*>(texture);
    auto dstBuffer = static_cast<const VulkanBuffer*>(buffer);
    SkASSERT(dstBuffer->bufferUsageFlags() & VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // Obtain the VkFormat of the source texture so we can determine bytes per block.
    VulkanTextureInfo srcTextureInfo;
    texture->textureInfo().getVulkanTextureInfo(&srcTextureInfo);
    size_t bytesPerBlock = VkFormatBytesPerBlock(srcTextureInfo.fFormat);

    // Set up copy region
    VkBufferImageCopy region;
    memset(&region, 0, sizeof(VkBufferImageCopy));
    region.bufferOffset = bufferOffset;
    // Vulkan expects bufferRowLength in texels, not bytes.
    region.bufferRowLength = (uint32_t)(bufferRowBytes/bytesPerBlock);
    region.bufferImageHeight = 0; // Tightly packed
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, /*mipLevel=*/0, 0, 1 };
    region.imageOffset = { srcRect.left(), srcRect.top(), /*z=*/0 };
    region.imageExtent = { (uint32_t)srcRect.width(), (uint32_t)srcRect.height(), /*depth=*/1 };

    // Enable editing of the source texture so we can change its layout so it can be copied from.
    const_cast<VulkanTexture*>(srcTexture)->setImageLayout(this,
                                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                           VK_ACCESS_TRANSFER_READ_BIT,
                                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                           false);
    // Set current access mask for buffer
    const_cast<VulkanBuffer*>(dstBuffer)->setBufferAccess(this,
                                                          VK_ACCESS_TRANSFER_WRITE_BIT,
                                                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                          false);

    this->submitPipelineBarriers();

    VULKAN_CALL(fSharedContext->interface(),
                CmdCopyImageToBuffer(fPrimaryCommandBuffer,
                                     srcTexture->vkImage(),
                                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     dstBuffer->vkBuffer(),
                                     /*regionCount=*/1,
                                     &region));
    return true;
}

bool VulkanCommandBuffer::onCopyBufferToTexture(const Buffer* buffer,
                                                const Texture* texture,
                                                const BufferTextureCopyData* copyData,
                                                int count) {
    auto srcBuffer = static_cast<const VulkanBuffer*>(buffer);
    SkASSERT(srcBuffer->bufferUsageFlags() & VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    const VulkanTexture* dstTexture = static_cast<const VulkanTexture*>(texture);

    // Obtain the VkFormat of the destination texture so we can determine bytes per block.
    VulkanTextureInfo dstTextureInfo;
    dstTexture->textureInfo().getVulkanTextureInfo(&dstTextureInfo);
    size_t bytesPerBlock = VkFormatBytesPerBlock(dstTextureInfo.fFormat);

    // Set up copy regions.
    TArray<VkBufferImageCopy> regions(count);
    for (int i = 0; i < count; ++i) {
        VkBufferImageCopy& region = regions.push_back();
        memset(&region, 0, sizeof(VkBufferImageCopy));
        region.bufferOffset = copyData[i].fBufferOffset;
        // copyData provides row length in bytes, but Vulkan expects bufferRowLength in texels.
        region.bufferRowLength = (uint32_t)(copyData[i].fBufferRowBytes/bytesPerBlock);
        region.bufferImageHeight = 0; // Tightly packed
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, copyData[i].fMipLevel, 0, 1 };
        region.imageOffset = { copyData[i].fRect.left(),
                               copyData[i].fRect.top(),
                               /*z=*/0 };
        region.imageExtent = { (uint32_t)copyData[i].fRect.width(),
                               (uint32_t)copyData[i].fRect.height(),
                               /*depth=*/1 };
    }

    // Enable editing of the destination texture so we can change its layout so it can be copied to.
    const_cast<VulkanTexture*>(dstTexture)->setImageLayout(this,
                                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                           VK_ACCESS_TRANSFER_WRITE_BIT,
                                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                           false);

    this->submitPipelineBarriers();

    VULKAN_CALL(fSharedContext->interface(),
            CmdCopyBufferToImage(fPrimaryCommandBuffer,
                                 srcBuffer->vkBuffer(),
                                 dstTexture->vkImage(),
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 regions.size(),
                                 regions.begin()));
    return true;
}

bool VulkanCommandBuffer::onCopyTextureToTexture(const Texture* src,
                                                 SkIRect srcRect,
                                                 const Texture* dst,
                                                 SkIPoint dstPoint) {
    const VulkanTexture* srcTexture = static_cast<const VulkanTexture*>(src);
    const VulkanTexture* dstTexture = static_cast<const VulkanTexture*>(dst);

    VkImageCopy copyRegion;
    memset(&copyRegion, 0, sizeof(VkImageCopy));
    copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.srcOffset = { srcRect.fLeft, srcRect.fTop, 0 };
    copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.dstOffset = { dstPoint.fX, dstPoint.fY, 0 };
    copyRegion.extent = { (uint32_t)srcRect.width(), (uint32_t)srcRect.height(), 1 };

    // Enable editing of the src texture so we can change its layout so it can be copied from.
    const_cast<VulkanTexture*>(srcTexture)->setImageLayout(this,
                                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                           VK_ACCESS_TRANSFER_READ_BIT,
                                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                           false);
    // Enable editing of the destination texture so we can change its layout so it can be copied to.
    const_cast<VulkanTexture*>(dstTexture)->setImageLayout(this,
                                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                           VK_ACCESS_TRANSFER_WRITE_BIT,
                                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                           false);

    this->submitPipelineBarriers();

    VULKAN_CALL(fSharedContext->interface(),
                CmdCopyImage(fPrimaryCommandBuffer,
                             srcTexture->vkImage(),
                             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                             dstTexture->vkImage(),
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             /*regionCount=*/1,
                             &copyRegion));

    return true;
}

bool VulkanCommandBuffer::onSynchronizeBufferToCpu(const Buffer* buffer, bool* outDidResultInWork) {
    static_cast<const VulkanBuffer*>(buffer)->setBufferAccess(this,
                                                              VK_ACCESS_HOST_READ_BIT,
                                                              VK_PIPELINE_STAGE_HOST_BIT,
                                                              false);

    *outDidResultInWork = true;
    return true;
}

bool VulkanCommandBuffer::onClearBuffer(const Buffer*, size_t offset, size_t size) {
    return false;
}

void VulkanCommandBuffer::addBufferMemoryBarrier(const Resource* resource,
                                                 VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask,
                                                 bool byRegion,
                                                 VkBufferMemoryBarrier* barrier) {
    SkASSERT(resource);
    this->pipelineBarrier(resource,
                          srcStageMask,
                          dstStageMask,
                          byRegion,
                          kBufferMemory_BarrierType,
                          barrier);
}

void VulkanCommandBuffer::addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask,
                                                 bool byRegion,
                                                 VkBufferMemoryBarrier* barrier) {
    // We don't pass in a resource here to the command buffer. The command buffer only is using it
    // to hold a ref, but every place where we add a buffer memory barrier we are doing some other
    // command with the buffer on the command buffer. Thus those other commands will already cause
    // the command buffer to be holding a ref to the buffer.
    this->pipelineBarrier(/*resource=*/nullptr,
                          srcStageMask,
                          dstStageMask,
                          byRegion,
                          kBufferMemory_BarrierType,
                          barrier);
}

void VulkanCommandBuffer::addImageMemoryBarrier(const Resource* resource,
                                                VkPipelineStageFlags srcStageMask,
                                                VkPipelineStageFlags dstStageMask,
                                                bool byRegion,
                                                VkImageMemoryBarrier* barrier) {
    SkASSERT(resource);
    this->pipelineBarrier(resource,
                          srcStageMask,
                          dstStageMask,
                          byRegion,
                          kImageMemory_BarrierType,
                          barrier);
}

void VulkanCommandBuffer::pipelineBarrier(const Resource* resource,
                                          VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask,
                                          bool byRegion,
                                          BarrierType barrierType,
                                          void* barrier) {
    // TODO: Do we need to handle wrapped command buffers?
    // SkASSERT(!this->isWrapped());
    SkASSERT(fActive);
#ifdef SK_DEBUG
    // For images we can have barriers inside of render passes but they require us to add more
    // support in subpasses which need self dependencies to have barriers inside them. Also, we can
    // never have buffer barriers inside of a render pass. For now we will just assert that we are
    // not in a render pass.
    bool isValidSubpassBarrier = false;
    if (barrierType == kImageMemory_BarrierType) {
        VkImageMemoryBarrier* imgBarrier = static_cast<VkImageMemoryBarrier*>(barrier);
        isValidSubpassBarrier = (imgBarrier->newLayout == imgBarrier->oldLayout) &&
            (imgBarrier->srcQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) &&
            (imgBarrier->dstQueueFamilyIndex == VK_QUEUE_FAMILY_IGNORED) &&
            byRegion;
    }
    SkASSERT(!fActiveRenderPass || isValidSubpassBarrier);
#endif

    if (barrierType == kBufferMemory_BarrierType) {
        const VkBufferMemoryBarrier* barrierPtr = static_cast<VkBufferMemoryBarrier*>(barrier);
        fBufferBarriers.push_back(*barrierPtr);
    } else {
        SkASSERT(barrierType == kImageMemory_BarrierType);
        const VkImageMemoryBarrier* barrierPtr = static_cast<VkImageMemoryBarrier*>(barrier);
        // We need to check if we are adding a pipeline barrier that covers part of the same
        // subresource range as a barrier that is already in current batch. If it does, then we must
        // submit the first batch because the vulkan spec does not define a specific ordering for
        // barriers submitted in the same batch.
        // TODO: Look if we can gain anything by merging barriers together instead of submitting
        // the old ones.
        for (int i = 0; i < fImageBarriers.size(); ++i) {
            VkImageMemoryBarrier& currentBarrier = fImageBarriers[i];
            if (barrierPtr->image == currentBarrier.image) {
                const VkImageSubresourceRange newRange = barrierPtr->subresourceRange;
                const VkImageSubresourceRange oldRange = currentBarrier.subresourceRange;
                SkASSERT(newRange.aspectMask == oldRange.aspectMask);
                SkASSERT(newRange.baseArrayLayer == oldRange.baseArrayLayer);
                SkASSERT(newRange.layerCount == oldRange.layerCount);
                uint32_t newStart = newRange.baseMipLevel;
                uint32_t newEnd = newRange.baseMipLevel + newRange.levelCount - 1;
                uint32_t oldStart = oldRange.baseMipLevel;
                uint32_t oldEnd = oldRange.baseMipLevel + oldRange.levelCount - 1;
                if (std::max(newStart, oldStart) <= std::min(newEnd, oldEnd)) {
                    this->submitPipelineBarriers();
                    break;
                }
            }
        }
        fImageBarriers.push_back(*barrierPtr);
    }
    fBarriersByRegion |= byRegion;
    fSrcStageMask = fSrcStageMask | srcStageMask;
    fDstStageMask = fDstStageMask | dstStageMask;

    if (resource) {
        this->trackResource(sk_ref_sp(resource));
    }
    if (fActiveRenderPass) {
        this->submitPipelineBarriers(true);
    }
}

void VulkanCommandBuffer::submitPipelineBarriers(bool forSelfDependency) {
    SkASSERT(fActive);

    // TODO: Do we need to handle SecondaryCommandBuffers as well?

    // Currently we never submit a pipeline barrier without at least one buffer or image barrier.
    if (fBufferBarriers.size() || fImageBarriers.size()) {
        // For images we can have barriers inside of render passes but they require us to add more
        // support in subpasses which need self dependencies to have barriers inside them. Also, we
        // can never have buffer barriers inside of a render pass. For now we will just assert that
        // we are not in a render pass.
        SkASSERT(!fActiveRenderPass || forSelfDependency);
        // TODO: Do we need to handle wrapped CommandBuffers?
        //  SkASSERT(!this->isWrapped());
        SkASSERT(fSrcStageMask && fDstStageMask);

        VkDependencyFlags dependencyFlags = fBarriersByRegion ? VK_DEPENDENCY_BY_REGION_BIT : 0;
        VULKAN_CALL(fSharedContext->interface(),
                    CmdPipelineBarrier(fPrimaryCommandBuffer, fSrcStageMask, fDstStageMask,
                                       dependencyFlags,
                                       /*memoryBarrierCount=*/0, /*pMemoryBarrier=*/nullptr,
                                       fBufferBarriers.size(), fBufferBarriers.begin(),
                                       fImageBarriers.size(), fImageBarriers.begin()));
        fBufferBarriers.clear();
        fImageBarriers.clear();
        fBarriersByRegion = false;
        fSrcStageMask = 0;
        fDstStageMask = 0;
    }
    SkASSERT(!fBufferBarriers.size());
    SkASSERT(!fImageBarriers.size());
    SkASSERT(!fBarriersByRegion);
    SkASSERT(!fSrcStageMask);
    SkASSERT(!fDstStageMask);
}


} // namespace skgpu::graphite

