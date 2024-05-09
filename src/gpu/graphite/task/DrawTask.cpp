/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/task/DrawTask.h"

#include "src/gpu/graphite/ScratchResourceManager.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

DrawTask::DrawTask(sk_sp<TextureProxy> target) : fTarget(std::move(target)) {}

DrawTask::~DrawTask() = default;

Task::Status DrawTask::prepareResources(ResourceProvider* resourceProvider,
                                        ScratchResourceManager* scratchManager,
                                        const RuntimeEffectDictionary* rteDict) {
    const int pendingReadCount = scratchManager->pendingReadCount(fTarget.get());
    if (pendingReadCount) {
        // This DrawTask defines the content of a scratch device that has incremented the pending
        // read count before snap() was called. The target may have already been instantiated if
        // we've processed this task's children before.
        SkASSERT(!fTarget->isLazy());
        // Even though we may discard the task, we always want to mark it as in-use to track the
        // pending reads to know when to return the texture.;
        scratchManager->markResourceInUse(this);

        if (fPrepared) {
            // If the task has already had prepareResources() called once, it should have had
            // its target instantiated.
            SkASSERT(fTarget->isInstantiated());
            // Return kDiscard so that this reference to the task is removed and the original
            // encounter in the graph will be the only time addCommands() is invoked.
            return Status::kDiscard;
        }
    } else {
        // A non-scratch DrawTask should only ever be in the task graph one time.
        SkASSERT(!fPrepared);
    }

    fPrepared = true;
    // NOTE: This prepareResources() pushes a new scope for scratch resource management, which is
    // what we want since the child tasks are what will actually instantiate any scratch device and
    // trigger returns of any grand-child resources. The above markResourceInUse() should happen
    // above this so that pending returns are handled in caller's scope.
    return fChildTasks.prepareResources(resourceProvider, scratchManager, rteDict);
}

void DrawTask::onUseCompleted(ScratchResourceManager* scratchManager) {
    // Now that the render task has completed, actually decrement the read count of the target proxy
    // If the count hits zero, this was the last pending read that needed to use the DrawTask's
    // results so we can return the texture to the ScratchResourceManager for reuse.
    SkASSERT(!fTarget->isLazy() && fTarget->isInstantiated());
    SkASSERT(scratchManager->pendingReadCount(fTarget.get()) > 0);
    if (scratchManager->removePendingRead(fTarget.get())) {
        scratchManager->returnTexture(fTarget->refTexture());
    }
}

Task::Status DrawTask::addCommands(Context* ctx,
                                   CommandBuffer* commandBuffer,
                                   ReplayTargetData replayTarget) {
    SkASSERT(fTarget->isInstantiated());
    return fChildTasks.addCommands(ctx, commandBuffer, replayTarget);
}

} // namespace skgpu::graphite
