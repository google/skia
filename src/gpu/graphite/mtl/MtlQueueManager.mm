/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlQueueManager.h"

#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"

namespace skgpu::graphite {

MtlQueueManager::MtlQueueManager(const SharedContext* sharedContext)
        : QueueManager(sharedContext) {}

const MtlSharedContext* MtlQueueManager::mtlSharedContext() const {
    return static_cast<const MtlSharedContext*>(fSharedContext);
}

sk_sp<CommandBuffer> MtlQueueManager::getNewCommandBuffer() {
    return MtlCommandBuffer::Make(this->mtlSharedContext());
}

class WorkSubmission final : public GpuWorkSubmission {
public:
    WorkSubmission(sk_sp<CommandBuffer> cmdBuffer)
        : GpuWorkSubmission(std::move(cmdBuffer)) {}
    ~WorkSubmission() override {}

    bool isFinished() override {
        return static_cast<MtlCommandBuffer*>(this->commandBuffer())->isFinished();
    }
    void waitUntilFinished(const SharedContext*) override {
        return static_cast<MtlCommandBuffer*>(this->commandBuffer())->waitUntilFinished();
    }
};

QueueManager::OutstandingSubmission MtlQueueManager::onSubmitToGpu() {
    SkASSERT(fCurrentCommandBuffer);
    MtlCommandBuffer* mtlCmdBuffer = static_cast<MtlCommandBuffer*>(fCurrentCommandBuffer.get());
    if (!mtlCmdBuffer->commit()) {
        fCurrentCommandBuffer->callFinishedProcs(/*success=*/false);
        return nullptr;
    }

    std::unique_ptr<GpuWorkSubmission> submission(
            new WorkSubmission(std::move(fCurrentCommandBuffer)));
    return submission;
}

} // namespace skgpu::graphite

