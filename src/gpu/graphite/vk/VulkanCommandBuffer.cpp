/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"

#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/BackendSemaphore.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/DataUtils.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/DescriptorData.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanFramebuffer.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

using namespace skia_private;

namespace skgpu::graphite {

class VulkanDescriptorSet;

std::unique_ptr<VulkanCommandBuffer> VulkanCommandBuffer::Make(
        const VulkanSharedContext* sharedContext,
        VulkanResourceProvider* resourceProvider,
        Protected isProtected) {
    // Create VkCommandPool
    VkCommandPoolCreateFlags cmdPoolCreateFlags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    if (isProtected == Protected::kYes) {
        cmdPoolCreateFlags |= VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    }

    const VkCommandPoolCreateInfo cmdPoolInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,  // sType
            nullptr,                                     // pNext
            cmdPoolCreateFlags,                          // CmdPoolCreateFlags
            sharedContext->queueIndex(),                 // queueFamilyIndex
    };
    VkResult result;
    VkCommandPool pool;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreateCommandPool(sharedContext->device(), &cmdPoolInfo, nullptr, &pool));
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
    VULKAN_CALL_RESULT(
            sharedContext,
            result,
            AllocateCommandBuffers(sharedContext->device(), &cmdInfo, &primaryCmdBuffer));
    if (result != VK_SUCCESS) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyCommandPool(sharedContext->device(), pool, nullptr));
        return nullptr;
    }

    return std::unique_ptr<VulkanCommandBuffer>(new VulkanCommandBuffer(pool,
                                                                        primaryCmdBuffer,
                                                                        sharedContext,
                                                                        resourceProvider,
                                                                        isProtected));
}

VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool pool,
                                         VkCommandBuffer primaryCommandBuffer,
                                         const VulkanSharedContext* sharedContext,
                                         VulkanResourceProvider* resourceProvider,
                                         Protected isProtected)
        : CommandBuffer(isProtected)
        , fPool(pool)
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
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyFence(fSharedContext->device(), fSubmitFence, nullptr));
    }
    // This should delete any command buffers as well.
    VULKAN_CALL(fSharedContext->interface(),
                DestroyCommandPool(fSharedContext->device(), fPool, nullptr));
}

void VulkanCommandBuffer::onResetCommandBuffer() {
    SkASSERT(!fActive);
    VULKAN_CALL_ERRCHECK(fSharedContext, ResetCommandPool(fSharedContext->device(), fPool, 0));
    fActiveGraphicsPipeline = nullptr;
    fBindUniformBuffers = true;
    fBoundIndirectBuffer = VK_NULL_HANDLE;
    fBoundIndirectBufferOffset = 0;
    fTargetTexture = nullptr;
    fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
    fNumTextureSamplers = 0;
    fUniformBuffersToBind.fill({});
    for (int i = 0; i < 4; ++i) {
        fCachedBlendConstant[i] = -1.0;
    }
}

bool VulkanCommandBuffer::setNewCommandBufferResources() {
    this->begin();
    return true;
}

void VulkanCommandBuffer::begin() {
    SkASSERT(!fActive);
    VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VULKAN_CALL_ERRCHECK(fSharedContext,
                         BeginCommandBuffer(fPrimaryCommandBuffer, &cmdBufferBeginInfo));
    fActive = true;

    // Set all the dynamic state that Graphite never changes once at the beginning of the command
    // buffer.  The following state are constants in Graphite:
    //
    // * lineWidth
    // * depthBiasEnable, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor
    // * min/maxDepthBounds, depthBoundsTestEnable
    // * primitiveRestartEnable
    // * cullMode
    // * frontFace
    // * rasterizerDiscardEnable

    if (fSharedContext->caps()->useBasicDynamicState()) {
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetLineWidth(fPrimaryCommandBuffer,
                                    /*lineWidth=*/1.0));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetDepthBias(fPrimaryCommandBuffer,
                                    /*depthBiasConstantFactor=*/0.0f,
                                    /*depthBiasClamp=*/0.0f,
                                    /*depthBiasSlopeFactor=*/0.0f));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetDepthBounds(fPrimaryCommandBuffer,
                                      /*minDepthBounds=*/0.0f,
                                      /*maxDepthBounds=*/1.0f));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetDepthBoundsTestEnable(fPrimaryCommandBuffer,
                                                /*depthBoundsTestEnable=*/VK_FALSE));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetDepthBiasEnable(fPrimaryCommandBuffer,
                                          /*depthBiasEnable=*/VK_FALSE));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetPrimitiveRestartEnable(fPrimaryCommandBuffer,
                                                 /*primitiveRestartEnable=*/VK_FALSE));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetCullMode(fPrimaryCommandBuffer,
                                   /*cullMode=*/VK_CULL_MODE_NONE));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetFrontFace(fPrimaryCommandBuffer,
                                    /*frontFace=*/VK_FRONT_FACE_COUNTER_CLOCKWISE));
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetRasterizerDiscardEnable(fPrimaryCommandBuffer,
                                                  /*rasterizerDiscardEnable=*/VK_FALSE));
    }
}

void VulkanCommandBuffer::end() {
    SkASSERT(fActive);
    SkASSERT(!fActiveRenderPass);

    this->submitPipelineBarriers();

    VULKAN_CALL_ERRCHECK(fSharedContext, EndCommandBuffer(fPrimaryCommandBuffer));

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
            fWaitSemaphores.push_back(BackendSemaphores::GetVkSemaphore(semaphore));
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
            fSignalSemaphores.push_back(BackendSemaphores::GetVkSemaphore(semaphore));
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
    VkImageLayout newLayout = skgpu::MutableTextureStates::GetVkImageLayout(newState);
    if (newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        newLayout = texture->currentLayout();
    }
    VkPipelineStageFlags dstStage = VulkanTexture::LayoutToPipelineSrcStageFlags(newLayout);
    VkAccessFlags dstAccess = VulkanTexture::LayoutToSrcAccessMask(newLayout);

    uint32_t currentQueueFamilyIndex = texture->currentQueueFamilyIndex();
    uint32_t newQueueFamilyIndex = skgpu::MutableTextureStates::GetVkQueueFamilyIndex(newState);
    auto isSpecialQueue = [](uint32_t queueFamilyIndex) {
        return queueFamilyIndex == VK_QUEUE_FAMILY_EXTERNAL ||
               queueFamilyIndex == VK_QUEUE_FAMILY_FOREIGN_EXT;
    };
    if (isSpecialQueue(currentQueueFamilyIndex) && isSpecialQueue(newQueueFamilyIndex)) {
        // It is illegal to have both the new and old queue be special queue families (i.e. external
        // or foreign).
        return;
    }

    this->trackCommandBufferResource(sk_ref_sp(texture));

    texture->setImageLayoutAndQueueIndex(this,
                                         newLayout,
                                         dstAccess,
                                         dstStage,
                                         false,
                                         newQueueFamilyIndex);
}

