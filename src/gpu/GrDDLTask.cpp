/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDeferredDisplayList.h"
#include "src/core/SkDeferredDisplayListPriv.h"
#include "src/gpu/GrDDLTask.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrResourceAllocator.h"

GrDDLTask::GrDDLTask(GrDrawingManager* drawingMgr,
                     sk_sp<GrRenderTargetProxy> ddlTarget,
                     sk_sp<const SkDeferredDisplayList> ddl,
                     int xOffset, int yOffset)
        : fDDL(std::move(ddl))
        , fDDLTarget(std::move(ddlTarget))
        , fXOffset(xOffset)
        , fYOffset(yOffset) {
    for (auto& task : fDDL->priv().renderTasks1()) {
        for (int i = 0; i < task->numTargets(); ++i) {
            this->addTarget(drawingMgr, task->target(i));
        }
    }
}

GrDDLTask::~GrDDLTask() { }

void GrDDLTask::endFlush(GrDrawingManager* drawingManager) {
    for (auto& task : fDDL->priv().renderTasks1()) {
        task->endFlush(drawingManager);
    }

    INHERITED::endFlush(drawingManager);
}

void GrDDLTask::disown(GrDrawingManager* drawingManager) {
    for (auto& task : fDDL->priv().renderTasks()) {
        task->disown(drawingManager);
    }

    INHERITED::disown(drawingManager);
}

bool GrDDLTask::onIsUsed(GrSurfaceProxy* proxy) const {
    for (auto& task : fDDL->priv().renderTasks()) {
        if (task->onIsUsed(proxy)) {
            return true;
        }
    }

    return false;
}

void GrDDLTask::gatherProxyIntervals(GrResourceAllocator* alloc) const {
    // We don't have any proxies, but the resource allocator will still bark
    // if a task doesn't claim any op indices, so we oblige it.
    alloc->incOps();

    for (auto& task : fDDL->priv().renderTasks()) {
        task->gatherProxyIntervals(alloc);
    }
}

GrRenderTask::ExpectedOutcome GrDDLTask::onMakeClosed(const GrCaps& caps,
                                                      SkIRect* targetUpdateBounds) {
    for (auto& task : fDDL->priv().renderTasks()) {
        SkASSERT(task->isClosed());
    }

    return ExpectedOutcome::kTargetUnchanged;
}

void GrDDLTask::onPrepare(GrOpFlushState* flushState) {
    for (auto& task : fDDL->priv().renderTasks()) {
        task->prepare(flushState);
    }
}

bool GrDDLTask::onExecute(GrOpFlushState* flushState) {
    flushState->gpu()->setViewport(fXOffset, fYOffset,
                                   fDDL->priv().width(), fDDL->priv().height());

    bool anyCommandsIssued = false;
    for (auto& task : fDDL->priv().renderTasks()) {
        if (task->execute(flushState)) {
            anyCommandsIssued = true;
        }
    }

    flushState->gpu()->setViewport(0, 0,
                                   fDDLTarget->backingStoreDimensions().width(),
                                   fDDLTarget->backingStoreDimensions().height());
    return anyCommandsIssued;
}
