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

void GrDDLTask::gatherIDs(SkSTArray<8, uint32_t, true>* idArray) const {
    for (auto& task : fDDL->priv().renderTasks1()) {
        task->gatherIDs(idArray);
    }
}

GrDDLTask::GrDDLTask(GrDrawingManager* drawingMgr,
                     sk_sp<GrRenderTargetProxy> ddlTarget,
                     sk_sp<const SkDeferredDisplayList> ddl)
        : fDDL(std::move(ddl))
        , fDDLTarget(std::move(ddlTarget)) {
    for (auto& task : fDDL->priv().renderTasks1()) {
        for (int i = 0; i < task->numTargets(); ++i) {
            SkASSERT(task->isClosed());
            this->addTarget(drawingMgr, task->target(i));
        }
    }

    this->setFlag(kClosed_Flag);
}

GrDDLTask::~GrDDLTask() { }

void GrDDLTask::endFlush(GrDrawingManager* drawingManager) {
    for (auto& task : fDDL->priv().renderTasks1()) {
        task->endFlush(drawingManager);
    }

    INHERITED::endFlush(drawingManager);
}

void GrDDLTask::disown(GrDrawingManager* drawingManager) {
    for (auto& task : fDDL->priv().renderTasks1()) {
        task->disown(drawingManager);
    }

    INHERITED::disown(drawingManager);
}

bool GrDDLTask::onIsUsed(GrSurfaceProxy* proxy) const {
    for (auto& task : fDDL->priv().renderTasks1()) {
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

    for (auto& task : fDDL->priv().renderTasks1()) {
        task->gatherProxyIntervals(alloc);
    }
}

GrRenderTask::ExpectedOutcome GrDDLTask::onMakeClosed(const GrCaps& caps,
                                                      SkIRect* targetUpdateBounds) {
    SkASSERT(0);
    for (auto& task : fDDL->priv().renderTasks1()) {
        SkASSERT(task->isClosed());
    }

    return ExpectedOutcome::kTargetUnchanged;
}

void GrDDLTask::onPrepare(GrOpFlushState* flushState) {
    for (auto& task : fDDL->priv().renderTasks1()) {
        task->prepare(flushState);
    }
}

bool GrDDLTask::onExecute(GrOpFlushState* flushState) {
    bool anyCommandsIssued = false;
    for (auto& task : fDDL->priv().renderTasks1()) {
        if (task->execute(flushState)) {
            anyCommandsIssued = true;
        }
    }

    return anyCommandsIssued;
}