static VkResult submit_to_queue(const VulkanSharedContext* sharedContext,
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
    VkProtectedSubmitInfo protectedSubmitInfo = {};
    if (protectedContext == Protected::kYes) {
        protectedSubmitInfo.sType = VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO;
        protectedSubmitInfo.protectedSubmit = VK_TRUE;
    }

    VkSubmitInfo submitInfo = {};
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
    VULKAN_CALL_RESULT(sharedContext, result, QueueSubmit(queue, 1, &submitInfo, fence));
    return result;
}

bool VulkanCommandBuffer::submit(VkQueue queue) {
    this->end();

    auto device = fSharedContext->device();
    VkResult err;

    if (fSubmitFence == VK_NULL_HANDLE) {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VULKAN_CALL_RESULT(
                fSharedContext, err, CreateFence(device, &fenceInfo, nullptr, &fSubmitFence));
        if (err) {
            fSubmitFence = VK_NULL_HANDLE;
            return false;
        }
    } else {
        // This cannot return DEVICE_LOST so we assert we succeeded.
        VULKAN_CALL_RESULT(fSharedContext, err, ResetFences(device, 1, &fSubmitFence));
        SkASSERT(err == VK_SUCCESS);
    }

    SkASSERT(fSubmitFence != VK_NULL_HANDLE);
    int waitCount = fWaitSemaphores.size();
    TArray<VkPipelineStageFlags> vkWaitStages(waitCount);
    for (int i = 0; i < waitCount; ++i) {
        vkWaitStages.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                               VK_PIPELINE_STAGE_TRANSFER_BIT);
    }

    VkResult submitResult = submit_to_queue(fSharedContext,
                                            queue,
                                            fSubmitFence,
                                            waitCount,
                                            fWaitSemaphores.data(),
                                            vkWaitStages.data(),
                                            /*commandBufferCount*/ 1,
                                            &fPrimaryCommandBuffer,
                                            fSignalSemaphores.size(),
                                            fSignalSemaphores.data(),
                                            this->isProtected());
    fWaitSemaphores.clear();
    fSignalSemaphores.clear();
    if (submitResult != VK_SUCCESS) {
        // If we failed to submit because of a device lost, we still need to wait for the fence to
        // signal before deleting. However, there is an ARM bug (b/359822580) where the driver early
        // outs on the fence wait if in a device lost state and thus we can't wait on it. Instead,
        // we just wait on the queue to finish. We're already in a state that's going to cause us to
        // restart the whole device, so waiting on the queue shouldn't have any performance impact.
        if (submitResult == VK_ERROR_DEVICE_LOST) {
            VULKAN_CALL(fSharedContext->interface(), QueueWaitIdle(queue));
        } else {
            SkASSERT(submitResult == VK_ERROR_OUT_OF_HOST_MEMORY ||
                     submitResult == VK_ERROR_OUT_OF_DEVICE_MEMORY);
        }

        VULKAN_CALL(fSharedContext->interface(), DestroyFence(device, fSubmitFence, nullptr));
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
    VULKAN_CALL_ERRCHECK(fSharedContext,
                         WaitForFences(fSharedContext->device(),
                                       1,
                                       &fSubmitFence,
                                       /*waitAll=*/true,
                                       /*timeout=*/UINT64_MAX));
}

void VulkanCommandBuffer::pushConstants(const PushConstantInfo& pushConstantInfo,
                                        VkPipelineLayout compatibleLayout) {
    // size must be within limits. Vulkan spec dictates each device supports at least 128 bytes
    SkASSERT(pushConstantInfo.fSize < 128);
    // offset and size must be a multiple of 4
    SkASSERT(!SkToBool(pushConstantInfo.fOffset & 0x3));
    SkASSERT(!SkToBool(pushConstantInfo.fSize & 0x3));

    VULKAN_CALL(fSharedContext->interface(),
                CmdPushConstants(fPrimaryCommandBuffer,
                                 compatibleLayout,
                                 pushConstantInfo.fShaderStageFlagBits,
                                 pushConstantInfo.fOffset,
                                 pushConstantInfo.fSize,
                                 pushConstantInfo.fValues));
}

bool VulkanCommandBuffer::onAddRenderPass(const RenderPassDesc& rpDesc,
                                          SkIRect renderPassBounds,
                                          const Texture* colorTexture,
                                          const Texture* resolveTexture,
                                          const Texture* depthStencilTexture,
                                          SkIPoint resolveOffset,
                                          SkIRect viewport,
                                          const DrawPassList& drawPasses) {
    SkASSERT(resolveOffset.isZero());
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
        }
    }
    if (fDstCopy.first) {
        VulkanTexture* vulkanTexture =
                const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(fDstCopy.first));
        vulkanTexture->setImageLayout(this,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      false);
    }

    this->setViewport(viewport);

    if (!this->beginRenderPass(
                rpDesc, renderPassBounds, colorTexture, resolveTexture, depthStencilTexture)) {
        return false;
    }

    // After loading msaa from resolve (if needed), perform any updates that only need to occur
    // once per renderpass.
    this->performOncePerRPUpdates(
            viewport, rpDesc.fDstReadStrategy == DstReadStrategy::kReadFromInput);

    for (const auto& drawPass : drawPasses) {
        this->addDrawPass(drawPass.get());
    }

    this->endRenderPass();
    return true;
}

void VulkanCommandBuffer::performOncePerRPUpdates(SkIRect viewport, bool bindDstAsInputAttachment) {
    // Updating push constant values and - if any draw within the RP reads from the dst as an
    // input attachment - binding the dst texture as an input attachment only need to occur once per
    // RP. This is because intrinsic constant vlues (dst copy bounds and rtAdjust) and the render
    // target do not change throughout the course of a RenderPass. Even if no pipeline is bound yet,
    // we can use a compatible mock pipeline layout to perform these operations.

    // TODO(b/374997389): Somehow convey & enforce Layout::kStd430 for push constants.
    UniformManager intrinsicValues{Layout::kStd140};
    CollectIntrinsicUniforms(fSharedContext->caps(), viewport, fDstReadBounds, &intrinsicValues);
    SkSpan<const char> bytes = intrinsicValues.finish();
    SkASSERT(bytes.size_bytes() == VulkanResourceProvider::kIntrinsicConstantSize);

    PushConstantInfo pushConstantInfo;
    pushConstantInfo.fOffset = 0;
    pushConstantInfo.fSize = VulkanResourceProvider::kIntrinsicConstantSize;
    pushConstantInfo.fShaderStageFlagBits = VulkanResourceProvider::kIntrinsicConstantStageFlags;
    pushConstantInfo.fValues = bytes.data();
    this->pushConstants(pushConstantInfo, fResourceProvider->mockPipelineLayout());

    if (bindDstAsInputAttachment) {
        // TODO(b/390458117): This assert can be removed once the sample loading shader supports
        // sample counts > 1.
        SkASSERT(fTargetTexture && fTargetTexture->numSamples() == 1);
        this->updateAndBindInputAttachment(*fTargetTexture,
                                            VulkanGraphicsPipeline::kDstAsInputDescSetIndex,
                                            fResourceProvider->mockPipelineLayout());
    }
}

