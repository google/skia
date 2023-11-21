/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnQueueManager.h"

#include "src/gpu/graphite/dawn/DawnAsyncWait.h"
#include "src/gpu/graphite/dawn/DawnCommandBuffer.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"

namespace skgpu::graphite {
namespace {
class DawnWorkSubmission final : public GpuWorkSubmission {
public:
    DawnWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                       DawnQueueManager* queueManager,
                       const DawnSharedContext* sharedContext)
            : GpuWorkSubmission(std::move(cmdBuffer), queueManager), fAsyncWait(sharedContext) {
        queueManager->dawnQueue().OnSubmittedWorkDone(
#if defined(__EMSCRIPTEN__)
                // This is parameter is being removed:
                // https://github.com/webgpu-native/webgpu-headers/issues/130
                /*signalValue=*/0,
#endif
                [](WGPUQueueWorkDoneStatus, void* userData) {
                    auto asyncWaitPtr = static_cast<DawnAsyncWait*>(userData);
                    asyncWaitPtr->signal();
                },
                &fAsyncWait);
    }
    ~DawnWorkSubmission() override {}

private:
    bool onIsFinished() override { return fAsyncWait.yieldAndCheck(); }
    void onWaitUntilFinished() override { fAsyncWait.busyWait(); }

    DawnAsyncWait fAsyncWait;
};
} // namespace

DawnQueueManager::DawnQueueManager(wgpu::Queue queue, const SharedContext* sharedContext)
        : QueueManager(sharedContext), fQueue(std::move(queue)) {}

void DawnQueueManager::tick() const { DawnTickDevice(this->dawnSharedContext()); }

const DawnSharedContext* DawnQueueManager::dawnSharedContext() const {
    return static_cast<const DawnSharedContext*>(fSharedContext);
}

std::unique_ptr<CommandBuffer> DawnQueueManager::getNewCommandBuffer(
        ResourceProvider* resourceProvider) {
    return DawnCommandBuffer::Make(dawnSharedContext(),
                                   static_cast<DawnResourceProvider*>(resourceProvider));
}

QueueManager::OutstandingSubmission DawnQueueManager::onSubmitToGpu() {
    SkASSERT(fCurrentCommandBuffer);
    DawnCommandBuffer* dawnCmdBuffer = static_cast<DawnCommandBuffer*>(fCurrentCommandBuffer.get());
    auto wgpuCmdBuffer = dawnCmdBuffer->finishEncoding();
    if (!wgpuCmdBuffer) {
        fCurrentCommandBuffer->callFinishedProcs(/*success=*/false);
        return nullptr;
    }

    fQueue.Submit(/*commandCount=*/1, &wgpuCmdBuffer);

    return std::make_unique<DawnWorkSubmission>(std::move(fCurrentCommandBuffer),
                                                this,
                                                dawnSharedContext());
}

#if defined(GRAPHITE_TEST_UTILS)
void DawnQueueManager::startCapture() {
    // TODO: Dawn doesn't have capturing feature yet.
}

void DawnQueueManager::stopCapture() {
    // TODO: Dawn doesn't have capturing feature yet.
}
#endif

} // namespace skgpu::graphite
