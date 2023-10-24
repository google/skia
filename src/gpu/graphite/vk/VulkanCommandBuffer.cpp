/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/BackendSemaphore.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/DescriptorTypes.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanFramebuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

using namespace skia_private;

namespace skgpu::graphite {

class VulkanDescriptorSet;

namespace { // anonymous namespace

uint64_t clamp_ubo_binding_size(const uint64_t& offset,
                                const uint64_t& bufferSize,
                                const uint64_t& maxSize) {
    SkASSERT(offset <= bufferSize);
    auto remainSize = bufferSize - offset;
    return remainSize > maxSize ? maxSize : remainSize;
}

} // anonymous namespace

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
    fBoundIndexBuffer = VK_NULL_HANDLE;
    fBoundIndexBufferOffset = 0;
    fBoundIndirectBuffer = VK_NULL_HANDLE;
    fBoundIndirectBufferOffset = 0;
    fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
    fNumTextureSamplers = 0;
    fUniformBuffersToBind.fill({nullptr, 0});
    for (int i = 0; i < 4; ++i) {
        fCachedBlendConstant[i] = -1.0;
    }
    for (auto& boundInputBuffer : fBoundInputBuffers) {
        boundInputBuffer = VK_NULL_HANDLE;
    }
    for (auto& boundInputOffset : fBoundInputBufferOffsets) {
        boundInputOffset = 0;
    }
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

void VulkanCommandBuffer::addWaitSemaphores(size_t numWaitSemaphores,
                                            const BackendSemaphore* waitSemaphores) {
    if (!waitSemaphores) {
        SkASSERT(numWaitSemaphores == 0);
        return;
    }

    for (size_t i = 0; i < numWaitSemaphores; ++i) {
        auto& semaphore = waitSemaphores[i];
        if (semaphore.isValid() && semaphore.backend() == BackendApi::kVulkan) {
            fWaitSemaphores.push_back(semaphore.getVkSemaphore());
        }
    }
}

void VulkanCommandBuffer::addSignalSemaphores(size_t numSignalSemaphores,
                                              const BackendSemaphore* signalSemaphores) {
    if (!signalSemaphores) {
        SkASSERT(numSignalSemaphores == 0);
        return;
    }

    for (size_t i = 0; i < numSignalSemaphores; ++i) {
        auto& semaphore = signalSemaphores[i];
        if (semaphore.isValid() && semaphore.backend() == BackendApi::kVulkan) {
            fSignalSemaphores.push_back(semaphore.getVkSemaphore());
        }
    }
}

void VulkanCommandBuffer::prepareSurfaceForStateUpdate(SkSurface* targetSurface,
                                                       const MutableTextureState* newState) {
    TextureProxy* textureProxy = static_cast<Surface*>(targetSurface)->backingTextureProxy();
    VulkanTexture* texture = static_cast<VulkanTexture*>(textureProxy->texture());

    // Even though internally we use this helper for getting src access flags and stages they
    // can also be used for general dst flags since we don't know exactly what the client
    // plans on using the image for.
    VkImageLayout newLayout = newState->getVkImageLayout();
    if (newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        newLayout = texture->currentLayout();
    }
    VkPipelineStageFlags dstStage = VulkanTexture::LayoutToPipelineSrcStageFlags(newLayout);
    VkAccessFlags dstAccess = VulkanTexture::LayoutToSrcAccessMask(newLayout);

    uint32_t currentQueueFamilyIndex = texture->currentQueueFamilyIndex();
    uint32_t newQueueFamilyIndex = newState->getQueueFamilyIndex();
    auto isSpecialQueue = [](uint32_t queueFamilyIndex) {
        return queueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
               queueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT;
    };
    if (isSpecialQueue(currentQueueFamilyIndex) && isSpecialQueue(newQueueFamilyIndex)) {
        // It is illegal to have both the new and old queue be special queue families (i.e. external
        // or foreign).
        return;
    }

    texture->setImageLayoutAndQueueIndex(this,
                                         newLayout,
                                         dstAccess,
                                         dstStage,
                                         false,
                                         newQueueFamilyIndex);
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
    int waitCount = fWaitSemaphores.size();
    TArray<VkPipelineStageFlags> vkWaitStages(waitCount);
    for (int i = 0; i < waitCount; ++i) {
        vkWaitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                               VK_PIPELINE_STAGE_TRANSFER_BIT);
    }