bool VulkanCommandBuffer::updateAndBindInputAttachment(const VulkanTexture& texture,
                                                       const int setIdx,
                                                       VkPipelineLayout piplineLayout) {
    // Fetch a descriptor set that contains one input attachment (we do not support using more than
    // one per set at this time).
    STArray<1, DescriptorData> inputDesc = {VulkanGraphicsPipeline::kInputAttachmentDescriptor};
    sk_sp<VulkanDescriptorSet> set = fResourceProvider->findOrCreateDescriptorSet(
            SkSpan<DescriptorData>{&inputDesc.front(), inputDesc.size()});
    if (!set) {
        return false;
    }

    // Update and write to the descriptor given the provided texture, binding it afterwards.
    VkDescriptorImageInfo textureInfo = {};
    textureInfo.sampler = VK_NULL_HANDLE;
    textureInfo.imageView =
            texture.getImageView(VulkanImageView::Usage::kAttachment)->imageView();
    textureInfo.imageLayout = texture.currentLayout();

    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = *set->descriptorSet();
    writeInfo.dstBinding = 0;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = DsTypeEnumToVkDs(DescriptorType::kInputAttachment);
    writeInfo.pImageInfo = &textureInfo;

    VULKAN_CALL(fSharedContext->interface(),
                UpdateDescriptorSets(fSharedContext->device(),
                                     /*descriptorWriteCount=*/1,
                                     &writeInfo,
                                     /*descriptorCopyCount=*/0,
                                     /*pDescriptorCopies=*/nullptr));

    VULKAN_CALL(fSharedContext->interface(),
                CmdBindDescriptorSets(fPrimaryCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      piplineLayout,
                                      setIdx,
                                      /*setCount=*/1,
                                      set->descriptorSet(),
                                      /*dynamicOffsetCount=*/0,
                                      /*dynamicOffsets=*/nullptr));

    this->trackResource(std::move(set));
    return true;
}

bool VulkanCommandBuffer::loadMSAAFromResolve(const RenderPassDesc& rpDesc,
                                              VulkanTexture& resolveTexture,
                                              SkISize dstDimensions,
                                              const SkIRect nativeDrawBounds) {
    sk_sp<VulkanGraphicsPipeline> loadPipeline =
            fResourceProvider->findOrCreateLoadMSAAPipeline(rpDesc);
    if (!loadPipeline) {
        SKGPU_LOG_E("Unable to create pipeline to load resolve texture into MSAA attachment");
        return false;
    }

    // Update and bind uniform descriptor set
    int w = nativeDrawBounds.width();
    int h = nativeDrawBounds.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dstDimensions.width();
    int dh = dstDimensions.height();
    float dx0 = 2.f * nativeDrawBounds.fLeft / dw - 1.f;
    float dx1 = 2.f * (nativeDrawBounds.fLeft + w) / dw - 1.f;
    float dy0 = 2.f * nativeDrawBounds.fTop / dh - 1.f;
    float dy1 = 2.f * (nativeDrawBounds.fTop + h) / dh - 1.f;
    float uniData[] = {dx1 - dx0, dy1 - dy0, dx0, dy0};  // posXform
    SkASSERT(sizeof(uniData) == VulkanResourceProvider::kLoadMSAAPushConstantSize);

    this->bindGraphicsPipeline(loadPipeline.get());

    PushConstantInfo loadMsaaPushConstantInfo;
    loadMsaaPushConstantInfo.fOffset = 0;
    loadMsaaPushConstantInfo.fSize = VulkanResourceProvider::kLoadMSAAPushConstantSize;
    loadMsaaPushConstantInfo.fShaderStageFlagBits =
            VulkanResourceProvider::kLoadMSAAPushConstantStageFlags;
    loadMsaaPushConstantInfo.fValues = uniData;
    this->pushConstants(loadMsaaPushConstantInfo, loadPipeline->layout());

    // Make sure we do not attempt to bind uniform or texture/sampler descriptors because we do
    // not use them for loading MSAA from resolve.
    fBindUniformBuffers = false;
    fBindTextureSamplers = false;

    this->setScissor(SkIRect::MakeXYWH(0, 0, dstDimensions.width(), dstDimensions.height()));

    if (!this->updateAndBindInputAttachment(
            resolveTexture,
            VulkanGraphicsPipeline::kLoadMsaaFromResolveInputDescSetIndex,
            fActiveGraphicsPipeline->layout())) {
        SKGPU_LOG_E("Unable to update and bind an input attachment descriptor for loading MSAA "
                    "from resolve");
        return false;
    }

    this->draw(PrimitiveType::kTriangleStrip, /*baseVertex=*/0, /*vertexCount=*/4);

    // After loading the resolve attachment, proceed to the next subpass.
    this->nextSubpass();
    // While transitioning to the next subpass, the layout of the resolve texture gets changed
    // internally to accommodate its usage within the following subpass. Thus, we need to update
    // our tracking of the layout to match the new/final layout. We do not need to use a general
    // layout because we do not expect to later treat the resolve texture as a dst to read from.
    resolveTexture.updateImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // After using a distinct descriptor set layout for loading MSAA from resolve, we will need to
    // (re-)bind any descriptor sets.
    fBindUniformBuffers = true;
    fBindTextureSamplers = true;
    return true;
}

