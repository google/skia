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
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

using namespace skia_private;

namespace skgpu::graphite {

class VulkanDescriptorSet;

/**
 * Since intrinsic uniforms need to be read in the vertex shader, we cannot use protected buffers
 * for them when submitting protected work. Thus in order to upload data to them, we need to make
 * them mappable instead of using commands to copy data to them (would require them to be
 * protected if we did). This helper class manages rotating through buffers and writing each new
 * occurrence of a set of intrinsic uniforms into the current buffer.
 *
 * Ideally we would remove this class and instead use push constants for all intrinsic uniforms.
 */
class VulkanCommandBuffer::IntrinsicConstantsManager {
public:
    BindBufferInfo add(VulkanCommandBuffer* cb, UniformDataBlock intrinsicValues) {
        static constexpr int kNumSlots = 8;

        BindBufferInfo* existing = fCachedIntrinsicValues.find(intrinsicValues);
        if (existing) {
            return *existing;
        }

        SkASSERT(!cb->fActiveRenderPass);

        const Caps* caps = cb->fSharedContext->caps();
        const uint32_t stride =
                SkAlignTo(intrinsicValues.size(), caps->requiredUniformBufferAlignment());
        if (!fCurrentBuffer || fSlotsUsed == kNumSlots) {
            VulkanResourceProvider* resourceProvider = cb->fResourceProvider;
            sk_sp<Buffer> buffer = resourceProvider->findOrCreateBuffer(stride * kNumSlots,
                                                                        BufferType::kUniform,
                                                                        AccessPattern::kHostVisible,
                                                                        "IntrinsicConstantBuffer");
            if (!buffer) {
                return {};
            }
            VulkanBuffer* ptr = static_cast<VulkanBuffer*>(buffer.release());
            fCurrentBuffer = sk_sp<VulkanBuffer>(ptr);

            fSlotsUsed = 0;

            if (!fCurrentBuffer) {
                // If we failed to create a GPU buffer to hold the intrinsic uniforms, we will fail
                // the Recording being inserted, so return an empty bind info.
                return {};
            }
            cb->trackResource(fCurrentBuffer);
        }

        SkASSERT(fCurrentBuffer && fSlotsUsed < kNumSlots);
        void* mapPtr = fCurrentBuffer->map();
        if (!mapPtr) {
            return {};
        }
        uint32_t offset = (fSlotsUsed++) * stride;
        mapPtr = SkTAddOffset<void>(mapPtr, static_cast<ptrdiff_t>(offset));
        memcpy(mapPtr, intrinsicValues.data(), intrinsicValues.size());
        fCurrentBuffer->unmap();
        BindBufferInfo binding{
                fCurrentBuffer.get(), offset, SkTo<uint32_t>(intrinsicValues.size())};
        fCachedIntrinsicValues.set(UniformDataBlock::Make(intrinsicValues, &fUniformData), binding);
        return binding;
    }

private:
    // The current buffer being filled up, as well as the how much of it has been written to.
    sk_sp<VulkanBuffer> fCurrentBuffer;
    int fSlotsUsed = 0;  // in multiples of the intrinsic uniform size and UBO binding requirement

    // All uploaded intrinsic uniform sets and where they are on the GPU. All uniform sets are
    // cached for the duration of a CommandBuffer since the maximum number of elements in this
    // collection will equal the number of render passes and the intrinsic constants aren't that
    // large. This maximizes the chance for reuse between passes.
    skia_private::THashMap<UniformDataBlock, BindBufferInfo, UniformDataBlock::Hash>
            fCachedIntrinsicValues;
    SkArenaAlloc fUniformData{0};
};


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
    fIntrinsicConstants = nullptr;
    fBindUniformBuffers = true;
    fBoundIndexBuffer = VK_NULL_HANDLE;
    fBoundIndexBufferOffset = 0;
    fBoundIndirectBuffer = VK_NULL_HANDLE;
    fBoundIndirectBufferOffset = 0;
    fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
    fNumTextureSamplers = 0;
    fUniformBuffersToBind.fill({});
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

