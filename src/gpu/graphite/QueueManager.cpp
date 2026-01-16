/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/QueueManager.h"

#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recording.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecordingPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/UploadBufferManager.h"
#include "src/gpu/graphite/task/Task.h"
#include "src/gpu/graphite/task/TaskList.h"

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
    if (fSharedContext->caps()->allowCpuSync()) {
        this->checkForFinishedWork(SyncToCpu::kYes);
    } else if (!fOutstandingSubmissions.empty()) {
        SKGPU_LOG_F("When ContextOptions::fNeverYieldToWebGPU is specified all GPU work must be "
                    "finished before destroying Context.");
    }
}

std::vector<std::unique_ptr<CommandBuffer>>*
QueueManager::getAvailableCommandBufferList(Protected isProtected) {
    return isProtected == Protected::kNo ? &fAvailableCommandBuffers
                                         : &fAvailableProtectedCommandBuffers;
}


bool QueueManager::setupCommandBuffer(ResourceProvider* resourceProvider, Protected isProtected) {
    if (!fCurrentCommandBuffer) {
        std::vector<std::unique_ptr<CommandBuffer>>* bufferList =
                this->getAvailableCommandBufferList(isProtected);
        if (!bufferList->empty()) {
            fCurrentCommandBuffer = std::move(bufferList->back());
            bufferList->pop_back();
            if (!fCurrentCommandBuffer->setNewCommandBufferResources()) {
                fCurrentCommandBuffer.reset();
            }
        }
    } else {
        if (fCurrentCommandBuffer->isProtected() != isProtected) {
            // If we're doing things where we are switching between using protected and unprotected
            // command buffers, it is our job to make sure previous work was submitted.
            SKGPU_LOG_E("Trying to use a CommandBuffer with protectedness that differs from our "
                        "current active command buffer.");
            return false;
        }
    }
    if (!fCurrentCommandBuffer) {
        fCurrentCommandBuffer = this->getNewCommandBuffer(resourceProvider, isProtected);
    }
    if (!fCurrentCommandBuffer) {
        return false;
    }

    return true;
}

InsertStatus QueueManager::addRecording(const InsertRecordingInfo& info, Context* context) {
    TRACE_EVENT0_ALWAYS("skia.gpu", TRACE_FUNC);

    // Configure the callback before validation so that failures are propagated to the finish
    // procs that were registered on `info` as well.
    bool addTimerQuery = false;
    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedWithStatsProc) {
        addTimerQuery = info.fGpuStatsFlags & GpuStatsFlags::kElapsedTime;
        if (addTimerQuery && !(context->supportedGpuStats() & GpuStatsFlags::kElapsedTime)) {
            addTimerQuery = false;
            SKGPU_LOG_W("Requested elapsed time reporting but not supported by Context.");
        }
        callback = RefCntedCallback::Make(info.fFinishedWithStatsProc, info.fFinishedContext);
    } else if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