namespace {
// Helpers for determining + updating texture layouts.
void assign_color_texture_layout(VulkanCommandBuffer* cmdBuf,
                                 VulkanTexture* colorTexture,
                                 bool rpReadsDstAsInput) {
    VkAccessFlags access =
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // If any draws within a render pass read from the dst color texture as an input attachment,
    // we must use a general image layout and add additional pipeline stage + access flags.
    if (rpReadsDstAsInput) {
        stageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        access |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        // Note: Using VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT may be more optimal,
        // though not many devices support it.
        layout = VK_IMAGE_LAYOUT_GENERAL;
    }

    colorTexture->setImageLayout(cmdBuf, layout, access, stageFlags, /*byRegion=*/false);
}

void assign_resolve_texture_layout(VulkanCommandBuffer* cmdBuf,
                                   VulkanTexture* resolveTexture,
                                   bool loadMSAAFromResolve) {
    VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkAccessFlags access = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    VkImageLayout layout = loadMSAAFromResolve ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                               : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // If loading MSAA from resolve, then the resolve texture is used in the first subpass
    // as an input attachment and is referenced within the fragment shader. Add to the access and
    // pipeline stage flags accordingly.
    if (loadMSAAFromResolve) {
        access |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        stageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        // Otherwise, add write access.
        access |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    resolveTexture->setImageLayout(cmdBuf, layout, access, stageFlags, /*byRegion=*/false);
}

void setup_texture_layouts(VulkanCommandBuffer* cmdBuf,
                           VulkanTexture* colorTexture,
                           VulkanTexture* resolveTexture,
                           VulkanTexture* depthStencilTexture,
                           bool loadMSAAFromResolve,
                           bool rpReadsDstAsInput) {
    if (colorTexture) {
        assign_color_texture_layout(cmdBuf, colorTexture, rpReadsDstAsInput);
        if (resolveTexture) {
            // rpReadsDstAsInput does not matter here given that we do not anticipate reading
            // from the resolve texture as a dst input attachment.
            assign_resolve_texture_layout(cmdBuf, resolveTexture, loadMSAAFromResolve);
        }
    }

    if (depthStencilTexture) {
        depthStencilTexture->setImageLayout(cmdBuf,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                            /*byRegion=*/false);
    }
}

static constexpr int kMaxNumAttachments = 3;
void gather_clear_values(const RenderPassDesc& rpDesc,
                         STArray<kMaxNumAttachments, VkClearValue>* clearValues) {
    // NOTE: This must stay in sync with the attachment order defined in VulkanRenderPass::Metadata
    if (rpDesc.fColorAttachment.fFormat != TextureFormat::kUnsupported) {
        VkClearValue& colorAttachmentClear = clearValues->push_back();
        colorAttachmentClear.color = {{rpDesc.fClearColor[0],
                                       rpDesc.fClearColor[1],
                                       rpDesc.fClearColor[2],
                                       rpDesc.fClearColor[3]}};
    }
    // The resolve attachment (if defined) should never be cleared, but add a value to keep the
    // attachment indices in sync.
    if (rpDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported) {
        SkASSERT(rpDesc.fColorResolveAttachment.fLoadOp != LoadOp::kClear);
        clearValues->push_back({});
    }

    // Vulkan takes the clear depth and clear stencil regardless of whether or not the DS attachment
    // only has a single aspect or both.
    if (rpDesc.fDepthStencilAttachment.fFormat != TextureFormat::kUnsupported) {
        VkClearValue& depthStencilAttachmentClear = clearValues->push_back();
        depthStencilAttachmentClear.depthStencil = {rpDesc.fClearDepth, rpDesc.fClearStencil};
    }
}

// The RenderArea bounds we pass into BeginRenderPass must have a start x value that is a multiple
// of the granularity. The width must also be a multiple of the granularity or equal to the width
// of the entire attachment. Similar requirements apply to the y and height components.
VkRect2D get_render_area(const SkIRect& srcBounds,
                         const VkExtent2D& granularity,
                         int maxWidth,
                         int maxHeight) {
    SkIRect dstBounds;
    // Adjust Width
    if (granularity.width == 0 || granularity.width == 1) {
        dstBounds.fLeft = srcBounds.fLeft;
        dstBounds.fRight = srcBounds.fRight;
    } else {
        // Start with the right side of rect so we know if we end up going past the maxWidth.
        int rightAdj = srcBounds.fRight % granularity.width;
        if (rightAdj != 0) {
            rightAdj = granularity.width - rightAdj;
        }
        dstBounds.fRight = srcBounds.fRight + rightAdj;
        if (dstBounds.fRight > maxWidth) {
            dstBounds.fRight = maxWidth;
            dstBounds.fLeft = 0;
        } else {
            dstBounds.fLeft = srcBounds.fLeft - srcBounds.fLeft % granularity.width;
        }
    }

    if (granularity.height == 0 || granularity.height == 1) {
        dstBounds.fTop = srcBounds.fTop;
        dstBounds.fBottom = srcBounds.fBottom;
    } else {
        // Start with the bottom side of rect so we know if we end up going past the maxHeight.
        int bottomAdj = srcBounds.fBottom % granularity.height;
        if (bottomAdj != 0) {
            bottomAdj = granularity.height - bottomAdj;
        }
        dstBounds.fBottom = srcBounds.fBottom + bottomAdj;
        if (dstBounds.fBottom > maxHeight) {
            dstBounds.fBottom = maxHeight;
            dstBounds.fTop = 0;
        } else {
            dstBounds.fTop = srcBounds.fTop - srcBounds.fTop % granularity.height;
        }
    }

    VkRect2D renderArea;
    renderArea.offset = { dstBounds.fLeft , dstBounds.fTop };
    renderArea.extent = { (uint32_t)dstBounds.width(), (uint32_t)dstBounds.height() };
    return renderArea;
}
} // anonymous namespace

bool VulkanCommandBuffer::beginRenderPass(const RenderPassDesc& rpDesc,
                                          SkIRect renderPassBounds,
                                          const Texture* colorTexture,
                                          const Texture* resolveTexture,
                                          const Texture* depthStencilTexture) {
    // Validate attachment descs and textures
    SkDEBUGCODE(const auto& colorInfo = rpDesc.fColorAttachment;)
    SkDEBUGCODE(const auto& resolveInfo = rpDesc.fColorResolveAttachment;)
    SkDEBUGCODE(const auto& depthStencilInfo = rpDesc.fDepthStencilAttachment;)
    SkASSERT(colorTexture ? colorInfo.isCompatible(colorTexture->textureInfo())
                          : colorInfo.fFormat == TextureFormat::kUnsupported);
    SkASSERT(resolveTexture ? resolveInfo.isCompatible(resolveTexture->textureInfo())
                            : resolveInfo.fFormat == TextureFormat::kUnsupported);
    SkASSERT(depthStencilTexture ? depthStencilInfo.isCompatible(depthStencilTexture->textureInfo())
                                 : depthStencilInfo.fFormat == TextureFormat::kUnsupported);

    fTargetTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(colorTexture));
    VulkanTexture* vulkanResolveTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(resolveTexture));
    VulkanTexture* vulkanDepthStencilTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(depthStencilTexture));

    // Determine if we need to load MSAA from resolve, and if so, make certain that key conditions
    // are met before proceeding.
    const bool loadMSAAFromResolve = RenderPassDescWillLoadMSAAFromResolve(rpDesc);
    if (loadMSAAFromResolve && (!vulkanResolveTexture || !fTargetTexture ||
                                !vulkanResolveTexture->supportsInputAttachmentUsage())) {
        SKGPU_LOG_E("Cannot begin render pass. In order to load MSAA from resolve, the color "
                    "attachment must have input attachment usage and both the color and resolve "
                    "attachments must be valid.");
        return false;
    }

    // Before beginning a renderpass, set all textures to the appropriate image layout. Whether a RP
    // must support reading from the dst as an input attachment affects some layout selections.
    setup_texture_layouts(
            this,
            fTargetTexture,
            vulkanResolveTexture,
            vulkanDepthStencilTexture,
            loadMSAAFromResolve,
            /*rpReadsDstAsInput=*/rpDesc.fDstReadStrategy == DstReadStrategy::kReadFromInput);

    // Gather clear values needed for RenderPassBeginInfo. Indexed by attachment number.
    STArray<kMaxNumAttachments, VkClearValue> clearValues;
    gather_clear_values(rpDesc, &clearValues);

    sk_sp<VulkanRenderPass> vulkanRenderPass =
            fResourceProvider->findOrCreateRenderPass(rpDesc, /*compatibleOnly=*/false);
    if (!vulkanRenderPass) {
        SKGPU_LOG_W("Could not create Vulkan RenderPass");
        return false;
    }
    this->submitPipelineBarriers();
    this->trackResource(vulkanRenderPass);

    int frameBufferWidth = 0;
    int frameBufferHeight = 0;
    if (colorTexture) {
        frameBufferWidth = colorTexture->dimensions().width();
        frameBufferHeight = colorTexture->dimensions().height();
    } else if (depthStencilTexture) {
        frameBufferWidth = depthStencilTexture->dimensions().width();
        frameBufferHeight = depthStencilTexture->dimensions().height();
    }
    sk_sp<VulkanFramebuffer> framebuffer =
            fResourceProvider->findOrCreateFramebuffer(fSharedContext,
                                                       fTargetTexture,
                                                       vulkanResolveTexture,
                                                       vulkanDepthStencilTexture,
                                                       rpDesc,
                                                       *vulkanRenderPass,
                                                       frameBufferWidth,
                                                       frameBufferHeight);
    if (!framebuffer) {
        SKGPU_LOG_W("Could not find or create Vulkan Framebuffer");
        return false;
    }

    bool useFullBounds = loadMSAAFromResolve &&
                         fSharedContext->vulkanCaps().mustLoadFullImageForMSAA();

    VkRect2D renderArea = get_render_area(useFullBounds ? SkIRect::MakeWH(frameBufferWidth,
                                                                          frameBufferHeight)
                                                        : renderPassBounds,
                                          vulkanRenderPass->granularity(),
                                          frameBufferWidth,
                                          frameBufferHeight);

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = vulkanRenderPass->renderPass();
    beginInfo.framebuffer = framebuffer->framebuffer();
    beginInfo.renderArea = renderArea;
    beginInfo.clearValueCount = clearValues.size();
    beginInfo.pClearValues = clearValues.begin();

    // Submit pipeline barriers to ensure any image layout transitions are recorded prior to
    // beginning the render pass.
    this->submitPipelineBarriers();
    // TODO: If we add support for secondary command buffers, dynamically determine subpass contents
    VULKAN_CALL(fSharedContext->interface(),
                CmdBeginRenderPass(fPrimaryCommandBuffer,
                                   &beginInfo,
                                   VK_SUBPASS_CONTENTS_INLINE));
    fActiveRenderPass = true;

    SkIRect nativeBounds = SkIRect::MakeXYWH(renderArea.offset.x,
                                             renderArea.offset.y,
                                             renderArea.extent.width,
                                             renderArea.extent.height);

    if (loadMSAAFromResolve && !this->loadMSAAFromResolve(rpDesc,
                                                          *vulkanResolveTexture,
                                                          fTargetTexture->dimensions(),
                                                          nativeBounds)) {
        SKGPU_LOG_E("Failed to load MSAA from resolve");
        this->endRenderPass();
        return false;
    }

    // Once we have an active render pass, the command buffer should hold on to a frame buffer ref.
    this->trackResource(std::move(framebuffer));
    return true;
}