    bool submitted = submit_to_queue(interface,
                                     queue,
                                     fSubmitFence,
                                     waitCount,
                                     fWaitSemaphores.data(),
                                     vkWaitStages.data(),
                                     /*commandBufferCount*/1,
                                     &fPrimaryCommandBuffer,
                                     fSignalSemaphores.size(),
                                     fSignalSemaphores.data(),
                                     fSharedContext->isProtected());
    fWaitSemaphores.clear();
    fSignalSemaphores.clear();
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

void VulkanCommandBuffer::updateRtAdjustUniform(const SkRect& viewport) {
    // vkCmdUpdateBuffer can only be called outside of a render pass.
    SkASSERT(fActive && !fActiveRenderPass);

    // Vulkan's framebuffer space has (0, 0) at the top left. This agrees with Skia's device coords.
    // However, in NDC (-1, -1) is the bottom left. So we flip the origin here (assuming all
    // surfaces we have are TopLeft origin). We then store the adjustment values as a uniform.
    const float x = viewport.x() - fReplayTranslation.x();
    const float y = viewport.y() - fReplayTranslation.y();
    float invTwoW = 2.f / viewport.width();
    float invTwoH = 2.f / viewport.height();
    const float rtAdjust[4] = {invTwoW, invTwoH, -1.f - x * invTwoW, -1.f - y * invTwoH};

    auto intrinsicUniformBuffer = fResourceProvider->refIntrinsicConstantBuffer();
    const auto intrinsicVulkanBuffer = static_cast<VulkanBuffer*>(intrinsicUniformBuffer.get());
    SkASSERT(intrinsicVulkanBuffer);

    fUniformBuffersToBind[VulkanGraphicsPipeline::kIntrinsicUniformBufferIndex] =
            {intrinsicUniformBuffer.get(), /*offset=*/0};

    // TODO(b/307577875): Once synchronization for uniform buffers as a whole is improved, we should
    // be able to rely upon the bindUniformBuffers() call to handle buffer access changes for us,
    // removing the need to call setBufferAccess(...) within this method.

    // Per the spec, vkCmdUpdateBuffer is treated as a “transfer" operation for the purposes of
    // synchronization barriers. Ensure this write operation occurs after any previous read
    // operations and without clobbering any other write operations on the same memory in the cache.
    intrinsicVulkanBuffer->setBufferAccess(this, VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT);
    this->submitPipelineBarriers();

    VULKAN_CALL(fSharedContext->interface(), CmdUpdateBuffer(
            fPrimaryCommandBuffer,
            intrinsicVulkanBuffer->vkBuffer(),
            /*dstOffset=*/0,
            VulkanResourceProvider::kIntrinsicConstantSize,
            &rtAdjust));

    // Ensure the buffer update is completed and made visible before reading
    intrinsicVulkanBuffer->setBufferAccess(this, VK_ACCESS_UNIFORM_READ_BIT,
                                           VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
    this->trackResource(std::move(intrinsicUniformBuffer));
}

bool VulkanCommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                          const Texture* colorTexture,
                                          const Texture* resolveTexture,
                                          const Texture* depthStencilTexture,
                                          SkRect viewport,
                                          const DrawPassList& drawPasses) {
    for (const auto& drawPass : drawPasses) {
        // Our current implementation of setting texture image layouts does not allow layout changes
        // once we have already begun a render pass, so prior to any other commands, set the layout
        // of all sampled textures from the drawpass so they can be sampled from the shader.
        const skia_private::TArray<sk_sp<TextureProxy>>& sampledTextureProxies =
                drawPass->sampledTextures();
        for (const sk_sp<TextureProxy>& textureProxy : sampledTextureProxies) {
            VulkanTexture* vulkanTexture = const_cast<VulkanTexture*>(
                                           static_cast<const VulkanTexture*>(
                                           textureProxy->texture()));
            vulkanTexture->setImageLayout(this,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                          VK_ACCESS_SHADER_READ_BIT,
                                          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                          false);
            this->submitPipelineBarriers();
        }
    }

    this->updateRtAdjustUniform(viewport);
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

    if (!this->beginRenderPass(renderPassDesc, colorTexture, resolveTexture, depthStencilTexture)) {
        return false;
    }

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
    // Before beginning a renderpass, set all textures to an appropriate image layout.
    // TODO: Check that Textures match RenderPassDesc
    VulkanTexture* vulkanColorTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(colorTexture));
    VulkanTexture* vulkanDepthStencilTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(depthStencilTexture));
    VulkanTexture* vulkanResolveTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(resolveTexture));
    if (vulkanColorTexture) {
        vulkanColorTexture->setImageLayout(this,
                                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                           VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           false);
        if (vulkanResolveTexture) {
            SkASSERT(renderPassDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore);
            vulkanResolveTexture->setImageLayout(this,
                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                false);
        }
    }
    if (vulkanDepthStencilTexture) {
        vulkanDepthStencilTexture->setImageLayout(this,
                                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                  false);
    }

    sk_sp<VulkanRenderPass> vulkanRenderPass =
            fResourceProvider->findOrCreateRenderPass(renderPassDesc, /*compatibleOnly=*/false);
    if (!vulkanRenderPass) {
        SKGPU_LOG_W("Could not create Vulkan RenderPass");
        return false;
    }
    this->submitPipelineBarriers();
    this->trackResource(vulkanRenderPass);

    skia_private::TArray<VkImageView> attachmentViews; // Needed for frame buffer
    TArray<VkClearValue> clearValues(3); // Needed for RenderPassBeginInfo, indexed by attach. num.
    clearValues.push_back_n(3);
    int attachmentNumber = 0;
    if (colorTexture) {
        VkImageView& colorAttachmentView = attachmentViews.push_back();
        colorAttachmentView =
                vulkanColorTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();

        VkClearValue& colorAttachmentClear = clearValues.at(attachmentNumber++);
        memset(&colorAttachmentClear, 0, sizeof(VkClearValue));
        colorAttachmentClear.color = {{renderPassDesc.fClearColor[0],
                                       renderPassDesc.fClearColor[1],
                                       renderPassDesc.fClearColor[2],
                                       renderPassDesc.fClearColor[3]}};

        this->trackResource(sk_ref_sp(colorTexture));
        if (resolveTexture) {
            VkImageView& resolveView = attachmentViews.push_back();
            resolveView = vulkanResolveTexture->getImageView(VulkanImageView::Usage::kAttachment)->
                          imageView();
            attachmentNumber++;
            this->trackResource(sk_ref_sp(resolveTexture));
        }
    }

    if (depthStencilTexture) {
        VkImageView& stencilView = attachmentViews.push_back();
        memset(&stencilView, 0, sizeof(VkImageView));
        stencilView = vulkanDepthStencilTexture->getImageView(VulkanImageView::Usage::kAttachment)->
                      imageView();

        VkClearValue& depthStencilAttachmentClear = clearValues.at(attachmentNumber);
        memset(&depthStencilAttachmentClear, 0, sizeof(VkClearValue));
        depthStencilAttachmentClear.depthStencil = {renderPassDesc.fClearDepth,
                                                    renderPassDesc.fClearStencil};
        this->trackResource(sk_ref_sp(depthStencilTexture));
    }

    int frameBufferWidth = 0;
    int frameBufferHeight = 0;
    if (colorTexture) {
        frameBufferWidth = colorTexture->dimensions().width();
        frameBufferHeight = colorTexture->dimensions().height();
    } else if (depthStencilTexture) {
        frameBufferWidth = depthStencilTexture->dimensions().width();
        frameBufferHeight = depthStencilTexture->dimensions().height();
    }
    sk_sp<VulkanFramebuffer> framebuffer = fResourceProvider->createFramebuffer(fSharedContext,
                                                                                attachmentViews,
                                                                                *vulkanRenderPass,
                                                                                frameBufferWidth,
                                                                                frameBufferHeight);
    if (!framebuffer) {
        SKGPU_LOG_W("Could not create Vulkan Framebuffer");
        return false;
    }

    VkRenderPassBeginInfo beginInfo;
    memset(&beginInfo, 0, sizeof(VkRenderPassBeginInfo));
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = vulkanRenderPass->renderPass();
    beginInfo.framebuffer = framebuffer->framebuffer();
    // TODO: Get render area from RenderPassDesc. Account for granularity if it wasn't already.
    // For now, simply set the render area to be the entire frame buffer.
    beginInfo.renderArea = {{ 0, 0 },
                            { (unsigned int) frameBufferWidth, (unsigned int) frameBufferHeight }};
    beginInfo.clearValueCount = clearValues.size();
    beginInfo.pClearValues = clearValues.begin();

    // TODO: If needed, load MSAA from resolve

    // Submit pipeline barriers to ensure any image layout transitions are recorded prior to
    // beginning the render pass.
    this->submitPipelineBarriers();
    // TODO: If we add support for secondary command buffers, dynamically determine subpass contents
    VULKAN_CALL(fSharedContext->interface(),
                CmdBeginRenderPass(fPrimaryCommandBuffer,
                                   &beginInfo,
                                   VK_SUBPASS_CONTENTS_INLINE));
    this->trackResource(std::move(framebuffer));
    fActiveRenderPass = true;
    return true;
}

