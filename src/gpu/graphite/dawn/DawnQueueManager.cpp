/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnQueueManager.h"

#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

DawnQueueManager::DawnQueueManager(wgpu::Queue queue,
                                 const SharedContext* sharedContext)
        : QueueManager(sharedContext)
        , fQueue(std::move(queue))
{
}

const DawnSharedContext* DawnQueueManager::dawnSharedContext() const {
    return static_cast<const DawnSharedContext*>(fSharedContext);
}

std::unique_ptr<CommandBuffer> DawnQueueManager::getNewCommandBuffer(ResourceProvider* resourceProvider) {
    return nullptr;
}

// class WorkSubmission final : public GpuWorkSubmission {
// public:
//     WorkSubmission(sk_sp<CommandBuffer> cmdBuffer)
//         : GpuWorkSubmission(std::move(cmdBuffer)) {}
//     ~WorkSubmission() override {}

//     bool isFinished() override {
//         return static_cast<MtlCommandBuffer*>(this->commandBuffer())->isFinished();
//     }
//     void waitUntilFinished(const SharedContext* context) override {
//         return static_cast<MtlCommandBuffer*>(this->commandBuffer())->waitUntilFinished(context);
//     }
// };

QueueManager::OutstandingSubmission DawnQueueManager::onSubmitToGpu() {
    // SkASSERT(fCurrentCommandBuffer);
    // MtlCommandBuffer* mtlCmdBuffer = static_cast<MtlCommandBuffer*>(fCurrentCommandBuffer.get());
    // if (!mtlCmdBuffer->commit()) {
    //     fCurrentCommandBuffer->callFinishedProcs(/*success=*/false);
    //     return nullptr;
    // }

    // std::unique_ptr<GpuWorkSubmission> submission(
    //         new WorkSubmission(std::move(fCurrentCommandBuffer)));
    // return submission;
    return nullptr;
}

#if GRAPHITE_TEST_UTILS
void DawnQueueManager::startCapture() {
}

void DawnQueueManager::stopCapture() {
}
#endif

} // namespace skgpu::graphite