void VulkanCommandBuffer::endRenderPass() {
    SkASSERT(fActive);
    VULKAN_CALL(fSharedContext->interface(), CmdEndRenderPass(fPrimaryCommandBuffer));
    fActiveRenderPass = false;
    fTargetTexture = nullptr;
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
            case DrawPassCommands::Type::kBindStaticDataBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindStaticDataBuffer*>(cmdPtr);
                this->bindInputBuffer(bdb->fStaticData.fBuffer, bdb->fStaticData.fOffset,
                                      VulkanGraphicsPipeline::kStaticDataBufferIndex);
                break;
            }
            case DrawPassCommands::Type::kBindAppendDataBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindAppendDataBuffer*>(cmdPtr);
                this->bindInputBuffer(bdb->fAppendData.fBuffer, bdb->fAppendData.fOffset,
                                      VulkanGraphicsPipeline::kAppendDataBufferIndex);
                break;
            }
            case DrawPassCommands::Type::kBindIndexBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindIndexBuffer*>(cmdPtr);
                this->bindIndexBuffer(
                        bdb->fIndices.fBuffer, bdb->fIndices.fOffset);
                break;
            }
            case DrawPassCommands::Type::kBindIndirectBuffer: {
                auto bdb = static_cast<DrawPassCommands::BindIndirectBuffer*>(cmdPtr);
                this->bindIndirectBuffer(
                        bdb->fIndirect.fBuffer, bdb->fIndirect.fOffset);
                break;
            }
            case DrawPassCommands::Type::kBindTexturesAndSamplers: {
                auto bts = static_cast<DrawPassCommands::BindTexturesAndSamplers*>(cmdPtr);
                this->recordTextureAndSamplerDescSet(drawPass, bts);
                break;
            }
            case DrawPassCommands::Type::kSetScissor: {
                auto ss = static_cast<DrawPassCommands::SetScissor*>(cmdPtr);
                this->setScissor(ss->fScissor);
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
            case DrawPassCommands::Type::kAddBarrier: {
                auto barrierCmd = static_cast<DrawPassCommands::AddBarrier*>(cmdPtr);
                this->addBarrier(barrierCmd->fType);
                break;
            }
        }
    }
}