#define RETURN_FAIL_IF(failureCase, status, fmt, ...)               \
    if (failureCase) {                                              \
        if (callback) { callback->setFailureResult(); }             \
        info.fRecording->priv().setFailureResultForFinishedProcs(); \
        info.fRecording->priv().deinstantiateVolatileLazyProxies(); \
        SKGPU_LOG_E(fmt, ##__VA_ARGS__);                            \
        return status;                                              \
    } do {} while(false)
#define SIMULATE_FAIL(status) \
    RETURN_FAIL_IF(info.fSimulatedStatus == status, status, "Simulating '" #status "' failure")

    RETURN_FAIL_IF(!info.fRecording,
                   InsertStatus::kInvalidRecording,
                   "No valid Recording passed into addRecording call");

    // Recordings from a Recorder that requires ordered recordings will have a valid recorder ID.
    // Recordings that don't have any required order are assigned SK_InvalidID.
    uint32_t recorderID = info.fRecording->priv().recorderID();
    if (recorderID != SK_InvalidGenID) {
        uint32_t* recordingID = fLastAddedRecordingIDs.find(recorderID);
        RETURN_FAIL_IF(recordingID && info.fRecording->priv().uniqueID() != *recordingID + 1,
                       InsertStatus::kOutOfOrderRecording,
                       "Recordings are expected to be replayed in order");

        // Note the new Recording ID.
        fLastAddedRecordingIDs.set(recorderID, info.fRecording->priv().uniqueID());
    }

    RETURN_FAIL_IF(info.fTargetSurface && !asSB(info.fTargetSurface)->isGraphiteBacked(),
                   InsertStatus::kInvalidRecording,
                    "Target surface passed into addRecording call is not Graphite-backed");

    SIMULATE_FAIL(InsertStatus::kInvalidRecording);

    auto resourceProvider = context->priv().resourceProvider();
    // Technically no commands have been added yet, but if this fails, things are in a bad state
    // so signal the unrecoverable status.
    RETURN_FAIL_IF(!this->setupCommandBuffer(resourceProvider, fSharedContext->isProtected()),
                   InsertStatus::kAddCommandsFailed,
                   "CommandBuffer creation failed");

    // This must happen before instantiating the lazy proxies, because the target for draws in this
    // recording may itself be a lazy proxy whose instantiation must be handled specially here.
    // We must also make sure the lazy proxies are instantiated successfully before we make any
    // modifications to the current command buffer, so we can't just do all this work in
    // Recording::addCommands below.
    TextureProxy* deferredTargetProxy = info.fRecording->priv().deferredTargetProxy();
    AutoDeinstantiateTextureProxy autoDeinstantiateTargetProxy(deferredTargetProxy);
    const Texture* replayTarget = nullptr;
    if (deferredTargetProxy) {
        RETURN_FAIL_IF(!info.fTargetSurface,
                       InsertStatus::kPromiseImageInstantiationFailed,
                       "No surface provided to instantiate deferred replay target");

        replayTarget = info.fRecording->priv().setupDeferredTarget(
                resourceProvider,
                static_cast<Surface*>(info.fTargetSurface),
                info.fTargetTranslation,
                info.fTargetClip);

        RETURN_FAIL_IF(!replayTarget,
                        InsertStatus::kPromiseImageInstantiationFailed,
                        "Failed to set up deferred replay target");
    }

    RETURN_FAIL_IF(info.fRecording->priv().hasNonVolatileLazyProxies() &&
                   !info.fRecording->priv().instantiateNonVolatileLazyProxies(resourceProvider),
                   InsertStatus::kPromiseImageInstantiationFailed,
                   "Non-volatile PromiseImage instantiation has failed");

    RETURN_FAIL_IF(info.fRecording->priv().hasVolatileLazyProxies() &&
                   !info.fRecording->priv().instantiateVolatileLazyProxies(resourceProvider),
                   InsertStatus::kPromiseImageInstantiationFailed,
                   "Volitile PromiseImage instantiation has failed");

    SIMULATE_FAIL(InsertStatus::kPromiseImageInstantiationFailed);

    if (addTimerQuery) {
        fCurrentCommandBuffer->startTimerQuery();
    }
    fCurrentCommandBuffer->addWaitSemaphores(info.fNumWaitSemaphores, info.fWaitSemaphores);
    if (!info.fRecording->priv().addCommands(context,
                                             fCurrentCommandBuffer.get(),
                                             replayTarget,
                                             info.fTargetTranslation,
                                             info.fTargetClip)) {
        // If the commands failed, iterate over all the used pipelines to see if their async
        // compilation was the reason for failure. Clients that manage pipeline disk caches may
        // want to handle the failure differently than when any other GPU command failed.
        const bool validPipelines = info.fRecording->priv().taskList()->visitPipelines(
                [](const GraphicsPipeline* pipeline) {
                    return !pipeline->didAsyncCompilationFail();
                });

        // We are already definitely going to fail, it's just a matter of which status to return
        RETURN_FAIL_IF(validPipelines,
                       InsertStatus::kAddCommandsFailed,
                       "Adding Recording commands to the CommandBuffer has failed");
        RETURN_FAIL_IF(true,
                       InsertStatus::kAsyncShaderCompilesFailed,
                       "Async pipeline compiles failed, unable to add Recording commands");
    }

    SIMULATE_FAIL(InsertStatus::kAddCommandsFailed);
    SIMULATE_FAIL(InsertStatus::kAsyncShaderCompilesFailed);

    fCurrentCommandBuffer->addSignalSemaphores(info.fNumSignalSemaphores, info.fSignalSemaphores);
    if (info.fTargetTextureState) {
        fCurrentCommandBuffer->prepareSurfaceForStateUpdate(info.fTargetSurface,
                                                            info.fTargetTextureState);
    }
    if (addTimerQuery) {
        fCurrentCommandBuffer->endTimerQuery();
    }

    if (callback) {
        fCurrentCommandBuffer->addFinishedProc(std::move(callback));
    }

    info.fRecording->priv().deinstantiateVolatileLazyProxies();

    // If we got here, the simulated status should be kSuccess or it means we missed returning the
    // simulated error earlier.
    SkASSERT(info.fSimulatedStatus == InsertStatus::kSuccess);
    return InsertStatus::kSuccess;
}

bool QueueManager::addTask(Task* task,
                           Context* context,
                           Protected isProtected) {
    SkASSERT(task);
    if (!task) {
        SKGPU_LOG_E("No valid Task passed into addTask call");
        return false;
    }

    if (!this->setupCommandBuffer(context->priv().resourceProvider(), isProtected)) {
        SKGPU_LOG_E("CommandBuffer creation failed");
        return false;
    }

    if (task->addCommands(context, fCurrentCommandBuffer.get(), {}) == Task::Status::kFail) {
        SKGPU_LOG_E("Adding Task commands to the CommandBuffer has failed");
        return false;
    }

    return true;
}

bool QueueManager::addFinishInfo(const InsertFinishInfo& info,
                                 ResourceProvider* resourceProvider,
                                 SkSpan<const sk_sp<Buffer>> buffersToAsyncMap) {
    sk_sp<RefCntedCallback> callback;
    if (info.fFinishedProc) {
        callback = RefCntedCallback::Make(info.fFinishedProc, info.fFinishedContext);
    }

    if (!this->setupCommandBuffer(resourceProvider, fSharedContext->isProtected())) {
        if (callback) {
            callback->setFailureResult();
        }
        SKGPU_LOG_E("CommandBuffer creation failed");
        return false;
    }

    if (callback) {
        fCurrentCommandBuffer->addFinishedProc(std::move(callback));
    }
    fCurrentCommandBuffer->addBuffersToAsyncMapOnSubmit(buffersToAsyncMap);

    return true;
}

bool QueueManager::submitToGpu(const SubmitInfo& submitInfo) {
    TRACE_EVENT0_ALWAYS("skia.gpu", TRACE_FUNC);

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

    auto submission = this->onSubmitToGpu(submitInfo);
    if (!submission) {
        return false;
    }

    new (fOutstandingSubmissions.push_back()) OutstandingSubmission(std::move(submission));
    return true;
}

bool QueueManager::hasUnfinishedGpuWork() { return !fOutstandingSubmissions.empty(); }

void QueueManager::checkForFinishedWork(SyncToCpu sync) {
    TRACE_EVENT1("skia.gpu", TRACE_FUNC, "sync", sync == SyncToCpu::kYes);

    if (sync == SyncToCpu::kYes) {
        SkASSERT(fSharedContext->caps()->allowCpuSync());
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
    while (front && (*front)->isFinished(fSharedContext)) {
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
    std::vector<std::unique_ptr<CommandBuffer>>* bufferList =
            this->getAvailableCommandBufferList(commandBuffer->isProtected());
    bufferList->push_back(std::move(commandBuffer));
}

void QueueManager::addUploadBufferManagerRefs(UploadBufferManager* uploadManager) {
    SkASSERT(fCurrentCommandBuffer);
    uploadManager->transferToCommandBuffer(fCurrentCommandBuffer.get());
}


} // namespace skgpu::graphite
