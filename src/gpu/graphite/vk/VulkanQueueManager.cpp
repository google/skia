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
        ResourceProvider* resourceProvider) {
    VulkanResourceProvider* vkResourceProvider =
            static_cast<VulkanResourceProvider*>(resourceProvider);

    auto cmdBuffer = VulkanCommandBuffer::Make(this->vkSharedContext(), vkResourceProvider);
    return cmdBuffer;
}

class VulkanWorkSubmission final : public GpuWorkSubmission {
public:
    VulkanWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer, QueueManager* queueManager)
        : GpuWorkSubmission(std::move(cmdBuffer), queueManager) {}
    ~VulkanWorkSubmission() override {}

    bool isFinished() override {
        return static_cast<VulkanCommandBuffer*>(this->commandBuffer())->isFinished();
    }
    void waitUntilFinished() override {
        return static_cast<VulkanCommandBuffer*>(this->commandBuffer())->waitUntilFinished();
    }
};

QueueManager::OutstandingSubmission VulkanQueueManager::onSubmitToGpu() {
    SkASSERT(fCurrentCommandBuffer);
    VulkanCommandBuffer* vkCmdBuffer =
            static_cast<VulkanCommandBuffer*>(fCurrentCommandBuffer.get());
    if (!vkCmdBuffer->submit(fQueue)) {
        fCurrentCommandBuffer->callFinishedProcs(/*success=*/false);
        return nullptr;
    }

    std::unique_ptr<GpuWorkSubmission> submission(
            new VulkanWorkSubmission(std::move(fCurrentCommandBuffer), this));
    return submission;
}

} // namespace skgpu::graphite