void VulkanCommandBuffer::bindGraphicsPipeline(const GraphicsPipeline* graphicsPipeline) {
    SkASSERT(fActiveRenderPass);
    // TODO(b/414645289): Once the front-end is made aware of dynamic state, it could recognize when
    // only dynamic state has changed.  In that case, since the pipeline doesn't change, this call
    // can be avoided.  The logic after this would then have to move to another place; for example
    // setting dynamic states should move to a separate VulkanCommandBuffer call.
    const auto* previousGraphicsPipeline = fActiveGraphicsPipeline;
    fActiveGraphicsPipeline = static_cast<const VulkanGraphicsPipeline*>(graphicsPipeline);
    VULKAN_CALL(fSharedContext->interface(), CmdBindPipeline(fPrimaryCommandBuffer,
                                                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                             fActiveGraphicsPipeline->pipeline()));
    // TODO(b/293924877): Compare pipeline layouts. If 2 pipelines have the same pipeline layout,
    // then descriptor sets do not need to be re-bound. For now, simply force a re-binding of
    // descriptor sets with any new bindGraphicsPipeline DrawPassCommand.
    fBindUniformBuffers = true;

    if (graphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy &&
        graphicsPipeline->numFragTexturesAndSamplers() == 1) {
        // The only texture-sampler that the pipeline declares must be the dstCopy, which means
        // there are no other textures that will trigger BindTextureAndSampler commands in a
        // DrawPass (e.g. solid-color + dst-read-requiring blend). Configure the texture binding
        // up front in this case.
        this->recordTextureAndSamplerDescSet(/*drawPass=*/nullptr, /*command=*/nullptr);
    }

    fActiveGraphicsPipeline->updateDynamicState(
            fSharedContext, fPrimaryCommandBuffer, previousGraphicsPipeline);
}

void VulkanCommandBuffer::setBlendConstants(float* blendConstants) {
    SkASSERT(fActive);
    if (0 != memcmp(blendConstants, fCachedBlendConstant, 4 * sizeof(float))) {
        VULKAN_CALL(fSharedContext->interface(),
                    CmdSetBlendConstants(fPrimaryCommandBuffer, blendConstants));
        memcpy(fCachedBlendConstant, blendConstants, 4 * sizeof(float));
    }
}

void VulkanCommandBuffer::addBarrier(BarrierType type) {
    SkASSERT(fTargetTexture);

    VkPipelineStageFlags dstStage;
    VkAccessFlags dstAccess;
    if (type == BarrierType::kAdvancedNoncoherentBlend) {
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstAccess = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
    } else {
        // If input reads are coherent, no barrier is needed
        if (fSharedContext->vulkanCaps().isInputAttachmentReadCoherent()) {
            return;
        }

        SkASSERT(type == BarrierType::kReadDstFromInput);
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dstAccess = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    }
    VkImageMemoryBarrier imageMemoryBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            /*pNext=*/nullptr,
            /*srcAccessMask=*/VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            dstAccess,
            /*oldLayout=*/VK_IMAGE_LAYOUT_GENERAL,
            /*newLayout=*/VK_IMAGE_LAYOUT_GENERAL,
            /*srcQueueFamilyIndex=*/VK_QUEUE_FAMILY_IGNORED,
            /*dstQueueFamilyIndex=*/VK_QUEUE_FAMILY_IGNORED,
            fTargetTexture->vkImage(),
            /*subresourceRange=*/{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }};
    this->addImageMemoryBarrier(fTargetTexture,
                                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                dstStage,
                                /*byRegion=*/true,
                                &imageMemoryBarrier);
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
        case UniformSlot::kGradient:
            bufferIndex = VulkanGraphicsPipeline::kGradientBufferIndex;
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

    // Define a container with size reserved for up to kNumUniformBuffers descriptors. Only add
    // DescriptorData for uniforms that actually are used and need to be bound.
    STArray<VulkanGraphicsPipeline::kNumUniformBuffers, DescriptorData> descriptors;

    // Up to kNumUniformBuffers can be used and require rebinding depending upon render pass info.
    DescriptorType uniformBufferType =
            fSharedContext->caps()->storageBufferSupport() ? DescriptorType::kStorageBuffer
                                                           : DescriptorType::kUniformBuffer;
    if (fActiveGraphicsPipeline->hasStepUniforms() &&
        fUniformBuffersToBind[VulkanGraphicsPipeline::kRenderStepUniformBufferIndex].fBuffer) {
        descriptors.push_back({
                uniformBufferType,
                /*count=*/1,
                VulkanGraphicsPipeline::kRenderStepUniformBufferIndex,
                PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader });
    }
    if (fActiveGraphicsPipeline->hasPaintUniforms() &&
        fUniformBuffersToBind[VulkanGraphicsPipeline::kPaintUniformBufferIndex].fBuffer) {
        descriptors.push_back({
                uniformBufferType,
                /*count=*/1,
                VulkanGraphicsPipeline::kPaintUniformBufferIndex,
                PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader });
    }
    if (fActiveGraphicsPipeline->hasGradientBuffer() &&
        fUniformBuffersToBind[VulkanGraphicsPipeline::kGradientBufferIndex].fBuffer) {
        SkASSERT(fSharedContext->caps()->gradientBufferSupport() &&
                 fSharedContext->caps()->storageBufferSupport());
        descriptors.push_back({ DescriptorType::kStorageBuffer, /*count=*/1,
                                VulkanGraphicsPipeline::kGradientBufferIndex,
                                PipelineStageFlags::kFragmentShader });
    }

    // If no uniforms are used, we can go ahead and return since no descriptors need to be bound.
    if (descriptors.empty()) {
        return;
    }

    skia_private::AutoSTMalloc<VulkanGraphicsPipeline::kNumUniformBuffers, uint32_t>
            dynamicOffsets(descriptors.size());
    for (int i = 0; i < descriptors.size(); i++) {
        int descriptorBindingIndex = descriptors[i].fBindingIndex;
        SkASSERT(static_cast<unsigned long>(descriptorBindingIndex) < fUniformBuffersToBind.size());
        const auto& bindInfo = fUniformBuffersToBind[descriptorBindingIndex];
#ifdef SK_DEBUG
        if (descriptors[i].fPipelineStageFlags & PipelineStageFlags::kVertexShader) {
            SkASSERT(bindInfo.fBuffer->isProtected() == Protected::kNo);
        }
#endif
        dynamicOffsets[i] = bindInfo.fOffset;
    }

    sk_sp<VulkanDescriptorSet> descSet = fResourceProvider->findOrCreateUniformBuffersDescriptorSet(
            descriptors, fUniformBuffersToBind);
    if (!descSet) {
        SKGPU_LOG_E("Unable to find or create uniform descriptor set");
        return;
    }

    VULKAN_CALL(fSharedContext->interface(),
                CmdBindDescriptorSets(fPrimaryCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      fActiveGraphicsPipeline->layout(),
                                      VulkanGraphicsPipeline::kUniformBufferDescSetIndex,
                                      /*setCount=*/1,
                                      descSet->descriptorSet(),
                                      descriptors.size(),
                                      dynamicOffsets.get()));
    this->trackResource(std::move(descSet));
}