void VulkanCommandBuffer::endRenderPass() {
    SkASSERT(fActive);
    VULKAN_CALL(fSharedContext->interface(), CmdEndRenderPass(fPrimaryCommandBuffer));
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
                auto bub = static_cast<DrawPassCommands::BindUniformBuffer*>(cmdPtr);
                this->recordBufferBindingInfo(bub->fInfo, bub->fSlot);
                break;
            }
            case DrawPassCommands::Type::kBindDrawBuffers: {
                auto bdb = static_cast<DrawPassCommands::BindDrawBuffers*>(cmdPtr);
                this->bindDrawBuffers(
                        bdb->fVertices, bdb->fInstances, bdb->fIndices, bdb->fIndirect);
                break;
            }
            case DrawPassCommands::Type::kBindTexturesAndSamplers: {
                auto bts = static_cast<DrawPassCommands::BindTexturesAndSamplers*>(cmdPtr);
                this->recordTextureAndSamplerDescSet(*drawPass, *bts);
                break;
            }
            case DrawPassCommands::Type::kSetScissor: {
                auto ss = static_cast<DrawPassCommands::SetScissor*>(cmdPtr);
                const SkIRect& rect = ss->fScissor;
                this->setScissor(rect.fLeft, rect.fTop, rect.width(), rect.height());
                break;
            }
            case DrawPassCommands::Type::kDraw: {
                auto draw = static_cast<DrawPassCommands::Draw*>(cmdPtr);
                this->draw(draw->fType, draw->fBaseVertex, draw->fVertexCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexed: {
                auto draw = static_cast<DrawPassCommands::DrawIndexed*>(cmdPtr);
                this->drawIndexed(
                        draw->fType, draw->fBaseIndex, draw->fIndexCount, draw->fBaseVertex);
                break;
            }
            case DrawPassCommands::Type::kDrawInstanced: {
                auto draw = static_cast<DrawPassCommands::DrawInstanced*>(cmdPtr);
                this->drawInstanced(draw->fType,
                                    draw->fBaseVertex,
                                    draw->fVertexCount,
                                    draw->fBaseInstance,
                                    draw->fInstanceCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedInstanced: {
                auto draw = static_cast<DrawPassCommands::DrawIndexedInstanced*>(cmdPtr);
                this->drawIndexedInstanced(draw->fType,
                                           draw->fBaseIndex,
                                           draw->fIndexCount,
                                           draw->fBaseVertex,
                                           draw->fBaseInstance,
                                           draw->fInstanceCount);
                break;
            }
            case DrawPassCommands::Type::kDrawIndirect: {
                auto draw = static_cast<DrawPassCommands::DrawIndirect*>(cmdPtr);
                this->drawIndirect(draw->fType);
                break;
            }
            case DrawPassCommands::Type::kDrawIndexedIndirect: {
                auto draw = static_cast<DrawPassCommands::DrawIndexedIndirect*>(cmdPtr);
                this->drawIndexedIndirect(draw->fType);
                break;
            }
        }
    }
}

void VulkanCommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
    fActiveGraphicsPipeline = static_cast<const VulkanGraphicsPipeline*>(graphicsPipeline);
    SkASSERT(fActiveRenderPass);
    VULKAN_CALL(fSharedContext->interface(), CmdBindPipeline(fPrimaryCommandBuffer,
                                                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                             fActiveGraphicsPipeline->pipeline()));
    // TODO(b/293924877): Compare pipeline layouts. If 2 pipelines have the same pipeline layout,
    // then descriptor sets do not need to be re-bound. For now, simply force a re-binding of
    // descriptor sets with any new bindGraphicsPipeline DrawPassCommand.
    fBindUniformBuffers = true;
}

