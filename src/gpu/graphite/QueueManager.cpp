/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/QueueManager.h"

#include "include/gpu/graphite/Recording.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecordingPriv.h"

namespace skgpu::graphite {

// This constant determines how many OutstandingSubmissions are allocated together as a block in
// the deque. As such it needs to balance allocating too much memory vs. incurring
// allocation/deallocation thrashing. It should roughly correspond to the max number of outstanding
// submissions we expect to see.
static constexpr int kDefaultOutstandingAllocCnt = 8;

QueueManager::QueueManager(const SharedContext* sharedContext)
        : fSharedContext(sharedContext)
        , fOutstandingSubmissions(sizeof(OutstandingSubmission), kDefaultOutstandingAllocCnt) {
}

QueueManager::~QueueManager() {
    this->checkForFinishedWork(SyncToCpu::kYes);
}

void QueueManager::addRecording(const InsertRecordingInfo& info,
                                ResourceProvider* resourceProvider) {
    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

    SkASSERT(info.fRecording);
    if (!info.fRecording) {
        if (callback) {
            callback->setFailureResult();
        }
        SKGPU_LOG_W("No valid Recording passed into addRecording call");
        return;
    }

    if (!fCurrentCommandBuffer) {
        if (fAvailableCommandBuffers.size()) {
            fCurrentCommandBuffer = std::move(fAvailableCommandBuffers.back());
            fAvailableCommandBuffers.pop_back();
            if (!fCurrentCommandBuffer->setNewCommandBufferResources()) {
                fCurrentCommandBuffer.reset();
            }
        }
    }
    if (!fCurrentCommandBuffer) {
        fCurrentCommandBuffer = this->getNewCommandBuffer(resourceProvider);
    }
    if (!fCurrentCommandBuffer) {
        if (callback) {
            callback->setFailureResult();
        }
        return;
    }

    if (!info.fRecording->priv().addCommands(resourceProvider, fCurrentCommandBuffer.get())) {
        if (callback) {
            callback->setFailureResult();
        }
        return;
    }

    if (callback) {
        fCurrentCommandBuffer->addFinishedProc(std::move(callback));
    }
}

bool QueueManager::submitToGpu() {
    if (!fCurrentCommandBuffer) {
        SKGPU_LOG_W("Submit called with no active command buffer!");
        return false;
    }

#ifdef SK_DEBUG
    if (!fCurrentCommandBuffer->hasWork()) {
        SKGPU_LOG_W("Submitting empty command buffer!");
    }
#endif

    auto submission = this->onSubmitToGpu();
    if (!submission) {
        return false;
    }

    new (fOutstandingSubmissions.push_back()) OutstandingSubmission(std::move(submission));
    return true;
}

void QueueManager::checkForFinishedWork(SyncToCpu sync) {
    if (sync == SyncToCpu::kYes) {
        // wait for the last submission to finish
        OutstandingSubmission* back = (OutstandingSubmission*)fOutstandingSubmissions.back();
        if (back) {
            (*back)->waitUntilFinished(fSharedContext);
        }
    }

    // Iterate over all the outstanding submissions to see if any have finished. The work
    // submissions are in order from oldest to newest, so we start at the front to check if they
    // have finished. If so we pop it off and move onto the next.
    // Repeat till we find a submission that has not finished yet (and all others afterwards are
    // also guaranteed to not have finished).
    OutstandingSubmission* front = (OutstandingSubmission*)fOutstandingSubmissions.front();
    while (front && (*front)->isFinished()) {
        // Make sure we remove before deleting as deletion might try to kick off another submit
        // (though hopefully *not* in Graphite).
        fOutstandingSubmissions.pop_front();
        // Since we used placement new we are responsible for calling the destructor manually.
        front->~OutstandingSubmission();
        front = (OutstandingSubmission*)fOutstandingSubmissions.front();
    }
    SkASSERT(sync == SyncToCpu::kNo || fOutstandingSubmissions.empty());
}

void QueueManager::returnCommandBuffer(std::unique_ptr<CommandBuffer> commandBuffer) {
    fAvailableCommandBuffers.push_back(std::move(commandBuffer));
}

} // namespace skgpu::graphite