void VulkanCommandBuffer::bindInputBuffer(const Buffer* inputBuffer, VkDeviceSize offset,
                                          uint32_t binding) {
    if (inputBuffer) {
        SkASSERT(inputBuffer->isProtected() == Protected::kNo);
        VkBuffer vkBuffer = static_cast<const VulkanBuffer*>(inputBuffer)->vkBuffer();
        SkASSERT(vkBuffer != VK_NULL_HANDLE);
        VULKAN_CALL(fSharedContext->interface(), CmdBindVertexBuffers(fPrimaryCommandBuffer,
                                                                      binding,
                                                                      /*bindingCount=*/1,
                                                                      &vkBuffer,
                                                                      &offset));
    }
}

void VulkanCommandBuffer::bindIndexBuffer(const Buffer* indexBuffer, size_t offset) {
    if (indexBuffer) {
        SkASSERT(indexBuffer->isProtected() == Protected::kNo);
        VkBuffer vkBuffer = static_cast<const VulkanBuffer*>(indexBuffer)->vkBuffer();
        SkASSERT(vkBuffer != VK_NULL_HANDLE);
        VULKAN_CALL(fSharedContext->interface(), CmdBindIndexBuffer(fPrimaryCommandBuffer,
                                                                     vkBuffer,
                                                                     offset,
                                                                     VK_INDEX_TYPE_UINT16));
    }
}

void VulkanCommandBuffer::bindIndirectBuffer(const Buffer* indirectBuffer, size_t offset) {
    // Indirect buffers are not bound via the command buffer, but specified in the draw cmd.
    if (indirectBuffer) {
        SkASSERT(indirectBuffer->isProtected() == Protected::kNo);
        fBoundIndirectBuffer = static_cast<const VulkanBuffer*>(indirectBuffer)->vkBuffer();
        fBoundIndirectBufferOffset = offset;
    } else {
        fBoundIndirectBuffer = VK_NULL_HANDLE;
        fBoundIndirectBufferOffset = 0;
    }
}

void VulkanCommandBuffer::recordTextureAndSamplerDescSet(
        const DrawPass* drawPass, const DrawPassCommands::BindTexturesAndSamplers* command) {
    SkASSERT(SkToBool(drawPass) == SkToBool(command));
    SkASSERT(fActiveGraphicsPipeline);
    // Add one extra texture for dst copies, which is not included in the command itself.
    int numTexSamplers = command ? command->fNumTexSamplers : 0;
    if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy) {
        numTexSamplers++;
    }

    if (numTexSamplers == 0) {
        fNumTextureSamplers = 0;
        fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
        fBindTextureSamplers = false;
        return;
    }

    sk_sp<VulkanDescriptorSet> set;
    const VulkanTexture* singleTexture = nullptr;
    const Sampler* singleSampler = nullptr;
    if (numTexSamplers == 1) {
        if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy) {
            singleTexture = static_cast<const VulkanTexture*>(fDstCopy.first);
            singleSampler = static_cast<const VulkanSampler*>(fDstCopy.second);
        } else {
            SkASSERT(command);
            singleTexture = static_cast<const VulkanTexture*>(
                    drawPass->getTexture(command->fTextureIndices[0]));
            singleSampler = drawPass->getSampler(command->fSamplerIndices[0]);
        }
        SkASSERT(singleTexture && singleSampler);
        set = singleTexture->getCachedSingleTextureDescriptorSet(singleSampler);
    }

    if (!set) {
        // Query resource provider to obtain a descriptor set for the texture/samplers
        TArray<DescriptorData> descriptors(numTexSamplers);
        if (command) {
            for (int i = 0; i < command->fNumTexSamplers; i++) {
                auto sampler = static_cast<const VulkanSampler*>(
                        drawPass->getSampler(command->fSamplerIndices[i]));

                const Sampler* immutableSampler = (sampler && sampler->ycbcrConversion()) ? sampler
                                                                                          : nullptr;
                descriptors.push_back({DescriptorType::kCombinedTextureSampler,
                                       /*count=*/1,
                                       /*bindingIdx=*/i,
                                       PipelineStageFlags::kFragmentShader,
                                       immutableSampler});
            }
        }
        // If required the dst copy texture+sampler is the last one in the descriptor set
        if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy) {
            descriptors.push_back({DescriptorType::kCombinedTextureSampler,
                                   /*count=*/1,
                                   /*bindingIdx=*/numTexSamplers-1,
                                   PipelineStageFlags::kFragmentShader,
                                   /*immutableSampler=*/nullptr});
        }
        SkASSERT(descriptors.size() == numTexSamplers);
        set = fResourceProvider->findOrCreateDescriptorSet(
                SkSpan<DescriptorData>{&descriptors.front(), descriptors.size()});

        if (!set) {
            SKGPU_LOG_E("Unable to find or create descriptor set");
            fNumTextureSamplers = 0;
            fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
            fBindTextureSamplers = false;
            return;
        }
        // Populate the descriptor set with texture/sampler descriptors
        TArray<VkWriteDescriptorSet> writeDescriptorSets(numTexSamplers);
        TArray<VkDescriptorImageInfo> descriptorImageInfos(numTexSamplers);
        auto appendTextureSampler = [&](const VulkanTexture* texture,
                                        const VulkanSampler* sampler) {
            if (!texture || !sampler) {
                // TODO(b/294198324): Investigate the root cause for null texture or samplers on
                // Ubuntu QuadP400 GPU
                SKGPU_LOG_E("Texture and sampler must not be null");
                fNumTextureSamplers = 0;
                fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
                fBindTextureSamplers = false;
                return false;
            }

            VkDescriptorImageInfo& textureInfo = descriptorImageInfos.push_back();
            textureInfo = {};
            textureInfo.sampler = sampler->ycbcrConversion() ? VK_NULL_HANDLE
                                                             : sampler->vkSampler();
            textureInfo.imageView =
                    texture->getImageView(VulkanImageView::Usage::kShaderInput)->imageView();
            textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet& writeInfo = writeDescriptorSets.push_back();
            writeInfo = {};
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.dstSet = *set->descriptorSet();
            writeInfo.dstBinding = writeDescriptorSets.size() - 1;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeInfo.pImageInfo = &textureInfo;

            return true;
        };

        if (command) {
            for (int i = 0; i < command->fNumTexSamplers; ++i) {
                auto texture = static_cast<const VulkanTexture*>(
                        drawPass->getTexture(command->fTextureIndices[i]));
                auto sampler = static_cast<const VulkanSampler*>(
                        drawPass->getSampler(command->fSamplerIndices[i]));
                if (!appendTextureSampler(texture, sampler)) {
                    return;
                }
            }
        }
        if (fActiveGraphicsPipeline->dstReadStrategy() == DstReadStrategy::kTextureCopy) {
            auto texture = static_cast<const VulkanTexture*>(fDstCopy.first);
            auto sampler = static_cast<const VulkanSampler*>(fDstCopy.second);
            if (!appendTextureSampler(texture, sampler)) {
                return;
            }
        }

        SkASSERT(writeDescriptorSets.size() == numTexSamplers &&
                 descriptorImageInfos.size() == numTexSamplers);
        VULKAN_CALL(fSharedContext->interface(),
                    UpdateDescriptorSets(fSharedContext->device(),
                                         numTexSamplers,
                                         &writeDescriptorSets[0],
                                         /*descriptorCopyCount=*/0,
                                         /*pDescriptorCopies=*/nullptr));

        if (numTexSamplers == 1) {
            SkASSERT(singleTexture && singleSampler);
            singleTexture->addCachedSingleTextureDescriptorSet(set, sk_ref_sp(singleSampler));
        }
    }

    // Store the updated descriptor set to be actually bound later on. This avoids binding and
    // potentially having to re-bind in cases where earlier descriptor sets change while going
    // through drawpass commands.
    fTextureSamplerDescSetToBind = *set->descriptorSet();
    fBindTextureSamplers = true;
    fNumTextureSamplers = numTexSamplers;
    this->trackResource(std::move(set));
}

