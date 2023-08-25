/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrDDLTask.h"

#include "include/private/chromium/GrDeferredDisplayList.h"
#include "src/gpu/ganesh/GrDeferredDisplayListPriv.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrResourceAllocator.h"

GrDDLTask::GrDDLTask(GrDrawingManager* drawingMgr,
                     sk_sp<GrRenderTargetProxy> ddlTarget,
                     sk_sp<const GrDeferredDisplayList> ddl)
        : fDDL(std::move(ddl))
        , fDDLTarget(std::move(ddlTarget)) {

    for (auto& task : fDDL->priv().renderTasks()) {
        SkASSERT(task->isClosed());

        for (int i = 0; i < task->numTargets(); ++i) {
            drawingMgr->setLastRenderTask(task->target(i), task.get());
        }
    }

    // The DDL task never accepts additional tasks
    this->setFlag(kClosed_Flag);
}

GrDDLTask::~GrDDLTask() { }

void GrDDLTask::endFlush(GrDrawingManager* drawingManager) {
    for (auto& task : fDDL->priv().renderTasks()) {
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
    if (proxy == fDDLTarget.get()) {
        return true;
    }

    for (auto& task : fDDL->priv().renderTasks()) {
        if (task->isUsed(proxy)) {
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

GrRenderTask::ExpectedOutcome GrDDLTask::onMakeClosed(GrRecordingContext*,
                                                      SkIRect* targetUpdateBounds) {
    SkASSERT(0);
    return ExpectedOutcome::kTargetUnchanged;
}

void GrDDLTask::onPrepare(GrOpFlushState* flushState) {
    for (auto& task : fDDL->priv().renderTasks()) {
        task->prepare(flushState);
    }
}

bool GrDDLTask::onExecute(GrOpFlushState* flushState) {
    bool anyCommandsIssued = false;
    for (auto& task : fDDL->priv().renderTasks()) {
        if (task->execute(flushState)) {
            anyCommandsIssued = true;
        }
    }

    return anyCommandsIssued;
}

#if defined(GR_TEST_UTILS)
void GrDDLTask::dump(const SkString& label,
                     SkString indent,
                     bool printDependencies,
                     bool close) const {
    INHERITED::dump(label, indent, printDependencies, false);

    SkDebugf("%sDDL Target: ", indent.c_str());
    if (fDDLTarget) {
        SkString proxyStr = fDDLTarget->dump();
        SkDebugf("%s", proxyStr.c_str());
    }
    SkDebugf("\n");

    SkDebugf("%s%d sub-tasks\n", indent.c_str(), fDDL->priv().numRenderTasks());

    SkString subIndent(indent);
    subIndent.append("    ");

    int index = 0;
    for (auto& task : fDDL->priv().renderTasks()) {
        SkString subLabel;
        subLabel.printf("sub-task %d/%d", index++, fDDL->priv().numRenderTasks());
        task->dump(subLabel, subIndent, printDependencies, true);
    }

    if (close) {
        SkDebugf("%s--------------------------------------------------------------\n\n",
                 indent.c_str());
    }
}
#endif
