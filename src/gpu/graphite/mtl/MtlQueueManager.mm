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

MtlQueueManager::MtlQueueManager(sk_cfp<id<MTLCommandQueue>> queue,
                                 const SharedContext* sharedContext)
        : QueueManager(sharedContext)
        , fQueue(std::move(queue)) {}

const MtlSharedContext* MtlQueueManager::mtlSharedContext() const {
    return static_cast<const MtlSharedContext*>(fSharedContext);
}

sk_sp<CommandBuffer> MtlQueueManager::getNewCommandBuffer() {
    return MtlCommandBuffer::Make(fQueue.get(), this->mtlSharedContext());
}

class WorkSubmission final : public GpuWorkSubmission {
public:
    WorkSubmission(sk_sp<CommandBuffer> cmdBuffer)
        : GpuWorkSubmission(std::move(cmdBuffer)) {}
    ~WorkSubmission() override {}

    bool isFinished() override {
        return static_cast<MtlCommandBuffer*>(this->commandBuffer())->isFinished();
    }
    void waitUntilFinished(const SharedContext* context) override {
        return static_cast<MtlCommandBuffer*>(this->commandBuffer())->waitUntilFinished(context);
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

#if GRAPHITE_TEST_UTILS
void MtlQueueManager::testingOnly_startCapture() {
    if (@available(macOS 10.13, iOS 11.0, *)) {
        // TODO: add newer Metal interface as well
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        if (captureManager.isCapturing) {
            return;
        }
        if (@available(macOS 10.15, iOS 13.0, *)) {
            MTLCaptureDescriptor* captureDescriptor = [[MTLCaptureDescriptor alloc] init];
            captureDescriptor.captureObject = fQueue.get();

            NSError *error;
            if (![captureManager startCaptureWithDescriptor: captureDescriptor error:&error])
            {
                NSLog(@"Failed to start capture, error %@", error);
            }
        } else {
            [captureManager startCaptureWithCommandQueue: fQueue.get()];
        }
     }
}

void MtlQueueManager::testingOnly_endCapture() {
    if (@available(macOS 10.13, iOS 11.0, *)) {
        MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
        if (captureManager.isCapturing) {
            [captureManager stopCapture];
        }
    }
}
#endif

} // namespace skgpu::graphite