void VulkanCommandBuffer::bindTextureSamplers() {
    fBindTextureSamplers = false;
    if (fTextureSamplerDescSetToBind != VK_NULL_HANDLE &&
        fActiveGraphicsPipeline->numFragTexturesAndSamplers() == fNumTextureSamplers) {
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

void VulkanCommandBuffer::setScissor(const Scissor& scissor) {
    this->setScissor(scissor.getRect(fReplayTranslation, fRenderPassBounds));
}

void VulkanCommandBuffer::setScissor(const SkIRect& rect) {
    VkRect2D scissor = {
            {rect.x(), rect.y()},
            {static_cast<unsigned int>(rect.width()), static_cast<unsigned int>(rect.height())}};
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

bool VulkanCommandBuffer::onAddComputePass(DispatchGroupSpan) { return false; }

bool VulkanCommandBuffer::onCopyBufferToBuffer(const Buffer* srcBuffer,
                                               size_t srcOffset,
                                               const Buffer* dstBuffer,
                                               size_t dstOffset,
                                               size_t size) {
    auto vkSrcBuffer = static_cast<const VulkanBuffer*>(srcBuffer);
    auto vkDstBuffer = static_cast<const VulkanBuffer*>(dstBuffer);

    SkASSERT(vkSrcBuffer->bufferUsageFlags() & VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    SkASSERT(vkDstBuffer->bufferUsageFlags() & VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    vkSrcBuffer->setBufferAccess(this, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    vkDstBuffer->setBufferAccess(
        this, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferCopy region = {};
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

    // TODO (b/394121386): We don't currently have a list of tracked buffers that are used on a
    // RenderPass in order to put in any needed barriers (like we do for textures). If we did have
    // one, then we would add the needed barriers for the buffers at the start of a render pass.
    // Until we have such a system, we need to do some hackyness here to put in a barrier with the
    // assumption that the buffer will be read after this write from the copy. The only buffer types
    // we allow to be used as the dst of a transfer are vertex and index buffers. So we check the
    // buffers usages for either of those and then set the corresponding access flag.
    VkAccessFlags dstAccess = 0;
    VkPipelineStageFlags dstStageMask = 0;
    VkBufferUsageFlags bufferUsageFlags = vkDstBuffer->bufferUsageFlags();
    if (bufferUsageFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
        dstAccess = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    } else if (bufferUsageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
        dstAccess = VK_ACCESS_INDEX_READ_BIT;
        dstStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    } else if (vkDstBuffer->bufferUsedForCpuRead()) {
        dstAccess = VK_ACCESS_HOST_READ_BIT;
        dstStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    } else {
        SkDEBUGFAIL("Unhandled type of buffer to buffer copy\n");
        return false;
    }
    SkASSERT(dstAccess);
    vkDstBuffer->setBufferAccess(this, dstAccess, dstStageMask);

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

    size_t bytesPerBlock = VkFormatBytesPerBlock(srcTexture->vulkanTextureInfo().fFormat);

    // Set up copy region
    VkBufferImageCopy region = {};
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

    TextureFormat format = TextureInfoPriv::ViewFormat(dstTexture->textureInfo());
    size_t bytesPerBlock = TextureFormatBytesPerBlock(format);
    SkISize oneBlockDims = CompressedDimensions(TextureFormatCompressionType(format), {1, 1});

    // Set up copy regions.
    TArray<VkBufferImageCopy> regions(count);
    for (int i = 0; i < count; ++i) {
        VkBufferImageCopy& region = regions.push_back();
        region = {};
        region.bufferOffset = copyData[i].fBufferOffset;
        // copyData provides row length in bytes, but Vulkan expects bufferRowLength in texels.
        // For compressed this is the number of logical pixels not the number of blocks.
        region.bufferRowLength =
                (uint32_t)((copyData[i].fBufferRowBytes/bytesPerBlock) * oneBlockDims.fWidth);
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

    VkImageCopy copyRegion = {};
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
                                          PipelineBarrierType barrierType,
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

    if (fActiveRenderPass) {
        this->submitPipelineBarriers(true);
    }
}

void VulkanCommandBuffer::submitPipelineBarriers(bool forSelfDependency) {
    SkASSERT(fActive);

    // TODO: Do we need to handle SecondaryCommandBuffers as well?

    // Currently we never submit a pipeline barrier without at least one buffer or image barrier.
    if (!fBufferBarriers.empty() || !fImageBarriers.empty()) {
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
    SkASSERT(fBufferBarriers.empty());
    SkASSERT(fImageBarriers.empty());
    SkASSERT(!fBarriersByRegion);
    SkASSERT(!fSrcStageMask);
    SkASSERT(!fDstStageMask);
}

void VulkanCommandBuffer::nextSubpass() {
    // TODO: Use VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS if we add secondary cmd buffers
    VULKAN_CALL(fSharedContext->interface(),
                CmdNextSubpass(fPrimaryCommandBuffer, VK_SUBPASS_CONTENTS_INLINE));
}

void VulkanCommandBuffer::setViewport(SkIRect viewport) {
    VkViewport vkViewport = {
        (float) viewport.fLeft,
        (float) viewport.fTop,
        (float) viewport.width(),
        (float) viewport.height(),
        0.0f, // minDepth
        1.0f, // maxDepth
    };
    VULKAN_CALL(fSharedContext->interface(),
                CmdSetViewport(fPrimaryCommandBuffer,
                               /*firstViewport=*/0,
                               /*viewportCount=*/1,
                               &vkViewport));
}

} // namespace skgpu::graphite
