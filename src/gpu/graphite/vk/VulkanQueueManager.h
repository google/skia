/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanQueueManager_DEFINED
#define skgpu_graphite_VulkanQueueManager_DEFINED

#include "src/gpu/graphite/QueueManager.h"

#include "include/gpu/vk/VulkanTypes.h"

namespace skgpu::graphite {

class VulkanSharedContext;

class VulkanQueueManager final : public QueueManager {
public:
    VulkanQueueManager(VkQueue queue, const SharedContext*);
    ~VulkanQueueManager() override {}

private:
    const VulkanSharedContext* vkSharedContext() const;

    std::unique_ptr<CommandBuffer> getNewCommandBuffer(ResourceProvider*, Protected) override;
    OutstandingSubmission onSubmitToGpu() override;

#if defined(GPU_TEST_UTILS)
    // TODO: Implement these
    void startCapture() override {}
    void stopCapture() override {}
#endif

    VkQueue fQueue;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanQueueManager_DEFINED
