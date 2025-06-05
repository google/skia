/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlQueueManager.h"

#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"

namespace skgpu::graphite {

MtlQueueManager::MtlQueueManager(sk_cfp<id<MTLCommandQueue>> queue,
                                 const SharedContext* sharedContext)
        : QueueManager(sharedContext)
        , fQueue(std::move(queue))
{
}

const MtlSharedContext* MtlQueueManager::mtlSharedContext() const {
    return static_cast<const MtlSharedContext*>(fSharedContext);
}

std::unique_ptr<CommandBuffer> MtlQueueManager::getNewCommandBuffer(
        ResourceProvider* resourceProvider, Protected) {
    MtlResourceProvider* mtlResourceProvider = static_cast<MtlResourceProvider*>(resourceProvider);
    auto cmdBuffer = MtlCommandBuffer::Make(fQueue.get(),
                                            this->mtlSharedContext(),
                                            mtlResourceProvider);
    return cmdBuffer;
}

class MtlWorkSubmission final : public GpuWorkSubmission {
public:
    MtlWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer, QueueManager* queueManager)
        : GpuWorkSubmission(std::move(cmdBuffer), queueManager) {}
    ~MtlWorkSubmission() override {}

private:
    bool onIsFinished(const SharedContext*) override {
        return static_cast<MtlCommandBuffer*>(this->commandBuffer())->isFinished();
    }
    void onWaitUntilFinished(const SharedContext*) override {
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
            new MtlWorkSubmission(std::move(fCurrentCommandBuffer), this));
    return submission;
}

#if defined(GPU_TEST_UTILS)
void MtlQueueManager::startCapture() {
    // TODO: add newer Metal interface as well
    MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
    if (captureManager.isCapturing) {
        return;
    }
    MTLCaptureDescriptor* captureDescriptor = [[MTLCaptureDescriptor alloc] init];
    captureDescriptor.captureObject = fQueue.get();

    NSError *error;
    if (![captureManager startCaptureWithDescriptor: captureDescriptor error:&error]) {
        NSLog(@"Failed to start capture, error %@", error);
    }
}

void MtlQueueManager::stopCapture() {
    MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
    if (captureManager.isCapturing) {
        [captureManager stopCapture];
    }
}
#endif

} // namespace skgpu::graphite
