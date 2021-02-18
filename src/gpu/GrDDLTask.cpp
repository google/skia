/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDDLTask.h"

#include "include/core/SkDeferredDisplayList.h"
#include "src/core/SkDeferredDisplayListPriv.h"
#include "src/gpu/GrResourceAllocator.h"

GrDDLTask::GrDDLTask(GrDrawingManager* drawingMgr,
                     sk_sp<GrRenderTargetProxy> ddlTarget,
                     sk_sp<const SkDeferredDisplayList> ddl,
                     SkIPoint offset)
        : fDDL(std::move(ddl))
        , fDDLTarget(std::move(ddlTarget))
        , fOffset(offset) {
    (void) fOffset;  // fOffset will be used shortly

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

GrRenderTask::ExpectedOutcome GrDDLTask::onMakeClosed(const GrCaps& caps,
                                                      SkIRect* targetUpdateBounds) {
    SkASSERT(0);
    return ExpectedOutcome::kTargetUnchanged;
}

void GrDDLTask::gatherIDs(SkSTArray<8, uint32_t, true>* idArray) const {
    for (auto& task : fDDL->priv().renderTasks()) {
        task->gatherIDs(idArray);
    }
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

#if GR_TEST_UTILS
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
