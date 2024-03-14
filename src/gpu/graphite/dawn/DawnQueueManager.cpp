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
// GpuWorkSubmission with AsyncWait. This is useful for wasm where wgpu::Future
// is not available yet.
class [[maybe_unused]] DawnWorkSubmissionWithAsyncWait final : public GpuWorkSubmission {
public:
    DawnWorkSubmissionWithAsyncWait(std::unique_ptr<CommandBuffer> cmdBuffer,
                                    DawnQueueManager* queueManager,
                                    const DawnSharedContext* sharedContext);

private:
    bool onIsFinished(const SharedContext* sharedContext) override;
    void onWaitUntilFinished(const SharedContext* sharedContext) override;

    DawnAsyncWait fAsyncWait;
};

DawnWorkSubmissionWithAsyncWait::DawnWorkSubmissionWithAsyncWait(
        std::unique_ptr<CommandBuffer> cmdBuffer,
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

bool DawnWorkSubmissionWithAsyncWait::onIsFinished(const SharedContext*) {
    return fAsyncWait.yieldAndCheck();
}

void DawnWorkSubmissionWithAsyncWait::onWaitUntilFinished(const SharedContext*) {
    fAsyncWait.busyWait();
}

#if !defined(__EMSCRIPTEN__)

// The version with wgpu::Future. This is not available in wasm yet so we have
// to guard behind #if
class DawnWorkSubmissionWithFuture final : public GpuWorkSubmission {
public:
    DawnWorkSubmissionWithFuture(std::unique_ptr<CommandBuffer> cmdBuffer,
                                 DawnQueueManager* queueManager);

private:
    bool onIsFinished(const SharedContext* sharedContext) override;
    void onWaitUntilFinished(const SharedContext* sharedContext) override;

    wgpu::Future fSubmittedWorkDoneFuture;
};

DawnWorkSubmissionWithFuture::DawnWorkSubmissionWithFuture(std::unique_ptr<CommandBuffer> cmdBuffer,
                                                           DawnQueueManager* queueManager)
        : GpuWorkSubmission(std::move(cmdBuffer), queueManager) {
    wgpu::QueueWorkDoneCallbackInfo callbackInfo{};
    callbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
    callbackInfo.callback = [](WGPUQueueWorkDoneStatus, void*) {};

    fSubmittedWorkDoneFuture = queueManager->dawnQueue().OnSubmittedWorkDone(callbackInfo);
}

bool DawnWorkSubmissionWithFuture::onIsFinished(const SharedContext* sharedContext) {
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = fSubmittedWorkDoneFuture;
    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext)
                                   ->device()
                                   .GetAdapter()
                                   .GetInstance();
    if (instance.WaitAny(1, &waitInfo, /*timeoutNS=*/0) != wgpu::WaitStatus::Success) {
        return false;
    }

    return waitInfo.completed;
}

void DawnWorkSubmissionWithFuture::onWaitUntilFinished(const SharedContext* sharedContext) {
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = fSubmittedWorkDoneFuture;
    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext)
                                   ->device()
                                   .GetAdapter()
                                   .GetInstance();
    [[maybe_unused]] auto status =
            instance.WaitAny(1, &waitInfo, /*timeoutNS=*/std::numeric_limits<uint64_t>::max());
    SkASSERT(status == wgpu::WaitStatus::Success);
    SkASSERT(waitInfo.completed);
}

#endif  // !defined(__EMSCRIPTEN__)

} // namespace

DawnQueueManager::DawnQueueManager(wgpu::Queue queue, const SharedContext* sharedContext)
        : QueueManager(sharedContext), fQueue(std::move(queue)) {}

void DawnQueueManager::tick() const { this->dawnSharedContext()->tick(); }

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

#if defined(__EMSCRIPTEN__)
    return std::make_unique<DawnWorkSubmissionWithAsyncWait>(
            std::move(fCurrentCommandBuffer), this, dawnSharedContext());
#else
    return std::make_unique<DawnWorkSubmissionWithFuture>(std::move(fCurrentCommandBuffer), this);
#endif
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