void VulkanCommandBuffer::setBlendConstants(float* blendConstants) {
    SkASSERT(fActive);
    if (0 != memcmp(blendConstants, fCachedBlendConstant, 4 * sizeof(float))) {
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetBlendConstants(fPrimaryCommandBuffer, blendConstants));
        memcpy(fCachedBlendConstant, blendConstants, 4 * sizeof(float));
    }
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

    // We always bind at least one uniform buffer descriptor for intrinsic uniforms, but can bind
    // up to three (one for render step uniforms, one for paint uniforms).
    STArray<VulkanGraphicsPipeline::kNumUniformBuffers, DescriptorData> descriptors;
    descriptors.push_back(VulkanGraphicsPipeline::kIntrinsicUniformBufferDescriptor);
    if (fActiveGraphicsPipeline->hasStepUniforms() &&
            fUniformBuffersToBind[VulkanGraphicsPipeline::kRenderStepUniformBufferIndex].fBuffer) {
        descriptors.push_back(VulkanGraphicsPipeline::kRenderStepUniformDescriptor);
    }
    if (fActiveGraphicsPipeline->hasFragmentUniforms() &&
            fUniformBuffersToBind[VulkanGraphicsPipeline::kPaintUniformBufferIndex].fBuffer) {
        descriptors.push_back(VulkanGraphicsPipeline::kPaintUniformDescriptor);
    }
    sk_sp<VulkanDescriptorSet> set = fResourceProvider->findOrCreateDescriptorSet(
            SkSpan<DescriptorData>{&descriptors.front(), descriptors.size()});

    if (!set) {
        SKGPU_LOG_E("Unable to find or create descriptor set");
        return;
    }
    static uint64_t maxUniformBufferRange = static_cast<const VulkanSharedContext*>(
            fSharedContext)->vulkanCaps().maxUniformBufferRange();

    for (int i = 0; i < descriptors.size(); i++) {
        int descriptorBindingIndex = descriptors.at(i).bindingIndex;
        SkASSERT(static_cast<unsigned long>(descriptorBindingIndex)
                    < fUniformBuffersToBind.size());
        if (fUniformBuffersToBind[descriptorBindingIndex].fBuffer) {
            VkDescriptorBufferInfo bufferInfo;
            memset(&bufferInfo, 0, sizeof(VkDescriptorBufferInfo));
            auto vulkanBuffer = static_cast<const VulkanBuffer*>(
                    fUniformBuffersToBind[descriptorBindingIndex].fBuffer);
            bufferInfo.buffer = vulkanBuffer->vkBuffer();
            bufferInfo.offset = fUniformBuffersToBind[descriptorBindingIndex].fOffset;
            bufferInfo.range = clamp_ubo_binding_size(bufferInfo.offset, vulkanBuffer->size(),
                                                      maxUniformBufferRange);

            VkWriteDescriptorSet writeInfo;
            memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = *set->descriptorSet();
            writeInfo.dstBinding = descriptorBindingIndex;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = descriptors.at(i).count;
            writeInfo.descriptorType = DsTypeEnumToVkDs(descriptors.at(i).type);
            writeInfo.pImageInfo = nullptr;
            writeInfo.pBufferInfo = &bufferInfo;
            writeInfo.pTexelBufferView = nullptr;

            // TODO(b/293925059): Migrate to updating all the uniform descriptors with one driver
            // call. Calling UpdateDescriptorSets once to encapsulate updates to all uniform
            // descriptors would be ideal, but that led to issues with draws where all the UBOs
            // within that set would unexpectedly be assigned the same offset. Updating them one at
            // a time within this loop works in the meantime but is suboptimal.
            VULKAN_CALL(fSharedContext->interface(),
                        UpdateDescriptorSets(fSharedContext->device(),
                                            /*descriptorWriteCount=*/1,
                                            &writeInfo,
                                            /*descriptorCopyCount=*/0,
                                            /*pDescriptorCopies=*/nullptr));
        }
    }
    VULKAN_CALL(fSharedContext->interface(),
                CmdBindDescriptorSets(fPrimaryCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      fActiveGraphicsPipeline->layout(),
                                      VulkanGraphicsPipeline::kUniformBufferDescSetIndex,
                                      /*setCount=*/1,
                                      set->descriptorSet(),
                                      /*dynamicOffsetCount=*/0,
                                      /*dynamicOffsets=*/nullptr));
    this->trackResource(std::move(set));
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
    if (command.fNumTexSamplers == 0) {
        fNumTextureSamplers = 0;
        fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
        fBindTextureSamplers = false;
        return;
    }
    // Query resource provider to obtain a descriptor set for the texture/samplers
    TArray<DescriptorData> descriptors(command.fNumTexSamplers);
    for (int i = 0; i < command.fNumTexSamplers; i++) {
        descriptors.push_back({DescriptorType::kCombinedTextureSampler, 1, i});
    }
    sk_sp<VulkanDescriptorSet> set = fResourceProvider->findOrCreateDescriptorSet(
            SkSpan<DescriptorData>{&descriptors.front(), descriptors.size()});

    if (!set) {
        SKGPU_LOG_E("Unable to find or create descriptor set");
        fNumTextureSamplers = 0;
        fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
        fBindTextureSamplers = false;
        return;
    } else {
        // Populate the descriptor set with texture/sampler descriptors
        TArray<VkWriteDescriptorSet> writeDescriptorSets(command.fNumTexSamplers);
        TArray<VkDescriptorImageInfo> descriptorImageInfos(command.fNumTexSamplers);
        for (int i = 0; i < command.fNumTexSamplers; ++i) {
            auto texture = const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(
                    drawPass.getTexture(command.fTextureIndices[i])));
            auto sampler = static_cast<const VulkanSampler*>(
                    drawPass.getSampler(command.fSamplerIndices[i]));
            if (!texture || !sampler) {
                // TODO(b/294198324): Investigate the root cause for null texture or samplers on
                // Ubuntu QuadP400 GPU
                SKGPU_LOG_E("Texture and sampler must not be null");
                fNumTextureSamplers = 0;
                fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
                fBindTextureSamplers = false;
                return;
            }

            VkDescriptorImageInfo& textureInfo = descriptorImageInfos.push_back();
            memset(&textureInfo, 0, sizeof(VkDescriptorImageInfo));
            textureInfo.sampler = sampler->vkSampler();
            textureInfo.imageView =
                    texture->getImageView(VulkanImageView::Usage::kShaderInput)->imageView();
            textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet& writeInfo = writeDescriptorSets.push_back();
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
        fTextureSamplerDescSetToBind = *set->descriptorSet();
        fBindTextureSamplers = true;
        fNumTextureSamplers = command.fNumTexSamplers;
        this->trackResource(std::move(set));
    }
}

