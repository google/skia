/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/QueueManager.h"

#include "include/gpu/graphite/Recording.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Task.h"

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

bool QueueManager::setupCommandBuffer(ResourceProvider* resourceProvider) {
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
        return false;
    }

    return true;
}

bool QueueManager::addRecording(const InsertRecordingInfo& info, Context* context) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

    SkASSERT(info.fRecording);
    if (!info.fRecording) {
        if (callback) {
            callback->setFailureResult();
        }
        SKGPU_LOG_E("No valid Recording passed into addRecording call");
        return false;
    }

    if (this->fSharedContext->caps()->requireOrderedRecordings()) {
        uint32_t* recordingID = fLastAddedRecordingIDs.find(info.fRecording->priv().recorderID());
        if (recordingID &&
            info.fRecording->priv().uniqueID() != *recordingID+1) {
            SKGPU_LOG_E("Recordings are expected to be replayed in order");
            return false;
        }

        // Note the new Recording ID.
        fLastAddedRecordingIDs.set(info.fRecording->priv().recorderID(),
                                   info.fRecording->priv().uniqueID());
    }

    if (info.fTargetSurface &&
        !static_cast<const SkSurface_Base*>(info.fTargetSurface)->isGraphiteBacked()) {
        if (callback) {
            callback->setFailureResult();
        }
        info.fRecording->priv().setFailureResultForFinishedProcs();
        SKGPU_LOG_E("Target surface passed into addRecording call is not graphite-backed");
        return false;
    }

    auto resourceProvider = context->priv().resourceProvider();
    if (!this->setupCommandBuffer(resourceProvider)) {
        if (callback) {
            callback->setFailureResult();
        }
        info.fRecording->priv().setFailureResultForFinishedProcs();
        SKGPU_LOG_E("CommandBuffer creation failed");
        return false;
    }

    if (info.fRecording->priv().hasNonVolatileLazyProxies()) {
        if (!info.fRecording->priv().instantiateNonVolatileLazyProxies(resourceProvider)) {
            if (callback) {
                callback->setFailureResult();
            }
            info.fRecording->priv().setFailureResultForFinishedProcs();
            SKGPU_LOG_E("Non-volatile PromiseImage instantiation has failed");
            return false;
        }
    }

    if (info.fRecording->priv().hasVolatileLazyProxies()) {
        if (!info.fRecording->priv().instantiateVolatileLazyProxies(resourceProvider)) {
            if (callback) {
                callback->setFailureResult();
            }
            info.fRecording->priv().setFailureResultForFinishedProcs();
            info.fRecording->priv().deinstantiateVolatileLazyProxies();
            SKGPU_LOG_E("Volatile PromiseImage instantiation has failed");
            return false;
        }
    }

    fCurrentCommandBuffer->addWaitSemaphores(info.fNumWaitSemaphores, info.fWaitSemaphores);
    if (!info.fRecording->priv().addCommands(context,
                                             fCurrentCommandBuffer.get(),
                                             static_cast<Surface*>(info.fTargetSurface),
                                             info.fTargetTranslation)) {
        if (callback) {
            callback->setFailureResult();
        }
        info.fRecording->priv().setFailureResultForFinishedProcs();
        info.fRecording->priv().deinstantiateVolatileLazyProxies();
        SKGPU_LOG_E("Adding Recording commands to the CommandBuffer has failed");
        return false;
    }
    fCurrentCommandBuffer->addSignalSemaphores(info.fNumSignalSemaphores, info.fSignalSemaphores);
    if (info.fTargetTextureState) {
        fCurrentCommandBuffer->prepareSurfaceForStateUpdate(info.fTargetSurface,
                                                            info.fTargetTextureState);
    }

    if (callback) {
        fCurrentCommandBuffer->addFinishedProc(std::move(callback));
    }

    info.fRecording->priv().deinstantiateVolatileLazyProxies();

    return true;
}

bool QueueManager::addTask(Task* task,
                           Context* context) {
    SkASSERT(task);
    if (!task) {
        SKGPU_LOG_E("No valid Task passed into addTask call");
        return false;
    }

    if (!this->setupCommandBuffer(context->priv().resourceProvider())) {
        SKGPU_LOG_E("CommandBuffer creation failed");
        return false;
    }

    if (!task->addCommands(context, fCurrentCommandBuffer.get(), {})) {
        SKGPU_LOG_E("Adding Task commands to the CommandBuffer has failed");
        return false;
    }

    return true;
}

bool QueueManager::addFinishInfo(const InsertFinishInfo& info,
                                 ResourceProvider* resourceProvider) {
    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

    if (!this->setupCommandBuffer(resourceProvider)) {
        if (callback) {
            callback->setFailureResult();
        }
        SKGPU_LOG_E("CommandBuffer creation failed");
        return false;
    }

    if (callback) {
        fCurrentCommandBuffer->addFinishedProc(std::move(callback));
    }

    return true;
}

bool QueueManager::submitToGpu() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    if (!fCurrentCommandBuffer) {
        // We warn because this probably representative of a bad client state, where they don't
        // need to submit but didn't notice, but technically the submit itself is fine (no-op), so
        // we return true.
        SKGPU_LOG_D("Submit called with no active command buffer!");
        return true;
    }

#ifdef SK_DEBUG
    if (!fCurrentCommandBuffer->hasWork()) {
        SKGPU_LOG_D("Submitting empty command buffer!");
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
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "sync", sync == SyncToCpu::kYes);

    if (sync == SyncToCpu::kYes) {
        // wait for the last submission to finish
        OutstandingSubmission* back = (OutstandingSubmission*)fOutstandingSubmissions.back();
        if (back) {
            (*back)->waitUntilFinished();
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
