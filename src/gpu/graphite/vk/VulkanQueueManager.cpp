/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanQueueManager.h"

#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

VulkanQueueManager::VulkanQueueManager(VkQueue queue, const SharedContext* sharedContext)
        : QueueManager(sharedContext)
        , fQueue(queue) {
}

const VulkanSharedContext* VulkanQueueManager::vkSharedContext() const {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

std::unique_ptr<CommandBuffer> VulkanQueueManager::getNewCommandBuffer(
        ResourceProvider* resourceProvider, Protected isProtected) {
    VulkanResourceProvider* vkResourceProvider =
            static_cast<VulkanResourceProvider*>(resourceProvider);

    auto cmdBuffer = VulkanCommandBuffer::Make(this->vkSharedContext(),
                                               vkResourceProvider,
                                               isProtected);
    return cmdBuffer;
}

class VulkanWorkSubmission final : public GpuWorkSubmission {
public:
    VulkanWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                         std::unique_ptr<CommandBuffer> protectedCmdBuffer,
                         QueueManager* queueManager)
        : GpuWorkSubmission(std::move(cmdBuffer), std::move(protectedCmdBuffer), queueManager) {}
    VulkanWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer, QueueManager* queueManager)
        : GpuWorkSubmission(std::move(cmdBuffer), queueManager) {}
    ~VulkanWorkSubmission() override {}

private:
    bool onIsFinished(const SharedContext*) override {
        if (this->commandBuffer() &&
            !static_cast<VulkanCommandBuffer*>(this->commandBuffer())->isFinished()) {
            return false;
        }
        if (this->protectedCommandBuffer() &&
            !static_cast<VulkanCommandBuffer*>(this->protectedCommandBuffer())->isFinished()) {
            return false;
        }
        return true;
    }
    void onWaitUntilFinished(const SharedContext*) override {
        if (this->commandBuffer()) {
            static_cast<VulkanCommandBuffer*>(this->commandBuffer())->waitUntilFinished();
        }
        if (this->protectedCommandBuffer()) {
            static_cast<VulkanCommandBuffer*>(this->protectedCommandBuffer())->waitUntilFinished();
        }
    }
};

QueueManager::OutstandingSubmission VulkanQueueManager::onSubmitToGpu(
        const SubmitInfo& submitInfo) {
    if (!fCurrentCommandBuffer && !fCurrentProtectedCommandBuffer) {
        SKIA_LOG_W("VulkanQueueManager::onSubmitToGpu - no command buffers to submit");
        return nullptr;
    }

    auto onFailure = [this]() {
        fCurrentCommandBuffer.reset();
        fCurrentProtectedCommandBuffer.reset();
    };

    if (fCurrentCommandBuffer) {
        VulkanCommandBuffer* vkCmdBuffer =
                static_cast<VulkanCommandBuffer*>(fCurrentCommandBuffer.get());
        if (!vkCmdBuffer->submit(fQueue, submitInfo,
                                 /*includeFence=*/!fCurrentProtectedCommandBuffer)) {
            SKIA_LOG_E("VulkanQueueManager::onSubmitToGpu - unprotected submission failed!");
            onFailure();
            return nullptr;
        }
    }

    if (fCurrentProtectedCommandBuffer) {
        VulkanCommandBuffer* vkCmdBuffer =
                static_cast<VulkanCommandBuffer*>(fCurrentProtectedCommandBuffer.get());
        if (!vkCmdBuffer->submit(fQueue, submitInfo, /*includeFence=*/true)) {
            SKIA_LOG_E("VulkanQueueManager::onSubmitToGpu - protected submission failed!");
            onFailure();
            return nullptr;
        }
    }

    return std::make_unique<VulkanWorkSubmission>(std::move(fCurrentCommandBuffer),
                                                  std::move(fCurrentProtectedCommandBuffer),
                                                  this);
}

} // namespace skgpu::graphite
