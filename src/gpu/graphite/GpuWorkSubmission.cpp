/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GpuWorkSubmission.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/QueueManager.h"

namespace skgpu::graphite {

GpuWorkSubmission::GpuWorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                                     QueueManager* queueManager)
        : fCommandBuffer(std::move(cmdBuffer))
        , fQueueManager(queueManager) {
    SkSpan<const sk_sp<Buffer>> buffers = fCommandBuffer->buffersToAsyncMapOnSubmit();
    if (!buffers.empty()) {
        fOutstandingAsyncMapCounter = sk_make_sp<SkRefCnt>();
        for (auto& buffer : fCommandBuffer->buffersToAsyncMapOnSubmit()) {
            SkASSERT(!buffer->isUnmappable());
            fOutstandingAsyncMapCounter->ref();
            buffer->asyncMap([](GpuFinishedContext c, CallbackResult) {
                                static_cast<SkRefCnt*>(c)->unref();
                             },
                             fOutstandingAsyncMapCounter.get());
        }
    }
}

GpuWorkSubmission::~GpuWorkSubmission() {
    fCommandBuffer->callFinishedProcs(/*success=*/true);
    fCommandBuffer->resetCommandBuffer();
    fQueueManager->returnCommandBuffer(std::move(fCommandBuffer));
}

bool GpuWorkSubmission::isFinished(const SharedContext* sharedContext) {
    return this->onIsFinished(sharedContext) &&
           (!fOutstandingAsyncMapCounter || fOutstandingAsyncMapCounter->unique());
}

void GpuWorkSubmission::waitUntilFinished(const SharedContext* sharedContext) {
    this->onWaitUntilFinished(sharedContext);
    if (fOutstandingAsyncMapCounter) {
        while (!fOutstandingAsyncMapCounter->unique()) {
            fQueueManager->tick();
        }
    }
}

} // namespace skgpu::graphite