void VulkanCommandBuffer::bindTextureSamplers() {
    fBindTextureSamplers = false;
    if (fTextureSamplerDescSetToBind != VK_NULL_HANDLE &&
        fActiveGraphicsPipeline->numTextureSamplers() == fNumTextureSamplers) {
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
                        /*instanceCount=*/1,
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
                               /*instanceCount=*/1,
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
                                                          VK_PIPELINE_STAGE_TRANSFER_BIT);

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
                                                 SkIPoint dstPoint,
                                                 int mipLevel) {
    const VulkanTexture* srcTexture = static_cast<const VulkanTexture*>(src);
    const VulkanTexture* dstTexture = static_cast<const VulkanTexture*>(dst);

    VkImageCopy copyRegion;
    memset(&copyRegion, 0, sizeof(VkImageCopy));
    copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.srcOffset = { srcRect.fLeft, srcRect.fTop, 0 };
    copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, (uint32_t)mipLevel, 0, 1 };
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
                                                              VK_PIPELINE_STAGE_HOST_BIT);

    *outDidResultInWork = true;
    return true;
}

bool VulkanCommandBuffer::onClearBuffer(const Buffer*, size_t offset, size_t size) {
    return false;
}

void VulkanCommandBuffer::addBufferMemoryBarrier(const Resource* resource,
                                                 VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask,
                                                 VkBufferMemoryBarrier* barrier) {
    SkASSERT(resource);
    this->pipelineBarrier(resource,
                          srcStageMask,
                          dstStageMask,
                          /*byRegion=*/false,
                          kBufferMemory_BarrierType,
                          barrier);
}

void VulkanCommandBuffer::addBufferMemoryBarrier(VkPipelineStageFlags srcStageMask,
                                                 VkPipelineStageFlags dstStageMask,
                                                 VkBufferMemoryBarrier* barrier) {
    // We don't pass in a resource here to the command buffer. The command buffer only is using it
    // to hold a ref, but every place where we add a buffer memory barrier we are doing some other
    // command with the buffer on the command buffer. Thus those other commands will already cause
    // the command buffer to be holding a ref to the buffer.
    this->pipelineBarrier(/*resource=*/nullptr,
                          srcStageMask,
                          dstStageMask,
                          /*byRegion=*/false,
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