    VULKAN_CALL_ERRCHECK(fSharedContext,
                         BeginCommandBuffer(fPrimaryCommandBuffer, &cmdBufferBeginInfo));
    fIntrinsicConstants = std::make_unique<IntrinsicConstantsManager>();
    fActive = true;
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
    VULKAN_CALL_RESULT(sharedContext, result, QueueSubmit(queue, 1, &submitInfo, fence));
    return result;
}

bool VulkanCommandBuffer::submit(VkQueue queue) {
    this->end();

    auto device = fSharedContext->device();
    VkResult err;

    if (fSubmitFence == VK_NULL_HANDLE) {
        VkFenceCreateInfo fenceInfo;
        memset(&fenceInfo, 0, sizeof(VkFenceCreateInfo));
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

bool VulkanCommandBuffer::updateIntrinsicUniforms(SkIRect viewport) {
    SkASSERT(fActive && !fActiveRenderPass);

    // The SkSL has declared these as a top-level interface block, which will use std140 in Vulkan.
    // If we switch to supporting push constants here, it would be std430 instead.
    UniformManager intrinsicValues{Layout::kStd140};
    CollectIntrinsicUniforms(fSharedContext->caps(), viewport, fDstCopyBounds, &intrinsicValues);
    BindBufferInfo binding =
            fIntrinsicConstants->add(this, UniformDataBlock::Wrap(&intrinsicValues));
    if (!binding) {
        return false;
    } else if (binding ==
               fUniformBuffersToBind[VulkanGraphicsPipeline::kIntrinsicUniformBufferIndex]) {
        return true;  // no binding change needed
    }

    fUniformBuffersToBind[VulkanGraphicsPipeline::kIntrinsicUniformBufferIndex] = binding;
    return true;
}

bool VulkanCommandBuffer::onAddRenderPass(const RenderPassDesc& renderPassDesc,
                                          SkIRect renderPassBounds,
                                          const Texture* colorTexture,
                                          const Texture* resolveTexture,
                                          const Texture* depthStencilTexture,
                                          SkIRect viewport,
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

    if (!this->updateIntrinsicUniforms(viewport)) {
        return false;
    }
    this->setViewport(viewport);

    if (!this->beginRenderPass(renderPassDesc,
                               renderPassBounds,
                               colorTexture,
                               resolveTexture,
                               depthStencilTexture)) {
        return false;
    }

    for (const auto& drawPass : drawPasses) {
        this->addDrawPass(drawPass.get());
    }

    this->endRenderPass();
    return true;
}

bool VulkanCommandBuffer::updateAndBindLoadMSAAInputAttachment(const VulkanTexture& resolveTexture)
{
    // Fetch a descriptor set that contains one input attachment
    STArray<1, DescriptorData> inputDescriptors =
            {VulkanGraphicsPipeline::kInputAttachmentDescriptor};
    sk_sp<VulkanDescriptorSet> set = fResourceProvider->findOrCreateDescriptorSet(
            SkSpan<DescriptorData>{&inputDescriptors.front(), inputDescriptors.size()});
    if (!set) {
        return false;
    }

    VkDescriptorImageInfo textureInfo;
    memset(&textureInfo, 0, sizeof(VkDescriptorImageInfo));
    textureInfo.sampler = VK_NULL_HANDLE;
    textureInfo.imageView =
            resolveTexture.getImageView(VulkanImageView::Usage::kAttachment)->imageView();
    textureInfo.imageLayout = resolveTexture.currentLayout();

    VkWriteDescriptorSet writeInfo;
    memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext = nullptr;
    writeInfo.dstSet = *set->descriptorSet();
    writeInfo.dstBinding = VulkanGraphicsPipeline::kInputAttachmentBindingIndex;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = DsTypeEnumToVkDs(DescriptorType::kInputAttachment);
    writeInfo.pImageInfo = &textureInfo;
    writeInfo.pBufferInfo = nullptr;
    writeInfo.pTexelBufferView = nullptr;

    VULKAN_CALL(fSharedContext->interface(),
                UpdateDescriptorSets(fSharedContext->device(),
                                     /*descriptorWriteCount=*/1,
                                     &writeInfo,
                                     /*descriptorCopyCount=*/0,
                                     /*pDescriptorCopies=*/nullptr));

    VULKAN_CALL(fSharedContext->interface(),
                CmdBindDescriptorSets(fPrimaryCommandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      fActiveGraphicsPipeline->layout(),
                                      VulkanGraphicsPipeline::kInputAttachmentDescSetIndex,
                                      /*setCount=*/1,
                                      set->descriptorSet(),
                                      /*dynamicOffsetCount=*/0,
                                      /*dynamicOffsets=*/nullptr));

    this->trackResource(std::move(set));
    return true;
}

bool VulkanCommandBuffer::loadMSAAFromResolve(const RenderPassDesc& renderPassDesc,
                                              VulkanTexture& resolveTexture,
                                              SkISize dstDimensions,
                                              const SkIRect nativeDrawBounds) {
    sk_sp<VulkanGraphicsPipeline> loadPipeline =
            fResourceProvider->findOrCreateLoadMSAAPipeline(renderPassDesc);
    if (!loadPipeline) {
        SKGPU_LOG_E("Unable to create pipeline to load resolve texture into MSAA attachment");
        return false;
    }

    this->bindGraphicsPipeline(loadPipeline.get());
    // Make sure we do not attempt to bind uniform or texture/sampler descriptors because we do
    // not use them for loading MSAA from resolve.
    fBindUniformBuffers = false;
    fBindTextureSamplers = false;

    this->setScissor(/*left=*/0, /*top=*/0, dstDimensions.width(), dstDimensions.height());

    if (!this->updateAndBindLoadMSAAInputAttachment(resolveTexture)) {
        SKGPU_LOG_E("Unable to update and bind an input attachment descriptor for loading MSAA "
                    "from resolve");
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

    this->pushConstants(VK_SHADER_STAGE_VERTEX_BIT,
                        /*offset=*/0,
                        /*size=*/sizeof(uniData),
                        uniData);

    this->draw(PrimitiveType::kTriangleStrip, /*baseVertex=*/0, /*vertexCount=*/4);
    this->nextSubpass();

    // If we loaded the resolve attachment, then we would have set the image layout to be
    // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL so that it could be used at the start as an
    // input attachment. However, when we switched to the main subpass it will transition the
    // layout internally to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL. Thus we need to update our
    // tracking of the layout to match the new layout.
    resolveTexture.updateImageLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // After using a distinct descriptor set layout for loading MSAA from resolve, we will need to
    // (re-)bind any descriptor sets.
    fBindUniformBuffers = true;
    fBindTextureSamplers = true;
    return true;
}

namespace {
void setup_texture_layouts(VulkanCommandBuffer* cmdBuf,
                           VulkanTexture* colorTexture,
                           VulkanTexture* resolveTexture,
                           VulkanTexture* depthStencilTexture,
                           bool loadMSAAFromResolve) {
    if (colorTexture) {
        colorTexture->setImageLayout(cmdBuf,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     /*byRegion=*/false);
        if (resolveTexture) {
            if (loadMSAAFromResolve) {
                // When loading MSAA from resolve, the texture is used in the first subpass as an
                // input attachment. Subsequent subpass(es) need the resolve texture to provide read
                // access to the color attachment (for use cases such as blending), so add access
                // and pipeline stage flags for both usages.
                resolveTexture->setImageLayout(cmdBuf,
                                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                               VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
                                               VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                               /*byRegion=*/false);
            } else {
                resolveTexture->setImageLayout(cmdBuf,
                                               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                               VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                               /*byRegion=*/false);
            }
        }
    }
    if (depthStencilTexture) {
        depthStencilTexture->setImageLayout(cmdBuf,
                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                            /*byRegion=*/false);
    }
}

void track_attachments(VulkanCommandBuffer* cmdBuf,
                       VulkanTexture* colorTexture,
                       VulkanTexture* resolveTexture,
                       VulkanTexture* depthStencilTexture) {
    if (colorTexture) {
        cmdBuf->trackResource(sk_ref_sp(colorTexture));
    }
    if (resolveTexture){
        cmdBuf->trackResource(sk_ref_sp(resolveTexture));
    }
    if (depthStencilTexture) {
        cmdBuf->trackResource(sk_ref_sp(depthStencilTexture));
    }
}

void gather_attachment_views(skia_private::TArray<VkImageView>& attachmentViews,
                             VulkanTexture* colorTexture,
                             VulkanTexture* resolveTexture,
                             VulkanTexture* depthStencilTexture) {
    if (colorTexture) {
        VkImageView& colorAttachmentView = attachmentViews.push_back();
        colorAttachmentView =
                colorTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();

        if (resolveTexture) {
            VkImageView& resolveView = attachmentViews.push_back();
            resolveView =
                    resolveTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();
        }
    }

    if (depthStencilTexture) {
        VkImageView& stencilView = attachmentViews.push_back();
        stencilView =
                depthStencilTexture->getImageView(VulkanImageView::Usage::kAttachment)->imageView();
    }
}

void gather_clear_values(
        STArray<VulkanRenderPass::kMaxExpectedAttachmentCount, VkClearValue>& clearValues,
        const RenderPassDesc& renderPassDesc,
        VulkanTexture* colorTexture,
        VulkanTexture* depthStencilTexture,
        int depthStencilAttachmentIdx) {
    clearValues.push_back_n(VulkanRenderPass::kMaxExpectedAttachmentCount);
    if (colorTexture) {
        VkClearValue& colorAttachmentClear =
                clearValues.at(VulkanRenderPass::kColorAttachmentIdx);
        memset(&colorAttachmentClear, 0, sizeof(VkClearValue));
        colorAttachmentClear.color = {{renderPassDesc.fClearColor[0],
                                       renderPassDesc.fClearColor[1],
                                       renderPassDesc.fClearColor[2],
                                       renderPassDesc.fClearColor[3]}};
    }
    // Resolve texture does not have a clear value
    if (depthStencilTexture) {
        VkClearValue& depthStencilAttachmentClear = clearValues.at(depthStencilAttachmentIdx);
        memset(&depthStencilAttachmentClear, 0, sizeof(VkClearValue));
        depthStencilAttachmentClear.depthStencil = {renderPassDesc.fClearDepth,
                                                    renderPassDesc.fClearStencil};
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

bool VulkanCommandBuffer::beginRenderPass(const RenderPassDesc& renderPassDesc,
                                          SkIRect renderPassBounds,
                                          const Texture* colorTexture,
                                          const Texture* resolveTexture,
                                          const Texture* depthStencilTexture) {
    // TODO: Check that Textures match RenderPassDesc
    VulkanTexture* vulkanColorTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(colorTexture));
    VulkanTexture* vulkanResolveTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(resolveTexture));
    VulkanTexture* vulkanDepthStencilTexture =
            const_cast<VulkanTexture*>(static_cast<const VulkanTexture*>(depthStencilTexture));

    SkASSERT(resolveTexture ? renderPassDesc.fColorResolveAttachment.fStoreOp == StoreOp::kStore
                            : true);

    // Determine if we need to load MSAA from resolve, and if so, make certain that key conditions
    // are met before proceeding.
    bool loadMSAAFromResolve = renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid() &&
                               renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
    if (loadMSAAFromResolve && (!vulkanResolveTexture || !vulkanColorTexture ||
                                !vulkanResolveTexture->supportsInputAttachmentUsage())) {
        SKGPU_LOG_E("Cannot begin render pass. In order to load MSAA from resolve, the color "
                    "attachment must have input attachment usage and both the color and resolve "
                    "attachments must be valid.");
        return false;
    }

    track_attachments(this, vulkanColorTexture, vulkanResolveTexture, vulkanDepthStencilTexture);

    // Before beginning a renderpass, set all textures to the appropriate image layout.
    setup_texture_layouts(this,
                          vulkanColorTexture,
                          vulkanResolveTexture,
                          vulkanDepthStencilTexture,
                          loadMSAAFromResolve);

    static constexpr int kMaxNumAttachments = 3;
    // Gather attachment views neeeded for frame buffer creation.
    skia_private::TArray<VkImageView> attachmentViews;
    gather_attachment_views(
            attachmentViews, vulkanColorTexture, vulkanResolveTexture, vulkanDepthStencilTexture);

    // Gather clear values needed for RenderPassBeginInfo. Indexed by attachment number.
    STArray<kMaxNumAttachments, VkClearValue> clearValues;
    // The depth/stencil attachment can be at attachment index 1 or 2 depending on whether there is
    // a resolve texture attachment for this renderpass.
    int depthStencilAttachmentIndex = resolveTexture ? 2 : 1;
    gather_clear_values(clearValues,
                        renderPassDesc,
                        vulkanColorTexture,
                        vulkanDepthStencilTexture,
                        depthStencilAttachmentIndex);

    sk_sp<VulkanRenderPass> vulkanRenderPass =
            fResourceProvider->findOrCreateRenderPass(renderPassDesc, /*compatibleOnly=*/false);
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
    sk_sp<VulkanFramebuffer> framebuffer = fResourceProvider->createFramebuffer(fSharedContext,
                                                                                attachmentViews,
                                                                                *vulkanRenderPass,
                                                                                frameBufferWidth,
                                                                                frameBufferHeight);
    if (!framebuffer) {
        SKGPU_LOG_W("Could not create Vulkan Framebuffer");
        return false;
    }

    VkExtent2D granularity;
    // Get granularity for this render pass
    VULKAN_CALL(fSharedContext->interface(),
                GetRenderAreaGranularity(fSharedContext->device(),
                                         vulkanRenderPass->renderPass(),
                                         &granularity));

    bool useFullBounds = loadMSAAFromResolve &&
                         fSharedContext->vulkanCaps().mustLoadFullImageForMSAA();

    VkRect2D renderArea = get_render_area(useFullBounds ? SkIRect::MakeWH(frameBufferWidth,
                                                                          frameBufferHeight)
                                                        : renderPassBounds,
                                          granularity,
                                          frameBufferWidth,
                                          frameBufferHeight);

    VkRenderPassBeginInfo beginInfo;
    memset(&beginInfo, 0, sizeof(VkRenderPassBeginInfo));
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
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
    if (loadMSAAFromResolve && !this->loadMSAAFromResolve(renderPassDesc,
                                                          *vulkanResolveTexture,
                                                          vulkanColorTexture->dimensions(),
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
                this->recordTextureAndSamplerDescSet(drawPass, bts);
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
    SkASSERT(fActiveRenderPass);
    fActiveGraphicsPipeline = static_cast<const VulkanGraphicsPipeline*>(graphicsPipeline);
    VULKAN_CALL(fSharedContext->interface(), CmdBindPipeline(fPrimaryCommandBuffer,
                                                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                             fActiveGraphicsPipeline->pipeline()));
    // TODO(b/293924877): Compare pipeline layouts. If 2 pipelines have the same pipeline layout,
    // then descriptor sets do not need to be re-bound. For now, simply force a re-binding of
    // descriptor sets with any new bindGraphicsPipeline DrawPassCommand.
    fBindUniformBuffers = true;

    if (graphicsPipeline->dstReadRequirement() == DstReadRequirement::kTextureCopy &&
        graphicsPipeline->numFragTexturesAndSamplers() == 1) {
        // The only texture-sampler that the pipeline declares must be the dstCopy, which means
        // there are no other textures that will trigger BindTextureAndSampler commands in a
        // DrawPass (e.g. solid-color + dst-read-requiring blend). Configure the texture binding
        // up front in this case.
        this->recordTextureAndSamplerDescSet(/*drawPass=*/nullptr, /*command=*/nullptr);
    }
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

    // We always bind at least one uniform buffer descriptor for intrinsic uniforms, but can bind
    // up to three (one for render step uniforms, one for paint uniforms).
    STArray<VulkanGraphicsPipeline::kNumUniformBuffers, DescriptorData> descriptors;
    descriptors.push_back(VulkanGraphicsPipeline::kIntrinsicUniformBufferDescriptor);

    DescriptorType uniformBufferType = fSharedContext->caps()->storageBufferSupport()
                                            ? DescriptorType::kStorageBuffer
                                            : DescriptorType::kUniformBuffer;
    if (fActiveGraphicsPipeline->hasStepUniforms() &&
        fUniformBuffersToBind[VulkanGraphicsPipeline::kRenderStepUniformBufferIndex].fBuffer) {
        descriptors.push_back({
            uniformBufferType,
            /*count=*/1,
            VulkanGraphicsPipeline::kRenderStepUniformBufferIndex,
            PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader});
    }
    if (fActiveGraphicsPipeline->hasPaintUniforms() &&
        fUniformBuffersToBind[VulkanGraphicsPipeline::kPaintUniformBufferIndex].fBuffer) {
        descriptors.push_back({
            uniformBufferType,
            /*count=*/1,
            VulkanGraphicsPipeline::kPaintUniformBufferIndex,
            PipelineStageFlags::kFragmentShader});
    }
    if (fActiveGraphicsPipeline->hasGradientBuffer() &&
        fUniformBuffersToBind[VulkanGraphicsPipeline::kGradientBufferIndex].fBuffer) {
        SkASSERT(fSharedContext->caps()->gradientBufferSupport() &&
                 fSharedContext->caps()->storageBufferSupport());
        descriptors.push_back({
            DescriptorType::kStorageBuffer,
            /*count=*/1,
            VulkanGraphicsPipeline::kGradientBufferIndex,
            PipelineStageFlags::kFragmentShader});
    }

    sk_sp<VulkanDescriptorSet> descSet = fResourceProvider->findOrCreateUniformBuffersDescriptorSet(
            descriptors, fUniformBuffersToBind);
    if (!descSet) {
        SKGPU_LOG_E("Unable to find or create uniform descriptor set");
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
            // TODO (b/356874190): Renable once we fix the intrinsic uniform buffer to not be
            // protected.
            //SkASSERT(bindInfo.fBuffer->isProtected() == Protected::kNo);
        }
#endif
        dynamicOffsets[i] = bindInfo.fOffset;
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
        SkASSERT(buffer->isProtected() == Protected::kNo);
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
        SkASSERT(indexBuffer->isProtected() == Protected::kNo);
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
        SkASSERT(indirectBuffer->isProtected() == Protected::kNo);
        fBoundIndirectBuffer = static_cast<const VulkanBuffer*>(indirectBuffer)->vkBuffer();
        fBoundIndirectBufferOffset = offset;
        this->trackResource(sk_ref_sp(indirectBuffer));
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
    if (fActiveGraphicsPipeline->dstReadRequirement() == DstReadRequirement::kTextureCopy) {
        numTexSamplers++;
    }

    if (numTexSamplers == 0) {
        fNumTextureSamplers = 0;
        fTextureSamplerDescSetToBind = VK_NULL_HANDLE;
        fBindTextureSamplers = false;
        return;
    }

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
    if (fActiveGraphicsPipeline->dstReadRequirement() == DstReadRequirement::kTextureCopy) {
        descriptors.push_back({DescriptorType::kCombinedTextureSampler,
                               /*count=*/1,
                               /*bindingIdx=*/numTexSamplers-1,
                               PipelineStageFlags::kFragmentShader,
                               /*immutableSampler=*/nullptr});
    }
    SkASSERT(descriptors.size() == numTexSamplers);
    sk_sp<VulkanDescriptorSet> set = fResourceProvider->findOrCreateDescriptorSet(
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
    auto appendTextureSampler = [&](const VulkanTexture* texture, const VulkanSampler* sampler) {
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
        memset(&textureInfo, 0, sizeof(VkDescriptorImageInfo));
        textureInfo.sampler = sampler->ycbcrConversion() ? VK_NULL_HANDLE : sampler->vkSampler();
        textureInfo.imageView =
                texture->getImageView(VulkanImageView::Usage::kShaderInput)->imageView();
        textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet& writeInfo = writeDescriptorSets.push_back();
        memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
        writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.pNext = nullptr;
        writeInfo.dstSet = *set->descriptorSet();
        writeInfo.dstBinding = writeDescriptorSets.size() - 1;
        writeInfo.dstArrayElement = 0;
        writeInfo.descriptorCount = 1;
        writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeInfo.pImageInfo = &textureInfo;
        writeInfo.pBufferInfo = nullptr;
        writeInfo.pTexelBufferView = nullptr;

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
    if (fActiveGraphicsPipeline->dstReadRequirement() == DstReadRequirement::kTextureCopy) {
        auto texture = static_cast<const VulkanTexture*>(fDstCopy.first);
        auto sampler = static_cast<const VulkanSampler*>(fDstCopy.second);
        if (!appendTextureSampler(texture, sampler)) {
            return;
        }
    }

    SkASSERT(writeDescriptorSets.size() == numTexSamplers &&
             descriptorImageInfos.size() == numTexSamplers);
    VULKAN_CALL(fSharedContext->interface(), UpdateDescriptorSets(fSharedContext->device(),
                                                                  numTexSamplers,
                                                                  &writeDescriptorSets[0],
                                                                  /*descriptorCopyCount=*/0,
                                                                  /*pDescriptorCopies=*/nullptr));

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

void VulkanCommandBuffer::setScissor(unsigned int left, unsigned int top, unsigned int width,
                                     unsigned int height) {
    SkIRect scissor = SkIRect::MakeXYWH(
            left + fReplayTranslation.x(), top + fReplayTranslation.y(), width, height);
    if (!scissor.intersect(SkIRect::MakeSize(fColorAttachmentSize)) ||
        (!fReplayClip.isEmpty() && !scissor.intersect(fReplayClip))) {
        scissor.setEmpty();
    }

    VkRect2D vkScissor = {{scissor.x(), scissor.y()},
                          {static_cast<unsigned int>(scissor.width()),
                           static_cast<unsigned int>(scissor.height())}};
    VULKAN_CALL(fSharedContext->interface(),
                CmdSetScissor(fPrimaryCommandBuffer,
                              /*firstScissor=*/0,
                              /*scissorCount=*/1,
                              &vkScissor));
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
    SkAssertResult(TextureInfos::GetVulkanTextureInfo(texture->textureInfo(), &srcTextureInfo));
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
    SkAssertResult(TextureInfos::GetVulkanTextureInfo(dstTexture->textureInfo(), &dstTextureInfo));
    size_t bytesPerBlock = VkFormatBytesPerBlock(dstTextureInfo.fFormat);
    SkISize oneBlockDims = CompressedDimensions(dstTexture->textureInfo().compressionType(),
                                                {1, 1});

    // Set up copy regions.
    TArray<VkBufferImageCopy> regions(count);
    for (int i = 0; i < count; ++i) {
        VkBufferImageCopy& region = regions.push_back();
        memset(&region, 0, sizeof(VkBufferImageCopy));
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


bool VulkanCommandBuffer::pushConstants(VkShaderStageFlags stageFlags,
                                        uint32_t offset,
                                        uint32_t size,
                                        const void* values) {
    SkASSERT(fActiveGraphicsPipeline);
    // offset and size must be a multiple of 4
    SkASSERT(!SkToBool(offset & 0x3));
    SkASSERT(!SkToBool(size & 0x3));

    VULKAN_CALL(fSharedContext->interface(),
                CmdPushConstants(fPrimaryCommandBuffer,
                                 fActiveGraphicsPipeline->layout(),
                                 stageFlags,
                                 offset,
                                 size,
                                 values));
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

void VulkanCommandBuffer::updateBuffer(const VulkanBuffer* buffer,
                                       const void* data,
                                       size_t dataSize,
                                       size_t dstOffset) {
    // vkCmdUpdateBuffer can only be called outside of a render pass.
    SkASSERT(fActive && !fActiveRenderPass);
    if (!buffer || buffer->vkBuffer() == VK_NULL_HANDLE) {
        SKGPU_LOG_W("VulkanCommandBuffer::updateBuffer requires a valid VulkanBuffer pointer backed"
                    "by a valid VkBuffer handle");
        return;
    }

    // Per the spec, vkCmdUpdateBuffer is treated as a “transfer" operation for the purposes of
    // synchronization barriers. Ensure this write operation occurs after any previous read
    // operations and without clobbering any other write operations on the same memory in the cache.
    buffer->setBufferAccess(this, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    this->submitPipelineBarriers();

    VULKAN_CALL(fSharedContext->interface(), CmdUpdateBuffer(fPrimaryCommandBuffer,
                                                             buffer->vkBuffer(),
                                                             dstOffset,
                                                             dataSize,
                                                             data));
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
